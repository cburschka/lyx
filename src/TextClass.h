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

#include "ColorCode.h"
#include "Counters.h"
#include "FloatList.h"
#include "FontInfo.h"
#include "Layout.h"
#include "LayoutEnums.h"

#include "insets/InsetLayout.h"

#include "support/docstring.h"
#include "support/types.h"

#include <boost/noncopyable.hpp>

#include <map>
#include <set>
#include <vector>
#include <list>

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
	ProtectCopy(const ProtectCopy &) {};
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
	virtual ~TextClass() {};
	///////////////////////////////////////////////////////////////////
	// typedefs
	///////////////////////////////////////////////////////////////////
	/// The individual paragraph layouts comprising the document class
	// NOTE Do NOT try to make this a container of Layout pointers, e.g.,
	// std::vector<Layout *>. This will lead to problems. The reason is
	// that DocumentClass objects are generally created by copying a 
	// LayoutFile, which serves as a base for the DocumentClass. If the
	// LayoutList is a container of pointers, then every DocumentClass
	// that derives from a given LayoutFile (e.g., article) will SHARE
	// a basic set of layouts. So if one Buffer were to modify a layout
	// (say, Standard), that would modify that layout for EVERY Buffer
	// that was based upon the same DocumentClass. (Of course, if you 
	// really, REALLY want to make LayoutList a vector<Layout *>, then
	// you can implement custom assignment and copy constructors.)
	//
	// NOTE: Layout pointers are directly assigned to paragraphs so a
	// container that does not invalidate these pointers after insertion
	// is needed.
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
			{ return operator[](emptylayout_); };
	/// the name of the plain layout
	docstring const & plainLayoutName() const 
			{ return emptylayout_; }
	/// Enumerate the paragraph styles.
	size_t layoutCount() const { return layoutlist_.size(); }
	///
	bool hasLayout(docstring const & name) const;
	///
	Layout const & operator[](docstring const & vname) const;
	/// Inset layouts of this doc class
	InsetLayouts const & insetLayouts() const { return insetlayoutlist_; };

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
		ERROR,
		FORMAT_MISMATCH
	};

	/// Performs the read of the layout file.
	/// \return true on success.
	bool read(support::FileName const & filename, ReadType rt = BASECLASS);
	///
	bool read(std::string const & str, ReadType rt = BASECLASS);
	///
	ReturnValues read(Lexer & lex, ReadType rt = BASECLASS);
	/// validates the layout information passed in str
	static bool validate(std::string const & str);

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
	std::string const & name() const { return name_; };
	///
	std::string const & description() const {	return description_; };
	///
	std::string const & latexname() const { return latexname_; }
protected:
	/// Protect construction
	TextClass();
	///
	Layout & operator[](docstring const & vname);
	/// Create an new, empty layout for this textclass.
	/** \param unknown Set to true if this layout is an empty layout used to
	 * represent an unknown layout
	 */
	Layout createEmptyLayout(docstring const & name, bool unknown = false) const;
	
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
	bool texClassAvail_;
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
	static const docstring emptylayout_;
	/// preamble text to support layout styles
	docstring preamble_;
	/// latex packages loaded by document class.
	std::set<std::string> provides_;
	/// latex packages requested by document class.
	std::set<std::string> requires_;
	/// modules wanted by document class
	std::set<std::string> usemod_;
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
private:
	///////////////////////////////////////////////////////////////////
	// helper routines for reading layout files
	///////////////////////////////////////////////////////////////////
	///
	bool deleteLayout(docstring const &);
	///
	bool convertLayoutFormat(support::FileName const &, ReadType);
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
	void readFloat(Lexer &);
	///
	void readCounter(Lexer &);
};


/// A DocumentClass represents the layout information associated with a
/// Buffer. It is based upon a LayoutFile, but may be modified by loading
/// various Modules. 
/// 
/// In that regard, DocumentClass objects are "dynamic". But this is really
/// an illusion, since DocumentClass objects are not (currently) changed
/// when, say, a new Module is loaded. Rather, the old DocumentClass is
/// discarded---actually, it's kept around in case something on the cut
/// stack needs it---and a new one is created from scratch. 
/// 
/// In the main LyX code, DocumentClass objects are created only by
/// DocumentClassBundle, for which see below.
/// 
class DocumentClass : public TextClass, boost::noncopyable {
public:
	///
	virtual ~DocumentClass() {}

	///////////////////////////////////////////////////////////////////
	// Layout Info
	///////////////////////////////////////////////////////////////////
	/// \return true if there is a Layout with latexname lay
	bool hasLaTeXLayout(std::string const & lay) const;
	/// A DocumentClass nevers count as loaded, since it is dynamic
	virtual bool loaded() { return false; }
	/// \return the layout object of an inset given by name. If the name
	/// is not found as such, the part after the ':' is stripped off, and
	/// searched again. In this way, an error fallback can be provided:
	/// An erroneous 'CharStyle:badname' (e.g., after a documentclass switch)
	/// will invoke the layout object defined by name = 'CharStyle'.
	/// If that doesn't work either, an empty object returns (shouldn't
	/// happen).  -- Idea JMarc, comment MV
	InsetLayout const & insetLayout(docstring const & name) const;
	/// a plain inset layout for use as a default
	static InsetLayout const & plainInsetLayout() { return empty_insetlayout_; }
	/// add a new, empty layout \c name if it does not exist in layoutlist_
	void addLayoutIfNeeded(docstring const & name) const;

	///////////////////////////////////////////////////////////////////
	// accessors
	///////////////////////////////////////////////////////////////////
	/// the list of floats defined in the document class
	FloatList const & floats() const { return floatlist_; }
	///
	Counters & counters() const { return counters_; }
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
	/// is this feature already provided by the class?
	bool provides(std::string const & p) const;
	/// features required by the class?
	std::set<std::string> const & requires() const { return requires_; }
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
	TitleLatexType titletype() const { return titletype_; };
	/// The name of the title command
	std::string const & titlename() const { return titlename_; };
	///
	int size() const { return layoutlist_.size(); }
	/// The minimal TocLevel of sectioning layouts
	int min_toclevel() const { return min_toclevel_; }
	/// The maximal TocLevel of sectioning layouts
	int max_toclevel() const { return max_toclevel_; }
	/// returns true if the class has a ToC structure
	bool hasTocLevels() const;
	/// Can be LaTeX, DocBook, etc.
	OutputType outputType() const { return outputType_; }
protected:
	/// Constructs a DocumentClass based upon a LayoutFile.
	DocumentClass(LayoutFile const & tc);
	/// Needed in tex2lyx
	DocumentClass() {}
private:
	/// The only class that can create a DocumentClass is
	/// DocumentClassBundle, which calls the protected constructor.
	friend class DocumentClassBundle;
	///
	static InsetLayout empty_insetlayout_;
};


/// DocumentClassBundle is a container for DocumentClass objects, so that 
/// they stay in memory for use by Insets, CutAndPaste, and the like, even
/// when their associated Buffers are destroyed.
/// FIXME Some sort of garbage collection or reference counting wouldn't
/// be a bad idea here. It might be enough to check when a Buffer is closed
/// (or makeDocumentClass is called) whether the old DocumentClass is in use 
/// anywhere.
///
/// This is a singleton class. Its sole instance is accessed via 
/// DocumentClassBundle::get().
class DocumentClassBundle : boost::noncopyable {
public:
	/// \return Pointer to a new class equal to baseClass
	DocumentClass & newClass(LayoutFile const & baseClass);
	/// \return The sole instance of this class.
	static DocumentClassBundle & get();
private:
	/// control instantiation
	DocumentClassBundle() {}
	/// clean up
	~DocumentClassBundle();
	///
	std::vector<DocumentClass *> documentClasses_;
};


/// convert page sides option to text 1 or 2
std::ostream & operator<<(std::ostream & os, PageSides p);


} // namespace lyx

#endif
