// -*- C++ -*-

#ifndef PARAMETERSTRUCT_H
#define PARAMETERSTRUCT_H

#include "vspace.h"
#include "Spacing.h"
#include "layout.h"
#include "lyxlength.h"

#ifdef __GNUG__
#pragma interface
#endif

///
struct ParameterStruct {
	///
	ParameterStruct();
	///
	typedef unsigned int depth_type;
	///
	VSpace added_space_top;
	///
	VSpace added_space_bottom;
	///
	Spacing spacing;
	///
	bool noindent;
	///
	bool line_top;
	///
	bool line_bottom;
	///
	bool pagebreak_top;
	///
	bool pagebreak_bottom;
	///
	LyXAlignment align;
	///
	depth_type depth;
	///
	bool start_of_appendix;
	///
	bool appendix;
	///
	string labelstring;
	///
	string labelwidthstring;
	///
	LyXLength leftindent;
};


inline
ParameterStruct::ParameterStruct()
	: noindent(false), line_top(false), line_bottom(false),
	  pagebreak_top(false), pagebreak_bottom(false),
	  align(LYX_ALIGN_BLOCK), depth(0), start_of_appendix(false),
	  appendix(false)
{}


inline
bool operator==(ParameterStruct const & ps1,
		ParameterStruct const & ps2)
{
	return ps1.added_space_top == ps2.added_space_top
		&& ps1.added_space_bottom == ps2.added_space_bottom
		&& ps1.spacing == ps2.spacing
		&& ps1.noindent == ps2.noindent
		&& ps1.line_top == ps2.line_top
		&& ps1.line_bottom == ps2.line_bottom
		&& ps1.pagebreak_top == ps2.pagebreak_top
		&& ps1.pagebreak_bottom == ps2.pagebreak_bottom
		&& ps1.align == ps2.align
		&& ps1.depth == ps2.depth
		&& ps1.start_of_appendix == ps2.start_of_appendix
		&& ps1.appendix == ps2.appendix
		&& ps1.labelstring == ps2.labelstring
		&& ps1.labelwidthstring == ps2.labelwidthstring
		&& ps1.leftindent == ps2.leftindent;

}

#endif
