// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#ifndef LAYOUT_H
#define LAYOUT_H

enum layout_default {
	///
	LYX_LAYOUT_DEFAULT = 99
};


/// The different output types
enum OutputType {
	///
	LATEX = 1,
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
	MARGIN_MANUAL = 1,
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

///
inline
void operator|=(LyXAlignment & la1, LyXAlignment la2) {
	la1 = static_cast<LyXAlignment>(la1 | la2);
}


/// The different LaTeX-Types
enum LYX_LATEX_TYPES {
	///
	LATEX_PARAGRAPH = 1,
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
	LABEL_COUNTER_ENUMIV
};


///
enum LYX_END_LABEL_TYPES {
	///
	END_LABEL_NO_LABEL,
	///
	END_LABEL_BOX,
	///
	END_LABEL_FILLED_BOX,
	///
	END_LABEL_STATIC,
	///
	END_LABEL_ENUM_FIRST = END_LABEL_NO_LABEL,
	///
	END_LABEL_ENUM_LAST = END_LABEL_STATIC
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


#endif
