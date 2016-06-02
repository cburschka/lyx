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
#include "Buffer.h"

#include "frontends/alert.h"

#include "support/debug.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/TempFile.h"

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace Alert = frontend::Alert;


LyXVC::LyXVC()
{
	owner_ = 0;
}


bool LyXVC::fileInVC(FileName const & fn)
{
	if (!RCS::findFile(fn).empty())
		return true;
	if (!CVS::findFile(fn).empty())
		return true;
	if (!SVN::findFile(fn).empty())
		return true;
	if (!GIT::findFile(fn).empty())
		return true;
	return false;
}


bool LyXVC::file_found_hook(FileName const & fn)
{
	FileName found_file;
	// Check if file is under RCS
	if (!(found_file = RCS::findFile(fn)).empty()) {
		vcs.reset(new RCS(found_file, owner_));
		return true;
	}
	// Check if file is under CVS
	if (!(found_file = CVS::findFile(fn)).empty()) {
		vcs.reset(new CVS(found_file, owner_));
		return true;
	}
	// Check if file is under SVN
	if (!(found_file = SVN::findFile(fn)).empty()) {
		vcs.reset(new SVN(found_file, owner_));
		return true;
	}
	// Check if file is under GIT
	if (!(found_file = GIT::findFile(fn)).empty()) {
		vcs.reset(new GIT(found_file, owner_));
		return true;
	}

	// file is not under any VCS.
	vcs.reset(0);
	return false;
}


bool LyXVC::file_not_found_hook(FileName const & fn)
{
	// Check if file is under RCS.
	// This happens if we are trying to load non existent
	// file on disk, but existent in ,v version.
	bool foundRCS = !RCS::findFile(fn).empty();
	bool foundCVS = foundRCS ? false : !CVS::findFile(fn).empty();
	bool foundSVN = (foundRCS || foundCVS) ? false : !SVN::findFile(fn).empty();
	bool foundGIT = (foundRCS || foundCVS || foundSVN) ? false : !GIT::findFile(fn).empty();
	if (foundRCS || foundCVS || foundSVN || foundGIT) {
		docstring const file = makeDisplayPath(fn.absFileName(), 20);
		docstring const text =
			bformat(_("Do you want to retrieve the document"
						   " %1$s from version control?"), file);
		int const ret = Alert::prompt(_("Retrieve from version control?"),
			text, 0, 1, _("&Retrieve"), _("&Cancel"));

		if (ret == 0) {
			// Since the retrieve commands are implemented using
			// more general update commands we need to ensure that
			// we do not change an existing file by accident.
			if (fn.exists())
				return false;
			if (foundRCS)
				return RCS::retrieve(fn);
			else if (foundCVS)
				return CVS::retrieve(fn);
			else if (foundSVN)
				return SVN::retrieve(fn);
			else
				return GIT::retrieve(fn);
		}
	}
	return false;
}


void LyXVC::setBuffer(Buffer * buf)
{
	owner_ = buf;
}


bool LyXVC::registrer()
{
	FileName const filename = owner_->fileName();

	// there must be a file to save
	if (!filename.isReadableFile()) {
		Alert::error(_("Document not saved"),
			     _("You must save the document "
					    "before it can be registered."));
		return false;
	}

	// it is very likely here that the vcs is not created yet...
	if (!vcs) {
		//check in the root directory of the document
		FileName const cvs_entries(onlyPath(filename.absFileName()) + "/CVS/Entries");
		FileName const svn_entries(onlyPath(filename.absFileName()) + "/.svn/entries");
		FileName const git_index(onlyPath(filename.absFileName()) + "/.git/index");

		if (git_index.isReadableFile()) {
			LYXERR(Debug::LYXVC, "LyXVC: registering "
				<< to_utf8(filename.displayName()) << " with GIT");
			vcs.reset(new GIT(git_index, owner_));

		} else if (svn_entries.isReadableFile()) {
			LYXERR(Debug::LYXVC, "LyXVC: registering "
				<< to_utf8(filename.displayName()) << " with SVN");
			vcs.reset(new SVN(svn_entries, owner_));

		} else if (cvs_entries.isReadableFile()) {
			LYXERR(Debug::LYXVC, "LyXVC: registering "
				<< to_utf8(filename.displayName()) << " with CVS");
			vcs.reset(new CVS(cvs_entries, owner_));

		} else {
			LYXERR(Debug::LYXVC, "LyXVC: registering "
				<< to_utf8(filename.displayName()) << " with RCS");
			vcs.reset(new RCS(FileName(), owner_));
		}
	}

	LYXERR(Debug::LYXVC, "LyXVC: registrer");
	docstring response;
	bool ok = Alert::askForText(response, _("LyX VC: Initial description"),
			_("(no initial description)"));
	if (!ok) {
		LYXERR(Debug::LYXVC, "LyXVC: user cancelled");
		vcs.reset(0);
		return false;
	}
	if (response.empty())
		response = _("(no initial description)");
	vcs->registrer(to_utf8(response));
	return true;
}


string LyXVC::rename(FileName const & fn)
{
	LYXERR(Debug::LYXVC, "LyXVC: rename");
	if (!vcs || fileInVC(fn))
		return string();
	docstring response;
	bool ok = Alert::askForText(response, _("LyX VC: Log message"),
			_("(no log message)"));
	if (!ok) {
		LYXERR(Debug::LYXVC, "LyXVC: user cancelled");
		return string();
	}
	if (response.empty())
		response = _("(no log message)");
	string ret = vcs->rename(fn, to_utf8(response));
	return ret;
}


string LyXVC::copy(FileName const & fn)
{
	LYXERR(Debug::LYXVC, "LyXVC: copy");
	if (!vcs || fileInVC(fn))
		return string();
	docstring response;
	bool ok = Alert::askForText(response, _("LyX VC: Log message"),
			_("(no log message)"));
	if (!ok) {
		LYXERR(Debug::LYXVC, "LyXVC: user cancelled");
		return string();
	}
	if (response.empty())
		response = _("(no log message)");
	string ret = vcs->copy(fn, to_utf8(response));
	return ret;
}


LyXVC::CommandResult LyXVC::checkIn(string & log)
{
	LYXERR(Debug::LYXVC, "LyXVC: checkIn");
	if (!vcs)
		return ErrorBefore;
	docstring empty(_("(no log message)"));
	docstring response;
	bool ok = true;
	if (vcs->isCheckInWithConfirmation())
		ok = Alert::askForText(response, _("LyX VC: Log Message"));
	if (ok) {
		if (response.empty())
			response = empty;
		//shell collisions
		response = subst(response, from_ascii("\""), from_ascii("\\\""));
		return vcs->checkIn(to_utf8(response), log);
	} else {
		LYXERR(Debug::LYXVC, "LyXVC: user cancelled");
		return Cancelled;
	}
}


string LyXVC::checkOut()
{
	if (!vcs)
		return string();
	//RCS allows checkOut only in ReadOnly mode
	if (vcs->toggleReadOnlyEnabled() && !owner_->isReadonly())
		return string();

	LYXERR(Debug::LYXVC, "LyXVC: checkOut");
	return vcs->checkOut();
}


string LyXVC::repoUpdate()
{
	LYXERR(Debug::LYXVC, "LyXVC: repoUpdate");
	if (!vcs)
		return string();
	return vcs->repoUpdate();
}


string LyXVC::lockingToggle()
{
	LYXERR(Debug::LYXVC, "LyXVC: toggle locking property");
	if (!vcs)
		return string();
	return vcs->lockingToggle();
}


bool LyXVC::revert()
{
	LYXERR(Debug::LYXVC, "LyXVC: revert");
	if (!vcs)
		return false;

	docstring const file = owner_->fileName().displayName(20);
	docstring text = bformat(_("Reverting to the stored version of the "
				"document %1$s will lose all current changes.\n\n"
				"Do you want to revert to the older version?"), file);
	int ret = 0;
	if (vcs->isRevertWithConfirmation())
		ret = Alert::prompt(_("Revert to stored version of document?"),
			text, 0, 1, _("&Revert"), _("&Cancel"));

	return ret == 0 && vcs->revert();
}


void LyXVC::undoLast()
{
	if (!vcs)
		return;
	vcs->undoLast();
}


string LyXVC::toggleReadOnly()
{
	if (!vcs)
		return string();
	if (!vcs->toggleReadOnlyEnabled())
		return string();

	switch (vcs->status()) {
	case VCS::UNLOCKED:
		LYXERR(Debug::LYXVC, "LyXVC: toggle to locked");
		return checkOut();
	case VCS::LOCKED: {
		LYXERR(Debug::LYXVC, "LyXVC: toggle to unlocked");
		string log;
		if (checkIn(log) != VCSuccess)
			return string();
		return log;
	}
	case VCS::NOLOCKING:
		Buffer * b = vcs->owner();
		bool const newstate = !b->isReadonly();
		string result = "LyXVC: toggle to ";
		result += (newstate ? "readonly" : "readwrite");
		LYXERR(Debug::LYXVC, result);
		b->setReadonly(newstate);
		return result;
	}
	return string();
}


bool LyXVC::inUse() const
{
	if (vcs)
		return true;
	return false;
}


string const LyXVC::versionString() const
{
	if (!vcs)
		return string();
	return vcs->versionString();
}


bool LyXVC::locking() const
{
	if (!vcs)
		return false;
	return vcs->status() != VCS::NOLOCKING;
}


string const LyXVC::getLogFile() const
{
	if (!vcs)
		return string();

	TempFile tempfile("lyxvclog");
	tempfile.setAutoRemove(false);
	FileName const tmpf = tempfile.name();
	if (tmpf.empty()) {
		LYXERR(Debug::LYXVC, "Could not generate logfile " << tmpf);
		return string();
	}
	LYXERR(Debug::LYXVC, "Generating logfile " << tmpf);
	vcs->getLog(tmpf);
	return tmpf.absFileName();
}


string LyXVC::revisionInfo(RevisionInfo const info) const
{
	if (!vcs)
		return string();

	return vcs->revisionInfo(info);
}


bool LyXVC::renameEnabled() const
{
	if (!inUse())
		return false;
	return vcs->renameEnabled();
}


bool LyXVC::copyEnabled() const
{
	if (!inUse())
		return false;
	return vcs->copyEnabled();
}


bool LyXVC::checkOutEnabled() const
{
	return vcs && vcs->checkOutEnabled();
}


bool LyXVC::checkInEnabled() const
{
	return vcs && vcs->checkInEnabled();
}


bool LyXVC::isCheckInWithConfirmation() const
{
	return vcs && vcs->isCheckInWithConfirmation();
}


bool LyXVC::lockingToggleEnabled() const
{
	return vcs && vcs->lockingToggleEnabled();
}


bool LyXVC::undoLastEnabled() const
{
	return vcs && vcs->undoLastEnabled();
}


bool LyXVC::repoUpdateEnabled() const
{
	return vcs && vcs->repoUpdateEnabled();
}
	
	
bool LyXVC::prepareFileRevision(string const & rev, std::string & f)
{
	return vcs && vcs->prepareFileRevision(rev, f);
}


bool LyXVC::prepareFileRevisionEnabled()
{
	return vcs && vcs->prepareFileRevisionEnabled();
}

} // namespace lyx
