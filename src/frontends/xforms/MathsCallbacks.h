/**
 * \file MathsCallbacks.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon 
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef MATHSCALLBACKS_H
#define MATHSCALLBACKS_H
 
/// values used by the xforms callbacks
enum MathsCallbackValues {
	MM_GREEK,
	MM_ARROW,
	MM_BOP,
	MM_BRELATS,
	MM_VARSIZE,
	MM_MISC,
	MM_FRAC,
	MM_SQRT,
	MM_DELIM,
	MM_SUPER,
	MM_SUB,
	MM_SUBSUPER,
	MM_MATRIX,
	MM_EQU,
	MM_DECO,
	MM_SPACE,
	MM_STYLE,
	MM_DOTS,
	MM_AMS_MISC,
	MM_AMS_ARROWS,
	MM_AMS_BREL,
	MM_AMS_NREL,
	MM_AMS_OPS,
	MM_FUNC
};

#endif // MATHSCALLBACKS_H
