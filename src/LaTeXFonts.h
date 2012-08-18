// -*- C++ -*-
/**
 * \file LaTeXFonts.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LATEXFONTS_H
#define LATEXFONTS_H

#include "support/docstring.h"

#include <map>
#include <vector>


namespace lyx {

class Lexer;

/// LaTeX Font definition
class LaTeXFont {
public:
	/// TeX font
	LaTeXFont() : switchdefault_(false) {}
	/// The font name
	docstring const & name() { return name_; }
	/// The name to appear in the document dialog
	docstring const & guiname() { return guiname_; }
	/// Font family (rm, sf, tt)
	docstring const & family() { return family_; }
	/// The package that provides this font
	docstring const & package() { return package_; }
	/// Alternative packages if package() is not available
	std::vector<docstring> const & altpackages() { return altpackages_; }
	/// A package that provides all families
	docstring const & completepackage() { return completepackage_; }
	/// A package specifically needed for OT1 font encoding
	docstring const & ot1package() { return ot1package_; }
	/// A package that provides Old Style Figures for this font
	docstring const & osfpackage() { return osfpackage_; }
	/// A package option needed to load this font
	docstring const & packageoption() { return packageoption_; }
	/// A package option for Old Style Figures
	docstring const & osfoption() { return osfoption_; }
	/// A package option for true SmallCaps
	docstring const & scoption() { return scoption_; }
	/// A package option for both Old Style Figures and SmallCaps
	docstring const & osfscoption() { return osfscoption_; }
	/// A package option for font scaling
	docstring const & scaleoption() { return scaleoption_; }
	/// Alternative requirement to test for
	docstring const & requires() { return requires_; }
	/// Issue the familydefault switch
	bool switchdefault() const { return switchdefault_; }
	/// Is this font available?
	bool available(bool ot1 = false) const;
	/// Does this font provide Old Style Figures?
	bool providesOSF(bool ot1 = false) const;
	/// Does this font provide optional true SmallCaps?
	bool providesSC(bool ot1 = false) const;
	/// Does this font provide scaling?
	bool providesScale(bool ot1 = false) const;
	/// Return the LaTeX Code
	std::string const getLaTeXCode(bool dryrun, bool ot1, bool complete,
				       bool sc, bool osf,
				       int const & scale = 100);
	///
	bool read(Lexer & lex);
	///
	bool readFont(Lexer & lex);
private:
	/// Return the preferred available package 
	std::string const getAvailablePackage(bool dryrun,
					      bool ot1,
					      bool complete,
					      bool & alt);
	/// Return the package options
	std::string const getPackageOptions(bool ot1,
					    bool sc,
					    bool osf,
					    int scale);
	///
	docstring name_;
	///
	docstring guiname_;
	///
	docstring family_;
	///
	docstring package_;
	///
	std::vector<docstring> altpackages_;
	///
	docstring completepackage_;
	///
	docstring ot1package_;
	///
	docstring osfpackage_;
	///
	docstring packageoption_;
	///
	docstring osfoption_;
	///
	docstring scoption_;
	///
	docstring osfscoption_;
	///
	docstring scaleoption_;
	///
	docstring requires_;
	///
	bool switchdefault_;
	///
	bool available_;
	///
	bool available_ot1_;
};
  
  
/** The list of available LaTeX fonts
 */
class LaTeXFonts {
public:
	///
	typedef std::map<docstring, LaTeXFont> TexFontMap;
	/// Get all LaTeXFonts
	TexFontMap getLaTeXFonts();
	/// Get a specific LaTeXFont \p name
	LaTeXFont getLaTeXFont(docstring const & name);
private:
	///
	void readLaTeXFonts();
	///
	TexFontMap texfontmap_;
};

/// Implementation is in LyX.cpp
extern LaTeXFonts & theLaTeXFonts();


} // namespace lyx

#endif
