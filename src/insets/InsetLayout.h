// -*- C++ -*-
/**
 * \file InsetLayout.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_LAYOUT_H
#define INSET_LAYOUT_H

#include "ColorCode.h"
#include "FontInfo.h"

#include "support/docstring.h"

#include <set>
#include <string>

namespace lyx {

///
class InsetLayout {
public:
	std::string name;
	std::string lyxtype;
	docstring labelstring;
	std::string decoration;
	std::string latextype;
	std::string latexname;
	std::string latexparam;
	FontInfo font;
	FontInfo labelfont;
	ColorCode bgcolor;
	std::string preamble;
	std::set<std::string> requires;
	bool multipar;
	bool passthru;
	bool needprotect;
	bool freespacing;
	bool keepempty;
	bool forceltr;
};

} // namespace lyx

#endif
