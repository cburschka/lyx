#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include <unistd.h>

#include FORMS_H_LOCATION
#include "lyxvc.h"
#include "vc-backend.h"
#include "debug.h"
#include "lyx_gui_misc.h"
#include "buffer.h"
#include "gettext.h"
#include "support/filetools.h"
#include "support/lyxlib.h"
#include "lyxfunc.h"
#include "LyXView.h"

using std::endl;
using std::pair;

LyXVC::LyXVC()
{
	vcs = 0;
	browser = 0;
	owner_ = 0;
}


LyXVC::~LyXVC()
{
	if (browser) {
		if (browser->LaTeXLog->visible)
			fl_hide_form(browser->LaTeXLog);
		fl_free_form(browser->LaTeXLog);
	}
	delete vcs;
}


bool LyXVC::file_found_hook(string const & fn)
{
	string found_file;
	// Check if file is under RCS
	if (!(found_file = RCS::find_file(fn)).empty()) {
		vcs = new RCS(found_file);
		vcs->owner(owner_);
		return true;
	}
	// Check if file is under CVS
	if (!(found_file = CVS::find_file(fn)).empty()) {
		vcs = new CVS(found_file, fn);
		vcs->owner(owner_);
		return true;
	}
	// file is not under any VCS.
	return false;
}


bool LyXVC::file_not_found_hook(string const & fn)
{
	// Check if file is under RCS
	if (!RCS::find_file(fn).empty())
		return true;
	if (!CVS::find_file(fn).empty())
		return true;
	return false;
}


void LyXVC::buffer(Buffer * buf)
{
	owner_ = buf;
}


void LyXVC::registrer()
{
	// it is very likely here that the vcs is not created yet...
	// so... we use RCS as default, later this should perhaps be
	// a lyxrc option.
	if (!vcs) {
		vcs = new RCS(owner_->fileName());
		vcs->owner(owner_);
	}
	
	// If the document is changed, we might want to save it
	if (!vcs->owner()->isLyxClean() && 
	    AskQuestion(_("Changes in document:"),
			MakeDisplayPath(vcs->owner()->fileName(), 50),
			_("Save document and proceed?"))) {
		vcs->owner()->getUser()->owner()
			->getLyXFunc()->Dispatch(LFUN_MENUWRITE);
	}

	// Maybe the save fails, or we answered "no". In both cases,
	// the document will be dirty, and we abort.
	if (!vcs->owner()->isLyxClean()) {
		return;
	}

	lyxerr[Debug::LYXVC] << "LyXVC: registrer" << endl;
	pair<bool, string> tmp =
		askForText(_("LyX VC: Initial description"),
			   _("(no initial description)"));
	if (!tmp.first || tmp.second.empty()) {
		// should we insist on checking tmp.second.empty()?
		lyxerr[Debug::LYXVC] << "LyXVC: user cancelled" << endl;
		WriteAlert(_("Info"),
			   _("This document has NOT been registered."));
		return;
	}
	
	vcs->registrer(tmp.second);
}


void LyXVC::checkIn()
{
	// If the document is changed, we might want to save it
	if (!vcs->owner()->isLyxClean() && 
	    AskQuestion(_("Changes in document:"),
			MakeDisplayPath(vcs->owner()->fileName(), 50),
			_("Save document and proceed?"))) {
		vcs->owner()->getUser()->owner()
			->getLyXFunc()->Dispatch(LFUN_MENUWRITE);
	}

	// Maybe the save fails, or we answered "no". In both cases,
	// the document will be dirty, and we abort.
	if (!vcs->owner()->isLyxClean()) {
		return;
	}

	lyxerr[Debug::LYXVC] << "LyXVC: checkIn" << endl;
	pair<bool, string> tmp = askForText(_("LyX VC: Log Message"));
	if (tmp.first) {
		if (tmp.second.empty()) {
			tmp.second = _("(no log message)");
		}
		vcs->checkIn(tmp.second);
	} else {
		lyxerr[Debug::LYXVC] << "LyXVC: user cancelled" << endl;
	}
}


void LyXVC::checkOut()
{
	lyxerr[Debug::LYXVC] << "LyXVC: checkOut" << endl;
	if (!vcs->owner()->isLyxClean() 
	    && !AskQuestion(_("Changes in document:"),
			   MakeDisplayPath(vcs->owner()->fileName(), 50),
			   _("Ignore changes and proceed with check out?"))) {
		return;
	}

	vcs->checkOut();
	
}


void LyXVC::revert()
{
	lyxerr[Debug::LYXVC] << "LyXVC: revert" << endl;
	// Here we should check if the buffer is dirty. And if it is
	// we should warn the user that reverting will discard all
	// changes made since the last check in.
	if (AskQuestion(_("When you revert, you will loose all changes made"),
			_("to the document since the last check in."),
			_("Do you still want to do it?"))) {

		vcs->revert();
	}
}


void LyXVC::undoLast()
{
	vcs->undoLast();
}


void LyXVC::toggleReadOnly()
{
	switch (vcs->status()) {
	case VCS::UNLOCKED:
		lyxerr[Debug::LYXVC] << "LyXVC: toggle to locked" << endl;
		checkOut();
		break;
	case VCS::LOCKED:
		lyxerr[Debug::LYXVC] << "LyXVC: toggle to unlocked" << endl;
		checkIn();
		break;
	}
}


bool LyXVC::inUse()
{
	if (vcs) return true;
	return false;
}


string const & LyXVC::version() const
{
	return vcs->version();
}


string const & LyXVC::locker() const
{
	return vcs->locker();
}


// This is a hack anyway so I'll put it here in the mean time.
void LyXVC::logClose(FL_OBJECT * obj, long)
{
	LyXVC * This = static_cast<LyXVC*>(obj->form->u_vdata);
	fl_hide_form(This->browser->LaTeXLog);
}


// and, hack over hack, here is a C wrapper :)
extern "C" void C_LyXVC_logClose(FL_OBJECT * ob, long data)
{
	LyXVC::logClose(ob, data);
}


void LyXVC::logUpdate(FL_OBJECT * obj, long)
{
	LyXVC * This = static_cast<LyXVC*>(obj->form->u_vdata);
	This->showLog();
}

extern "C" void C_LyXVC_logUpdate(FL_OBJECT *ob, long data)
{
	LyXVC::logUpdate(ob, data);
}


void LyXVC::viewLog(string const & fil)
{
	static int ow = -1, oh;

	if (!browser) {
		FL_OBJECT * obj;
		browser = (FD_LaTeXLog *) fl_calloc(1, sizeof(*browser));
		
		browser->LaTeXLog = fl_bgn_form(FL_NO_BOX, 470, 380);
		browser->LaTeXLog->u_vdata = this;
		obj = fl_add_box(FL_UP_BOX, 0, 0, 470, 380, "");
		browser->browser_latexlog = fl_add_browser(FL_NORMAL_BROWSER,
							   10, 10,
							   450, 320, "");
		obj = fl_add_button(FL_RETURN_BUTTON, 270, 340, 90, 30,
				    _("Close"));
		fl_set_object_lsize(obj, FL_NORMAL_SIZE);
		fl_set_object_callback(obj, C_LyXVC_logClose, 0);
		obj = fl_add_button(FL_NORMAL_BUTTON, 370, 340, 90, 30,
				    idex(_("Update|#Uu")));
		fl_set_button_shortcut(obj, scex(_("Update|#Uu")), 1);
		fl_set_object_lsize(obj, FL_NORMAL_SIZE);
		fl_set_object_callback(obj, C_LyXVC_logUpdate, 0);
		fl_end_form();
		fl_set_form_atclose(browser->LaTeXLog, CancelCloseBoxCB, 0);
	}

	if (!fl_load_browser(browser->browser_latexlog, fil.c_str()))
		fl_add_browser_line(browser->browser_latexlog,
				    _("No VC History!"));
	
	if (browser->LaTeXLog->visible) {
		fl_raise_form(browser->LaTeXLog);
	} else {
		fl_show_form(browser->LaTeXLog,
			     FL_PLACE_MOUSE | FL_FREE_SIZE, FL_TRANSIENT,
			     _("VC History"));
		if (ow < 0) {
			ow = browser->LaTeXLog->w;
			oh = browser->LaTeXLog->h;
		}
		fl_set_form_minsize(browser->LaTeXLog, ow, oh);
	}
}


void LyXVC::showLog()
{
	string tmpf = lyx::tempName(string(), "lyxvclog");
	vcs->getLog(tmpf);
	viewLog(tmpf);
	lyx::unlink(tmpf);
}
