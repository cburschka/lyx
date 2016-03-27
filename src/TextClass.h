// -*- C++ -*-
/**
 * \file TextClass.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef TEXTCLASS_H
#define TEXTCLASS_H

#include "Citation.h"
#include "Counters.h"
#include "DocumentClassPtr.h"
#include "FloatList.h"
#include "FontInfo.h"
#include "Layout.h"
#include "LayoutEnums.h"
#include "LayoutModuleList.h"

#include "insets/InsetLayout.h"

#include "support/docstring.h"
#include "support/types.h"

#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#ifdef ERROR
#undef ERROR
#endif

namespace lyx {

namespace support { class FileName; }

class Counters;
class FloatList;
class Layout;
class LayoutFile;
class Lexer;

/// Based upon ideas in boost::noncopyable, inheriting from this
/// class effectively makes the copy constructor protected but the
/// assignment constructor private.
class ProtectCopy
{
protected:
	ProtectCopy() {}
	~ProtectCopy() {}
	ProtectCopy(const ProtectCopy &) {}
private:
	const ProtectCopy & operator=(const ProtectCopy &);
};


/// A TextClass represents a collection of layout information: At the
/// moment, this includes Layout's and InsetLayout's.
///
/// There are two major subclasses of TextClass: LayoutFile and
/// DocumentClass. These subclasses are what are actually used in LyX.
/// Simple TextClass objects are not directly constructed in the main
/// LyX code---the constructor is protected. (That said, in tex2lyx
/// there are what amount to simple TextClass objects.)
///
/// A LayoutFile (see LayoutFile.{h,cpp}) represents a *.layout file.
/// These are generally static objects---though they can be reloaded
/// from disk via LFUN_LAYOUT_RELOAD, so one should not assume that
/// they will never change.
///
/// A DocumentClass (see below) represents the layout information that
/// is associated with a given Buffer. These are static, in the sense
/// that they will not themselves change, but which DocumentClass is
/// associated with a Buffer can change, as modules are loaded and
/// unloaded, for example.
///
class TextClass : protected ProtectCopy {
public:
	///
	virtual ~TextClass() {}
	///////////////////////////////////////////////////////////////////
	// typedefs
	///////////////////////////////////////////////////////////////////
	// NOTE Do NOT try to make this a container of Layout pointers, e.g.,
	// std::list<Layout *>. This will lead to problems. The reason is
	// that DocumentClass objects are generally created by copying a
	// LayoutFile, which serves as a base for the DocumentClass. If the
	// LayoutList is a container of pointers, then every DocumentClass
	// that derives from a given LayoutFile (e.g., article) will SHARE
	// a basic set of layouts. So if one Buffer were to modify a layout
	// (say, Standard), that would modify that layout for EVERY Buffer
	// that was based upon the same DocumentClass.
	//
	// NOTE: Layout pointers are directly assigned to paragraphs so a
	// container that does not invalidate these pointers after insertion
	// is needed.
	/// The individual paragraph layouts comprising the document class
	typedef std::list<Layout> LayoutList;
	/// The inset layouts available to this class
	typedef std::map<docstring, InsetLayout> InsetLayouts;
	///
	typedef LayoutList::const_iterator const_iterator;

	///////////////////////////////////////////////////////////////////
	// Iterators
	///////////////////////////////////////////////////////////////////
	///
	const_iterator begin() const { return layoutlist_.begin(); }
	///
	const_iterator end() const { return layoutlist_.end(); }


	///////////////////////////////////////////////////////////////////
	// Layout Info
	///////////////////////////////////////////////////////////////////
	///
	Layout const & defaultLayout() const;
	///
	docstring const & defaultLayoutName() const;
	///
	bool isDefaultLayout(Layout const &) const;
	///
	bool isPlainLayout(Layout const &) const;
	/// returns a special layout for use when we don't really want one,
	/// e.g., in table cells
	Layout const & plainLayout() const
			{ return operator[](plain_layout_); }
	/// the name of the plain layout
	docstring const & plainLayoutName() const
			{ return plain_layout_; }
	/// Enumerate the paragraph styles.
	size_t layoutCount() const { return layoutlist_.size(); }
	///
	bool hasLayout(docstring const & name) const;
	///
	bool hasInsetLayout(docstring const & name) const;
	///
	Layout const & operator[](docstring const & vname) const;
	/// Inset layouts of this doc class
	InsetLayouts const & insetLayouts() const { return insetlayoutlist_; }

	///////////////////////////////////////////////////////////////////
	// reading routines
	///////////////////////////////////////////////////////////////////
	/// Enum used with TextClass::read
	enum ReadType {
		BASECLASS, //>This is a base class, i.e., top-level layout file
		MERGE, //>This is a file included in a layout file
		MODULE, //>This is a layout module
		VALIDATION //>We're just validating
	};
	/// return values for read()
	enum ReturnValues {
		OK,
		OK_OLDFORMAT,
		ERROR,
		FORMAT_MISMATCH
	};

	/// Performs the read of the layout file.
	/// \return true on success.
	// FIXME Should return ReturnValues....
	bool read(support::FileName const & filename, ReadType rt = BASECLASS);
	///
	ReturnValues read(std::string const & str, ReadType rt = MODULE);
	///
	ReturnValues read(Lexer & lex, ReadType rt = BASECLASS);
	/// validates the layout information passed in str
	static ReturnValues validate(std::string const & str);
	///
	static std::string convert(std::string const & str);

	///////////////////////////////////////////////////////////////////
	// loading
	///////////////////////////////////////////////////////////////////
	/// Sees to it the textclass structure has been loaded
	/// This function will search for $classname.layout in default directories
	/// and an optional path, but if path points to a file, it will be loaded
	/// directly.
	bool load(std::string const & path = std::string()) const;
	/// Has this layout file been loaded yet?
	/// Overridden by DocumentClass
	virtual bool loaded() const { return loaded_; }

	///////////////////////////////////////////////////////////////////
	// accessors
	///////////////////////////////////////////////////////////////////
	///
	std::string const & name() const { return name_; }
	///
	std::string const & category() const { return category_; }
	///
	std::string const & description() const { return description_; }
	///
	std::string const & latexname() const { return latexname_; }
	///
	std::string const & prerequisites(std::string const & sep = "\n\t") const;
	/// Can be LaTeX, DocBook, etc.
	OutputType outputType() const { return outputType_; }
	/// Can be latex, docbook ... (the name of a format)
	std::string outputFormat() const { return outputFormat_; }
	///
	docstring outlinerName(std::string const & type) const;
protected:
	/// Protect construction
	TextClass();
	///
	Layout & operator[](docstring const & name);
	/** Create an new, very basic layout for this textclass. This is used for
	    the Plain Layout common to all TextClass objects and also, in
	    DocumentClass, for the creation of new layouts `on the fly' when
	    previously unknown layouts are encountered.
	    \param unknown Set to true if this layout is used to represent an
	    unknown layout
	 */
	Layout createBasicLayout(docstring const & name, bool unknown = false) const;

	///////////////////////////////////////////////////////////////////
	// non-const iterators
	///////////////////////////////////////////////////////////////////
	///
	typedef LayoutList::iterator iterator;
	///
	iterator begin() { return layoutlist_.begin(); }
	///
	iterator end() { return layoutlist_.end(); }

	///////////////////////////////////////////////////////////////////
	// members
	///////////////////////////////////////////////////////////////////
	/// Paragraph styles used in this layout
	/// This variable is mutable because unknown layouts can be added
	/// to const textclass.
	mutable LayoutList layoutlist_;
	/// Layout file name
	std::string name_;
	/// Class category
	std::string category_;
	/// document class name
	std::string latexname_;
	/// document class description
	std::string description_;
	/// available types of float, eg. figure, algorithm.
	mutable FloatList floatlist_;
	/// Types of counters, eg. sections, eqns, figures, avail. in document class.
	mutable Counters counters_;
	/// Has this layout file been loaded yet?
	mutable bool loaded_;
	/// Is the TeX class available?
	bool tex_class_avail_;
	/// document class prerequisites
	mutable std::string prerequisites_;
	/// The possible cite engine types
	std::string opt_enginetype_;
	///
	std::string opt_fontsize_;
	///
	std::string opt_pagestyle_;
	/// Specific class options
	std::string options_;
	///
	std::string pagestyle_;
	///
	std::string class_header_;
	///
	docstring defaultlayout_;
	/// name of plain layout
	static const docstring plain_layout_;
	/// preamble text to support layout styles
	docstring preamble_;
	/// same, but for HTML output
	/// this is output as is to the header
	docstring htmlpreamble_;
	/// same, but specifically for CSS information
	docstring htmlstyles_;
	/// the paragraph style to use for TOCs, Bibliography, etc
	mutable docstring html_toc_section_;
	/// latex packages loaded by document class.
	std::set<std::string> provides_;
	/// latex packages requested by document class.
	std::set<std::string> requires_;
	///
	std::map<std::string, std::string> package_options_;
	/// default modules wanted by document class
	LayoutModuleList default_modules_;
	/// modules provided by document class
	LayoutModuleList provided_modules_;
	/// modules excluded by document class
	LayoutModuleList excluded_modules_;
	///
	unsigned int columns_;
	///
	PageSides sides_;
	/// header depth to have numbering
	int secnumdepth_;
	/// header depth to appear in table of contents
	int tocdepth_;
	/// Can be LaTeX, DocBook, etc.
	OutputType outputType_;
	/// Can be latex, docbook ... (the name of a format)
	std::string outputFormat_;
	/** Base font. The paragraph and layout fonts are resolved against
	    this font. This has to be fully instantiated. Attributes
	    FONT_INHERIT, FONT_IGNORE, and FONT_TOGGLE are
	    extremely illegal.
	*/
	FontInfo defaultfont_;
	/// Text that dictates how wide the left margin is on the screen
	docstring leftmargin_;
	/// Text that dictates how wide the right margin is on the screen
	docstring rightmargin_;
	/// The type of command used to produce a title
	TitleLatexType titletype_;
	/// The name of the title command
	std::string titlename_;
	/// Input layouts available to this layout
	InsetLayouts insetlayoutlist_;
	/// The minimal TocLevel of sectioning layouts
	int min_toclevel_;
	/// The maximal TocLevel of sectioning layouts
	int max_toclevel_;
	/// Citation formatting information
	std::map<CiteEngineType, std::map<std::string, std::string> > cite_formats_;
	/// Citation macros
	std::map<CiteEngineType, std::map<std::string, std::string> > cite_macros_;
	/// The default BibTeX bibliography style file
	std::string cite_default_biblio_style_;
	/// Whether full author lists are supported
	bool cite_full_author_list_;
	/// The possible citation styles
	std::map<CiteEngineType, std::vector<CitationStyle> > cite_styles_;
	///
	std::map<std::string, docstring> outliner_names_;
private:
	///////////////////////////////////////////////////////////////////
	// helper routines for reading layout files
	///////////////////////////////////////////////////////////////////
	///
	bool deleteLayout(docstring const &);
	///
	bool deleteInsetLayout(docstring const &);
	///
	bool convertLayoutFormat(support::FileName const &, ReadType);
	/// Reads the layout file without running layout2layout.
	ReturnValues readWithoutConv(support::FileName const & filename, ReadType rt);
	/// \return true for success.
	bool readStyle(Lexer &, Layout &) const;
	///
	void readOutputType(Lexer &);
	///
	void readTitleType(Lexer &);
	///
	void readMaxCounter(Lexer &);
	///
	void readClassOptions(Lexer &);
	///
	void readCharStyle(Lexer &, std::string const &);
	///
	bool readFloat(Lexer &);
	///
	bool readCiteEngine(Lexer &);
	///
	int readCiteEngineType(Lexer &) const;
	///
	bool readCiteFormat(Lexer &);
	///
	bool readOutlinerName(Lexer &);
};


/// A DocumentClass represents the layout information associated with a
/// Buffer. It is based upon a LayoutFile, but may be modified by loading
/// various Modules.
///
/// In that regard, DocumentClass objects are "dynamic". But this is really
/// an illusion, since DocumentClass objects are not (currently) changed
/// when, say, a new Module is loaded. Rather, the old DocumentClass is
/// discarded---actually, it will be kept around if something on the cut
/// stack needs it---and a new one is created from scratch.
class DocumentClass : public TextClass {
public:
	///
	virtual ~DocumentClass() {}

	///////////////////////////////////////////////////////////////////
	// Layout Info
	///////////////////////////////////////////////////////////////////
	/// \return true if there is a Layout with latexname lay
	bool hasLaTeXLayout(std::string const & lay) const;
	/// A DocumentClass nevers count as loaded, since it is dynamic
	virtual bool loaded() const { return false; }
	/// \return the layout object of an inset given by name. If the name
	/// is not found as such, the part after the ':' is stripped off, and
	/// searched again. In this way, an error fallback can be provided:
	/// An erroneous 'CharStyle:badname' (e.g., after a documentclass switch)
	/// will invoke the layout object defined by name = 'CharStyle'.
	/// If that doesn't work either, an empty object returns (shouldn't
	/// happen).  -- Idea JMarc, comment MV
	InsetLayout const & insetLayout(docstring const & name) const;
	/// a plain inset layout for use as a default
	static InsetLayout const & plainInsetLayout();
	/// add a new layout \c name if it does not exist in layoutlist_
	/// \return whether we had to add one.
	bool addLayoutIfNeeded(docstring const & name) const;
	/// Forced layouts in layout file syntax
	std::string forcedLayouts() const;

	///////////////////////////////////////////////////////////////////
	// accessors
	///////////////////////////////////////////////////////////////////
	/// the list of floats defined in the document class
	FloatList const & floats() const { return floatlist_; }
	///
	Counters & counters() const { return counters_; }
	///
	std::string const & opt_enginetype() const { return opt_enginetype_; }
	///
	std::string const & opt_fontsize() const { return opt_fontsize_; }
	///
	std::string const & opt_pagestyle() const { return opt_pagestyle_; }
	///
	std::string const & options() const { return options_; }
	///
	std::string const & class_header() const { return class_header_; }
	///
	std::string const & pagestyle() const { return pagestyle_; }
	///
	docstring const & preamble() const { return preamble_; }
	///
	docstring const & htmlpreamble() const { return htmlpreamble_; }
	///
	docstring const & htmlstyles() const { return htmlstyles_; }
	/// Looks for the layout of "highest level", other than Part (or other
	/// layouts with a negative toc number), for use in constructing TOCs and 
	/// similar information.
	Layout const & getTOCLayout() const;
	/// the paragraph style to use for TOCs, Bibliography, etc
	/// we will attempt to calculate this if it was not given
	Layout const & htmlTOCLayout() const;
	/// is this feature already provided by the class?
	bool provides(std::string const & p) const;
	/// features required by the class?
	std::set<std::string> const & requires() const { return requires_; }
	/// package options to write to LaTeX file
	std::map<std::string, std::string> const & packageOptions() const
		{ return package_options_; }
	///
	unsigned int columns() const { return columns_; }
	///
	PageSides sides() const { return sides_; }
	///
	int secnumdepth() const { return secnumdepth_; }
	///
	int tocdepth() const { return tocdepth_; }
	///
	FontInfo const & defaultfont() const { return defaultfont_; }
	/// Text that dictates how wide the left margin is on the screen
	docstring const & leftmargin() const { return leftmargin_; }
	/// Text that dictates how wide the right margin is on the screen
	docstring const & rightmargin() const { return rightmargin_; }
	/// The type of command used to produce a title
	TitleLatexType titletype() const { return titletype_; }
	/// The name of the title command
	std::string const & titlename() const { return titlename_; }
	///
	int size() const { return layoutlist_.size(); }
	/// The minimal TocLevel of sectioning layouts
	int min_toclevel() const { return min_toclevel_; }
	/// The maximal TocLevel of sectioning layouts
	int max_toclevel() const { return max_toclevel_; }
	/// returns true if the class has a ToC structure
	bool hasTocLevels() const;
	///
	std::string const & getCiteFormat(CiteEngineType const & type,
		std::string const & entry, std::string const & fallback = "") const;
	///
	std::string const & getCiteMacro(CiteEngineType const & type,
		std::string const & macro) const;
	///
	std::vector<std::string> const citeCommands(CiteEngineType const &) const;
	///
	std::vector<CitationStyle> const & citeStyles(CiteEngineType const &) const;
	///
	std::string const & defaultBiblioStyle() const { return cite_default_biblio_style_; }
	///
	bool const & fullAuthorList() const { return cite_full_author_list_; }
protected:
	/// Constructs a DocumentClass based upon a LayoutFile.
	DocumentClass(LayoutFile const & tc);
	/// Needed in tex2lyx
	DocumentClass() {}
private:
	/// The only way to make a DocumentClass is to call this function.
	friend DocumentClassPtr
		getDocumentClass(LayoutFile const &, LayoutModuleList const &,
				 bool const clone);
};


/// The only way to make a DocumentClass is to call this function.
/// The shared_ptr is needed because DocumentClass objects can be kept
/// in memory long after their associated Buffer is destroyed, mostly
/// on the CutStack.
DocumentClassPtr getDocumentClass(LayoutFile const & baseClass,
			LayoutModuleList const & modlist,
			bool const clone = false);

/// convert page sides option to text 1 or 2
std::ostream & operator<<(std::ostream & os, PageSides p);

/// current format of layout files
extern int const LAYOUT_FORMAT;


} // namespace lyx

#endif
