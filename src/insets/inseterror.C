/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "inseterror.h"
#include "gettext.h"
#include "lyx_gui_misc.h" // CancelCloseBoxCB
#include "Painter.h"

/* Error, used for the LaTeX-Error Messages */

InsetError::InsetError()
{
	form = 0;
}


InsetError::InsetError(string const & str)
	: contents(str)
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


int InsetError::ascent(Painter &, LyXFont const & font) const
{
	LyXFont efont;
	efont.setSize(font.size()).decSize();
	return efont.maxAscent() + 1;
}


int InsetError::descent(Painter &, LyXFont const & font) const
{
	LyXFont efont;
	efont.setSize(font.size()).decSize();
	return efont.maxDescent() + 1;
}


int InsetError::width(Painter &, LyXFont const & font) const
{
	LyXFont efont;
	efont.setSize(font.size()).decSize();
	return 6 + efont.textWidth(_("Error"), strlen(_("Error")));
}


void InsetError::draw(Painter & pain, LyXFont const & font,
		      int baseline, float & x) const
{
	LyXFont efont;
	efont.setSize(font.size()).decSize();
	efont.setColor(LColor::error);
   
	// Draw as "Error" in a framed box
	x += 1;
	pain.fillRectangle(int(x), baseline - ascent(pain, font) + 1,
			  width(pain, font) - 2,
			  ascent(pain, font) + descent(pain, font) - 2,
			   LColor::insetbg);
	pain.rectangle(int(x), baseline - ascent(pain, font) + 1,
		       width(pain, font) - 2,
		       ascent(pain, font) + descent(pain, font) - 2,
		       LColor::error);
	pain.text(int(x + 2), baseline, _("Error"), efont);

	x +=  width(pain, font) - 1;
}


void InsetError::Write(ostream &) const
{
}


void InsetError::Read(LyXLex &)
{
}


int InsetError::Latex(ostream &,
		      signed char /*fragile*/, bool /*fs*/) const
{
	return 0;
}


int InsetError::Linuxdoc(ostream &) const
{
	return 0;
}


int InsetError::DocBook(ostream &) const
{
	return 0;
}


bool InsetError::AutoDelete() const
{
	return true;
}


Inset::EDITABLE InsetError::Editable() const
{
	return IS_EDITABLE;
}


void InsetError::CloseErrorCB(FL_OBJECT * ob, long)
{
	InsetError * inset = static_cast<InsetError*>(ob->u_vdata);
	if (inset->form) {
		fl_hide_form(inset->form);
		fl_free_form(inset->form);
		inset->form = 0;
	}
}


// A C wrapper
extern "C" void C_InsetError_CloseErrorCB(FL_OBJECT * ob, long data)
{
        InsetError::CloseErrorCB(ob , data);
}


void InsetError::Edit(BufferView *, int, int, unsigned int)
{
	static int ow = 400, oh = 240;

	if (!form) {
		FL_OBJECT * obj;
		form = fl_bgn_form(FL_UP_BOX, ow, oh);
		strobj = fl_add_box(FL_FRAME_BOX, 10, 10, 380, 180, "");
		fl_set_object_color(strobj, FL_MCOL, FL_MCOL);
		fl_set_object_gravity(strobj, FL_NorthWest, FL_SouthEast);
		obj = fl_add_button(FL_RETURN_BUTTON, 140, 200, 120, 30,
				    _("Close"));
		fl_set_object_callback(obj, C_InsetError_CloseErrorCB, 0);
		obj->u_vdata = this;
		fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
		fl_set_object_resize(obj, FL_RESIZE_NONE);
		fl_end_form();
		fl_set_form_atclose(form, CancelCloseBoxCB, 0);
	}
	fl_set_object_label(strobj, contents.c_str());
	if (form->visible) {
		fl_raise_form(form);
	} else {
		fl_show_form(form, FL_PLACE_MOUSE | FL_FREE_SIZE,
			     FL_FULLBORDER, _("LaTeX Error"));
		fl_set_form_minsize(form, ow, oh);
	}
}


Inset * InsetError::Clone() const
{
	return new InsetError(contents);
}
