/**
 * \file FormTexinfo.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Herbert Voss <voss@lyx.org>
 * \date 2001-10-01
 */

#include <config.h>
#include <fstream>

#ifdef __GNUG__
#pragma implementation
#endif

#include "xformsBC.h"
#include "ControlTexinfo.h"
#include "FormTexinfo.h"
#include "form_texinfo.h"
#include "gettext.h"
#include "debug.h"
#include "helper_funcs.h"
#include "xforms_helpers.h"
#include "support/lstrings.h"
#include "support/filetools.h"
#include "support/LAssert.h"

extern string user_lyxdir; // home of *Files.lst
    
namespace {

string const bstFilename("bstFiles.lst");
string const clsFilename("clsFiles.lst");
string const styFilename("styFiles.lst");
string const errorMessage =_("Missing filelist. try Rescan");

// C function wrapper, required by xforms.
extern "C"
int C_FormTexinfoFeedbackCB(FL_OBJECT * ob, int event,
			    FL_Coord, FL_Coord, int, void *)
{
	// Note that the return value is important in the pre-emptive handler.
	// Don't return anything other than 0.

	lyx::Assert(ob);
	// Don't Assert this one, as it can happen quite reasonably when things
	// are being deleted in the d-tor.
	//Assert(ob->form);
	if (!ob->form) return 0;

	FormTexinfo * pre =
		static_cast<FormTexinfo*>(ob->form->u_vdata);
	pre->feedbackCB(ob, event);
	return 0;
}
	
void setPreHandler(FL_OBJECT * ob)
{
	lyx::Assert(ob);
	fl_set_object_prehandler(ob, C_FormTexinfoFeedbackCB);
}
 
} // namespace anon


typedef FormCB<ControlTexinfo, FormDB<FD_form_texinfo> > base_class;
FormTexinfo::FormTexinfo(ControlTexinfo & c)
	: base_class(c, _("TeX Infos")),
	  warningPosted(false), activeStyle(FormTexinfo::cls)
{}


void FormTexinfo::build() {
	dialog_.reset(build_texinfo());
	// courier medium
	fl_set_browser_fontstyle(dialog_->browser,FL_FIXED_STYLE);
	// with Path is default
	fl_set_button(dialog_->button_fullPath, 1);
	updateStyles(FormTexinfo::cls);

	setPreHandler(dialog_->button_rescan);
	setPreHandler(dialog_->button_view);
	setPreHandler(dialog_->button_texhash);
	setPreHandler(dialog_->button_fullPath);
	setPreHandler(dialog_->browser);
	setPreHandler(dialog_->radio_cls);
	setPreHandler(dialog_->radio_sty);
	setPreHandler(dialog_->radio_bst);
	setPreHandler(dialog_->message);
	setPreHandler(dialog_->help);
}


ButtonPolicy::SMInput FormTexinfo::input(FL_OBJECT * ob, long) {
	if (ob == dialog_->help) {
		controller().help();

	} else if (ob == dialog_->radio_cls) {
		updateStyles(FormTexinfo::cls); 

	} else if (ob == dialog_->radio_sty) {
		updateStyles(FormTexinfo::sty); 

	} else if (ob == dialog_->radio_bst) {
		updateStyles(FormTexinfo::bst); 

	} else if (ob == dialog_->button_rescan) {
		// build new *Files.lst
		controller().rescanStyles();
		updateStyles(activeStyle);

	} else if (ob == dialog_->button_fullPath) {
		setEnabled(dialog_->button_view,
			   fl_get_button(dialog_->button_fullPath));
		updateStyles(activeStyle);

	} else if (ob == dialog_->button_texhash) {
		// makes only sense if the rights are set well for
		// users (/var/lib/texmf/ls-R)
		controller().runTexhash();
		// update files in fact of texhash
		controller().rescanStyles();

	} else if (ob == dialog_->button_view) {
		unsigned int selection = fl_get_browser(dialog_->browser);
		// a valid entry?
		if (selection > 0) {
			controller().viewFile( 
				fl_get_browser_line(dialog_->browser,
						    selection));
		}
	}

	return ButtonPolicy::SMI_VALID;
}


namespace {

string const sortEntries(string & str_in)
{
	std::vector<string> dbase = getVectorFromString(str_in,"\n");
	std::sort(dbase.begin(), dbase.end()); 	// sort entries
	return getStringFromVector(dbase,"\n");
}

string const getContents(string const filename, bool withFullPath)
{
	string fileContents = GetFileContents(AddName(user_lyxdir,filename));
	// everything ok?
	if (!fileContents.empty()) {
		if (withFullPath)
			return(sortEntries(fileContents));
		else {
			int Entries = 1;
			string dummy = OnlyFilename(token(fileContents,'\n',1));
			string contents = dummy;
			do {
				dummy = OnlyFilename(token(fileContents,'\n',++Entries));
				contents += ("\n"+dummy);
			} while (!dummy.empty());
			return(sortEntries(contents));
		}
	} else
		return errorMessage;
}

} // namespace anon


void FormTexinfo::updateStyles(FormTexinfo::texFileSuffix whichStyle)
{
	fl_clear_browser(dialog_->browser); 

	bool const withFullPath = fl_get_button(dialog_->button_fullPath);

	switch (whichStyle) {
	case FormTexinfo::bst: 
	{
		string const str = getContents(bstFilename, withFullPath);
		fl_add_browser_line(dialog_->browser, str.c_str());
		break;

	}
	case FormTexinfo::cls: 
	{
		string const str = getContents(clsFilename, withFullPath);
		fl_add_browser_line(dialog_->browser, str.c_str());
		break;
	}
	case FormTexinfo::sty:
	{
		string const str = getContents(styFilename, withFullPath);
		fl_add_browser_line(dialog_->browser, str.c_str());
		break;
	}
	}

	activeStyle = whichStyle;
}


// preemptive handler for feedback messages
void FormTexinfo::feedbackCB(FL_OBJECT * ob, int event)
{
	lyx::Assert(ob);

	switch (event) {
	case FL_ENTER:
		warningPosted = false;
		feedback(ob);
		break;

	case FL_LEAVE:
		if (!warningPosted) {
			fl_set_object_label(dialog_->message, "");
			fl_redraw_object(dialog_->message);
		}
		break;

	default:
		break;
	}
}


void FormTexinfo::feedback(FL_OBJECT * ob)
{
	lyx::Assert(ob);

	string str;

	if (ob == dialog_->button_rescan) {
		str = _("run rescan ...");

	} else if (ob == dialog_->button_fullPath) {
		str = _("View full path or only file name");

	} else if (ob == dialog_->button_texhash) {
		str = _("run texhash and rescan...");

	} else if (ob == dialog_->button_view) {
		str = _("select a file to view");

	}
	
	fl_set_object_label(dialog_->message, str.c_str());
	fl_redraw_object(dialog_->message);
}
