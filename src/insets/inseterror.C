/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-1999 The LyX Team.
 *
 * ======================================================*/

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "definitions.h"
#include "inseterror.h"
#include "lyxdraw.h"
#include "gettext.h"
#include "lyx_gui_misc.h" // CancelCloseBoxCB

/* Error, used for the LaTeX-Error Messages */

InsetError::InsetError()
{
	form = 0;
}

InsetError::InsetError(string const & string)
	: contents(string)
{
	form = 0;
}


InsetError::~InsetError()
{
	if (form) {
		fl_hide_form(form);
		fl_free_form(form);
		form = 0;
	}
}


int InsetError::Ascent(LyXFont const &font) const
{
	LyXFont efont;
	efont.setSize(font.size()).decSize();
	return efont.maxAscent()+1;
}


int InsetError::Descent(LyXFont const &font) const
{
	LyXFont efont;
	efont.setSize(font.size()).decSize();
	return efont.maxDescent()+1;
}


int InsetError::Width(LyXFont const &font) const
{
	LyXFont efont;
	efont.setSize(font.size()).decSize();
	return 6 + efont.textWidth(_("Error"), strlen(_("Error")));
}


void InsetError::Draw(LyXFont font, LyXScreen &scr,
		      int baseline, float &x)
{
	LyXFont efont;
	efont.setSize(font.size()).decSize();
	efont.setLatex(LyXFont::ON);
   
	// Draw as "Error" in a framed box
	x += 1;
	//scr.drawFilledRectangle(int(x), baseline - Ascent(font)+1,
	//			Width(font)-2,Ascent(font)+ Descent(font)-2,
	//			FL_GRAY80);
	scr.fillRectangle(gc_lighted,
			  int(x), baseline - Ascent(font)+1,
			  Width(font)-2,Ascent(font)+ Descent(font)-2);
	//scr.drawRectangle(int(x), baseline-Ascent(font)+1,
	//		  Width(font)-2, Ascent(font)+Descent(font)-2,
	//		  FL_RED);
	scr.drawRectangle(gc_foot,
			  int(x), baseline-Ascent(font)+1,
			  Width(font)-2, Ascent(font)+Descent(font)-2); 
	scr.drawString(efont, _("Error"), baseline, int(x+2));

	x +=  Width(font) - 1;
}


void InsetError::Write(FILE *)
{
}

void InsetError::Read(LyXLex &)
{
}


int InsetError::Latex(FILE *, signed char /*fragile*/)
{
	return 0;
}


int InsetError::Latex(string &, signed char /*fragile*/)
{
	return 0;
}


int InsetError::Linuxdoc(string &)
{
	return 0;
}


int InsetError::DocBook(string &)
{
	return 0;
}


bool InsetError::AutoDelete() const
{
	return true;
}


unsigned char InsetError::Editable() const
{
	return 1;
}


void InsetError::CloseErrorCB(FL_OBJECT *, long data)
{
	InsetError *inset = (InsetError*) data;
	if (inset->form) {
		fl_hide_form(inset->form);
		fl_free_form(inset->form);
		inset->form = 0;
	}
}

// A C wrapper
extern "C" void C_InsetError_CloseErrorCB(FL_OBJECT *, long data)
{
        InsetError::CloseErrorCB(0,data);
}


void InsetError::Edit(int, int)
{
	static int ow = 400, oh = 240;

	if (!form) {
		FL_OBJECT *obj;
		form = fl_bgn_form(FL_UP_BOX,ow,oh);
		strobj = fl_add_box(FL_FRAME_BOX,10,10,380,180,"");
		fl_set_object_color(strobj,FL_MCOL,FL_MCOL);
		fl_set_object_gravity(strobj, FL_NorthWest, FL_SouthEast);
		obj = fl_add_button(FL_RETURN_BUTTON,140,200,120,30,_("Close"));
		fl_set_object_callback(obj, C_InsetError_CloseErrorCB, (long)this);
		fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
		fl_set_object_resize(obj, FL_RESIZE_NONE);
		fl_end_form();
		fl_set_form_atclose(form, CancelCloseBoxCB, 0);
	}
	fl_set_object_label(strobj, contents.c_str());
	if (form->visible) {
		fl_raise_form(form);
	} else {
		fl_show_form(form,FL_PLACE_MOUSE | FL_FREE_SIZE, FL_FULLBORDER, 
			     _("LaTeX Error"));
		fl_set_form_minsize(form, ow, oh);
	}
}


Inset* InsetError::Clone()
{
	InsetError *result = new InsetError(contents);
	return result;
}
