// -*- C++ -*-
/**
 * \file InsetLayout.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Martin Vermeer
 * \author Richard Heck
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

class Lexer;
	
///
class InsetLayout {
public:
	///
	InsetLayout();
	///
	bool read(Lexer & lexrc);
	///
	docstring name() const { return name_; };
	///
	std::string lyxtype() const { return lyxtype_; };
	///
	docstring labelstring() const { return labelstring_; };
	///
	//FIXME This could be an enum
	std::string decoration() const { return decoration_; };
	///
	std::string latextype() const { return latextype_; };
	///
	std::string latexname() const { return latexname_; };
	///
	std::string latexparam() const { return latexparam_; };
	///
	FontInfo font() const { return font_; };
	///
	FontInfo labelfont() const { return labelfont_; };
	///
	ColorCode bgcolor() const { return bgcolor_; };
	///
	std::string preamble() const { return preamble_; };
	///
	std::set<std::string> requires() const { return requires_; };
	///
	bool isMultiPar() const { return multipar_; };
	///
	bool isPassThru() const { return passthru_; };
	///
	bool isNeedProtect() const { return needprotect_; };
	///
	bool isFreeSpacing() const { return freespacing_; };
	///
	bool isKeepEmpty() const { return keepempty_; };
	///
	bool isForceLtr() const { return forceltr_; };
private:
	///
	docstring name_;
	///
	std::string lyxtype_;
	///
	docstring labelstring_;
	///
	std::string decoration_;
	///
	std::string latextype_;
	///
	std::string latexname_;
	///
	std::string latexparam_;
	///
	FontInfo font_;
	///
	FontInfo labelfont_;
	///
	ColorCode bgcolor_;
	///
	std::string preamble_;
	///
	std::set<std::string> requires_;
	///
	bool multipar_;
	///
	bool passthru_;
	///
	bool needprotect_;
	///
	bool freespacing_;
	///
	bool keepempty_;
	///
	bool forceltr_;
};

} // namespace lyx

#endif
