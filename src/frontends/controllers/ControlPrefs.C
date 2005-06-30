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

#include "support/filefilterlist.h"

#include <sstream>

using std::ostringstream;
using std::pair;
using std::string;
using std::vector;


extern BufferList bufferlist;

namespace lyx {

using support::FileFilterList;

namespace frontend {


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
	movers_ = ::movers;
	colors_.clear();
	redraw_gui_ = false;
	update_screen_font_ = false;

	return true;
}


void ControlPrefs::dispatchParams()
{
	ostringstream ss;
	rc_.write(ss, true);
	kernel().dispatch(FuncRequest(LFUN_LYXRC_APPLY, ss.str()));

	// FIXME: these need lfuns
	bufferlist.setCurrentAuthor(rc_.user_name, rc_.user_email);

	::formats = formats_;

	::converters = converters_;
	::converters.update(::formats);
	::converters.buildGraph();

	::movers = movers_;

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
	return browseLibFile("bind", file, "bind", _("Choose bind file"),
			  FileFilterList(_("LyX bind files (*.bind)")));
}


string const ControlPrefs::browseUI(string const & file) const
{
	return browseLibFile("ui", file, "ui", _("Choose UI file"),
			  FileFilterList(_("LyX UI files (*.ui)")));
}


string const ControlPrefs::browsekbmap(string const & file) const
{
	return browseLibFile("kbd", file, "kmap", _("Choose keyboard map"),
			  FileFilterList(_("LyX keyboard maps (*.kmap)")));
}


string const ControlPrefs::browsedict(string const & file) const
{
	return browseFile(file, _("Choose personal dictionary"),
			  FileFilterList(_("*.ispell")));
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

} // namespace frontend
} // namespace lyx
