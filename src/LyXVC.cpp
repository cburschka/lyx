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
	owner_ = nullptr;
}


docstring LyXVC::vcstatus() const
{
	if (!vcs_)
		return docstring();
	if (locking())
		return bformat(_("%1$s lock"), from_ascii(vcs_->vcname()));
	else
		return from_ascii(vcs_->vcname());
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
		vcs_.reset(new RCS(found_file, owner_));
		return true;
	}
	// Check if file is under CVS
	if (!(found_file = CVS::findFile(fn)).empty()) {
		vcs_.reset(new CVS(found_file, owner_));
		return true;
	}
	// Check if file is under SVN
	if (!(found_file = SVN::findFile(fn)).empty()) {
		vcs_.reset(new SVN(found_file, owner_));
		return true;
	}
	// Check if file is under GIT
	if (!(found_file = GIT::findFile(fn)).empty()) {
		vcs_.reset(new GIT(found_file, owner_));
		return true;
	}

	// file is not under any VCS.
	vcs_.reset(nullptr);
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
	if (!vcs_) {
		//check in the root directory of the document
		FileName const cvs_entries(onlyPath(filename.absFileName()) + "/CVS/Entries");
		FileName const svn_entries(onlyPath(filename.absFileName()) + "/.svn/entries");
		FileName const git_index(onlyPath(filename.absFileName()) + "/.git/index");

		if (git_index.isReadableFile()) {
			LYXERR(Debug::LYXVC, "LyXVC: registering "
				<< to_utf8(filename.displayName()) << " with GIT");
			vcs_.reset(new GIT(git_index, owner_));

		} else if (svn_entries.isReadableFile()) {
			LYXERR(Debug::LYXVC, "LyXVC: registering "
				<< to_utf8(filename.displayName()) << " with SVN");
			vcs_.reset(new SVN(svn_entries, owner_));

		} else if (cvs_entries.isReadableFile()) {
			LYXERR(Debug::LYXVC, "LyXVC: registering "
				<< to_utf8(filename.displayName()) << " with CVS");
			vcs_.reset(new CVS(cvs_entries, owner_));

		} else {
			LYXERR(Debug::LYXVC, "LyXVC: registering "
				<< to_utf8(filename.displayName()) << " with RCS");
			vcs_.reset(new RCS(FileName(), owner_));
		}
	}

	LYXERR(Debug::LYXVC, "LyXVC: registrer");
	docstring response;
	bool ok = Alert::askForText(response, _("LyX VC: Initial description"),
			_("(no initial description)"));
	if (!ok) {
		LYXERR(Debug::LYXVC, "LyXVC: user cancelled");
		vcs_.reset(nullptr);
		return false;
	}
	if (response.empty())
		response = _("(no initial description)");
	vcs_->registrer(to_utf8(response));
	return true;
}


string LyXVC::rename(FileName const & fn)
{
	LYXERR(Debug::LYXVC, "LyXVC: rename");
	if (!vcs_ || fileInVC(fn))
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
	string ret = vcs_->rename(fn, to_utf8(response));
	return ret;
}


string LyXVC::copy(FileName const & fn)
{
	LYXERR(Debug::LYXVC, "LyXVC: copy");
	if (!vcs_ || fileInVC(fn))
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
	string ret = vcs_->copy(fn, to_utf8(response));
	return ret;
}


LyXVC::CommandResult LyXVC::checkIn(string & log)
{
	LYXERR(Debug::LYXVC, "LyXVC: checkIn");
	if (!vcs_)
		return ErrorBefore;
	docstring empty(_("(no log message)"));
	docstring response;
	bool ok = true;
	if (vcs_->isCheckInWithConfirmation())
		ok = Alert::askForText(response, _("LyX VC: Log Message"));
	if (ok) {
		if (response.empty())
			response = empty;
		//shell collisions
		response = subst(response, from_ascii("\""), from_ascii("\\\""));
		return vcs_->checkIn(to_utf8(response), log);
	} else {
		LYXERR(Debug::LYXVC, "LyXVC: user cancelled");
		return Cancelled;
	}
}


string LyXVC::checkOut()
{
	if (!vcs_)
		return string();
	//RCS allows checkOut only in ReadOnly mode
	if (vcs_->toggleReadOnlyEnabled() && !owner_->hasReadonlyFlag())
		return string();

	LYXERR(Debug::LYXVC, "LyXVC: checkOut");
	return vcs_->checkOut();
}


string LyXVC::repoUpdate()
{
	LYXERR(Debug::LYXVC, "LyXVC: repoUpdate");
	if (!vcs_)
		return string();
	return vcs_->repoUpdate();
}


string LyXVC::lockingToggle()
{
	LYXERR(Debug::LYXVC, "LyXVC: toggle locking property");
	if (!vcs_)
		return string();
	return vcs_->lockingToggle();
}


bool LyXVC::revert()
{
	LYXERR(Debug::LYXVC, "LyXVC: revert");
	if (!vcs_)
		return false;

	docstring const file = owner_->fileName().displayName(20);
	docstring text = bformat(_("Reverting to the stored version of the "
				"document %1$s will lose all current changes.\n\n"
				"Do you want to revert to the older version?"), file);
	int ret = 0;
	if (vcs_->isRevertWithConfirmation())
		ret = Alert::prompt(_("Revert to stored version of document?"),
			text, 0, 1, _("&Revert"), _("&Cancel"));

	return ret == 0 && vcs_->revert();
}


void LyXVC::undoLast()
{
	if (!vcs_)
		return;
	vcs_->undoLast();
}


string LyXVC::toggleReadOnly()
{
	if (!vcs_)
		return string();
	if (!vcs_->toggleReadOnlyEnabled())
		return string();

	switch (vcs_->status()) {
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
		Buffer * b = vcs_->owner();
		bool const newstate = !b->hasReadonlyFlag();
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
	return vcs_ != nullptr;
}


string const LyXVC::versionString() const
{
	if (!vcs_)
		return string();
	return vcs_->versionString();
}


bool LyXVC::locking() const
{
	if (!vcs_)
		return false;
	return vcs_->status() != VCS::NOLOCKING;
}


string const LyXVC::getLogFile() const
{
	if (!vcs_)
		return string();

	TempFile tempfile("lyxvclog");
	tempfile.setAutoRemove(false);
	FileName const tmpf = tempfile.name();
	if (tmpf.empty()) {
		LYXERR(Debug::LYXVC, "Could not generate logfile " << tmpf);
		return string();
	}
	LYXERR(Debug::LYXVC, "Generating logfile " << tmpf);
	vcs_->getLog(tmpf);
	return tmpf.absFileName();
}


string LyXVC::revisionInfo(RevisionInfo const info) const
{
	if (!vcs_)
		return string();

	return vcs_->revisionInfo(info);
}


bool LyXVC::renameEnabled() const
{
	if (!inUse())
		return false;
	return vcs_->renameEnabled();
}


bool LyXVC::copyEnabled() const
{
	if (!inUse())
		return false;
	return vcs_->copyEnabled();
}


bool LyXVC::checkOutEnabled() const
{
	return vcs_ && vcs_->checkOutEnabled();
}


bool LyXVC::checkInEnabled() const
{
	return vcs_ && vcs_->checkInEnabled();
}


bool LyXVC::isCheckInWithConfirmation() const
{
	return vcs_ && vcs_->isCheckInWithConfirmation();
}


bool LyXVC::lockingToggleEnabled() const
{
	return vcs_ && vcs_->lockingToggleEnabled();
}


bool LyXVC::undoLastEnabled() const
{
	return vcs_ && vcs_->undoLastEnabled();
}


bool LyXVC::repoUpdateEnabled() const
{
	return vcs_ && vcs_->repoUpdateEnabled();
}


bool LyXVC::prepareFileRevision(string const & rev, std::string & f)
{
	return vcs_ && vcs_->prepareFileRevision(rev, f);
}


bool LyXVC::prepareFileRevisionEnabled()
{
	return vcs_ && vcs_->prepareFileRevisionEnabled();
}

} // namespace lyx
