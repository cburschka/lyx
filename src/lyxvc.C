#include <config.h>

#include "lyxvc.h"
#include "vc-backend.h"
#include "debug.h"
#include "buffer.h"
#include "BufferView.h"
#include "gettext.h"
#include "funcrequest.h"

#include "frontends/Alert.h"
#include "frontends/LyXView.h"

#include "support/filetools.h"
#include "support/lyxlib.h"

#include <unistd.h>

using std::endl;
using std::pair;


/* WARNING: Several of the vcs-> methods end up
 * deleting this object via BufferView::reload() !
 */

LyXVC::LyXVC()
{
	vcs = 0;
	owner_ = 0;
}


LyXVC::~LyXVC()
{
	delete vcs;
}


bool LyXVC::file_found_hook(string const & fn)
{
	string found_file;
	// Check if file is under RCS
	if (!(found_file = RCS::find_file(fn)).empty()) {
		vcs = new RCS(found_file);
		vcs->owner(owner_);
		return true;
	}
	// Check if file is under CVS
	if (!(found_file = CVS::find_file(fn)).empty()) {
		vcs = new CVS(found_file, fn);
		vcs->owner(owner_);
		return true;
	}
	// file is not under any VCS.
	return false;
}


bool LyXVC::file_not_found_hook(string const & fn)
{
	// Check if file is under RCS
	if (!RCS::find_file(fn).empty())
		return true;
	if (!CVS::find_file(fn).empty())
		return true;
	return false;
}


void LyXVC::buffer(Buffer * buf)
{
	owner_ = buf;
}


bool LyXVC::ensureClean()
{
	if (owner_->isClean())
		return true;

	string const file = MakeDisplayPath(owner_->fileName(), 30);
	string text = bformat(_("The document %1$s has unsaved changes.\n\n"
		"Do you want to save the document?"), file);
	int const ret = Alert::prompt(_("Save changed document?"),
		text, 0, 1, _("&Save"), _("&Cancel"));

	if (ret == 0) {
		vcs->owner()->getUser()->owner()->dispatch(FuncRequest(LFUN_MENUWRITE));
	}

	return owner_->isClean();
}


void LyXVC::registrer()
{
	string const filename = owner_->fileName();

	// there must be a file to save
	if (!IsFileReadable(filename)) {
		Alert::error(_("Document not saved"),
			_("You must save the document "
			  "before it can be registered."));
		return;
	}

	// it is very likely here that the vcs is not created yet...
	if (!vcs) {
		string const cvs_entries = "CVS/Entries";

		if (IsFileReadable(cvs_entries)) {
			lyxerr[Debug::LYXVC]
				<< "LyXVC: registering "
				<< MakeDisplayPath(filename)
				<< " with CVS" << endl;
			vcs = new CVS(cvs_entries, filename);

		} else {
			lyxerr[Debug::LYXVC]
				<< "LyXVC: registering "
				<< MakeDisplayPath(filename)
				<< " with RCS" << endl;
			vcs = new RCS(filename);
		}

		vcs->owner(owner_);
	}

	// Maybe the save fails, or we answered "no". In both cases,
	// the document will be dirty, and we abort.
	if (!ensureClean())
		return;

	lyxerr[Debug::LYXVC] << "LyXVC: registrer" << endl;
	pair<bool, string> tmp =
		Alert::askForText(_("LyX VC: Initial description"),
			   _("(no initial description)"));
	if (!tmp.first || tmp.second.empty()) {
		// should we insist on checking tmp.second.empty()?
		lyxerr[Debug::LYXVC] << "LyXVC: user cancelled" << endl;
		return;
	}

	vcs->registrer(tmp.second);
}


void LyXVC::checkIn()
{
	// If the document is changed, we might want to save it
	if (!vcs->owner()->isClean()) {
		vcs->owner()->getUser()->owner()
			->dispatch(FuncRequest(LFUN_MENUWRITE));
	}

	// Maybe the save fails, or we answered "no". In both cases,
	// the document will be dirty, and we abort.
	if (!vcs->owner()->isClean())
		return;

	lyxerr[Debug::LYXVC] << "LyXVC: checkIn" << endl;
	pair<bool, string> tmp = Alert::askForText(_("LyX VC: Log Message"));
	if (tmp.first) {
		if (tmp.second.empty()) {
			tmp.second = _("(no log message)");
		}
		vcs->checkIn(tmp.second);
	} else {
		lyxerr[Debug::LYXVC] << "LyXVC: user cancelled" << endl;
	}
}


void LyXVC::checkOut()
{
	lyxerr[Debug::LYXVC] << "LyXVC: checkOut" << endl;
	if (!ensureClean())
		return;

	vcs->checkOut();
}


void LyXVC::revert()
{
	lyxerr[Debug::LYXVC] << "LyXVC: revert" << endl;

	string const file = MakeDisplayPath(owner_->fileName(), 20);
	string text = bformat(_("Reverting to the stored version of the "
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
		lyxerr[Debug::LYXVC] << "LyXVC: toggle to locked" << endl;
		checkOut();
		break;
	case VCS::LOCKED:
		lyxerr[Debug::LYXVC] << "LyXVC: toggle to unlocked" << endl;
		checkIn();
		break;
	}
}


bool LyXVC::inUse()
{
	if (vcs) return true;
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

	string tmpf = lyx::tempName(string(), "lyxvclog");
	lyxerr[Debug::LYXVC] << "Generating logfile " << tmpf << endl;
	vcs->getLog(tmpf);
	return tmpf;
}
