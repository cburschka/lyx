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

#include <cctype>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetinfo.h"
#include "lyxparagraph.h"
#include "debug.h"
#include "gettext.h"
#include "lyx_gui_misc.h" // CancelCloseBoxCB
#include "buffer.h"
#include "support/lstrings.h"
#include "Painter.h"
#include "font.h"

using std::ostream;
using std::endl;

/* Info, used for the Info boxes */

extern BufferView * current_view;


InsetInfo::InsetInfo()
	: form(0), labelfont(LyXFont::ALL_SANE)
{
	labelfont.decSize().decSize()
		.setColor(LColor::note).setLatex(LyXFont::OFF);
}


InsetInfo::InsetInfo(string const & str)
	: contents(str), form(0), labelfont(LyXFont::ALL_SANE)
{
	labelfont.decSize().decSize()
		.setColor(LColor::note).setLatex(LyXFont::OFF);
}


InsetInfo::~InsetInfo()
{
	if (form) {
		fl_hide_form(form);
		fl_free_form(form);
		form = 0;
	}
}


int InsetInfo::ascent(BufferView *, LyXFont const &) const
{
	return lyxfont::maxAscent(labelfont) + 1;
}


int InsetInfo::descent(BufferView *, LyXFont const &) const
{
	return lyxfont::maxDescent(labelfont) + 1;
}


int InsetInfo::width(BufferView *, LyXFont const &) const
{
	return 6 + lyxfont::width(_("Note"), labelfont);
}


void InsetInfo::draw(BufferView * bv, LyXFont const &,
		     int baseline, float & x, bool) const
{
	Painter & pain = bv->painter();
#if 0
	LyXFont font(f);
	
	// Info-insets are never LaTeX, so just correct the font
	font.setLatex(LyXFont::OFF).setColor(LColor::note);
#endif
	// Draw as "Note" in a yellow box
	x += 1;
	pain.fillRectangle(int(x), baseline - ascent(bv, labelfont),
			   width(bv, labelfont) - 2,
			   ascent(bv, labelfont) + descent(bv, labelfont) - 2,
			   LColor::notebg);
	pain.rectangle(int(x), baseline - ascent(bv, labelfont),
		       width(bv, labelfont) - 2,
		       ascent(bv, labelfont) + descent(bv, labelfont) - 2,
		       LColor::noteframe);
	
	pain.text(int(x + 2), baseline, _("Note"), labelfont);
	x +=  width(bv, labelfont) - 1;
}


void InsetInfo::Write(Buffer const *, ostream & os) const
{
	os << "Info\n" << contents;
}


void InsetInfo::Read(Buffer const *, LyXLex & lex)
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
      

int InsetInfo::Latex(Buffer const *, ostream &,
		     bool /*fragile*/, bool /*free_spc*/) const
{
	return 0;
}


int InsetInfo::Ascii(Buffer const *, ostream &) const
{
	return 0;
}


int InsetInfo::Linuxdoc(Buffer const *, ostream &) const
{
	return 0;
}


int InsetInfo::DocBook(Buffer const *, ostream &) const
{
	return 0;
}


Inset::EDITABLE InsetInfo::Editable() const
{
	return IS_EDITABLE;
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
extern "C"
void C_InsetInfo_CloseInfoCB(FL_OBJECT * ob, long data) 
{
  	InsetInfo::CloseInfoCB(ob, data);
}


string const InsetInfo::EditMessage() const 
{
	return _("Opened note");
}


void InsetInfo::Edit(BufferView *bv, int, int, unsigned int)
{
	static int ow = -1, oh;

	if(bv->buffer()->isReadonly())
		WarnReadonly(bv->buffer()->fileName());
	
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


Inset * InsetInfo::Clone() const
{
	return new InsetInfo(contents);
}


Inset::Code InsetInfo::LyxCode() const
{
	return Inset::IGNORE_CODE;
}
