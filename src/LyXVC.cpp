/**
 * \file LyXVC.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 * \author John Levon
 * \author André Pönitz
 * \author Allan Rae
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "LyXVC.h"
#include "VCBackend.h"
#include "debug.h"
#include "Buffer.h"
#include "gettext.h"

#include "frontends/alert.h"

#include "support/filetools.h"
#include "support/lyxlib.h"


namespace lyx {

using support::bformat;
using support::FileName;
using support::makeAbsPath;
using support::makeDisplayPath;
using support::tempName;

using std::endl;
using std::string;
using std::pair;

namespace Alert = frontend::Alert;


LyXVC::LyXVC()
{
	owner_ = 0;
}


// for the sake of boost::scoped_ptr
LyXVC::~LyXVC()
{}


bool LyXVC::file_found_hook(FileName const & fn)
{
	FileName found_file;
	// Check if file is under RCS
	if (!(found_file = RCS::find_file(fn)).empty()) {
		vcs.reset(new RCS(found_file));
		vcs->owner(owner_);
		return true;
	}
	// Check if file is under CVS
	if (!(found_file = CVS::find_file(fn)).empty()) {
		vcs.reset(new CVS(found_file, fn));
		vcs->owner(owner_);
		return true;
	}
	// file is not under any VCS.
	return false;
}


bool LyXVC::file_not_found_hook(FileName const & fn)
{
	// Check if file is under RCS
	if (!RCS::find_file(fn).empty())
		return true;
	if (!CVS::find_file(fn).empty())
		return true;
	return false;
}


void LyXVC::setBuffer(Buffer * buf)
{
	owner_ = buf;
}


void LyXVC::registrer()
{
	FileName const filename(owner_->absFileName());

	// there must be a file to save
	if (!filename.isFileReadable()) {
		Alert::error(_("Document not saved"),
			     _("You must save the document "
					    "before it can be registered."));
		return;
	}

	// it is very likely here that the vcs is not created yet...
	if (!vcs) {
		FileName const cvs_entries(makeAbsPath("CVS/Entries"));

		if (cvs_entries.isFileReadable()) {
			LYXERR(Debug::LYXVC)
				<< "LyXVC: registering "
				<< to_utf8(makeDisplayPath(filename.absFilename()))
				<< " with CVS" << endl;
			vcs.reset(new CVS(cvs_entries, filename));

		} else {
			LYXERR(Debug::LYXVC)
				<< "LyXVC: registering "
				<< to_utf8(makeDisplayPath(filename.absFilename()))
				<< " with RCS" << endl;
			vcs.reset(new RCS(filename));
		}

		vcs->owner(owner_);
	}

	LYXERR(Debug::LYXVC) << "LyXVC: registrer" << endl;
	pair<bool, docstring> tmp =
		Alert::askForText(_("LyX VC: Initial description"),
			   _("(no initial description)"));
	if (!tmp.first || tmp.second.empty()) {
		// should we insist on checking tmp.second.empty()?
		LYXERR(Debug::LYXVC) << "LyXVC: user cancelled" << endl;
		return;
	}

	vcs->registrer(to_utf8(tmp.second));
}


void LyXVC::checkIn()
{

	LYXERR(Debug::LYXVC) << "LyXVC: checkIn" << endl;
	pair<bool, docstring> tmp = Alert::askForText(_("LyX VC: Log Message"));
	if (tmp.first) {
		if (tmp.second.empty()) {
			tmp.second = _("(no log message)");
		}
		vcs->checkIn(to_utf8(tmp.second));
	} else {
		LYXERR(Debug::LYXVC) << "LyXVC: user cancelled" << endl;
	}
}


void LyXVC::checkOut()
{
	LYXERR(Debug::LYXVC) << "LyXVC: checkOut" << endl;

	vcs->checkOut();
}


void LyXVC::revert()
{
	LYXERR(Debug::LYXVC) << "LyXVC: revert" << endl;

	docstring const file = makeDisplayPath(owner_->absFileName(), 20);
	docstring text = bformat(_("Reverting to the stored version of the "
		"document %1$s will lose all current changes.\n\n"
					     "Do you want to revert to the saved version?"), file);
	int const ret = Alert::prompt(_("Revert to stored version of document?"),
		text, 0, 1, _("&Revert"), _("&Cancel"));

	if (ret == 0)
		vcs->revert();
}


void LyXVC::undoLast()
{
	vcs->undoLast();
}


void LyXVC::toggleReadOnly()
{
	switch (vcs->status()) {
	case VCS::UNLOCKED:
		LYXERR(Debug::LYXVC) << "LyXVC: toggle to locked" << endl;
		checkOut();
		break;
	case VCS::LOCKED:
		LYXERR(Debug::LYXVC) << "LyXVC: toggle to unlocked" << endl;
		checkIn();
		break;
	}
}


bool LyXVC::inUse()
{
	if (vcs)
		return true;
	return false;
}


//string const & LyXVC::version() const
//{
//	return vcs->version();
//}


string const LyXVC::versionString() const
{
	return vcs->versionString();
}


string const & LyXVC::locker() const
{
	return vcs->locker();
}


string const LyXVC::getLogFile() const
{
	if (!vcs)
		return string();

	FileName const tmpf(tempName(FileName(), "lyxvclog"));
	if (tmpf.empty()) {
		LYXERR(Debug::LYXVC) << "Could not generate logfile "
				     << tmpf << endl;
		return string();
	}
	LYXERR(Debug::LYXVC) << "Generating logfile " << tmpf << endl;
	vcs->getLog(tmpf);
	return tmpf.absFilename();
}


} // namespace lyx
