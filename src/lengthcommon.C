/**
 * \file lengthcommon.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Matthias Ettrich
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "lengthcommon.h"

#include "gettext.h"


int const num_units = LyXLength::UNIT_NONE;

// I am not sure if "mu" should be possible to select (Lgb)

// unit_name is for compatibility. Can be deleted when all works well.
// means, when we have full language support for the lengths
// in all gui's				(Herbert 2002-11-01)
char const * unit_name[num_units] = {
	"sp", "pt", "bp", "dd", "mm", "pc",
	"cc", "cm", "in", "ex", "em", "mu",
	"text%",  "col%", "page%", "line%",
	"theight%", "pheight%" };

// the latex units
char const * unit_name_ltx[num_units] = {
	"sp", "pt", "bp", "dd", "mm", "pc",
	"cc", "cm", "in", "ex", "em", "mu",
	// in 1.4 the following names should be used. then no
	// translation into the latex ones are needed
//	"textheight",  "columnwidth", "pagewidth", "linewidth",
//	"textheight", "pageheight" };
	"text%",  "col%", "page%", "line%",
	"theight%", "pheight%" };

// the LyX gui units
char const * unit_name_gui[num_units] = {
	N_("sp"), N_("pt"), N_("bp"), N_("dd"), N_("mm"), N_("pc"),
	N_("cc"), N_("cm"), N_("in"), N_("ex"), N_("em"), N_("mu"),
	N_("text%"), N_("col%"), N_("page%"), N_("line%"),
	N_("theight%"), N_("pheight%") };

	// this one maybe better ???? but there can be problems with
	// xforms (Herbert)
//	N_("textwidth%"), N_("columnwidth%"), N_("pagewidth%"), N_("linewidth%"),
//	N_("textheight%"), N_("pageheight%") };

	// or altenative this ones
//	N_("twidth%"), N_("cwidth%"), N_("pwidth%"), N_("lwidth%"),
//	N_("theight%"), N_("pheight%") };

LyXLength::UNIT unitFromString(string const & data)
{
	int i = 0;
	while (i < num_units && data != unit_name[i])
		++i;
	return static_cast<LyXLength::UNIT>(i);
}
