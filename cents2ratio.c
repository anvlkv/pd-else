/* Plugin structure generated by Schiavoni Pure Data external Generator */
#include "m_pd.h"
#include <math.h>

// ---------------------------------------------------
// Class definition
// ---------------------------------------------------
static t_class *cents2ratio_class;

// ---------------------------------------------------
// Data structure definition
// ---------------------------------------------------
typedef struct _cents2ratio {
   t_object x_obj;
   t_outlet * x_outlet_out_float;
} t_cents2ratio;

// ---------------------------------------------------
// Functions signature
// ---------------------------------------------------
void * cents2ratio_new(void);// Constructor
void cents2ratio_destroy(t_cents2ratio *x); //Destructor
void cents2ratio_in_float_method(t_cents2ratio *x, t_floatarg f);// Active inlet function

// ---------------------------------------------------
// cents2ratio_in
// ---------------------------------------------------
void cents2ratio_in_float_method(t_cents2ratio *x, t_floatarg f){
//PUT YOUR CODE HERE
    
    float out = pow(2, (f/1200));
    
    outlet_float(x->x_outlet_out_float, out);
}

// ---------------------------------------------------
// Constructor of the class
// ---------------------------------------------------
void * cents2ratio_new(void){
   t_cents2ratio *x = (t_cents2ratio *) pd_new(cents2ratio_class);

   x->x_outlet_out_float = outlet_new(&x->x_obj, gensym("float"));
   return (void *) x;
}

// ---------------------------------------------------
// Destroy the class
// ---------------------------------------------------
void cents2ratio_destroy(t_cents2ratio *x) {
   outlet_free(x->x_outlet_out_float);
}

// ---------------------------------------------------
// Setup
// ---------------------------------------------------
void cents2ratio_setup(void) {
   cents2ratio_class = class_new(gensym("cents2ratio"),
      (t_newmethod) cents2ratio_new, // Constructor
      (t_method) cents2ratio_destroy, // Destructor
      sizeof (t_cents2ratio),
      CLASS_DEFAULT,
      0);//Must always ends with a zero

   class_addfloat(cents2ratio_class, cents2ratio_in_float_method);
}
// EOF---------------------------------------------------