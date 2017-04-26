
#include "m_pd.h"
//#include <math.h>

#define MAXINTPUT 500 //maximum number of channel inlets

typedef struct _selectx
{
    t_object   x_obj;
    t_float *x_mainsig; //the main signal being piped in
    int 	  x_sigputs; //inlets excluding selectx idx (1 indexed)
    t_float   x_state; //0 = closed, nonzero = index of inlet to pass (1 indexed)
    t_float  **x_ivecs; // copying from matrix
    t_float  *x_ovec; // single pointer since we're dealing with an array rather than an array of arrays
} t_selectx;

static t_class *selectx_class;

static t_int *selectx_perform(t_int *w)
{
    t_selectx *x = (t_selectx *)(w[1]);
    int nblock = (int)(w[2]);
    
    t_float *state = x->x_mainsig;
    t_float **ivecs = x->x_ivecs;
    t_float *ovec = x->x_ovec;
    
    int i,j;
    
    int sigputs = x->x_sigputs;
    
    for(i=0; i< nblock; i++){
        int curst = (int)state[i];
        if (curst > sigputs){
            curst = sigputs;
        }
        t_float output = 0;
        if(curst != 0){
            for(j=0; j<sigputs;j++){
                if(curst == (j+1)){
                        output = ivecs[j][i];
                };
            };
        };
        ovec[i] = output;
    };
    return (w + 3);
}


static void selectx_dsp(t_selectx *x, t_signal **sp)
{
    int i, nblock = sp[0]->s_n;
    t_signal **sigp = sp;
    x->x_mainsig = (*sigp++)->s_vec; //the first sig in is the selectx idx
    for (i = 0; i < x->x_sigputs; i++){ //now for the sigputs
        *(x->x_ivecs+i) = (*sigp++)->s_vec;
    };
    x->x_ovec = (*sigp++)->s_vec; //now for the outlet
    dsp_add(selectx_perform, 2, x, nblock);
}

static void *selectx_new(t_symbol *s, int argc, t_atom *argv)
{
    t_selectx *x = (t_selectx *)pd_new(selectx_class);
    t_float sigputs = 1; //inlets not counting selectx input
    t_float state = 0; //start off closed initially
    int i;
    int argnum = 0;
    while(argc > 0){
        if(argv -> a_type == A_FLOAT){
            t_float argval = atom_getfloatarg(0, argc, argv);
            switch(argnum){
                case 0:
                    sigputs = argval;
                    break;
                case 1:
                    state = argval;
                    break;
                default:
                    break;
            };
            argc--;
            argv++;
            argnum++;
        };
    };
    
    //bounds checking
    if(sigputs < 1){
        sigputs = 1;
    }
    else if(sigputs > (t_float)MAXINTPUT){
        sigputs = MAXINTPUT;
    };
    if(state < 0){
        state = 0;
    }
    else if(state > sigputs){
        state = sigputs;
    };
    
    x->x_sigputs = (int)sigputs;
    x->x_state = state;
    x->x_ivecs = getbytes(sigputs * sizeof(*x->x_ivecs));
    
    for (i = 0; i < sigputs; i++){
        inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    };
    outlet_new((t_object *)x, &s_signal);
    
    return (x);
}

void * selectx_free(t_selectx *x){
    freebytes(x->x_ivecs, x->x_sigputs * sizeof(*x->x_ivecs));
    return (void *) x;
}

void selectx_tilde_setup(void)
{
    selectx_class = class_new(gensym("selectx~"), (t_newmethod)selectx_new,
                    (t_method)selectx_free, sizeof(t_selectx), CLASS_DEFAULT, A_GIMME, 0);
    class_addmethod(selectx_class, (t_method)selectx_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(selectx_class, t_selectx, x_state);
}
