// Porres 2017

#include <math.h>
#include <stdlib.h>
#include "m_pd.h"

#define fbcomb_STACK 48000 //stack buf size, 1 sec at 48k for good measure
#define fbcomb_DELAY  100.0 //maximum delay
#define fbcomb_MIND 1 // minumum delay in ms
#define fbcomb_MAXD 4294967294 //max delay = 2**32 - 2

#define fbcomb_MINMS 0. //min delay in ms

#define fbcomb_DEFGAIN 0. //default gain

static t_class *fbcomb_class;

typedef struct _fbcomb
{
    t_object  x_obj;
    t_inlet  *x_dellet;
    t_inlet  *x_alet;
    t_outlet  *x_outlet;
    int     x_sr;
    //pointers to the delay bufs
    double  * x_ybuf;
    double x_ffstack[fbcomb_STACK];
    double * x_xbuf;
    double x_fbstack[fbcomb_STACK];
    int     x_alloc; //if we are using allocated bufs
    unsigned int     x_sz; //actual size of each delay buffer
    
    t_float     x_maxdel;  //maximum delay in ms
    unsigned int       x_wh;     //writehead
} t_fbcomb;

static void fbcomb_clear(t_fbcomb *x){
    unsigned int i;
    for(i=0; i<x->x_sz; i++){
        x->x_xbuf[i] = 0.;
        x->x_ybuf[i] = 0.;
    };
    x->x_wh = 0;
}

static void fbcomb_sz(t_fbcomb *x){
    //helper function to deal with allocation issues if needed
    //ie if wanted size x->x_maxdel is bigger than stack, allocate
    
    //convert ms to samps
    unsigned int newsz = (unsigned int)ceil((double)x->x_maxdel*0.001*(double)x->x_sr);
    newsz++; //add a sample for good measure since say bufsize is 2048 and
    //you want a delay of 2048 samples,.. problem!
    
    int alloc = x->x_alloc;
    unsigned int cursz = x->x_sz; //current size
    
    if(newsz < 0){
        newsz = 0;
    }
    else if(newsz > fbcomb_MAXD){
        newsz = fbcomb_MAXD;
    };
    if(!alloc && newsz > fbcomb_STACK){
        x->x_xbuf = (double *)malloc(sizeof(double)*newsz);
        x->x_ybuf = (double *)malloc(sizeof(double)*newsz);
        x->x_alloc = 1;
        x->x_sz = newsz;
    }
    else if(alloc && newsz > cursz){
        x->x_xbuf = (double *)realloc(x->x_xbuf, sizeof(double)*newsz);
        x->x_ybuf = (double *)realloc(x->x_ybuf, sizeof(double)*newsz);
        x->x_sz = newsz;
    }
    else if(alloc && newsz < fbcomb_STACK){
        free(x->x_xbuf);
        free(x->x_ybuf);
        x->x_sz = fbcomb_STACK;
        x->x_xbuf = x->x_ffstack;
        x->x_ybuf = x->x_fbstack;
        x->x_alloc = 0;
    };
    fbcomb_clear(x);
}




static double fbcomb_getlin(double tab[], unsigned int sz, double idx){
    //copying from my own library, linear interpolated reader - DK
    double output;
    unsigned int tabphase1 = (unsigned int)idx;
    unsigned int tabphase2 = tabphase1 + 1;
    double frac = idx - (double)tabphase1;
    if(tabphase1 >= sz - 1){
        tabphase1 = sz - 1; //checking to see if index falls within bounds
        output = tab[tabphase1];
    }
    else if(tabphase1 < 0){
        tabphase1 = 0;
        output = tab[tabphase1];
    }
    else{
        double yb = tab[tabphase2]; //linear interp
        double ya = tab[tabphase1];
        output = ya+((yb-ya)*frac);
        
    };
    return output;
}

static double fbcomb_readmsdelay(t_fbcomb *x, double arr[], t_float ms){
    //helper func, basically take desired ms delay, convert to samp, read from arr[]
    
    //eventual reading head
    double rh = (double)ms*((double)x->x_sr*0.001); //converting ms to samples
    //bounds checking for minimum delay in samples
    if(rh < fbcomb_MIND){
        rh = fbcomb_MIND;
    };
    rh = (double)x->x_wh+((double)x->x_sz-rh); //essentially subracting from writehead to find proper position in buffer
    //wrapping into length of delay buffer
    while(rh >= x->x_sz){
        rh -= (double)x->x_sz;
    };
    //now to read from the buffer!
    double output = fbcomb_getlin(arr, x->x_sz, rh);
    return output;
    
}



static t_int *fbcomb_perform(t_int *w)
{
    t_fbcomb *x = (t_fbcomb *)(w[1]);
    int n = (int)(w[2]);
    t_float *xin = (t_float *)(w[3]);
    t_float *din = (t_float *)(w[4]);
    t_float *ain = (t_float *)(w[5]);
    t_float *out = (t_float *)(w[6]);
    
    int i;
    for(i=0; i<n;i++){
        int wh = x->x_wh;
        double input = (double)xin[i];
        //first off, write input to delay buf
        x->x_xbuf[wh] = input;
        //get delayed values of x and y
        t_float delms = din[i];
        //first bounds checking
        if(delms < 0){
            delms = 0;
        }
        else if(delms > x->x_maxdel){
            delms = x->x_maxdel;
        };
        //now get those delayed vals
//        double delx = fbcomb_readmsdelay(x, x->x_xbuf, delms);
        double dely = fbcomb_readmsdelay(x, x->x_ybuf, delms);
        //figure out your current y term: y[n] = -a*x[n] + x[n-d] + a*y[n-d]
        
        if (ain[i] == 0)
            ain[i] = 0;
        else
            ain[i] = copysign(exp(log(0.001) * delms/fabs(ain[i])), ain[i]);
        
        double output = input + (double)ain[i] * dely;
        //stick this guy in the ybuffer and output
        x->x_ybuf[wh] = output;
        out[i] = output;
        
        //increment writehead
        x->x_wh = (wh + 1) % x->x_sz;
    };
    
    return (w + 7);
}

static void fbcomb_dsp(t_fbcomb *x, t_signal **sp)
{
    int sr = sp[0]->s_sr;
    if(sr != x->x_sr){
        //if new sample rate isn't old sample rate, need to realloc
        x->x_sr = sr;
        fbcomb_sz(x);
    };
    dsp_add(fbcomb_perform, 6, x, sp[0]->s_n,
            sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec,
            sp[3]->s_vec);
}

static void fbcomb_list(t_fbcomb *x, t_symbol *s, int argc, t_atom * argv){
    
    
    int argnum = 0; //current argument
    while(argc){
        if(argv -> a_type == A_FLOAT){
            t_float curf = atom_getfloatarg(0, argc, argv);
            switch(argnum){
                case 0:
                    //maxdel
                    x->x_maxdel = curf > 0 ? curf : fbcomb_DELAY;
                    fbcomb_sz(x);
                    break;
                case 1:
                    //initdel
                    if(curf < fbcomb_MINMS){
                        curf = fbcomb_MINMS;
                    }
                    else if(curf > x->x_maxdel){
                        curf = x->x_maxdel;
                    };
                    pd_float((t_pd *)x->x_dellet, curf);
                    break;
                case 2:
                    //gain
                    pd_float((t_pd *)x->x_alet, curf);
                    break;
                default:
                    break;
            };
            argnum++;
        };
        argc--;
        argv++;
    };
    
    
    
}


static void *fbcomb_new(t_symbol *s, int argc, t_atom * argv){
    t_fbcomb *x = (t_fbcomb *)pd_new(fbcomb_class);
    
    //defaults
    t_float maxdel = fbcomb_DELAY;
    t_float initdel = fbcomb_MINMS;
    t_float gain = fbcomb_DEFGAIN;
    x->x_sr = sys_getsr();
    
    x->x_alloc = 0;
    x->x_sz = fbcomb_STACK;
    //clear out stack bufs, set pointer to stack
    x->x_ybuf = x->x_fbstack;
    x->x_xbuf = x->x_ffstack;
    fbcomb_clear(x);
    
    int argnum = 0; //current argument
    while(argc){
        if(argv -> a_type == A_FLOAT){
            t_float curf = atom_getfloatarg(0, argc, argv);
            switch(argnum){
                case 0:
                    maxdel = curf;
                    break;
                case 1:
                    initdel = curf;
                    break;
                case 2:
                    gain = curf;
                    break;
                default:
                    break;
            };
            argnum++;
        };
        argc--;
        argv++;
    };
    
    
    x->x_maxdel = maxdel > 0 ? maxdel : fbcomb_DELAY;
    //ship off to the helper method to deal with allocation if necessary
    fbcomb_sz(x);
    //boundschecking
    //this is 1/44.1 (1/(sr*0.001) rounded up, good enough?
    
    if(initdel < fbcomb_MINMS){
        initdel = fbcomb_MINMS;
    }
    else if(initdel > x->x_maxdel){
        initdel = x->x_maxdel;
    };
    
    //inlets outlets
    x->x_dellet = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_dellet, initdel);
    x->x_alet = inlet_new((t_object *)x, (t_pd *)x, &s_signal, &s_signal);
    pd_float((t_pd *)x->x_alet, gain);
    x->x_outlet = outlet_new((t_object *)x, &s_signal);
    return (x);
}



static void * fbcomb_free(t_fbcomb *x){
    if(x->x_alloc){
        free(x->x_xbuf);
        free(x->x_ybuf);
    };
    inlet_free(x->x_dellet);
    inlet_free(x->x_alet);
    outlet_free(x->x_outlet);
    return (void *)x;
}

void fbcomb_tilde_setup(void)
{
    fbcomb_class = class_new(gensym("fbcomb~"),
                              (t_newmethod)fbcomb_new,
                              (t_method)fbcomb_free,
                              sizeof(t_fbcomb), 0,
                              A_GIMME, 0);
    class_addmethod(fbcomb_class, nullfn, gensym("signal"), 0);
    class_addmethod(fbcomb_class, (t_method)fbcomb_dsp, gensym("dsp"), A_CANT, 0);
    class_addmethod(fbcomb_class, (t_method)fbcomb_clear, gensym("clear"), 0);
    class_addlist(fbcomb_class, (t_method)fbcomb_list);
}
