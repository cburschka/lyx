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
#include "paper.h"
#include "LColor.h"

#include "support/filefilterlist.h"

#include <sstream>

using lyx::docstring;

using std::ostringstream;
using std::pair;
using std::string;
using std::vector;
using lyx::support::FileFilterList;

namespace lyx {

namespace frontend {


ControlPrefs::ControlPrefs(Dialog & parent)
	: Dialog::Controller(parent),
	  update_screen_font_(false)
{}


bool ControlPrefs::initialiseParams(std::string const &)
{
	rc_ = lyxrc;
	formats_ = lyx::formats;
	converters_ = lyx::converters;
	converters_.update(formats_);
	movers_ = lyx::movers;
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
	// FIXME UNICODE
	theBufferList().setCurrentAuthor(from_utf8(rc_.user_name), from_utf8(rc_.user_email));

	lyx::formats = formats_;

	lyx::converters = converters_;
	lyx::converters.update(lyx::formats);
	lyx::converters.buildGraph();

	lyx::movers = movers_;

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


docstring const ControlPrefs::browsebind(docstring const & file) const
{
	return browseLibFile(lyx::from_ascii("bind"), file, lyx::from_ascii("bind"),
			     _("Choose bind file"),
			     FileFilterList(_("LyX bind files (*.bind)")));
}


docstring const ControlPrefs::browseUI(docstring const & file) const
{
	return browseLibFile(lyx::from_ascii("ui"), file, lyx::from_ascii("ui"),
			     _("Choose UI file"),
			     FileFilterList(_("LyX UI files (*.ui)")));
}


docstring const ControlPrefs::browsekbmap(docstring const & file) const
{
	return browseLibFile(lyx::from_ascii("kbd"), file, lyx::from_ascii("kmap"),
			     _("Choose keyboard map"),
			     FileFilterList(_("LyX keyboard maps (*.kmap)")));
}


docstring const ControlPrefs::browsedict(docstring const & file) const
{
	return browseFile(file,
			  _("Choose personal dictionary"),
			  FileFilterList(_("*.ispell")));
}


docstring const ControlPrefs::browse(docstring const & file,
				  docstring const & title) const
{
	return browseFile(file, title, FileFilterList(), true);
}


docstring const ControlPrefs::browsedir(docstring const & path,
				     docstring const & title) const
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
