// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright (C) 1995 Matthias Ettrich
 *
 *======================================================*/
#ifndef _DEFINITIONS_H
#define _DEFINITIONS_H

// NOTE:
// We are trying to get rid of the global enums below, so try to _not_
// add any more of them. Preferably enums should be located inside classes.
// (i.e. class FooBar {
//            enum some {
//                more,
//                thing
//            };
//        }
//  to be accesed as FooBar::more and FooBar::thing

// ALL the enums below should be given names, this to get better typechecking.
// some don't have names now because this cause warings when compiling.


///
enum { // no good name for this
	///
        LYX_ENVIRONMENT_DEFAULT = 97,
	///
	LYX_LAYOUT_DEFAULT = 99
};
// Could this cause confusion that both DUMMY_LAYOUT and  LAYOUT_DEFAULT has
// the same value? (Lgb)
///
#define LYX_DUMMY_LAYOUT 99

///
enum LYX_META_KIND {
	///
	LYX_META_FOOTNOTE = 1,
	///
	LYX_META_MARGIN,
	///
	LYX_META_FIG,
	///
	LYX_META_TAB,
	///
	LYX_META_ALGORITHM,
	///
	LYX_META_WIDE_FIG,
	///
	LYX_META_WIDE_TAB,
	///
	LYX_META_HFILL,
	///
	LYX_META_NEWLINE,
	///
	LYX_META_PROTECTED_SEPARATOR,
	///
	LYX_META_INSET
};

///
enum LYX_PARSEP {
	///
	LYX_PARSEP_INDENT,
	///
	LYX_PARSEP_SKIP
};

///
enum OLD_LYX_PAPER_SIZE {
	///
	OLD_PAPER_A4PAPER,
	///
	OLD_PAPER_A4,
	///
	OLD_PAPER_A4WIDE,
	///
	OLD_PAPER_WIDEMARGINSA4,
	///
	OLD_PAPER_USLETTER,
	///
	OLD_PAPER_A5PAPER,
	///
	OLD_PAPER_B5PAPER,
	///
	OLD_PAPER_EXECUTIVEPAPER,
	///
	OLD_PAPER_LEGALPAPER
};

///
enum LYX_PAPER_SIZE {
	///
	PAPER_DEFAULT,
	///
	PAPER_USLETTER,
	///
	PAPER_LEGALPAPER,
	///
	PAPER_EXECUTIVEPAPER,
	///
	PAPER_A3PAPER,
	///
	PAPER_A4PAPER,
	///
	PAPER_A5PAPER,
	///
	PAPER_B5PAPER
};

///
enum LYX_VMARGIN_PAPER_TYPE {
	///
	VM_PAPER_DEFAULT,
	///
	VM_PAPER_CUSTOM,
	///
	VM_PAPER_USLETTER,
	///
	VM_PAPER_USLEGAL,
	///
	VM_PAPER_USEXECUTIVE,
	///
	VM_PAPER_A3,
	///
	VM_PAPER_A4,
	///
	VM_PAPER_A5,
	///
	VM_PAPER_B3,
	///
	VM_PAPER_B4,
	///
	VM_PAPER_B5
};

///
enum LYX_PAPER_PACKAGES {
	///
	PACKAGE_NONE,
	///
	PACKAGE_A4,
	///
    	PACKAGE_A4WIDE,
	///
	PACKAGE_WIDEMARGINSA4
};

///
enum LYX_PAPER_ORIENTATION {
	///
	ORIENTATION_PORTRAIT,
	///
	ORIENTATION_LANDSCAPE
};

///
enum LYX_PEXTRA_TYPE {
	///
        PEXTRA_NONE,
	///
        PEXTRA_INDENT,
	///
        PEXTRA_MINIPAGE,
	///
	PEXTRA_FLOATFLT
};

///
enum LYX_MINIPAGE_ALIGNMENT {
	///
        MINIPAGE_ALIGN_TOP,
	///
        MINIPAGE_ALIGN_MIDDLE,
	///
        MINIPAGE_ALIGN_BOTTOM
};

#endif
