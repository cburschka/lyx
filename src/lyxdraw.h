// -*- C++ -*-
#ifndef _LYX_DRAW_H
#define _LYX_DRAW_H

#include FORMS_H_LOCATION
#include "lyxfont.h"

///
enum gc_type {
	///
	gc_clear,
	///
	gc_latex,
	///
	gc_foot,
        ///
        gc_new_line,
	///
	gc_math,
	///
	gc_math_frame,
	///
	gc_fill,
	///
	gc_copy,
	///
	gc_select,
	///
	gc_on_off_line,
	///
	gc_thin_on_off_line,
	///
	gc_thick_line,
	///
	gc_lighted,
	///
	gc_selection,
	///
        gc_minipage,
	///
	gc_note,
	///
	gc_note_frame
};

///
extern GC getGC(gc_type typ);
///
extern GC GetAccentGC(LyXFont const &f, int line_width);
///
extern GC GetColorGC(LyXFont::FONT_COLOR color);

#endif
