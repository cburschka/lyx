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
#include "Kernel.h"

#include "bufferlist.h"
#include "gettext.h"
#include "funcrequest.h"
#include "LColor.h"

#include "support/filetools.h"
#include "support/globbing.h"
#include "support/path_defines.h"

#include "support/std_sstream.h"

using lyx::support::AddName;
using lyx::support::FileFilterList;
using lyx::support::system_lyxdir;
using lyx::support::user_lyxdir;

using std::ostringstream;
using std::pair;
using std::string;
using std::vector;


extern BufferList bufferlist;

ControlPrefs::ControlPrefs(Dialog & parent)
	: Dialog::Controller(parent),
	  redraw_gui_(false),
	  update_screen_font_(false)
{}


bool ControlPrefs::initialiseParams(std::string const &)
{
	rc_ = lyxrc;
	formats_ = ::formats;
	converters_ = ::converters;
	converters_.update(formats_);
	colors_.clear();
	redraw_gui_ = false;
	update_screen_font_ = false;

	return true;
}


void ControlPrefs::dispatchParams()
{
	ostringstream ss;
	rc_.write(ss);
	kernel().dispatch(FuncRequest(LFUN_LYXRC_APPLY, ss.str()));

	// FIXME: these need lfuns
	bufferlist.setCurrentAuthor(rc_.user_name, rc_.user_email);

	::formats = formats_;

	::converters = converters_;
	::converters.update(::formats);
	::converters.buildGraph();

	vector<string>::const_iterator it = colors_.begin();
	vector<string>::const_iterator const end = colors_.end();
	for (; it != end; ++it)
		kernel().dispatch(FuncRequest(LFUN_SET_COLOR, *it));
	colors_.clear();

	if (redraw_gui_) {
		kernel().redrawGUI();
		redraw_gui_ = false;
	}

	if (update_screen_font_) {
		kernel().dispatch(FuncRequest(LFUN_SCREEN_FONT_UPDATE));
		update_screen_font_ = false;
	}

	// The Save button has been pressed
	if (dialog().isClosing()) {
		kernel().dispatch(FuncRequest(LFUN_SAVEPREFERENCES));
	}
}


void ControlPrefs::redrawGUI()
{
	redraw_gui_ = true;
}


void ControlPrefs::setColor(LColor_color col, string const & hex)
{
	colors_.push_back(lcolor.getLyXName(col) + ' ' + hex);
}


void ControlPrefs::updateScreenFonts()
{
	update_screen_font_ = true;
}


string const ControlPrefs::browsebind(string const & file) const
{
	pair<string,string> dir1(_("System Bind|#S#s"),
				 AddName(system_lyxdir(), "bind"));

	pair<string,string> dir2(_("User Bind|#U#u"),
				 AddName(user_lyxdir(), "bind"));

	return browseFile(file, _("Choose bind file"),
			  FileFilterList("*.bind"), false, dir1, dir2);
}


string const ControlPrefs::browseUI(string const & file) const
{
	pair<string,string> const dir1(_("Sys UI|#S#s"),
				       AddName(system_lyxdir(), "ui"));

	pair<string,string> const dir2(_("User UI|#U#u"),
				       AddName(user_lyxdir(), "ui"));

	return browseFile(file, _("Choose UI file"),
			  FileFilterList("*.ui"), false, dir1, dir2);
}


string const ControlPrefs::browsekbmap(string const & file) const
{
	pair<string, string> dir(_("Key maps|#K#k"),
				 AddName(system_lyxdir(), "kbd"));

	return browseFile(file, _("Choose keyboard map"),
			  FileFilterList("*.kmap"), false, dir);
}


string const ControlPrefs::browsedict(string const & file) const
{
	return browseFile(file, _("Choose personal dictionary"),
			  FileFilterList("*.ispell"));
}


string const ControlPrefs::browse(string const & file,
				  string const & title) const
{
	return browseFile(file, title, FileFilterList(), true);
}


string const ControlPrefs::browsedir(string const & path,
				     string const & title) const
{
	return browseDir(path, title);
}
