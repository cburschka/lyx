// -*- C++ -*-
/**
 * \file BufferParams.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author John Levon
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef BUFFERPARAMS_H
#define BUFFERPARAMS_H

#include "BiblioInfo.h"
#include "TextClass.h"
#include "TextClassPtr.h"
#include "paper.h"

#include "insets/InsetQuotes.h"

#include "support/copied_ptr.h"
#include "support/FileName.h"
#include "support/types.h"

#include <vector>

namespace lyx {

class AuthorList;
class BranchList;
class Bullet;
class Encoding;
class Lexer;
class LatexFeatures;
class Spacing;
class TexRow;
class VSpace;
class Language;

/** Buffer parameters.
 *  This class contains all the parameters for this buffer's use. Some
 *  work needs to be done on this class to make it nice. Now everything
 *  is in public.
 */
class BufferParams {
public:
	///
	enum PARSEP {
		///
		PARSEP_INDENT,
		///
		PARSEP_SKIP
	};
	///
	BufferParams();
	~BufferParams();

	/// get l10n translated to the buffers language
	docstring const B_(std::string const & l10n) const;

	/// read a header token, if unrecognised, return it or an unknown class name
	std::string const readToken(Lexer & lex, std::string const & token);

	///
	void writeFile(std::ostream &) const;

	/** \returns true if the babel package is used (interogates
	 *  the BufferParams and a LyXRC variable).
	 *  This returned value can then be passed to the insets...
	 */
	bool writeLaTeX(odocstream &, LaTeXFeatures &, TexRow &) const;

	///
	void useClassDefaults();

	///
	bool hasClassDefaults() const;

	///
	VSpace const & getDefSkip() const;

	///
	void setDefSkip(VSpace const & vs);

	/** Whether paragraphs are separated by using a indent like in
	 *  articles or by using a little skip like in letters.
	 */
	PARSEP paragraph_separation;
	///
	InsetQuotes::quote_language quotes_language;
	///
	InsetQuotes::quote_times quotes_times;
	///
	std::string fontsize;
	///Get the LyX TextClass (that is, the layout file) this document is using.
	textclass_type getBaseClass() const;
	///Set the LyX TextClass (that is, the layout file) this document is using.
	///NOTE This also calls makeTextClass(), to update the local
	///TextClass.
	bool setBaseClass(textclass_type);
	///Returns the TextClass currently in use: the BaseClass as modified
	///by modules.
	TextClass const & getTextClass() const;
	///Returns a pointer to the TextClass currently in use: the BaseClass 
	///as modified by modules. (See \file TextClassPtr.h for the typedef.)
	TextClassPtr getTextClassPtr() const;
	///Set the LyX TextClass---layout file---this document is using.
	///This does NOT call makeTextClass() and so should be used with
	///care. This is most likely not what you want if you are operating on 
	///BufferParams that are actually associatd with a Buffer. If, on the
	///other hand, you are using a temporary set of BufferParams---say, in
	///a controller, it may well be, since in that case the local TextClass
	///has nothing to do.
	void setJustBaseClass(textclass_type);
	/// This bypasses the baseClass and sets the textClass directly.
	/// Should be called with care and would be better not being here,
	/// but it seems to be needed by CutAndPaste::putClipboard().
	void setTextClass(TextClassPtr);
	/// List of modules in use
	std::vector<std::string> const & getModules() const;
	/// Add a module to the list of modules in use.
	/// Returns true if module was successfully added.
	bool addLayoutModule(std::string modName, bool makeClass = true);
	/// Add a list of modules.
	/// Returns true if all modules were successfully added.
	bool addLayoutModules(std::vector<std::string>modNames);
	/// Clear the list
	void clearLayoutModules();

	/// returns the main font for the buffer (document)
	Font const getFont() const;

	/* this are for the PaperLayout */
	/// the papersize
	PAPER_SIZE papersize;
	///
	PAPER_ORIENTATION orientation;
	/// use custom margins
	bool use_geometry;
	///
	std::string paperwidth;
	///
	std::string paperheight;
	///
	std::string leftmargin;
	///
	std::string topmargin;
	///
	std::string rightmargin;
	///
	std::string bottommargin;
	///
	std::string headheight;
	///
	std::string headsep;
	///
	std::string footskip;

	/* some LaTeX options */
	/// The graphics driver
	std::string graphicsDriver;
	/// the rm font
	std::string fontsRoman;
	/// the sf font
	std::string fontsSans;
	/// the tt font
	std::string fontsTypewriter;
	/// the default family (rm, sf, tt)
	std::string fontsDefaultFamily;
	/// use expert Small Caps
	bool fontsSC;
	/// use Old Style Figures
	bool fontsOSF;
	/// the scale factor of the sf font
	int fontsSansScale;
	/// the scale factor of the tt font
	int fontsTypewriterScale;
	///
	Spacing & spacing();
	Spacing const & spacing() const;
	///
	int secnumdepth;
	///
	int tocdepth;
	///
	Language const * language;
	/// BranchList:
	BranchList & branchlist();
	BranchList const & branchlist() const;
	/**
	 * The input encoding for LaTeX. This can be one of
	 * - \c auto: find out the input encoding from the used languages
	 * - \c default: ditto
	 * - any encoding supported by the inputenc package
	 * The encoding of the LyX file is always utf8 and has nothing to
	 * do with this setting.
	 * The difference between \c auto and \c default is that \c auto also
	 * causes loading of the inputenc package, while \c default does not.
	 * \c default will not work unless the user takes additional measures
	 * (such as using special environments like the CJK environment from
	 * CJK.sty).
	 * \c default can be seen as an unspecified 8bit encoding, since LyX
	 * does not interpret it in any way apart from display on screen.
	 */
	std::string inputenc;
	/// The main encoding used by this buffer for LaTeX output.
	/// Individual pieces of text can use different encodings.
	Encoding const & encoding() const;
	///
	std::string preamble;
	///
	std::string options;
	///
	std::string float_placement;
	///
	unsigned int columns;
	/// parameters for the listings package
	std::string listings_params;
	///
	TextClass::PageSides sides;
	///
	std::string pagestyle;
	/// \param index should lie in the range 0 <= \c index <= 3.
	Bullet & temp_bullet(size_type index);
	Bullet const & temp_bullet(size_type index) const;
	/// \param index should lie in the range 0 <= \c index <= 3.
	Bullet & user_defined_bullet(size_type index);
	Bullet const & user_defined_bullet(size_type index) const;

	/// Whether to load a package such as amsmath or esint.
	/// The enum values must not be changed (file format!)
	enum Package {
		/// Don't load the package. For experts only.
		package_off = 0,
		/// Load the package if needed (recommended)
		package_auto = 1,
		/// Always load the package (e.g. if the document contains
		/// some ERT that needs the package)
		package_on = 2
	};
	/// Whether and how to load amsmath
	Package use_amsmath;
	/// Whether and how to load esint
	Package use_esint;
	///
	bool use_bibtopic;
	/// revision tracking for this buffer ?
	bool trackChanges;
	/** This param decides whether change tracking marks should be used
	 *  in output (irrespective of how these marks are actually defined;
	 *  for instance, they may differ for DVI and PDF generation)
	 */
	bool outputChanges;
	/// Time ago we agreed that this was a buffer property [ale990407]
	std::string parentname;
	///
	bool compressed;
	///
	bool embedded;

	/// the author list for the document
	AuthorList & authors();
	AuthorList const & authors() const;

	/// map of the file's author IDs to buffer author IDs
	std::vector<unsigned int> author_map;
	///
	std::string const dvips_options() const;
	///
	std::string const paperSizeName() const;
	/// set up if and how babel is called
	std::string const babelCall(std::string const & lang_opts) const;
	/// handle inputenc etc.
	void writeEncodingPreamble(odocstream & os, LaTeXFeatures & features,
					      TexRow & texrow) const;
	/// set up the document fonts
	std::string const loadFonts(std::string const & rm,
				     std::string const & sf, std::string const & tt,
				     bool const & sc, bool const & osf,
				     int const & sfscale, int const & ttscale) const;
	/// path of the current buffer
	std::string filepath;
	/// get the appropriate cite engine (natbib handling)
	biblio::CiteEngine getEngine() const;

	///
	void setCiteEngine(biblio::CiteEngine const);

private:
	///
	void readPreamble(Lexer &);
	///
	void readLanguage(Lexer &);
	///
	void readGraphicsDriver(Lexer &);
	///
	void readBullets(Lexer &);
	///
	void readBulletsLaTeX(Lexer &);
	///
	void readModules(Lexer &);
	/// Adds the module information to the baseClass information to
	/// create our local TextClass.
	void makeTextClass();

	
	/// for use with natbib
	biblio::CiteEngine cite_engine_;
	/// the base TextClass associated with the document
	textclass_type baseClass_;
	/// the possibly modular TextClass actually in use
	TextClassPtr textClass_;
	///
	typedef std::vector<std::string> LayoutModuleList;
	/// 
	LayoutModuleList layoutModules_;

	/** Use the Pimpl idiom to hide those member variables that would otherwise
	 *  drag in other header files.
	 */
	class Impl;
	class MemoryTraits {
	public:
		static Impl * clone(Impl const *);
		static void destroy(Impl *);
	};
	support::copied_ptr<Impl, MemoryTraits> pimpl_;

};

} // namespace lyx

#endif
