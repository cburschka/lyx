// -*- C++ -*-
/* This file is part of
 * ======================================================
* 
*           LyX, The Document Processor
* 	 
*	    Copyright (C) 1995 Matthias Ettrich
*
*======================================================*/

#ifndef _LAYOUT_H
#define _LAYOUT_H

#ifdef __GNUG__
#pragma interface
#endif

#include "definitions.h"
#include "lyxlex.h"
#include "lyxfont.h"
#include "Spacing.h"

/// Reads the style files
void LyXSetStyle();

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
enum LYX_ALIGNMENT {
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
* Correction: MARGIN_FIRST_DYNAMIC also usable with LABEL_STATIC */


/* There is a parindent and a parskip. Which one is used depends on the 
* paragraph_separation-flag of the text-object. 
* BUT: parindent is only thrown away, if a parskip is defined! So if you
* want a space between the paragraphs and a parindent at the same time, 
* you should set parskip to zero and use topsep, parsep and bottomsep.
* 
* The standard layout is an exception: its parindent is only set, if the 
* previous paragraph is standard too. Well, this is LateX and it is good! */ 


/// Attributes of a layout/paragraph environment
class LyXLayoutList;
///
class LyXLayout {
public:
	///
	LyXLayout ();

	///
	~LyXLayout ();

	///
	void Copy (LyXLayout const &l);

	///
	bool Read (LyXLex&, LyXLayoutList *);

	/// Name of the layout/paragraph environment
	LString name;

	/// Name of an layout that has replaced this layout. 
	/** This is used to rename a layout, while keeping backward
	    compatibility 
	*/
	LString obsoleted_by;

	/// Default font for this layout/environment
	/** The main font for this kind of environment. If an attribute has
	LyXFont::INHERITED_*, it means that the value is specified by
	the defaultfont for the entire layout. If we are nested, the font 
	is inherited from the font in the environment one level up until the 
	font is resolved. The values LyXFont::IGNORE_* and LyXFont::TOGGLE 
	are illegal here.
	*/
	LyXFont font;

	/// Default font for labels
	/** Interpretation the same as for font above */
	LyXFont labelfont;

	/// Resolved version of the font for this layout/environment
	/** This is a resolved version the default font. The font is resolved
	against the defaultfont of the entire layout. */
	LyXFont resfont;

	/// Resolved version of the font used for labels
	/** This is a resolved version the label font. The font is resolved
	against the defaultfont of the entire layout. */
	LyXFont reslabelfont;

	/// Text that dictates how wide the left margin is on the screen
	LString leftmargin;

	/// Text that dictates how wide the right margin is on the screen
	LString rightmargin;

	/// Text that dictates how much space to leave after a potential label
	LString labelsep;

	/// Text that dictates how much space to leave before a potential label
	LString labelindent;

	/// Text that dictates the width of the indentation of indented paragraphs
	LString parindent;

	/// Label string. "Abstract", "Reference", "Caption"...
	LString labelstring;

	/// Label string inside appendix. "Appendix", ...
	LString labelstring_appendix;

	/// LaTeX name for environment
	LString latexname;

	/// LaTeX parameter for environment
        LString latexparam;   //arrae970411

        /// Macro definitions needed for this layout
	LString preamble;

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
	char align; // add approp. signedness

	///
	char alignpossible; // add approp. signedness

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
};


///
class LyXLayoutList {
public:
	///
	LyXLayoutList();
	///
	~LyXLayoutList();
	///
	void Add (LyXLayout *l);
	///
	bool Delete (LString const &name);
        ///
	LyXLayout *GetLayout (LString const &name);
	///
	LyXLayout *ToAr();
	///
	int GetNum();
	///
	void Clean(); 
private:
	///
	struct LyXLayoutL {
		///
		LyXLayout* layout;
		///
		LyXLayoutL *next;
	};
	///
	LyXLayoutL * l;
	///
	LyXLayoutL *eol;
	///
	int num_layouts;
};


///
class LyXTextClass  {
public:
	///
	LyXTextClass (LString const & = LString(), 
		      LString const & = LString(), 
		      LString const & = LString());
	///
	~LyXTextClass();

	/// Not a real copy, just reference!
	void Copy (LyXTextClass const &l);

	/// Reads a textclass structure from file
	int Read (LString const & filename, LyXLayoutList *list = NULL);

	/// Sees to that the textclass structure has been loaded
	void load();

	///
	LString name;
	///
	LString latexname;
	///
	LString description;
        ///
        OutputType output_type;

	/// Specific class options
        LString opt_fontsize;
	///
        LString opt_pagestyle;
	///
	LString options;

	/// Packages that are already loaded by the class
	bool provides_amsmath;
	///
	bool provides_makeidx;
	/// 
	bool provides_url;
    
	/// Base font. This one has to be fully instantiated.
	/** Base font. The paragraph and layout fonts are resolved against
	this font. Attributes LyXFont::INHERIT, LyXFont::IGNORE, and 
	LyXFont::TOGGLE are extremely illegal.
	*/
	LyXFont defaultfont;
	/// Array of styles in this textclass
	LyXLayout* style;
	///
	unsigned char number_of_defined_layouts;
	///
	unsigned char columns;
	///
	unsigned char sides;
	///
	signed char secnumdepth;
	///
	signed char tocdepth;
	///
	LString pagestyle;
	///
	LString preamble;
	///
	/// Text that dictates how wide the left margin is on the screen
	LString leftmargin;
        ///
	/// Text that dictates how wide the right margin is on the screen
	LString rightmargin;
        ///
	char maxcounter; // add approp. signedness
private:
	/// Has this layout file been loaded yet?
	bool loaded;
};


///
class LyXTextClassList {
public:
	///
	LyXTextClassList();

	///
	~LyXTextClassList();

	/// Gets layout structure from layout number and textclass number
	LyXLayout *Style(char textclass, char layout);

	/// Gets layout number from textclass number and layout name
	char NumberOfLayout(char textclass, LString const & name); // add approp. signedness

	/// Gets a layout name from layout number and textclass number
	LString NameOfLayout(char textclass, char layout); // add approp. signedness

	/// Gets textclass number from name
	/** Returns -1 if textclass name does not exist */
	signed char NumberOfClass(LString const & textclass);

	///
	LString NameOfClass(char number); // add approp. signedness

	///
	LString LatexnameOfClass(char number); // add approp. signedness

	///
	LString DescOfClass(char number); // add approp. signedness

	///
	LyXTextClass * TextClass(char textclass); // add approp. signedness

	/// Read textclass list
	/** Returns false if this fails */
	bool Read();

	/// Load textclass
	/** Returns false if this fails */
	bool Load(const char number);
private:
	///
	struct LyXTextClassL {
		///
		LyXTextClass * textclass;
		///
		LyXTextClassL * next;
	};

	///
	LyXTextClassL * l;

	///
	int num_textclass;

	/// After reading, this arrays holds the textclasses
	LyXTextClass * ar;

	/// Add a textclass to list
	void Add (LyXTextClass *l);

	/// Convert final linked list to array
	/** We read the textclasses into a linked list. After the reading,
	the list is converted to an array, and the linked list is nuked. */
	void ToAr();
};

/// Should not be declared here!! (Lgb) Why not? (Asger)
extern LyXTextClassList lyxstyle;
 
#endif
