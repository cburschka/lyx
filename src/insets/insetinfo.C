/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-1999 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#include <cctype>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetinfo.h"
#include "lyxparagraph.h"
#include "debug.h"
#include "lyxdraw.h"
#include "gettext.h"
#include "lyx_gui_misc.h" // CancelCloseBoxCB
#include "buffer.h"
#include "support/lstrings.h"

/* Info, used for the Info boxes */

extern BufferView * current_view;

InsetInfo::InsetInfo()
{
	form = 0;
}


InsetInfo::InsetInfo(string const & string)
	: contents(string)
{
	form = 0;
}


InsetInfo::~InsetInfo()
{
	if (form){
		fl_hide_form(form);
		fl_free_form(form);
		form = 0;
	}
}


int InsetInfo::Ascent(LyXFont const & font) const
{
	return font.maxAscent() + 1;
}


int InsetInfo::Descent(LyXFont const & font) const
{
	return font.maxDescent() + 1;
}


int InsetInfo::Width(LyXFont const & font) const
{
	return 6 + font.textWidth(_("Note"), strlen(_("Note")));
}


void InsetInfo::Draw(LyXFont font, LyXScreen & scr,
		     int baseline, float & x)
{
	/* Info-insets are never LaTeX, so just correct the font */
	font.setLatex(LyXFont::OFF);

	// Draw as "Note" in a yellow box
	x += 1;
	scr.fillRectangle(gc_note,
			  int(x), baseline - Ascent(font)+1,
			  Width(font)-2, Ascent(font)+Descent(font)-2);
	scr.drawRectangle(gc_note_frame,
			  int(x), baseline- Ascent(font)+1,
			  Width(font)-2, Ascent(font)+Descent(font)-2);
	
	scr.drawString(font, _("Note"), baseline, int(x+2));
	x +=  Width(font) - 1;
}


void InsetInfo::Write(FILE * file)
{
	fprintf(file, "Info %s", contents.c_str());
}


void InsetInfo::Read(LyXLex & lex)
{
	string tmp = lex.GetString(); // should be "Info"
	if (tmp != "Info")
		lyxerr << "ERROR (InsetInfo::Read): "
			"consistency check 1 failed." << endl;

	while (lex.IsOK()) {
		if (!lex.EatLine())
			// blank line in the file being read
			// should we skip blank lines?
			continue;

		string const token = strip(lex.GetString());
		lyxerr[Debug::PARSER] << "Note: " << token << endl;
		
		if (token != "\\end_inset") {
			contents += token + '\n';
		}
		else // token == "\\end_inset"
			break;
	}
	// now remove the last '\n's
	contents = strip(contents, '\n');
}
      

int InsetInfo::Latex(FILE *, signed char /*fragile*/)
{
	return 0;
}


int InsetInfo::Latex(string &, signed char /*fragile*/)
{
	return 0;
}


int InsetInfo::Linuxdoc(string &)
{
	return 0;
}


int InsetInfo::DocBook(string &)
{
	return 0;
}


unsigned char InsetInfo::Editable() const
{
	return 1;
}


void InsetInfo::CloseInfoCB(FL_OBJECT * ob, long)
{
	InsetInfo * inset = static_cast<InsetInfo*>(ob->u_vdata);
	string tmp = fl_get_input(inset->strobj);
	Buffer * buffer = current_view->buffer();
	if(tmp != inset->contents && !(buffer->isReadonly()) ) {
		buffer->markDirty();
		inset->contents = tmp;
	}
	if (inset->form) {
		fl_hide_form(inset->form);
		fl_free_form(inset->form);
		inset->form = 0;
	}
}


// This is just a wrapper.
extern "C" void C_InsetInfo_CloseInfoCB(FL_OBJECT * ob, long data) 
{
  	InsetInfo::CloseInfoCB(ob, data);
}


void InsetInfo::Edit(int, int)
{
	static int ow = -1, oh;

	if(current_view->buffer()->isReadonly())
		WarnReadonly();
	
	if (!form) {
		FL_OBJECT *obj;
		form = fl_bgn_form(FL_UP_BOX, 400, 180);
		strobj = obj = fl_add_input(FL_MULTILINE_INPUT, 10, 10, 380, 120, "");
		fl_set_object_color(obj, FL_MCOL, FL_MCOL);
		fl_set_object_resize(obj, FL_RESIZE_ALL);
		fl_set_object_gravity(obj, NorthWestGravity, SouthEastGravity);
		obj = fl_add_button(FL_NORMAL_BUTTON, 130, 140, 120, 30, idex(_("Close|#C^[")));
		fl_set_object_resize(obj, FL_RESIZE_NONE);
		fl_set_object_gravity(obj, SouthEastGravity, SouthEastGravity);
		fl_set_object_callback(obj, C_InsetInfo_CloseInfoCB, 0);
		obj->u_vdata = this;
		fl_set_object_shortcut(obj, scex(_("Close|#C^[")), 1);
		fl_end_form();
		fl_set_form_atclose(form, CancelCloseBoxCB, 0);
	}
	fl_set_input(strobj, contents.c_str());
	if (form->visible) {
		fl_raise_form(form);
	} else {
		fl_show_form(form, FL_PLACE_MOUSE | FL_FREE_SIZE, FL_FULLBORDER, 
			     _("Note"));
		if (ow < 0) {
			ow = form->w;
			oh = form->h;
		}
		fl_set_form_minsize(form, ow, oh);
	}
}


Inset* InsetInfo::Clone()
{
	InsetInfo * result = new InsetInfo(contents);
	return result;
}


Inset::Code InsetInfo::LyxCode() const
{
	return Inset::IGNORE_CODE;
}
