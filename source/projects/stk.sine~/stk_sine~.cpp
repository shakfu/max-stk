/**
    @file
    stk~: stk for Max
*/
#include "SineWave.h"
// #include "RtWvOut.h"
#include <cstdlib>

#include "ext.h"
#include "ext_obex.h"
#include "z_dsp.h"


enum {
    TIME = 1, 
    PHASE, 
    PHASE_OFFSET,
    MAX_INLET_INDEX // -> maximum number of inlets (0-based)
};

// struct to represent the object's state
typedef struct _stk {
    t_pxobject ob;          // the object itself (t_pxobject in MSP instead of t_object)
    stk::SineWave* wave;    // stk sine wave object
    double freq;            // frequency
    double time;            // absolute time added in samples
    double phase;           // Add a time in cycles (one cycle = TABLE_SIZE (2048 samples))
    double phase_offset;    // Add a phase offset relative to any previous offset value
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

    t_class *c = class_new("stk.sine~", (method)stk_new, (method)stk_free, (long)sizeof(t_stk), 0L, A_GIMME, 0);

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

        x->wave = new stk::SineWave;
        x->freq = 0.0;
        x->time = 0.0;
        x->phase = 0.0;
        x->phase_offset = 0.0;        
    }
    return (x);
}


void stk_free(t_stk *x)
{
    delete x->wave;
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
            // post("received in inlet 0");
            x->freq = f;
            // post("freq: %f", x->freq);
            break;
        case 1:
            // post("received in inlet 2");
            x->time = f;
            // post("time: %f", x->time);
            break;
        case 2:
            // post("received in inlet 3");
            x->phase = f;
            // post("phase: %f", x->phase);
            break;
        case 3:
            // post("received in inlet 4");
            x->phase_offset = f;
            // post("phase_offset: %f", x->phase_offset);
            break;
    }
}



void stk_dsp64(t_stk *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
    post("sample rate: %f", samplerate);
    post("maxvectorsize: %d", maxvectorsize);

    stk::Stk::setSampleRate(samplerate);
    x->wave->reset();

    object_method(dsp64, gensym("dsp_add64"), x, stk_perform64, 0, NULL);
}


void stk_perform64(t_stk *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
    t_double *inL = ins[0];     // we get audio for each inlet of the object from the **ins argument
    t_double *outL = outs[0];   // we get audio for each outlet of the object from the **outs argument
    int n = sampleframes;       // n = 64
    x->wave->setFrequency(x->freq);
    x->wave->addTime(x->time);
    x->wave->addPhase(x->phase);
    x->wave->addPhaseOffset(x->phase_offset);

    while (n--) {
        *outL++ = x->wave->tick();
    }
}
