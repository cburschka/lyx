#include <config.h>

#include <cstdlib>
#include FORMS_H_LOCATION

#include "layout_forms.h"
#include "LString.h"
#include "minibuffer.h"
#include "vspace.h"
#include "buffer.h"
#include "BufferView.h"
#include "lyxparagraph.h"
#include "gettext.h"
#include "lyxtext.h"
#include "LyXView.h"

extern FD_form_paragraph_extra * fd_form_paragraph_extra;
extern BufferView * current_view;
static bool CheckInputWidth();

inline
void DeactivateParagraphExtraButtons ()
{
	fl_deactivate_object(fd_form_paragraph_extra->button_ok);
	fl_deactivate_object(fd_form_paragraph_extra->button_apply);
	fl_set_object_lcol(fd_form_paragraph_extra->button_ok, FL_INACTIVE);
	fl_set_object_lcol(fd_form_paragraph_extra->button_apply, FL_INACTIVE);
}

inline
void ActivateParagraphExtraButtons ()
{
	fl_activate_object(fd_form_paragraph_extra->button_ok);
	fl_activate_object(fd_form_paragraph_extra->button_apply);
	fl_set_object_lcol(fd_form_paragraph_extra->button_ok, FL_BLACK);
	fl_set_object_lcol(fd_form_paragraph_extra->button_apply, FL_BLACK);
}

inline
void DisableParagraphExtra ()
{
        DeactivateParagraphExtraButtons();
	fl_deactivate_object(fd_form_paragraph_extra->input_pextra_width);
	fl_deactivate_object(fd_form_paragraph_extra->input_pextra_widthp);
	fl_deactivate_object(fd_form_paragraph_extra->group_alignment);
	fl_deactivate_object(fd_form_paragraph_extra->radio_pextra_top);
	fl_deactivate_object(fd_form_paragraph_extra->radio_pextra_middle);
	fl_deactivate_object(fd_form_paragraph_extra->radio_pextra_bottom);
	fl_deactivate_object(fd_form_paragraph_extra->text_warning);
	fl_deactivate_object(fd_form_paragraph_extra->group_extraopt);
	fl_deactivate_object(fd_form_paragraph_extra->radio_pextra_indent);
	fl_deactivate_object(fd_form_paragraph_extra->radio_pextra_minipage);
	fl_deactivate_object(fd_form_paragraph_extra->radio_pextra_floatflt);
	fl_deactivate_object(fd_form_paragraph_extra->radio_pextra_hfill);
	fl_deactivate_object(fd_form_paragraph_extra->radio_pextra_startmp);
}	

inline
void EnableParagraphExtra ()
{
        ActivateParagraphExtraButtons();
	fl_activate_object(fd_form_paragraph_extra->input_pextra_width);
	fl_activate_object(fd_form_paragraph_extra->input_pextra_widthp);
	fl_activate_object(fd_form_paragraph_extra->group_alignment);
	fl_activate_object(fd_form_paragraph_extra->radio_pextra_top);
	fl_activate_object(fd_form_paragraph_extra->radio_pextra_middle);
	fl_activate_object(fd_form_paragraph_extra->radio_pextra_bottom);
	fl_activate_object(fd_form_paragraph_extra->text_warning);
	fl_activate_object(fd_form_paragraph_extra->group_extraopt);
	fl_activate_object(fd_form_paragraph_extra->radio_pextra_indent);
	fl_activate_object(fd_form_paragraph_extra->radio_pextra_minipage);
	fl_activate_object(fd_form_paragraph_extra->radio_pextra_floatflt);
	fl_activate_object(fd_form_paragraph_extra->radio_pextra_hfill);
	fl_activate_object(fd_form_paragraph_extra->radio_pextra_startmp);
}	


bool UpdateParagraphExtra()
{
	bool update = false;
	if (current_view->available()) {
		update = true;
		LyXParagraph * par = current_view->text->cursor.par;

		EnableParagraphExtra();

		fl_activate_object(fd_form_paragraph_extra->input_pextra_width);
		fl_activate_object(fd_form_paragraph_extra->input_pextra_widthp);
		fl_set_input(fd_form_paragraph_extra->input_pextra_width,
			     par->pextra_width.c_str());
		fl_set_input(fd_form_paragraph_extra->input_pextra_widthp,
			     par->pextra_widthp.c_str());
		switch(par->pextra_alignment) {
		case LyXParagraph::MINIPAGE_ALIGN_TOP:
			fl_set_button(fd_form_paragraph_extra->radio_pextra_top, 1);
			break;
		case LyXParagraph::MINIPAGE_ALIGN_MIDDLE:
			fl_set_button(fd_form_paragraph_extra->radio_pextra_middle, 1);
			break;
		case LyXParagraph::MINIPAGE_ALIGN_BOTTOM:
			fl_set_button(fd_form_paragraph_extra->radio_pextra_bottom, 1);
			break;
		}
		fl_set_button(fd_form_paragraph_extra->radio_pextra_hfill,
			      par->pextra_hfill);
		fl_set_button(fd_form_paragraph_extra->radio_pextra_startmp,
			      par->pextra_start_minipage);
		CheckInputWidth();
		if (par->pextra_type == LyXParagraph::PEXTRA_INDENT) {
			fl_set_button(fd_form_paragraph_extra->radio_pextra_indent, 1);
			fl_set_button(fd_form_paragraph_extra->radio_pextra_minipage, 0);
			fl_set_button(fd_form_paragraph_extra->radio_pextra_floatflt, 0);
			fl_deactivate_object(fd_form_paragraph_extra->radio_pextra_top);
			fl_deactivate_object(fd_form_paragraph_extra->radio_pextra_middle);
			fl_deactivate_object(fd_form_paragraph_extra->radio_pextra_bottom);
		} else if (par->pextra_type == LyXParagraph::PEXTRA_MINIPAGE) {
			fl_set_button(fd_form_paragraph_extra->radio_pextra_indent, 0);
			fl_set_button(fd_form_paragraph_extra->radio_pextra_minipage, 1);
			fl_set_button(fd_form_paragraph_extra->radio_pextra_floatflt, 0);
			fl_activate_object(fd_form_paragraph_extra->radio_pextra_top);
			fl_activate_object(fd_form_paragraph_extra->radio_pextra_middle);
			fl_activate_object(fd_form_paragraph_extra->radio_pextra_bottom);
		} else if (par->pextra_type == LyXParagraph::PEXTRA_FLOATFLT) {
			fl_set_button(fd_form_paragraph_extra->radio_pextra_indent, 0);
			fl_set_button(fd_form_paragraph_extra->radio_pextra_minipage, 0);
			fl_set_button(fd_form_paragraph_extra->radio_pextra_floatflt, 1);
			fl_deactivate_object(fd_form_paragraph_extra->radio_pextra_top);
			fl_deactivate_object(fd_form_paragraph_extra->radio_pextra_middle);
			fl_deactivate_object(fd_form_paragraph_extra->radio_pextra_bottom);
		} else {
			fl_set_button(fd_form_paragraph_extra->radio_pextra_indent, 0);
			fl_set_button(fd_form_paragraph_extra->radio_pextra_minipage, 0);
			fl_deactivate_object(fd_form_paragraph_extra->input_pextra_width);
			fl_deactivate_object(fd_form_paragraph_extra->input_pextra_widthp);
			fl_deactivate_object(fd_form_paragraph_extra->radio_pextra_top);
			fl_deactivate_object(fd_form_paragraph_extra->radio_pextra_middle);
			fl_deactivate_object(fd_form_paragraph_extra->radio_pextra_bottom);
		}
		if (par->pextra_type == LyXParagraph::PEXTRA_NONE)
			ActivateParagraphExtraButtons();
	}
	fl_hide_object(fd_form_paragraph_extra->text_warning);

	if (current_view->buffer()->isReadonly()) {
		DisableParagraphExtra();
		fl_set_object_label(fd_form_paragraph_extra->text_warning,
				    _("Document is read-only. "
				      "No changes to layout permitted."));
		fl_show_object(fd_form_paragraph_extra->text_warning);
	}
	if (!update && fd_form_paragraph_extra->form_paragraph_extra->visible) {
		fl_hide_form(fd_form_paragraph_extra->form_paragraph_extra);
	}
	return update;
}


void ParagraphExtraOpen(FL_OBJECT *, long)
{
	if (UpdateParagraphExtra()) {
		if (fd_form_paragraph_extra->form_paragraph_extra->visible) {
			fl_raise_form(fd_form_paragraph_extra->form_paragraph_extra);
		} else {
			fl_show_form(fd_form_paragraph_extra->form_paragraph_extra,
				     FL_PLACE_MOUSE, FL_FULLBORDER,
				     _("ParagraphExtra Layout"));
		}
	}
}


void ParagraphExtraApplyCB(FL_OBJECT *, long)
{
	if (current_view->available()) {
		FD_form_paragraph_extra const * fd = fd_form_paragraph_extra;
		char const * width = fl_get_input(fd->input_pextra_width);
		char const * widthp = fl_get_input(fd->input_pextra_widthp);
		LyXText * text = current_view->text;
		int type = LyXParagraph::PEXTRA_NONE;
		LyXParagraph::MINIPAGE_ALIGNMENT alignment =
			LyXParagraph::MINIPAGE_ALIGN_TOP;
		bool
			hfill = false,
			start_minipage = false;

		if (fl_get_button(fd_form_paragraph_extra->radio_pextra_indent)) {
			type = LyXParagraph::PEXTRA_INDENT;
		} else if (fl_get_button(fd_form_paragraph_extra->radio_pextra_minipage)) {
			type = LyXParagraph::PEXTRA_MINIPAGE;
			hfill = fl_get_button(fd_form_paragraph_extra->radio_pextra_hfill);
			start_minipage = 
				fl_get_button(fd_form_paragraph_extra->radio_pextra_startmp);
			if (fl_get_button(fd_form_paragraph_extra->radio_pextra_top))
				alignment = LyXParagraph::MINIPAGE_ALIGN_TOP;
			else if (fl_get_button(fd_form_paragraph_extra->radio_pextra_middle))
				alignment = LyXParagraph::MINIPAGE_ALIGN_MIDDLE;
			else if (fl_get_button(fd_form_paragraph_extra->radio_pextra_bottom))
				alignment = LyXParagraph::MINIPAGE_ALIGN_BOTTOM;
		} else if (fl_get_button(fd_form_paragraph_extra->radio_pextra_floatflt)) {
			type = LyXParagraph::PEXTRA_FLOATFLT;
		}
		text->SetParagraphExtraOpt(type, width, widthp, alignment, hfill,
					   start_minipage);
		current_view->update(1);
		current_view->owner()->getMiniBuffer()->
			Set(_("ParagraphExtra layout set"));
	}
	return;
}


void ParagraphExtraCancelCB(FL_OBJECT *, long)
{
	fl_hide_form(fd_form_paragraph_extra->form_paragraph_extra);
}


void ParagraphExtraOKCB(FL_OBJECT * ob, long data)
{
	ParagraphExtraCancelCB(ob, data);
	ParagraphExtraApplyCB(ob, data);
}


void CheckPExtraOptCB(FL_OBJECT * ob, long)
{
	ActivateParagraphExtraButtons();
	CheckInputWidth();
	if (ob == fd_form_paragraph_extra->radio_pextra_indent) {
		int n = fl_get_button(fd_form_paragraph_extra->radio_pextra_indent);
		if (n) {
			fl_set_button(fd_form_paragraph_extra->radio_pextra_minipage, 0);
			fl_set_button(fd_form_paragraph_extra->radio_pextra_floatflt, 0);
			fl_activate_object(fd_form_paragraph_extra->input_pextra_width);
			fl_activate_object(fd_form_paragraph_extra->input_pextra_widthp);
			fl_deactivate_object(fd_form_paragraph_extra->radio_pextra_top);
			fl_deactivate_object(fd_form_paragraph_extra->radio_pextra_middle);
			fl_deactivate_object(fd_form_paragraph_extra->radio_pextra_bottom);
		} else {
			fl_deactivate_object(fd_form_paragraph_extra->input_pextra_width);
			fl_deactivate_object(fd_form_paragraph_extra->input_pextra_widthp);
			ActivateParagraphExtraButtons();
		}
	} else if (ob == fd_form_paragraph_extra->radio_pextra_minipage) {
		int n = fl_get_button(fd_form_paragraph_extra->radio_pextra_minipage);
		if (n) {
			fl_set_button(fd_form_paragraph_extra->radio_pextra_indent, 0);
			fl_set_button(fd_form_paragraph_extra->radio_pextra_floatflt, 0);
			fl_activate_object(fd_form_paragraph_extra->input_pextra_width);
			fl_activate_object(fd_form_paragraph_extra->input_pextra_widthp);
			fl_activate_object(fd_form_paragraph_extra->radio_pextra_top);
			fl_activate_object(fd_form_paragraph_extra->radio_pextra_middle);
			fl_activate_object(fd_form_paragraph_extra->radio_pextra_bottom);
		} else {
			fl_deactivate_object(fd_form_paragraph_extra->input_pextra_width);
			fl_deactivate_object(fd_form_paragraph_extra->input_pextra_widthp);
			fl_deactivate_object(fd_form_paragraph_extra->radio_pextra_top);
			fl_deactivate_object(fd_form_paragraph_extra->radio_pextra_middle);
			fl_deactivate_object(fd_form_paragraph_extra->radio_pextra_bottom);
			ActivateParagraphExtraButtons();
		}
	} else if (ob == fd_form_paragraph_extra->radio_pextra_floatflt) {
		int n = fl_get_button(fd_form_paragraph_extra->radio_pextra_floatflt);
		if (n) {
			fl_set_button(fd_form_paragraph_extra->radio_pextra_indent, 0);
			fl_set_button(fd_form_paragraph_extra->radio_pextra_minipage, 0);
			fl_activate_object(fd_form_paragraph_extra->input_pextra_width);
			fl_activate_object(fd_form_paragraph_extra->input_pextra_widthp);
			fl_activate_object(fd_form_paragraph_extra->radio_pextra_top);
			fl_activate_object(fd_form_paragraph_extra->radio_pextra_middle);
			fl_activate_object(fd_form_paragraph_extra->radio_pextra_bottom);
		} else {
			fl_deactivate_object(fd_form_paragraph_extra->input_pextra_width);
			fl_deactivate_object(fd_form_paragraph_extra->input_pextra_widthp);
			fl_deactivate_object(fd_form_paragraph_extra->radio_pextra_top);
			fl_deactivate_object(fd_form_paragraph_extra->radio_pextra_middle);
			fl_deactivate_object(fd_form_paragraph_extra->radio_pextra_bottom);
			ActivateParagraphExtraButtons();
		}
	}
	return;
}


static
bool CheckInputWidth()
{
	string s1 = fl_get_input(fd_form_paragraph_extra->input_pextra_width);
	string s2 = fl_get_input(fd_form_paragraph_extra->input_pextra_widthp);
	if (s1.empty() && s2.empty()) {
		fl_activate_object(fd_form_paragraph_extra->input_pextra_width);
		fl_activate_object(fd_form_paragraph_extra->input_pextra_widthp);
		fl_hide_object(fd_form_paragraph_extra->text_warning);
		DeactivateParagraphExtraButtons();
		return false;
	}
	if (!s1.empty()) { // LyXLength parameter
		fl_activate_object(fd_form_paragraph_extra->input_pextra_width);
		fl_deactivate_object(fd_form_paragraph_extra->input_pextra_widthp);
		if (!isValidLength(s1)) {
			DeactivateParagraphExtraButtons();
			fl_set_object_label(fd_form_paragraph_extra->text_warning,
					    _("Warning: Invalid Length (valid example: 10mm)"));
			fl_show_object(fd_form_paragraph_extra->text_warning);
			return false;
		}
	} else { // !s2.empty() % parameter
		fl_deactivate_object(fd_form_paragraph_extra->input_pextra_width);
		fl_activate_object(fd_form_paragraph_extra->input_pextra_widthp);
		if ((atoi(s2.c_str()) < 0 ) || (atoi(s2.c_str()) > 100)) {
			DeactivateParagraphExtraButtons();
			fl_set_object_label(fd_form_paragraph_extra->text_warning,
					    _("Warning: Invalid percent value (0-100)"));
			fl_show_object(fd_form_paragraph_extra->text_warning);
			return false;
		}
	}
	fl_hide_object(fd_form_paragraph_extra->text_warning);
	return true;
}
