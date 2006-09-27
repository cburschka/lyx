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

#include "gettext.h"
#include "funcrequest.h"
#include "paper.h"
#include "LColor.h"

#include "frontends/Application.h"

#include "support/filefilterlist.h"

#include <sstream>

using std::ostringstream;
using std::pair;
using std::string;
using std::vector;


namespace lyx {

using support::FileFilterList;

namespace frontend {


ControlPrefs::ControlPrefs(Dialog & parent)
	: Dialog::Controller(parent),
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
	update_screen_font_ = false;

	return true;
}


void ControlPrefs::dispatchParams()
{
	ostringstream ss;
	rc_.write(ss, true);
	kernel().dispatch(FuncRequest(LFUN_LYXRC_APPLY, ss.str()));

	// FIXME: these need lfuns
	theApp->bufferList().setCurrentAuthor(rc_.user_name, rc_.user_email);

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

	if (update_screen_font_) {
		kernel().dispatch(FuncRequest(LFUN_SCREEN_FONT_UPDATE));
		update_screen_font_ = false;
	}

	// The Save button has been pressed
	if (dialog().isClosing()) {
		kernel().dispatch(FuncRequest(LFUN_PREFERENCES_SAVE));
	}
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
	// FIXME UNICODE
	return browseLibFile("bind", file, "bind",
			     lyx::to_utf8(_("Choose bind file")),
			     FileFilterList(lyx::to_utf8(_("LyX bind files (*.bind)"))));
}


string const ControlPrefs::browseUI(string const & file) const
{
	// FIXME UNICODE
	return browseLibFile("ui", file, "ui",
			     lyx::to_utf8(_("Choose UI file")),
			     FileFilterList(lyx::to_utf8(_("LyX UI files (*.ui)"))));
}


string const ControlPrefs::browsekbmap(string const & file) const
{
	// FIXME UNICODE
	return browseLibFile("kbd", file, "kmap",
			     lyx::to_utf8(_("Choose keyboard map")),
			     FileFilterList(lyx::to_utf8(_("LyX keyboard maps (*.kmap)"))));
}


string const ControlPrefs::browsedict(string const & file) const
{
	// FIXME UNICODE
	return browseFile(file,
			  lyx::to_utf8(_("Choose personal dictionary")),
			  FileFilterList(lyx::to_utf8(_("*.ispell"))));
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


// We support less paper sizes than the document dialog
// Therefore this adjustment is needed.
PAPER_SIZE const ControlPrefs::toPaperSize(int i) const
{
	switch (i) {
	case 0:
		return PAPER_DEFAULT;
	case 1:
		return PAPER_USLETTER;
	case 2:
		return PAPER_USLEGAL;
	case 3:
		return PAPER_USEXECUTIVE;
	case 4:
		return PAPER_A3;
	case 5:
		return PAPER_A4;
	case 6:
		return PAPER_A5;
	case 7:
		return PAPER_B5;
	default:
		// should not happen
		return PAPER_DEFAULT;
	}
}


int const ControlPrefs::fromPaperSize(PAPER_SIZE papersize) const
{
	switch (papersize) {
	case PAPER_DEFAULT:
		return 0;
	case PAPER_USLETTER:
		return 1;
	case PAPER_USLEGAL:
		return 2;
	case PAPER_USEXECUTIVE:
		return 3;
	case PAPER_A3:
		return 4;
	case PAPER_A4:
		return 5;
	case PAPER_A5:
		return 6;
	case PAPER_B5:
		return 7;
	default:
		// should not happen
		return 0;
	}
}

} // namespace frontend
} // namespace lyx
