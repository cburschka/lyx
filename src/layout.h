// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-1999 The LyX Team.
 *
 * ====================================================== */

#ifndef LAYOUT_H
#define LAYOUT_H

#ifdef __GNUG__
#pragma interface
#endif

#include <vector>

#include "definitions.h"
#include "lyxlex.h"
#include "lyxfont.h"
#include "Spacing.h"

/// Reads the style files
extern void LyXSetStyle();

/// The different output types
enum OutputType {
        ///
        LATEX,
	///
	LINUXDOC,
	///
	DOCBOOK,
	///
	LITERATE
};

/// The different margin types
enum LYX_MARGIN_TYPE {
	///
	MARGIN_MANUAL,
	///
	MARGIN_FIRST_DYNAMIC,
	///
	MARGIN_DYNAMIC,
	///
	MARGIN_STATIC,
	///
	MARGIN_RIGHT_ADDRESS_BOX
};

///
enum LyXAlignment {
	///
	LYX_ALIGN_NONE = 0,
	///
	LYX_ALIGN_BLOCK = 1,
	///
	LYX_ALIGN_LEFT = 2,
	///
	LYX_ALIGN_RIGHT = 4,
	///
	LYX_ALIGN_CENTER = 8,
	///
	LYX_ALIGN_LAYOUT = 16,
	///
        LYX_ALIGN_SPECIAL = 32
};
inline void operator|=(LyXAlignment & la1, LyXAlignment la2) {
	la1 = static_cast<LyXAlignment>(la1 | la2);
}

/// The different LaTeX-Types
enum LYX_LATEX_TYPES {
	///
	LATEX_PARAGRAPH,
	///
	LATEX_COMMAND,
	///
	LATEX_ENVIRONMENT,
	///
	LATEX_ITEM_ENVIRONMENT,
	///
	LATEX_LIST_ENVIRONMENT
};

/// The different label types
enum LYX_LABEL_TYPES {
	///
	LABEL_NO_LABEL,
	///
	LABEL_MANUAL,
	///
        LABEL_BIBLIO,
	///
	LABEL_TOP_ENVIRONMENT,
	///
	LABEL_CENTERED_TOP_ENVIRONMENT,

	// the flushright labels following now must start with LABEL_STATIC
	///
	LABEL_STATIC,
	///
	LABEL_SENSITIVE,
	///
	LABEL_COUNTER_CHAPTER,
	///
	LABEL_COUNTER_SECTION,
	///
	LABEL_COUNTER_SUBSECTION,
	///
	LABEL_COUNTER_SUBSUBSECTION,
	///
	LABEL_COUNTER_PARAGRAPH,
	///
	LABEL_COUNTER_SUBPARAGRAPH,
	///
	LABEL_COUNTER_ENUMI,
	///
	LABEL_COUNTER_ENUMII,
	///
	LABEL_COUNTER_ENUMIII,
	///
	LABEL_COUNTER_ENUMIV,
	///
	LABEL_FIRST_COUNTER = LABEL_COUNTER_CHAPTER
};


/* Fix labels are printed flushright, manual labels flushleft. 
 * MARGIN_MANUAL and MARGIN_FIRST_DYNAMIC are *only* for LABEL_MANUAL,
 * MARGIN_DYNAMIC and MARGIN_STATIC are *not* for LABEL_MANUAL. 
 * This seems a funny restriction, but I think other combinations are
 * not needed, so I will not change it yet. 
 * Correction: MARGIN_FIRST_DYNAMIC also usable with LABEL_STATIC
 */


/* There is a parindent and a parskip. Which one is used depends on the 
 * paragraph_separation-flag of the text-object. 
 * BUT: parindent is only thrown away, if a parskip is defined! So if you
 * want a space between the paragraphs and a parindent at the same time, 
 * you should set parskip to zero and use topsep, parsep and bottomsep.
 * 
 * The standard layout is an exception: its parindent is only set, if the 
 * previous paragraph is standard too. Well, this is LateX and it is good!
 */ 


/// Attributes of a layout/paragraph environment
class LyXTextClass;

///
class LyXLayout {
public:
	///
	LyXLayout ();

	///
	bool Read (LyXLex &, LyXTextClass const &);

	string const & name() const { return name_; }
	void name(string const & n) { name_ = n; }
	string const & obsoleted_by() const { return obsoleted_by_; }
	string const & latexname() const { return latexname_; }
	string const & labelstring() const { return labelstring_; }
	string const & preamble() const { return preamble_; }
	string const & latexparam() const { return latexparam_; }
	string const & labelstring_appendix() const { return labelstring_appendix_; }
	/** Default font for this layout/environment.
	    The main font for this kind of environment. If an attribute has
	    LyXFont::INHERITED_*, it means that the value is specified by
	    the defaultfont for the entire layout. If we are nested, the
	    font is inherited from the font in the environment one level
	    up until the font is resolved. The values LyXFont::IGNORE_*
	    and LyXFont::TOGGLE are illegal here.
	*/
	LyXFont font;

	/** Default font for labels.
	    Interpretation the same as for font above
	*/
	LyXFont labelfont;

	/** Resolved version of the font for this layout/environment.
	    This is a resolved version the default font. The font is resolved
	    against the defaultfont of the entire layout.
	*/
	LyXFont resfont;

	/** Resolved version of the font used for labels.
	    This is a resolved version the label font. The font is resolved
	    against the defaultfont of the entire layout.
	*/
	LyXFont reslabelfont;

	/// Text that dictates how wide the left margin is on the screen
	string leftmargin;

	/// Text that dictates how wide the right margin is on the screen
	string rightmargin;

	/// Text that dictates how much space to leave after a potential label
	string labelsep;

	/// Text that dictates how much space to leave before a potential label
	string labelindent;

	/** Text that dictates the width of the indentation of
	    indented paragraphs.
	*/
	string parindent;

	///
	float parskip;

	///
	float itemsep;

	///
	float topsep;

	///
	float bottomsep;

	///
	float labelbottomsep;

	///
	float parsep;

	///
	Spacing spacing;

	///
	LyXAlignment align; // add approp. signedness

	///
	LyXAlignment alignpossible; // add approp. signedness

	///
	char labeltype; // add approp. signedness

	///
	char margintype; // add approp. signedness

	///
	bool fill_top;

	///
	bool fill_bottom;

	///
	bool newline_allowed;

	///
	bool nextnoindent;

	///
	bool free_spacing;
	/// true when the fragile commands in the paragraph need to be
	/// \protect'ed.
	bool needprotect;
	/// true when empty paragraphs should be kept.
	bool keepempty;
	///
	bool isParagraph() const {
		return latextype == LATEX_PARAGRAPH;
	}
	///
	bool isCommand() const { 
	    return latextype == LATEX_COMMAND;
	}
	///
	bool isEnvironment() const {
		return (latextype == LATEX_ENVIRONMENT
			|| latextype == LATEX_ITEM_ENVIRONMENT
			|| latextype == LATEX_LIST_ENVIRONMENT);
	}
	/// Type of LaTeX object
	LYX_LATEX_TYPES latextype;
	/// Does this object belong in the title part of the document?
	bool intitle;
private:
	/// Name of the layout/paragraph environment
	string name_;

	/** Name of an layout that has replaced this layout.
	    This is used to rename a layout, while keeping backward
	    compatibility 
	*/
	string obsoleted_by_;

	/// LaTeX name for environment
	string latexname_;

	/// Label string. "Abstract", "Reference", "Caption"...
	string labelstring_;

	/// Label string inside appendix. "Appendix", ...
	string labelstring_appendix_;

	/// LaTeX parameter for environment
        string latexparam_;

        /// Macro definitions needed for this layout
	string preamble_;
};


///
class LyXTextClass {
public:
	///
	typedef vector<LyXLayout> LayoutList;

	///
	LyXTextClass (string const & = string(), 
		      string const & = string(), 
		      string const & = string());

	///
	LayoutList::const_iterator begin() const { return layoutlist.begin(); }
	///
	LayoutList::const_iterator end() const { return layoutlist.end(); }
	
	///
	bool Read(string const & filename, bool merge = false);

	///
	bool hasLayout(string const & name) const;

	///
	LyXLayout const & GetLayout(string const & vname) const;

	///
	LyXLayout & GetLayout(string const & vname);

	/// Sees to that the textclass structure has been loaded
	void load();

	///
	string const & name() const { return name_; }
	///
	string const & latexname() const { return latexname_; }
	///
	string const & description() const { return description_; }
	///
	string const & opt_fontsize() const { return opt_fontsize_; }
	///
	string const & opt_pagestyle() const { return opt_pagestyle_; }
	///
	string const & options() const { return options_; }
	///
	string const & pagestyle() const { return pagestyle_; }
	///
	string const & preamble() const { return preamble_; }

	/// Packages that are already loaded by the class
	enum Provides {
		nothing = 0,
		amsmath = 1,
		makeidx = 2,
		url = 4
	};
	bool provides(Provides p) const { return provides_ & p; }
	
	///
	unsigned int columns() const { return columns_; }
	///
	enum PageSides {
		OneSide,
		TwoSides
	};
	///
	PageSides sides() const { return sides_; }
	///
	int secnumdepth() const { return secnumdepth_; }
	///
	int tocdepth() const { return tocdepth_; }

        ///
        OutputType outputType() const { return outputType_; }

	///
	LyXFont const & defaultfont() const { return defaultfont_; }

	/// Text that dictates how wide the left margin is on the screen
	string const & leftmargin() const { return leftmargin_; }

	/// Text that dictates how wide the right margin is on the screen
	string const & rightmargin() const { return rightmargin_; }
        ///
	int maxcounter() const { return maxcounter_; }
	///
	LayoutList::size_type numLayouts() const { return layoutlist.size(); }
	///
	LyXLayout const & operator[](LayoutList::size_type i) const {
		return layoutlist[i];
	}
private:
	///
	bool delete_layout(string const &);
	///
	bool do_readStyle(LyXLex &, LyXLayout &);
	///
	string name_;
	///
	string latexname_;
	///
	string description_;
	/// Specific class options
        string opt_fontsize_;
	///
        string opt_pagestyle_;
	///
	string options_;
	///
	string pagestyle_;
	///
	string preamble_;
	///
	Provides provides_;
	///
	unsigned int columns_;
	///
	PageSides sides_;
	///
	int secnumdepth_;
	///
	int tocdepth_;
        ///
        OutputType outputType_;
	/** Base font. The paragraph and layout fonts are resolved against
	    this font. This has to be fully instantiated. Attributes
	    LyXFont::INHERIT, LyXFont::IGNORE, and LyXFont::TOGGLE are
	    extremely illegal.
	*/
	LyXFont defaultfont_;
	/// Text that dictates how wide the left margin is on the screen
	string leftmargin_;

	/// Text that dictates how wide the right margin is on the screen
	string rightmargin_;
        ///
	int maxcounter_; // add approp. signedness

	///
	LayoutList layoutlist;

	/// Has this layout file been loaded yet?
	bool loaded;
};

///
inline void operator|=(LyXTextClass::Provides & p1, LyXTextClass::Provides p2)
{
	p1 = static_cast<LyXTextClass::Provides>(p1 | p2);
}


///
class LyXTextClassList {
public:
	///
	typedef vector<LyXTextClass> ClassList;
	/// Gets layout structure from layout number and textclass number
	LyXLayout const & Style(ClassList::size_type textclass,
				LyXTextClass::LayoutList::size_type layout) const;

	/// Gets layout number from textclass number and layout name
	pair<bool, LyXTextClass::LayoutList::size_type>
	NumberOfLayout(ClassList::size_type textclass,
		       string const & name) const;

	/// Gets a layout name from layout number and textclass number
	string const &
	NameOfLayout(ClassList::size_type textclass,
		     LyXTextClass::LayoutList::size_type layout) const;

	/** Gets textclass number from name.
	    Returns -1 if textclass name does not exist
	*/
	pair<bool, ClassList::size_type>
	NumberOfClass(string const & textclass) const;

	///
	string const & NameOfClass(ClassList::size_type number) const;

	///
	string const & LatexnameOfClass(ClassList::size_type number) const;

	///
	string const & DescOfClass(ClassList::size_type number) const;

	///
	LyXTextClass const & TextClass(ClassList::size_type textclass) const;

	/** Read textclass list.
	    Returns false if this fails
	*/
	bool Read();

	/** Load textclass.
	    Returns false if this fails
	*/
	bool Load(ClassList::size_type number) const;
private:
	///
	mutable ClassList classlist;
	///
	void Add (LyXTextClass const &);
};

/// Should not be declared here!! (Lgb) Why not? (Asger)
extern LyXTextClassList textclasslist;
 
#endif
