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
	LaTeXFont() : osfdefault_(false), switchdefault_(false) {}
	/// The font name
	docstring const & name() { return name_; }
	/// The name to appear in the document dialog
	docstring const & guiname() { return guiname_; }
	/// Font family (rm, sf, tt)
	docstring const & family() { return family_; }
	/// The package that provides this font
	docstring const & package() { return package_; }
	/// Alternative font if package() is not available
	std::vector<docstring> const & altfonts() { return altfonts_; }
	/// A font that provides all families
	docstring const & completefont() { return completefont_; }
	/// A font specifically needed for OT1 font encoding
	docstring const & ot1font() { return ot1font_; }
	/// A font that provides Old Style Figures for this type face
	docstring const & osffont() { return osffont_; }
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
	/// Does this font provide a given \p feature
	bool provides(std::string const & name, bool ot1,
		      bool complete, bool nomath);
	/// Issue the familydefault switch
	bool switchdefault() const { return switchdefault_; }
	/// Does the font provide Old Style Figures as default?
	bool osfDefault() const { return osfdefault_; }
	/// Is this font available?
	bool available(bool ot1, bool nomath);
	/// Does this font provide an alternative without math? 
	bool providesNoMath(bool ot1, bool complete);
	/// Does this font provide Old Style Figures?
	bool providesOSF(bool ot1, bool complete, bool nomath);
	/// Does this font provide optional true SmallCaps?
	bool providesSC(bool ot1, bool complete, bool nomath);
	/** does this font provide OSF and Small Caps only via 
	 * a single, undifferentiated expert option?
	 */
	bool hasMonolithicExpertSet(bool ot1, bool complete, bool nomath);
	/// Does this font provide scaling?
	bool providesScale(bool ot1, bool complete, bool nomath);
	/// Return the LaTeX Code
	std::string const getLaTeXCode(bool dryrun, bool ot1, bool complete,
				       bool sc, bool osf, bool nomath,
				       int const & scale = 100);
	/// Return the actually used font
	docstring const getUsedFont(bool ot1, bool complete, bool nomath);
	/// Return the actually used package
	docstring const getUsedPackage(bool ot1, bool complete, bool nomath);
	///
	bool read(Lexer & lex);
	///
	bool readFont(Lexer & lex);
private:
	/// Return the preferred available package 
	std::string const getAvailablePackage(bool dryrun);
	/// Return the package options
	std::string const getPackageOptions(bool ot1,
					    bool complete,
					    bool sc,
					    bool osf,
					    int scale,
					    bool nomath);
	/// Return an alternative font
	LaTeXFont altFont(docstring const & name);
	///
	docstring name_;
	///
	docstring guiname_;
	///
	docstring family_;
	///
	docstring package_;
	///
	std::vector<docstring> altfonts_;
	///
	docstring completefont_;
	///
	docstring nomathfont_;
	///
	docstring ot1font_;
	///
	docstring osffont_;
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
	std::vector<std::string> provides_;
	///
	docstring requires_;
	///
	std::string preamble_;
	///
	bool osfdefault_;
	///
	bool switchdefault_;
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
	/// Get a specific AltFont \p name
	LaTeXFont getAltFont(docstring const & name);
private:
	///
	void readLaTeXFonts();
	///
	TexFontMap texfontmap_;
	///
	TexFontMap texaltfontmap_;
};

/// Implementation is in LyX.cpp
extern LaTeXFonts & theLaTeXFonts();


} // namespace lyx

#endif
