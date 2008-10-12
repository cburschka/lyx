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

#include "Font.h"
#include "Citation.h"
#include "paper.h"

#include "insets/InsetQuotes.h"

#include "support/copied_ptr.h"

#include <set>
#include <vector>

namespace lyx {

namespace support { class FileName; }

class AuthorList;
class BranchList;
class Bullet;
class DocumentClass;
class Encoding;
class Language;
class LatexFeatures;
class LayoutFile;
class LayoutFileIndex;
class Lexer;
class PDFOptions;
class Spacing;
class TexRow;
class VSpace;

/** Buffer parameters.
 *  This class contains all the parameters for this buffer's use. Some
 *  work needs to be done on this class to make it nice. Now everything
 *  is in public.
 */
class BufferParams {
public:
	///
	typedef std::vector<std::string> LayoutModuleList;
	///
	enum ParagraphSeparation {
		///
		ParagraphIndentSeparation,
		///
		ParagraphSkipSeparation
	};
	///
	BufferParams();

	/// get l10n translated to the buffers language
	docstring B_(std::string const & l10n) const;

	/// read a header token, if unrecognised, return it or an unknown class name
	std::string readToken(Lexer & lex,
		std::string const & token, ///< token to read.
		support::FileName const & filepath);

	///
	void writeFile(std::ostream &) const;

	/// check what features are implied by the buffer parameters.
	void validate(LaTeXFeatures &) const;

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
	ParagraphSeparation paragraph_separation;
	///
	InsetQuotes::QuoteLanguage quotes_language;
	///
	InsetQuotes::QuoteTimes quotes_times;
	///
	std::string fontsize;
	///Get the LayoutFile this document is using.
	LayoutFile const * baseClass() const;
	///
	LayoutFileIndex const & baseClassID() const;
	/// Set the LyX layout file this document is using.
	/// NOTE: This does not call makeDocumentClass() to update the local 
	/// DocumentClass. That needs to be done manually.
	/// \param filename the name of the layout file
	bool setBaseClass(std::string const & classname);
	/// Adds the module information to the baseClass information to
	/// create our local DocumentClass.
	void makeDocumentClass();
	/// Returns the DocumentClass currently in use: the BaseClass as modified
	/// by modules.
	DocumentClass const & documentClass() const;
	/// \return A pointer to the DocumentClass currently in use: the BaseClass 
	/// as modified by modules. 
	DocumentClass const * documentClassPtr() const;
	/// This bypasses the baseClass and sets the textClass directly.
	/// Should be called with care and would be better not being here,
	/// but it seems to be needed by CutAndPaste::putClipboard().
	void setDocumentClass(DocumentClass const * const);
	/// List of modules in use
	LayoutModuleList const & getModules() const { return layoutModules_; }
	/// List of default modules the user has removed
	std::set<std::string> const & getRemovedModules() const 
			{ return removedModules_; }
	///
	/// Add a module to the list of modules in use.
	/// Returns true if module was successfully added.
	/// The makeClass variable signals whether to call makeDocumentClass. This
	/// need not be done if we know this isn't the final time through, or if
	/// the BufferParams do not represent the parameters for an actual buffer
	/// (as in GuiDocument).
	bool addLayoutModule(std::string const & modName);
	///
	void addRemovedModule(std::string const & modName) 
			{ removedModules_.insert(modName); }
	/// Clear the list
	void clearLayoutModules() { layoutModules_.clear(); }
	/// Clear the removed module list
	void clearRemovedModules() { removedModules_.clear(); }

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
	///
	std::string columnsep;

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
	/// the font used by the CJK command
	std::string fontsCJK;
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
	std::string local_layout;
	///
	std::string options;
	/// use the class options defined in the layout?
	bool use_default_options;
	///
	std::string master;
	///
	std::string float_placement;
	///
	unsigned int columns;
	/// parameters for the listings package
	std::string listings_params;
	///
	PageSides sides;
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
	///
	bool compressed;

	/// the author list for the document
	AuthorList & authors();
	AuthorList const & authors() const;

	/// map of the file's author IDs to buffer author IDs
	std::vector<unsigned int> author_map;
	///
	std::string const dvips_options() const;
	/** The return value of paperSizeName() depends on the
	 *  purpose for which the paper size is needed, since they
	 *  support different subsets of paper sizes.
	*/
	enum PapersizePurpose {
		///
		DVIPS,
		///
		DVIPDFM,
		///
		XDVI
	};
	///
	std::string paperSizeName(PapersizePurpose purpose) const;
	/// set up if and how babel is called
	std::string babelCall(std::string const & lang_opts) const;
	/// handle inputenc etc.
	void writeEncodingPreamble(odocstream & os, LaTeXFeatures & features,
					      TexRow & texrow) const;
	/// set up the document fonts
	std::string const loadFonts(std::string const & rm,
				     std::string const & sf, std::string const & tt,
				     bool const & sc, bool const & osf,
				     int const & sfscale, int const & ttscale) const;

	/// get the appropriate cite engine (natbib handling)
	CiteEngine citeEngine() const;
	///
	void setCiteEngine(CiteEngine const);

	/// options for pdf output
	PDFOptions & pdfoptions();
	PDFOptions const & pdfoptions() const;

private:
	///
	void readPreamble(Lexer &);
	///
	void readLocalLayout(Lexer &);
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
	///
	void readRemovedModules(Lexer &);

	/// for use with natbib
	CiteEngine cite_engine_;
	///
	DocumentClass * doc_class_;
	/// 
	LayoutModuleList layoutModules_;
	/// this is for modules that are required by the document class but that
	/// the user has chosen not to use
	std::set<std::string> removedModules_;

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
