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

#include "Author.h"
#include "Citation.h"
#include "DocumentClassPtr.h"
#include "Format.h"
#include "LayoutModuleList.h"
#include "OutputParams.h"
#include "paper.h"

#include "insets/InsetQuotes.h"

#include "support/copied_ptr.h"

#include <map>
#include <vector>

namespace lyx {

namespace support { class FileName; }

class BranchList;
class Bullet;
class DocumentClass;
class Encoding;
class Font;
class HSpace;
class IndicesList;
class Language;
class LayoutFile;
class LayoutFileIndex;
class Lexer;
class PDFOptions;
class Spacing;
class VSpace;

/** Buffer parameters.
 *  This class contains all the parameters for this buffer's use. Some
 *  work needs to be done on this class to make it nice. Now everything
 *  is in public.
 */
class BufferParams {
public:
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
	void writeFile(std::ostream &, Buffer const *) const;

	/// check what features are implied by the buffer parameters.
	void validate(LaTeXFeatures &) const;

	/** \returns true if the babel package is used (interrogates
	 *  the BufferParams, a LyXRC variable, and the document class).
	 *  This returned value can then be passed to the insets...
	 */
	bool writeLaTeX(otexstream &, LaTeXFeatures &,
			support::FileName const &) const;

	///
	void useClassDefaults();
	///
	bool hasClassDefaults() const;

	///
	HSpace const & getIndentation() const;
	///
	void setIndentation(HSpace const & indent);
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
	std::string fontsize;
	/// Get the LayoutFile this document is using.
	LayoutFile const * baseClass() const;
	///
	LayoutFileIndex const & baseClassID() const;
	/// Set the LyX layout file this document is using.
	/// NOTE: This does not call makeDocumentClass() to update the local
	/// DocumentClass. That needs to be done manually.
	/// \param classname: the name of the layout file
	bool setBaseClass(std::string const & classname);
	/// Adds the module information to the baseClass information to
	/// create our local DocumentClass.
	/// NOTE: This should NEVER be called externally unless one immediately goes
	/// on to class BufferView::updateDocumentClass(). The exception, of course,
	/// is in GuiDocument, where we use a BufferParams simply to hold a copy of
	/// the parameters from the active Buffer.
	void makeDocumentClass(bool const clone = false);
	/// Returns the DocumentClass currently in use: the BaseClass as modified
	/// by modules.
	DocumentClass const & documentClass() const;
	/// \return A pointer to the DocumentClass currently in use: the BaseClass
	/// as modified by modules.
	DocumentClassConstPtr documentClassPtr() const;
	/// This bypasses the baseClass and sets the textClass directly.
	/// Should be called with care and would be better not being here,
	/// but it seems to be needed by CutAndPaste::putClipboard().
	void setDocumentClass(DocumentClassConstPtr);
	/// List of modules in use
	LayoutModuleList const & getModules() const { return layout_modules_; }
	/// List of default modules the user has removed
	std::list<std::string> const & getRemovedModules() const
			{ return removed_modules_; }
	///
	/// Add a module to the list of modules in use. This checks only that the
	/// module is not already in the list, so use layoutModuleCanBeAdeed first 
	/// if you want to check for compatibility.
	/// \return true if module was successfully added.
	bool addLayoutModule(std::string const & modName);
	/// checks to make sure module's requriements are satisfied, that it does
	/// not conflict with already-present modules, isn't already loaded, etc.
	bool layoutModuleCanBeAdded(std::string const & modName) const;
	/// same, but for citaton modules.
	bool citationModuleCanBeAdded(std::string const & modName) const;
	///
	void addRemovedModule(std::string const & modName)
			{ removed_modules_.push_back(modName); }
	/// Clear the list
	void clearLayoutModules() { layout_modules_.clear(); }
	/// Clear the removed module list
	void clearRemovedModules() { removed_modules_.clear(); }
	/// Get the local layouts
	std::string getLocalLayout(bool) const;
	/// Set the local layouts
	void setLocalLayout(std::string const &, bool);

	/// returns \c true if the buffer contains a LaTeX document
	bool isLatex() const;
	/// returns \c true if the buffer contains a DocBook document
	bool isDocBook() const;
	/// returns \c true if the buffer contains a Wed document
	bool isLiterate() const;

	/// return the format of the buffer on a string
	std::string bufferFormat() const;
	/// return the default output format of the current backend
	std::string getDefaultOutputFormat() const;
	/// return the output flavor of \p format or the default
	OutputParams::FLAVOR getOutputFlavor(
		  std::string const & format = std::string()) const;
	///
	bool isExportable(std::string const & format) const;
	///
	std::vector<Format const *> exportableFormats(bool only_viewable) const;
	///
	bool isExportableFormat(std::string const & format) const;
	/// the backends appropriate for use with this document.
	/// so, e.g., latex is excluded , if we're using non-TeX fonts
	std::vector<std::string> backends() const;

	/// List of included children (for includeonly)
	std::list<std::string> const & getIncludedChildren() const
			{ return included_children_; }
	///
	void addIncludedChildren(std::string const & child)
			{ included_children_.push_back(child); }
	/// Clear the list of included children
	void clearIncludedChildren() { included_children_.clear(); }

	/// update aux files of unincluded children (with \includeonly)
	bool maintain_unincluded_children;

	/// returns the main font for the buffer (document)
	Font const getFont() const;

	/// translate quote style string to enum value
	InsetQuotes::QuoteLanguage getQuoteStyle(std::string const & qs) const;

	/* these are for the PaperLayout */
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
	std::string graphics_driver;
	/// The default output format
	std::string default_output_format;
	/// customized bibliography processor
	std::string bibtex_command;
	/// customized index processor
	std::string index_command;
	/// font encoding(s) requested for this document
	std::string fontenc;
	/// the rm font: [0] for TeX fonts, [1] for non-TeX fonts
	std::string fonts_roman[2];
	/// the rm font
	std::string const & fontsRoman() const { return fonts_roman[useNonTeXFonts]; }
	/// the sf font: [0] for TeX fonts, [1] for non-TeX fonts
	std::string fonts_sans[2];
	/// the sf font
	std::string const & fontsSans() const { return fonts_sans[useNonTeXFonts]; }
	/// the tt font: [0] for TeX fonts, [1] for non-TeX fonts
	std::string fonts_typewriter[2];
	/// the tt font
	std::string const & fontsTypewriter() const { return fonts_typewriter[useNonTeXFonts]; }
	/// the math font: [0] for TeX fonts, [1] for non-TeX fonts
	std::string fonts_math[2];
	/// the math font
	std::string const & fontsMath() const { return fonts_math[useNonTeXFonts]; }
	/// the default family (rm, sf, tt)
	std::string fonts_default_family;
	/// use the fonts of the OS (OpenType, True Type) directly
	bool useNonTeXFonts;
	/// use expert Small Caps
	bool fonts_expert_sc;
	/// use Old Style Figures
	bool fonts_old_figures;
	/// the scale factor of the sf font: [0] for TeX fonts, [1] for non-TeX fonts
	int fonts_sans_scale[2];
	/// the scale factor of the sf font
	int fontsSansScale() const { return fonts_sans_scale[useNonTeXFonts]; }
	/// the scale factor of the tt font: [0] for TeX fonts, [1] for non-TeX fonts
	int fonts_typewriter_scale[2];
	/// the scale factor of the tt font
	int fontsTypewriterScale() const { return fonts_typewriter_scale[useNonTeXFonts]; }
	/// the font used by the CJK command
	std::string fonts_cjk;
	///
	Spacing & spacing();
	Spacing const & spacing() const;
	///
	int secnumdepth;
	///
	int tocdepth;
	///
	Language const * language;
	/// language package
	std::string lang_package;
	/// BranchList:
	BranchList & branchlist();
	BranchList const & branchlist() const;
	/// IndicesList:
	IndicesList & indiceslist();
	IndicesList const & indiceslist() const;
	/**
	 * The LyX name of the input encoding for LaTeX. This can be one of
	 * - \c auto: find out the input encoding from the used languages
	 * - \c default: ditto
	 * - any encoding defined in the file lib/encodings
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
	/// Output for XeTeX with 8-bit TeX fonts uses ASCII (set at runtime)
	/// instead of the value returned by this function.
	Encoding const & encoding() const;
	///
	std::string origin;
	///
	std::string preamble;
	///
	std::string options;
	/// use the class options defined in the layout?
	bool use_default_options;
	///
	std::string master;
	///
	bool suppress_date;
	///
	std::string float_placement;
	///
	unsigned int columns;
	///
	bool justification;
	/// parameters for the listings package
	std::string listings_params;
	///
	PageSides sides;
	///
	std::string pagestyle;
	///
	RGBColor backgroundcolor;
	///
	bool isbackgroundcolor;
	///
	RGBColor fontcolor;
	///
	bool isfontcolor;
	///
	RGBColor notefontcolor;
	///
	RGBColor boxbgcolor;
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
	/// Whether to load a package such as amsmath or esint.
	Package use_package(std::string const & p) const;
	/// Set whether to load a package such as amsmath or esint.
	void use_package(std::string const & p, Package u);
	/// All packages that can be switched on or off
	static std::map<std::string, std::string> const & auto_packages();
	/// Split bibliography?
	bool use_bibtopic;
	/// Split the index?
	bool use_indices;
	/// Save trensient properties?
	bool save_transient_properties;
	/// revision tracking for this buffer ? (this is a transient property)
	bool track_changes;
	/** This param decides whether change tracking marks should be used
	 *  in output (irrespective of how these marks are actually defined;
	 *  for instance, they may differ for DVI and PDF generation)
	 *  This is a transient property.
	 */
	bool output_changes;
	///
	bool compressed;

	/// the author list for the document
	AuthorList & authors();
	AuthorList const & authors() const;
	void addAuthor(Author a);

	/// map of the file's author IDs to AuthorList indexes
	typedef std::map<int, int> AuthorMap;
	AuthorMap author_map_;

	/// the buffer's active font encoding
	std::string const font_encoding() const;
	/// all font encodings requested by the prefs/document/main language.
	/// This does NOT include font encodings required by secondary languages
	std::vector<std::string> const font_encodings() const;

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
	std::string babelCall(std::string const & lang_opts, bool const langoptions) const;
	/// return supported drivers for specific packages
	docstring getGraphicsDriver(std::string const & package) const;
	/// handle inputenc etc.
	void writeEncodingPreamble(otexstream & os, LaTeXFeatures & features) const;
	///
	std::string const parseFontName(std::string const & name) const;
	/// set up the document fonts
	std::string const loadFonts(LaTeXFeatures & features) const;

	/// the cite engine modules
	LayoutModuleList const & citeEngine() const
		{ return cite_engine_; }
	/// the type of cite engine (authoryear or numerical)
	CiteEngineType const & citeEngineType() const
		{ return cite_engine_type_; }
	/// add the module to the cite engine modules
	bool addCiteEngine(std::string const &);
	/// add the modules to the cite engine modules
	bool addCiteEngine(std::vector<std::string> const &);
	/// clear the list of cite engine modules
	void clearCiteEngine() { cite_engine_.clear(); }
	/// set the cite engine module
	void setCiteEngine(std::string const &);
	/// set the cite engine modules
	void setCiteEngine(std::vector<std::string> const &);
	/// set the cite engine type
	void setCiteEngineType(CiteEngineType const & engine_type)
		{ cite_engine_type_ = engine_type; }

	/// the available citation commands
	std::vector<std::string> citeCommands() const;
	/// the available citation styles
	std::vector<CitationStyle> citeStyles() const;

	/// the default BibTeX style file for the document
	std::string biblio_style;
	/// the default BibTeX style file from the TextClass
	std::string const & defaultBiblioStyle() const;
	/// whether the BibTeX style supports full author lists
	bool const & fullAuthorList() const;

	/// options for pdf output
	PDFOptions & pdfoptions();
	PDFOptions const & pdfoptions() const;

	// do not change these values. we rely upon them.
	enum MathOutput {
		MathML = 0,
		HTML = 1,
		Images = 2,
		LaTeX = 3
	};
	/// what to use for math output. present choices are above
	MathOutput html_math_output;
	/// whether to attempt to be XHTML 1.1 compliant or instead be
	/// a little more mellow
	bool html_be_strict;
	///
	double html_math_img_scale;
	///
	double display_pixel_ratio;
	///
	std::string html_latex_start;
	///
	std::string html_latex_end;
	///
	bool html_css_as_file;
	/// generate output usable for reverse/forward search
	bool output_sync;
	/// custom LaTeX macro from user instead our own
	std::string output_sync_macro;
	/// use refstyle? or prettyref?
	bool use_refstyle;

	/// Return true if language could be set to lang,
	/// otherwise return false and do not change language
	bool setLanguage(std::string const & lang);

private:
	///
	void readPreamble(Lexer &);
	///
	void readLocalLayout(Lexer &, bool);
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
	///
	void readIncludeonly(Lexer &);
	/// A cache for the default flavors
	typedef std::map<std::string, OutputParams::FLAVOR> DefaultFlavorCache;
	///
	mutable DefaultFlavorCache default_flavors_;
	/// the cite engine modules
	LayoutModuleList cite_engine_;
	/// the type of cite engine (authoryear or numerical)
	CiteEngineType cite_engine_type_;
	///
	DocumentClassPtr doc_class_;
	///
	LayoutModuleList layout_modules_;
	/// this is for modules that are required by the document class but that
	/// the user has chosen not to use
	std::list<std::string> removed_modules_;
	/// The local layouts without the forced ones
	std::string local_layout_;
	/// Forced local layouts only for reading (use getLocalLayout() instead)
	std::string forced_local_layout_;

	/// the list of included children (for includeonly)
	std::list<std::string> included_children_;

	typedef std::map<std::string, Package> PackageMap;
	/** Whether and how to load packages like amsmath, esint, mhchem,
	 *  mathdots, stackrel, stmaryrd and undertilde.
	 */
	PackageMap use_packages;

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
