// -*- C++ -*-
/**
 * \file MathsSymbols.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Alejandro Aguilar Sierra
 * \author John Levon, moz@compsoc.man.ac.uk
 */

#ifndef MATHS_SYMBOLS_H
#define MATHS_SYMBOLS_H

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

extern char const * function_names[];
extern int const nr_function_names;
extern char const * latex_arrow[];
extern int const nr_latex_arrow;
extern char const * latex_bop[];
extern int const nr_latex_bop;
extern char const * latex_brel[];
extern int const nr_latex_brel;
extern char const * latex_dots[];
extern int const nr_latex_dots;
extern char const * latex_greek[];
extern int const nr_latex_greek;
extern char const * latex_misc[];
extern int const nr_latex_misc;
extern char const * latex_varsz[];
extern int const nr_latex_varsz;
extern char const * latex_ams_misc[];
extern int const nr_latex_ams_misc;
extern char const * latex_ams_arrows[];
extern int const nr_latex_ams_arrows;
extern char const * latex_ams_rel[];
extern int const nr_latex_ams_rel;
extern char const * latex_ams_nrel[];
extern int const nr_latex_ams_nrel;
extern char const * latex_ams_ops[];
extern int const nr_latex_ams_ops;

#endif /* MATHS_SYMBOLS_H */
