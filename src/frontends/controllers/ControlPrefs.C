/**
 * \file ControlPrefs.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlPrefs.h"

#include "helper_funcs.h"
#include "ViewBase.h"

#include "bufferlist.h"
#include "converter.h"
#include "format.h"
#include "gettext.h"
#include "funcrequest.h"
#include "LColor.h"
#include "lfuns.h"

#include "frontends/Dialogs.h"
#include "frontends/LyXView.h"

#include "support/filetools.h"
#include "support/path_defines.h"

#include <utility>

using lyx::support::AddName;
using lyx::support::system_lyxdir;
using lyx::support::user_lyxdir;

using std::pair;
using std::string;


extern BufferList bufferlist;

ControlPrefs::ControlPrefs(LyXView & lv, Dialogs & d)
	: ControlDialogBI(lv, d)
{}


void ControlPrefs::setParams()
{
	rc_ = lyxrc;
}


void ControlPrefs::apply()
{
	view().apply();
	lyxrc = rc_;

	bufferlist.setCurrentAuthor(rc_.user_name, rc_.user_email);

	// The Save button has been pressed
	if (isClosing()) {
		lv_.dispatch(FuncRequest(LFUN_SAVEPREFERENCES));
	}
}


string const ControlPrefs::browsebind(string const & file)
{
	string dir  = AddName(system_lyxdir(), "bind");
	// FIXME: stupid name
	string name = _("System Bind|#S#s");
	pair<string,string> dir1(name, dir);

	dir = AddName(user_lyxdir(), "bind");
	// FIXME: stupid name
	name = _("User Bind|#U#u");
	pair<string,string> dir2(name, dir);

	return browseFile(file, _("Choose bind file"), "*.bind", false, dir1, dir2);
}


string const ControlPrefs::browseUI(string const & file)
{
	string dir  = AddName(system_lyxdir(), "ui");
	// FIXME: stupid name
	string name = _("Sys UI|#S#s");
	pair<string,string> dir1(name, dir);

	dir = AddName(user_lyxdir(), "ui");
	// FIXME: stupid name
	name = _("User UI|#U#u");
	pair<string,string> dir2(name, dir);

	return browseFile(file, _("Choose UI file"), "*.ui", false, dir1, dir2);
}


string const ControlPrefs::browsekbmap(string const & file)
{
	string const dir = AddName(system_lyxdir(), "kbd");
	string const name = _("Key maps|#K#k");
	pair<string, string> dir1(name, dir);

	return browseFile(file, _("Choose keyboard map"), "*.kmap", false, dir1);
}


string const ControlPrefs::browsedict(string const & file)
{
	return browseFile(file, _("Choose personal dictionary"), "*.ispell");
}


string const ControlPrefs::browse(string const & file, string const & title)
{
	return browseFile(file, title, "*", true);
}


string const ControlPrefs::browsedir(string const & path, string const & title)
{
	return browseDir(path, title);
}


void ControlPrefs::redrawGUI()
{
	// we must be sure to get the new values first
	lyxrc = rc_;

	lv_.getDialogs().redrawGUI();
}


void ControlPrefs::setColor(LColor_color col, string const & hex)
{
	string const s = lcolor.getLyXName(col) + ' ' + hex;
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
