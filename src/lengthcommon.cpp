/**
 * \file lengthcommon.cpp
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

#include "support/gettext.h"
#include "Length.h"

#include <string>

using namespace std;

namespace lyx {

// I am not sure if "mu" should be possible to select (Lgb)

// the latex units
char const * const unit_name[] = {
	"bp", "cc", "cm", "dd", "em", "ex", "in", "mm", "mu",
	"pc", "pt", "sp",
	"text%",  "col%", "page%", "line%",
	"theight%", "pheight%", "" };

int const num_units = int(sizeof(unit_name) / sizeof(unit_name[0]) - 1);

// the LyX gui units
char const * const unit_name_gui[] = {
	N_("bp"), N_("cc[[unit of measure]]"), N_("cm"), N_("dd"), N_("em"),
	N_("ex"), N_("in"), N_("mm"), N_("mu[[unit of measure]]"), N_("pc"),
	N_("pt"), N_("sp"), N_("Text Width %"),
	N_("Column Width %"), N_("Page Width %"), N_("Line Width %"),
	N_("Text Height %"), N_("Page Height %"), "" };

Length::UNIT unitFromString(string const & data)
{
	int i = 0;
	while (i < num_units && data != unit_name[i])
		++i;
	return static_cast<Length::UNIT>(i);
}


} // namespace lyx
