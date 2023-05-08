/**
    @file
    stk.delay~: stk delay for Max
    lot to do... delay is very basic
*/
#include "Delay.h"
#include <cstdlib>

#include "ext.h"
#include "ext_obex.h"
#include "z_dsp.h"


enum {
    DELAY = 0, 
    WET,
    FEEDBACK,
    MAX_INLET_INDEX // -> maximum number of inlets (0-based)
};

// struct to represent the object's state
typedef struct _stk {
    t_pxobject ob;          // the object itself (t_pxobject in MSP instead of t_object)
    stk::Delay* delayline;  // stk non-interpolating delayline object
    double delay;           // delay
    double wet;             // dry/wet ratio (0.0-1.0)
    double feedback_ratio;  // feedback_ratio (0.0-2.0)
    double feedback;        // feedback
    long m_in;              // space for the inlet number used by all the proxies
    void *inlets[MAX_INLET_INDEX];
} t_stk;


// method prototypes
void *stk_new(t_symbol *s, long argc, t_atom *argv);
void stk_free(t_stk *x);
void stk_assist(t_stk *x, void *b, long m, long a, char *s);
void stk_bang(t_stk *x);
void stk_anything(t_stk* x, t_symbol* s, long argc, t_atom* argv);
void stk_float(t_stk *x, double f);
void stk_dsp64(t_stk *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void stk_perform64(t_stk *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);


// global class pointer variable
static t_class *stk_class = NULL;


//-----------------------------------------------------------------------------------------------

void ext_main(void *r)
{
    // object initialization, note the use of dsp_free for the freemethod, which is required
    // unless you need to free allocated memory, in which case you should call dsp_free from
    // your custom free function.

    t_class *c = class_new("stk.delay~", (method)stk_new, (method)stk_free, (long)sizeof(t_stk), 0L, A_GIMME, 0);

    class_addmethod(c, (method)stk_float,    "float",    A_FLOAT, 0);
    class_addmethod(c, (method)stk_anything, "anything", A_GIMME, 0);
    class_addmethod(c, (method)stk_bang,     "bang",              0);
    class_addmethod(c, (method)stk_dsp64,    "dsp64",    A_CANT,  0);
    class_addmethod(c, (method)stk_assist,   "assist",   A_CANT,  0);

    class_dspinit(c);
    class_register(CLASS_BOX, c);
    stk_class = c;
}

void *stk_new(t_symbol *s, long argc, t_atom *argv)
{
    t_stk *x = (t_stk *)object_alloc(stk_class);

    if (x) {
        dsp_setup((t_pxobject *)x, 1);  // MSP inlets: arg is # of signal inlets and is REQUIRED!
        // use 0 if you don't need signal inlets

        outlet_new(x, "signal");        // signal outlet (note "signal" rather than NULL)

        for(int i = (MAX_INLET_INDEX - 1); i > 0; i--) {
            x->inlets[i] = proxy_new((t_object *)x, i, &x->m_in);
        }

        x->delayline = new stk::Delay;
        x->delay = 0.0;
        x->feedback = 0.0;
        x->feedback_ratio = 0.1;
        x->wet = 0.0;

    }
    return (x);
}


void stk_free(t_stk *x)
{
    delete x->delayline;
    dsp_free((t_pxobject *)x);
    for(int i = (MAX_INLET_INDEX - 1); i > 0; i--) {
        object_free(x->inlets[i]);
    }

}


void stk_assist(t_stk *x, void *b, long m, long a, char *s)
{
    // FIXME: assign to inlets
    if (m == ASSIST_INLET) { //inlet
        sprintf(s, "I am inlet %ld", a);
    }
    else {  // outlet
        sprintf(s, "I am outlet %ld", a);
    }
}

void stk_bang(t_stk *x)
{
    post("bang");
}

void stk_anything(t_stk* x, t_symbol* s, long argc, t_atom* argv)
{

    if (s != gensym("")) {
        post("symbol: %s", s->s_name);
    }
}


void stk_float(t_stk *x, double f)
{
    switch (proxy_getinlet((t_object *)x)) {
        case 0:
            post("received in inlet 0");
            x->delay = f;
            post("delay: %f", x->delay);
            break;
        case 1:
            post("received in inlet 2");
            x->wet = f;
            post("wet: %f", x->wet);
            break;
        case 2:
            post("received in inlet 3");
            x->feedback_ratio = f;
            post("feedback_ratio: %f", x->feedback_ratio);
            break;
    }
}



void stk_dsp64(t_stk *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    post("sample rate: %f", samplerate);
    post("maxvectorsize: %d", maxvectorsize);

    object_method(dsp64, gensym("dsp_add64"), x, stk_perform64, 0, NULL);
}


void stk_perform64(t_stk *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
    t_double *inL = ins[0];     // we get audio for each inlet of the object from the **ins argument
    t_double *outL = outs[0];   // we get audio for each outlet of the object from the **outs argument
    t_double value, feedback, wet, wet_val, dry_val;
    int n = sampleframes;       // n = 64
    x->delayline->setDelay(x->delay);
    wet = x->wet;
    feedback = x->feedback;



    while (n--) {
        value = *inL++ + feedback;
        wet_val = wet * x->delayline->tick(value);
        feedback = x->feedback_ratio * wet_val;
        dry_val = (1 - wet) * value;
        *outL++ = dry_val + wet_val;
    }
    x->feedback = feedback;
}
