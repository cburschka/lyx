/**
 * \file ControlPrefs.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include <utility>
 
#include "ControlPrefs.h"
#include "ViewBase.h"

#include "frontends/LyXView.h"
#include "helper_funcs.h"
#include "gettext.h"
#include "support/filetools.h"
#include "frontends/Dialogs.h"
#include "converter.h"
#include "debug.h"

extern string system_lyxdir;
extern string user_lyxdir;
 
using std::endl;
using std::pair;
 
ControlPrefs::ControlPrefs(LyXView & lv, Dialogs & d)
	: ControlDialogBI(lv, d)
{}


void ControlPrefs::update()
{
	rc_ = lyxrc;
	view().update();
}


void ControlPrefs::apply()
{
	view().apply();
	lyxrc = rc_; 
}


void ControlPrefs::OKButton()
{
	ControlDialogBI::OKButton();
	lv_.dispatch(FuncRequest(LFUN_SAVEPREFERENCES));
}

 
string const ControlPrefs::browsebind(string const & file)
{
	string dir  = AddName(system_lyxdir, "bind");
	// FIXME: stupid name
	string name = _("System Bind|#S#s");
	pair<string,string> dir1(name, dir);
 
	dir = AddName(user_lyxdir, "bind");
	// FIXME: stupid name
	name = _("User Bind|#U#u");
	pair<string,string> dir2(name, dir);
 
	return browseFile(&lv_, file, _("Choose bind file"), "*.bind", dir1, dir2);
}


string const ControlPrefs::browseUI(string const & file)
{
	string dir  = AddName(system_lyxdir, "ui");
	// FIXME: stupid name
	string name = _("Sys UI|#S#s");
	pair<string,string> dir1(name, dir);
 
	dir = AddName(user_lyxdir, "ui");
	// FIXME: stupid name
	name = _("User UI|#U#u");
	pair<string,string> dir2(name, dir);
 
	return browseFile(&lv_, file, _("Choose UI file"), "*.ui", dir1, dir2);
}

 
string const ControlPrefs::browsekbmap(string const & file)
{
	string const dir = AddName(system_lyxdir, "kbd");
	string const name = _("Key maps|#K#k");
	pair<string, string> dir1(name, dir);
 
	return browseFile(&lv_, file, _("Choose keyboard map"), "*.kmap", dir1);
}

 
string const ControlPrefs::browsedict(string const & file)
{
	return browseFile(&lv_, file, _("Choose personal dictionary"), "*.ispell");
}
 
 
string const ControlPrefs::browse(string const & file, string const & title)
{
	return browseFile(&lv_, file, title, "*");
}


void ControlPrefs::redrawGUI()
{
	// we must be sure to get the new values first
	lyxrc = rc_;
 
	lv_.getDialogs().redrawGUI();
}

 
void ControlPrefs::setColor(LColor::color col, string const & hex)
{
	string const s = lcolor.getLyXName(col) + string(" ") + hex;
	lv_.dispatch(FuncRequest(LFUN_SET_COLOR, s));
}


void ControlPrefs::updateScreenFonts()
{
	// we must be sure to get the new values first
	lyxrc = rc_;
 
	lv_.dispatch(FuncRequest(LFUN_SCREEN_FONT_UPDATE));
}
 
 
void ControlPrefs::setConverters(Converters const & conv)
{ 
	converters = conv;
	converters.update(formats);
	converters.buildGraph();
}


void ControlPrefs::setFormats(Formats const & form)
{
	formats = form;
}
