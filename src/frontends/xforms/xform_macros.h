#ifndef XFORM_MACROS_H
#define XFORM_MACROS_H

/** Collection of useful macros to assist in correctly wrapping and
    mapping C callbacks to the C++ member functions.
 */

#define C_GENERICCB(FN, METHOD)				\
extern "C" void C_##FN##METHOD(FL_OBJECT * ob, long d)	\
{							\
	FN::METHOD(ob, d);				\
}

// Return CB's all seem to take a void* as the 2nd argument and return an int.
#define C_RETURNCB(FN, METHOD)					\
extern "C" int C_##FN##METHOD(FL_FORM * ob, void * d)		\
{								\
	return FN::METHOD(ob, d);				\
}

#endif
