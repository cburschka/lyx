// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-1999 The LyX Team.
 *
 * ======================================================*/

#include <config.h>

#include <cerrno>
#include "lyx_gui_misc.h"
#include "BufferView.h"
#include "bibforms.h"
#include "buffer.h"
#include "bullet_forms.h"
#include "form1.h"
#include "gettext.h"
#include "include_form.h"
#include "latexoptions.h"
#include "layout_forms.h"
#include "lyx.h"
#include "lyx_cb.h"
#include "lyx_main.h"
#include "mathed/math_forms.h"
#include "minibuffer.h"
#include "print_form.h"
#include "sp_form.h"
#include "insets/insetindex.h"

extern MiniBuffer *minibuffer;
extern BufferView *current_view;

extern FD_form_paragraph * fd_form_paragraph;
extern FD_form_paragraph_extra * fd_form_paragraph_extra;
extern FD_form_search * fd_form_search;
extern FD_form_character * fd_form_character;
extern FD_form_document * fd_form_document;
extern FD_form_paper * fd_form_paper;
extern FD_form_table_options * fd_form_table_options;
extern FD_form_quotes * fd_form_quotes;
extern FD_form_preamble * fd_form_preamble;
extern FD_form_table * fd_form_table;
extern FD_form_print * fd_form_print;
extern FD_form_sendto * fd_form_sendto;
extern FD_form_figure * fd_form_figure;
extern FD_form_toc * fd_form_toc;
extern FD_form_ref * fd_form_ref;
extern FD_form_paragraph_extra * fd_form_paragraph_extra;
extern FD_LaTeXLog * fd_latex_log;
extern FD_form_spell_check * fd_form_spell_check;
extern FD_form_bullet * fd_form_bullet;
extern FD_panel  * fd_panel;
extern FD_delim  * fd_delim;
extern FD_deco   * fd_deco;
extern FD_space  * fd_space;
extern FD_matrix * fd_matrix;
extern FD_citation_form * citation_form;
extern FD_bibitem_form * bibitem_form;
extern FD_include * form;
extern FD_index_form * index_form;

extern void TocUpdateCB();
extern void HideFiguresPopups();

// Prevents LyX from being killed when the close box is pressed in a popup.
extern "C" int CancelCloseBoxCB(FL_FORM *, void *)
{
	return FL_CANCEL;
}


// Prevents LyX from being killed when the close box is pressed in a popup.
extern "C" int IgnoreCloseBoxCB(FL_FORM *, void *)
{
	return FL_IGNORE;
}


// Prevents LyX from crashing when no buffers available
// This is also one of the functions that we _really_ dont want
// we should try to finds way to help us with that.
// The signal/slot mechanism can probably help. 
void CloseAllBufferRelatedPopups()
{
	// don't forget to check that dynamically created forms
	// have been created otherwise hiding one could cause a crash
	// need the visible check otherwise XForms prints a warning
	// if hiding an invisible form
	if (fd_form_paragraph->form_paragraph->visible) {
		fl_hide_form(fd_form_paragraph->form_paragraph);
	}
	if (fd_form_paragraph_extra->form_paragraph_extra->visible) {
		fl_hide_form(fd_form_paragraph_extra->form_paragraph_extra);
	}
	if (fd_form_search->form_search->visible) {
		fl_hide_form(fd_form_search->form_search);
	}
	if (fd_form_character->form_character->visible) {
		fl_hide_form(fd_form_character->form_character);
	}
	if (fd_form_document->form_document->visible) {
		fl_hide_form(fd_form_document->form_document);
	}
	if (fd_form_quotes->form_quotes->visible) {
		fl_hide_form(fd_form_quotes->form_quotes);
	}
	if (fd_form_preamble->form_preamble->visible) {
		fl_hide_form(fd_form_preamble->form_preamble);
	}
	if (fd_form_table->form_table->visible) {
		fl_hide_form(fd_form_table->form_table);
	}
	if (fd_form_print->form_print->visible) {
		fl_hide_form(fd_form_print->form_print);
	}
	if (fd_form_figure->form_figure->visible) {
		fl_hide_form(fd_form_figure->form_figure);
	}
	if (fd_form_toc->form_toc->visible) {
		fl_hide_form(fd_form_toc->form_toc);
	}
	if (fd_form_ref->form_ref->visible) {
		fl_hide_form(fd_form_ref->form_ref);
	}
	if (fd_form_paper->form_paper->visible) {
		fl_hide_form(fd_form_paper->form_paper);
	}
	if (fd_form_table_options->form_table_options->visible) {
		fl_hide_form(fd_form_table_options->form_table_options);
	}
	if (fd_form_sendto->form_sendto->visible) {
		fl_hide_form(fd_form_sendto->form_sendto);
	}
	if (fd_latex_log->LaTeXLog->visible) {
		fl_hide_form(fd_latex_log->LaTeXLog);
	}
	if (fd_form_spell_check) {
		if (fd_form_spell_check->form_spell_check->visible) {
			fl_trigger_object(fd_form_spell_check->done);
		}
	}
	if (fd_form_bullet) {
		if (fd_form_bullet->form_bullet->visible) {
			fl_hide_form(fd_form_bullet->form_bullet);
		}
	}
	if (fd_panel) {
		if (fd_panel->panel->visible) {
			fl_hide_form(fd_panel->panel);
		}
	}
	if (fd_delim) {
		if (fd_delim->delim->visible) {
			fl_hide_form(fd_delim->delim);
		}
	}
	if (fd_deco) {
		if (fd_deco->deco->visible) {
			fl_hide_form(fd_deco->deco);
		}
	}
	if (fd_space) {
		if (fd_space->space->visible) {
			fl_hide_form(fd_space->space);
		}
	}
	if (fd_matrix) {
		if (fd_matrix->matrix->visible) {
			fl_hide_form(fd_matrix->matrix);
		}
	}
	if (citation_form) {
		if (citation_form->citation_form->visible) {
			fl_hide_form(citation_form->citation_form);
		}
	}
	if (bibitem_form) {
		if (bibitem_form->bibitem_form->visible) {
			fl_hide_form(bibitem_form->bibitem_form);
		}
	}
	if (form) {
		if (form->include->visible) {
			fl_hide_form(form->include);
		}
	}
	if (index_form) {
		if (index_form->index_form->visible) {
			fl_hide_form(index_form->index_form);
		}
	}
	HideFiguresPopups();
}

// This is another function we really don't want.
// Again the Signal/Slot mechanism is tailor made for this task.
void updateAllVisibleBufferRelatedPopups()
{
	if (fd_form_document->form_document->visible) {
		UpdateLayoutDocument();
	}
	if (fd_form_preamble->form_preamble->visible) {
		UpdateLayoutPreamble();
	}
	if (fd_form_quotes->form_quotes->visible) {
		UpdateLayoutQuotes();
	}
	if (fd_form_paragraph->form_paragraph->visible) {
		UpdateLayoutParagraph();
	}
	if (fd_form_paragraph_extra->form_paragraph_extra->visible) {
		UpdateParagraphExtra();
	}
	if (fd_form_paper->form_paper->visible) {
		UpdateLayoutPaper();
	}
	if (fd_form_table_options->form_table_options->visible) {
		UpdateLayoutTable(1); // just like a right mouse click
	}
	if (fd_form_bullet) {
		if (fd_form_bullet->form_bullet->visible) {
			updateBulletForm();
		}
	}
	if (fd_form_toc->browser_toc->visible) {
		TocUpdateCB(0,0);
	}
#ifdef ALWAYS_UPDATE_REF
	// Ideally, the RefUpdateCB should be modified so that if the
	// current document is a subdocument the reference list should come
	// from master document (or even be a merger of references from all
	// its sibling documents).  For now this has effectively been
	// commented out to ensure that users of multipart documents can
	// access the reference list of a different document.  This is
	// of course a compromise and an annoyance for users switching
	// between separate documents.  ARRae 241198
	if (fd_form_ref->form_ref->visible) {
		RefUpdateCB(0,0);
	}
#endif
	if (current_view->currentBuffer()->isReadonly()) {
		// a little crude perhaps but it works. ARRae
		if (fd_form_character->form_character->visible) {
			fl_hide_form(fd_form_character->form_character);
		}
		if (fd_form_table->form_table->visible) {
			fl_hide_form(fd_form_table->form_table);
		}
	}

	// We have either changed buffers or changed the readonly status
	// so the safest thing to do is hide all inset popups that
	// are editting insets from the previous buffer or aren't
	// allowed in readonly docs.
	if (fd_panel) {
		if (fd_panel->panel->visible) {
			fl_hide_form(fd_panel->panel);
		}
	}
	if (fd_delim) {
		if (fd_delim->delim->visible) {
			fl_hide_form(fd_delim->delim);
		}
	}
	if (fd_deco) {
		if (fd_deco->deco->visible) {
			fl_hide_form(fd_deco->deco);
		}
	}
	if (fd_space) {
		if (fd_space->space->visible) {
			fl_hide_form(fd_space->space);
		}
	}
	if (fd_matrix) {
		if (fd_matrix->matrix->visible) {
			fl_hide_form(fd_matrix->matrix);
		}
	}
	if (citation_form) {
		if (citation_form->citation_form->visible) {
			fl_hide_form(citation_form->citation_form);
		}
	}
	if (bibitem_form) {
		if (bibitem_form->bibitem_form->visible) {
			fl_hide_form(bibitem_form->bibitem_form);
		}
	}
	if (form) {
		if (form->include->visible) {
			fl_hide_form(form->include);
		}
	}
	if (index_form) {
		if (index_form->index_form->visible) {
			fl_hide_form(index_form->index_form);
		}
	}
	HideFiguresPopups();
}

// Extract shortcut from <ident>|<shortcut> string
const char* flyx_shortcut_extract(const char*sc)
{
	// Find '|' in the sc and return the string after that.
	register const char *sd = sc;
	while(sd[0]!=0 && sd[0] != '|') sd++;

	if (sd[0]=='|') {
		sd++;
		//lyxerr << sd << endl;
		return sd;
	}
	return "";
}

// Extract identifier from <ident>|<shortcut> string
const char* flyx_ident_extract(const char *sc)
{
	register const char *se = sc;
	while(se[0]!=0 && se[0] != '|') se++;

	if (se[0]==0) return sc;
	
	char * sb = new char[se-sc + 1];
	int index = 0;
	register const char *sd = sc;
	while (sd != se) {
		sb[index] = sd[0];
		index++; sd++;
	}
	sb[index]=0;
	return sb;
}

//
void WriteAlert(string const & s1, string const & s2, string const & s3)
{
	if (minibuffer) {
		ProhibitInput();
		minibuffer->Set(s1, s2, s3);
	}

	fl_set_resource("flAlert.dismiss.label", _("Dismiss"));
	fl_show_alert(s1.c_str(), s2.c_str(), s3.c_str(), 0);
	if (minibuffer) {
		AllowInput();
	}
}


// Alarms user of something related to files
void WriteFSAlert(string const & s1, string const & s2)
{
	WriteAlert (s1, s2, strerror(errno));
}


bool AskQuestion(string const & s1, string const & s2, string const & s3)
{
	fl_set_resource("flQuestion.yes.label", idex(_("Yes|Yy#y")));
	fl_set_resource("flQuestion.no.label", idex(_("No|Nn#n")));
#if FL_REVISION > 85
	return fl_show_question((s1 + "\n" + s2 + "\n" + s3).c_str(), 0);
#else
	return fl_show_question(s1.c_str(), s2.c_str(), s3.c_str());
#endif
}

// Returns 1 for yes, 2 for no, 3 for cancel.
int AskConfirmation(string const & s1, string const & s2, string const & s3)
{
	fl_set_choices_shortcut(scex(_("Yes|Yy#y")),
				scex(_("No|Nn#n")),
				scex(_("Cancel|^[")));
#if FL_REVISION < 86
        return fl_show_choice(s1.c_str(), s2.c_str(), s3.c_str(), 
			      3, idex(_("Yes|Yy#y")),
			      idex(_("No|Nn#n")),
			      idex(_("Cancel|^[")));
#endif
#if FL_REVISION > 85
        return fl_show_choice(s1.c_str(), s2.c_str(), s3.c_str(), 
			      3, idex(_("Yes|Yy#y")),
			      idex(_("No|Nn#n")),
                              idex(_("Cancel|^[")), 3);
#endif
}


// Asks for a text
string askForText(string const & msg, string const & dflt)
{
	string tmp;
	fl_set_resource("flInput.cancel.label", idex(_("Cancel|^[")));
	fl_set_resource("flInput.ok.label", idex(_("OK|#O")));
	fl_set_resource("flInput.clear.label", idex(_("Clear|#e")));
	tmp = fl_show_input(msg.c_str(), dflt.c_str());
	return tmp;
}

// Inform the user that the buffer is read-only, and that changes in the
// dialog box that is to appear will be ignored.
void WarnReadonly()
{
	WriteAlert(_("Any changes will be ignored"),
		   _("The document is read-only:"),
		   current_view->currentBuffer()->getFileName());
}

