#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include FORMS_H_LOCATION
#include "lyxvc.h"
#include "error.h"
#include "lyx_gui_misc.h"
#include "bufferlist.h"
#include "syscall.h"
#include "pathstack.h"
#include "filetools.h"
#include "FileInfo.h"
#include "gettext.h"
#include "LyXView.h"
#include "lyxfunc.h"
#include "latexoptions.h"

extern BufferList bufferlist;
extern void MenuWrite(Buffer *);

#if 0
extern bool gsworking();
#endif

LyXVC::LyXVC()
{
	backend = UNKNOWN_VCS;
	_owner = 0;
	browser = 0;
}


LyXVC::~LyXVC()
{
	if (browser) {
		if (browser->LaTeXLog->visible)
			fl_hide_form(browser->LaTeXLog);
		fl_free_form(browser->LaTeXLog);
	}
}


bool LyXVC::file_found_hook(LString const & fn)
{
	LString tmp(fn);
	FileInfo f;
	// Check if *,v exists.
	tmp += ",v";
	lyxerr.debug(LString("Checking if file is under vc: ") + tmp, Error::LYXVC);
	if (f.newFile(tmp).readable()) {
		lyxerr.debug("Yes it is under vc.", Error::LYXVC);
		master = tmp;
		backend = RCS_VCS;
		scanMaster();
		return true;
	} else {
		// Check if RCS/*,v exists.
		tmp = AddName(AddPath(OnlyPath(fn), "RCS"), fn);
		tmp += ",v";
		lyxerr.debug("Checking if file is under vc: " + tmp, Error::LYXVC);
		if (f.newFile(tmp).readable()) {
			lyxerr.debug("Yes it is under vc.", Error::LYXVC);
			master = tmp;
			backend = RCS_VCS;
			scanMaster();
			return true;
		}
	}
	// If either one, return true

	// file is not under any VCS.
	return false;
}


bool LyXVC::file_not_found_hook(LString const &)
{
	// file is not under any VCS.
	return false;
}


void LyXVC::scanMaster()
{
	lyxerr.debug("LyXVC: This file is a VC file.", Error::LYXVC);

	LyXLex lex(NULL, 0);
	lex.setFile(master);

	LString token;
	bool read_enough = false;
	while (lex.IsOK() && !read_enough) {
		lex.next();
		token = lex.GetString();
		
		lyxerr.debug("LyXVC::scanMaster: current lex text: `"
				+token+"'", Error::LYXVC);

		if (token.empty())
			continue;
		else if (token == "head") {
			// get version here
			lex.next();
			LString tmv = lex.GetString();
			tmv.strip(';');
			version = tmv;
		} else if (token.contains("access")
			   || token.contains("symbols")
			   || token.contains("strict")) {
			// nothing
		} else if (token.contains("locks")) {
			// get locker here
			if (token.contains(";")) {
				locker = "Unlocked";
				vcstat = UNLOCKED;
				continue;
			}
			LString tmpt, s1, s2;
			do {
				lex.next();
				tmpt = lex.GetString();
				s1 = tmpt;
				s1.strip(';');
				// tmp is now in the format <user>:<version>
				s1.split(s2, ':');
				// s2 is user, and s1 is version
				if (s1 == version) {
					locker = s2;
					vcstat = LOCKED;
					break;
				}
			} while (!tmpt.contains(";"));
			
		} else if (token == "comment") {
			// we don't need to read any further than this.
			read_enough = true;
		} else {
			// unexpected
			lyxerr.debug("LyXVC::scanMaster(): unexpected token", Error::LYXVC);
		}
	}
}


void LyXVC::setBuffer(Buffer *buf)
{
	_owner = buf;
}


//
// I will probably add some backend_xxxx functions later to perform the
// version control system specific commands. Something like:
// void backend_revert(<params>) {
//        if (backend == "RCS") {
//        } else if (backend == "CVS") {
//        } else if (backend == "SCCS") {
//        }
//
// But for 0.12 we will only support RCS.
//

void LyXVC::registrer()
{
	// If the document is changed, we might want to save it
	if (!_owner->isLyxClean() && 
	    AskQuestion(_("Changes in document:"),
			MakeDisplayPath(_owner->getFileName(),50),
			_("Save document and proceed?"))) {
		MenuWrite(_owner);
	}

	// Maybe the save fails, or we answered "no". In both cases,
	// the document will be dirty, and we abort.
	if (!_owner->isLyxClean()) {
		return;
	}

	lyxerr.debug("LyXVC: registrer", Error::LYXVC);
	LString tmp = askForText(_("LyX VC: Initial description"),
				 _("(no initial description)"));
	if (tmp.empty()) {
		lyxerr.debug("LyXVC: user cancelled", Error::LYXVC);
		WriteAlert(_("Info"), _("This document has NOT been registered."));
		return;
	}
	LString cmd = "ci -q -u -i -t-\"";
	cmd += tmp;
	cmd += "\" \"";
	cmd += OnlyFilename(_owner->getFileName());
	cmd += "\"";
	doVCCommand(cmd);
	_owner->getUser()->getOwner()->getLyXFunc()->Dispatch("buffer-reload");
}


void LyXVC::checkIn()
{
	// If the document is changed, we might want to save it
	if (!_owner->isLyxClean() && 
	    AskQuestion(_("Changes in document:"),
			MakeDisplayPath(_owner->getFileName(),50),
			_("Save document and proceed?"))) {
		MenuWrite(_owner);
	}

	// Maybe the save fails, or we answered "no". In both cases,
	// the document will be dirty, and we abort.
	if (!_owner->isLyxClean()) {
		return;
	}

	lyxerr.debug("LyXVC: checkIn", Error::LYXVC);
	_owner->getUser()->getOwner()->getLyXFunc()->Dispatch(LFUN_MENUWRITE);
	LString tmp = askForText(_("LyX VC: Log Message"));
	if (tmp.empty()) tmp = "(no log msg)";
	doVCCommand("ci -q -u -m\"" + tmp + "\" \""
		    + OnlyFilename(_owner->getFileName()) + "\"");
	_owner->getUser()->getOwner()->getLyXFunc()->Dispatch("buffer-reload");
}


void LyXVC::checkOut()
{
	lyxerr.debug("LyXVC: checkOut", Error::LYXVC);
	if (!_owner->isLyxClean() 
	    && !AskQuestion(_("Changes in document:"),
			   MakeDisplayPath(_owner->getFileName(),50),
			   _("Ignore changes and proceed with check out?"))) {
		return;
	}

	_owner->markLyxClean();
	doVCCommand("co -q -l \""
		    + OnlyFilename(_owner->getFileName()) + "\"");
	_owner->getUser()->getOwner()->getLyXFunc()->Dispatch("buffer-reload");
}


void LyXVC::revert()
{
	lyxerr.debug("LyXVC: revert", Error::LYXVC);
	// Here we should check if the buffer is dirty. And if it is
	// we should warn the user that reverting will discard all
	// changes made since the last check in.
	if (AskQuestion(_("When you revert, you will loose all changes made"),
			_("to the document since the last check in."),
			_("Do you still want to do it?"))) {
		
		doVCCommand("co -f -u" + getVersion() + " \""
			    + OnlyFilename(_owner->getFileName()) + "\"");
		// We ignore changes and just reload!
		_owner->markLyxClean();
		_owner->getUser()->getOwner()->
			getLyXFunc()->Dispatch("buffer-reload");
	}
}


void LyXVC::undoLast()
{
	lyxerr.debug("LyXVC: undoLast", Error::LYXVC);
	doVCCommand("rcs -o" + getVersion() + " \""
		    + OnlyFilename(_owner->getFileName()) + "\"");
}


void LyXVC::toggleReadOnly()
{
	switch (vcstat) {
	case UNLOCKED:
		checkOut();
		break;
	case LOCKED:
		checkIn();
		break;
	}
}


bool LyXVC::inUse()
{
	if (!master.empty())
		return true;
	return false;
}


LString const LyXVC::getVersion() const
{
	return version;
}


LString const LyXVC::getLocker() const
{
	return locker;
}

// This is a hack anyway so I'll put it here in the mean time.
void LyXVC::logClose(FL_OBJECT *obj, long)
{
	LyXVC *This = (LyXVC*)obj->form->u_vdata;
	fl_hide_form(This->browser->LaTeXLog);
}


void LyXVC::logUpdate(FL_OBJECT *obj, long)
{
	LyXVC *This = (LyXVC*)obj->form->u_vdata;
	This->showLog();
}


void LyXVC::viewLog(LString const & fil)
{
	if (!browser) {
		FL_OBJECT *obj;
		browser = (FD_LaTeXLog *) fl_calloc(1, sizeof(*browser));
		
		browser->LaTeXLog = fl_bgn_form(FL_NO_BOX, 470, 380);
		browser->LaTeXLog->u_vdata = (void*)this;
		obj = fl_add_box(FL_UP_BOX, 0, 0, 470, 380, "");
		browser->browser_latexlog = fl_add_browser(FL_NORMAL_BROWSER, 10, 10, 450, 320, "");
		obj = fl_add_button(FL_RETURN_BUTTON, 270, 340, 90, 30, _("Close"));
		fl_set_object_lsize(obj, FL_NORMAL_SIZE);
		fl_set_object_callback(obj, logClose, 0);
		obj = fl_add_button(FL_NORMAL_BUTTON,370,340,90,30,
				    idex(_("Update|#Uu")));
		fl_set_button_shortcut(obj,scex(_("Update|#Uu")),1);
		fl_set_object_lsize(obj,FL_NORMAL_SIZE);
		fl_set_object_callback(obj,logUpdate,0);
		fl_end_form();
		fl_set_form_atclose(browser->LaTeXLog, CancelCloseBoxCB, NULL);
	}

	if (!fl_load_browser(browser->browser_latexlog, fil.c_str()))
		fl_add_browser_line(browser->browser_latexlog, _("No RCS History!"));
	
	if (browser->LaTeXLog->visible) {
		fl_raise_form(browser->LaTeXLog);
	} else {
		fl_show_form(browser->LaTeXLog,
			     FL_PLACE_MOUSE | FL_FREE_SIZE, FL_FULLBORDER,
			     _("RCS History"));
	}
}


void LyXVC::showLog()
{
	// This I really don't like, but we'll look at this problem
	// in 0.13. Then we can make a clean solution.
#if 0
	if (gsworking()) {
                WriteAlert(_("Sorry, can't do this while"
			     " pictures are being rendered."),
                           _("Please wait a few seconds for"
			     " this to finish and try again."),
                           _("(or kill runaway gs processes"
			     " by hand and try again.)"));
                return;
        }
        extern pid_t isp_pid; // from spellchecker.C
        if(isp_pid != -1) {
		WriteAlert(_("Can't do this while the"
			     " spellchecker is running."),
			   _("Stop the spellchecker first."));
		return;
	}
#endif
	LString tmpf = tmpnam(NULL);
	doVCCommand("rlog \""
		    + OnlyFilename(_owner->getFileName()) + "\" > " + tmpf);
	viewLog(tmpf);
	unlink(tmpf.c_str());
}


int LyXVC::doVCCommand(LString const & cmd)
{
	lyxerr.debug("doVCCommand: " + cmd, Error::LYXVC);
        Systemcalls one;
	PathPush(_owner->filepath);
	int ret = one.Startscript(Systemcalls::System, cmd);
	PathPop();
	return ret;
}
