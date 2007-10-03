/**
 * \file LyXView.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "LyXView.h"

#include "Dialogs.h"
#include "WorkArea.h"
#include "Gui.h"

#include "Buffer.h"
#include "buffer_funcs.h"
#include "BufferList.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "callback.h"
#include "Cursor.h"
#include "debug.h"
#include "ErrorList.h"
#include "FuncRequest.h"
#include "gettext.h"
#include "Intl.h"
#include "Layout.h"
#include "LyX.h"
#include "LyXFunc.h"
#include "LyXRC.h"
#include "MenuBackend.h"
#include "Paragraph.h"
#include "Session.h"
#include "Text.h"

#include "support/lstrings.h"
#include "support/filetools.h" // OnlyFilename()
#include "support/Timeout.h"

#include <boost/bind.hpp>


#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

using std::endl;
using std::string;

namespace lyx {

using support::bformat;
using support::FileName;
using support::makeDisplayPath;
using support::onlyFilename;

namespace frontend {

LyXView::LyXView(int id)
	: autosave_timeout_(new Timeout(5000)),
	  dialogs_(new Dialogs(*this)),
	  id_(id)
{
	// Start autosave timer
	if (lyxrc.autosave) {
		autosave_timeout_->timeout.connect(boost::bind(&LyXView::autoSave, this));
		autosave_timeout_->setTimeout(lyxrc.autosave * 1000);
		autosave_timeout_->start();
	}
}


LyXView::~LyXView()
{
	delete dialogs_;
	delete autosave_timeout_;
}


Buffer * LyXView::buffer()
{
	WorkArea * work_area = currentWorkArea();
	if (work_area)
		return &work_area->bufferView().buffer();
	return 0;
}


Buffer const * LyXView::buffer() const
{
	WorkArea const * work_area = currentWorkArea();
	if (work_area)
		return &work_area->bufferView().buffer();
	return 0;
}


void LyXView::setBuffer(Buffer * newBuffer)
{
	BOOST_ASSERT(newBuffer);
	busy(true);

	WorkArea * wa = workArea(*newBuffer);
	if (wa == 0) {
		updateLabels(*newBuffer->getMasterBuffer());
		wa = addWorkArea(*newBuffer);
	} else {
		//Disconnect the old buffer...there's no new one.
		disconnectBuffer();
	}
	connectBuffer(*newBuffer);
	connectBufferView(wa->bufferView());
	setCurrentWorkArea(wa);

	busy(false);
}


Buffer * LyXView::loadLyXFile(FileName const & filename, bool tolastfiles)
{
	busy(true);

	Buffer * newBuffer = checkAndLoadLyXFile(filename);

	if (!newBuffer) {
		message(_("Document not loaded."));
		updateStatusBar();
		busy(false);
		return 0;
	}

	WorkArea * wa = workArea(*newBuffer);
	if (wa == 0)
		wa = addWorkArea(*newBuffer);

	// scroll to the position when the file was last closed
	if (lyxrc.use_lastfilepos) {
		pit_type pit;
		pos_type pos;
		boost::tie(pit, pos) = LyX::ref().session().lastFilePos().load(filename);
		// if successfully move to pit (returned par_id is not zero),
		// update metrics and reset font
		wa->bufferView().moveToPosition(pit, pos, 0, 0);
	}

	if (tolastfiles)
		LyX::ref().session().lastFiles().add(filename);

	busy(false);
	return newBuffer;
}


void LyXView::connectBuffer(Buffer & buf)
{
	buf.setGuiDelegate(this);
}


void LyXView::disconnectBuffer()
{
	if (WorkArea * work_area = currentWorkArea())
		work_area->bufferView().setGuiDelegate(0);
}


void LyXView::connectBufferView(BufferView & bv)
{
	bv.setGuiDelegate(this);
}


void LyXView::disconnectBufferView()
{
	if (WorkArea * work_area = currentWorkArea())
		work_area->bufferView().setGuiDelegate(0);
}


void LyXView::showErrorList(string const & error_type)
{
	ErrorList & el = buffer()->errorList(error_type);
	if (!el.empty())
		getDialogs().show("errorlist", error_type);
}


void LyXView::showDialog(string const & name)
{
	getDialogs().show(name);
}


void LyXView::showDialogWithData(string const & name, string const & data)
{
	getDialogs().show(name, data);
}


void LyXView::showInsetDialog(string const & name, string const & data,
		Inset * inset)
{
	getDialogs().show(name, data, inset);
}


void LyXView::updateDialog(string const & name, string const & data)
{
	if (getDialogs().visible(name))
		getDialogs().update(name, data);
}


void LyXView::showReadonly(bool)
{
	updateWindowTitle();
	getDialogs().updateBufferDependent(false);
}


BufferView * LyXView::view()
{
	WorkArea * wa = currentWorkArea();
	return wa? &wa->bufferView() : 0;
}


void LyXView::updateToc()
{
	updateDialog("toc", "");
}


void LyXView::updateEmbeddedFiles()
{
	updateDialog("embedding", "");
}


void LyXView::autoSave()
{
	LYXERR(Debug::INFO) << "Running autoSave()" << endl;

	if (buffer())
		view()->buffer().autoSave();
}


void LyXView::resetAutosaveTimer()
{
	if (lyxrc.autosave)
		autosave_timeout_->restart();
}


void LyXView::updateWindowTitle()
{
	docstring maximize_title = from_ascii("LyX");
	docstring minimize_title = from_ascii("LyX");

	Buffer * buf = buffer();
	if (buf) {
		string const cur_title = buf->fileName();
		if (!cur_title.empty()) {
			maximize_title += ": " + makeDisplayPath(cur_title, 30);
			minimize_title = lyx::from_utf8(onlyFilename(cur_title));
			if (!buf->isClean()) {
				maximize_title += _(" (changed)");
				minimize_title += char_type('*');
			}
			if (buf->isReadonly())
				maximize_title += _(" (read only)");
		}
	}

	setWindowTitle(maximize_title, minimize_title);
}


void LyXView::dispatch(FuncRequest const & cmd)
{
	string const argument = to_utf8(cmd.argument());
	switch(cmd.action) {
		case LFUN_BUFFER_SWITCH:
			setBuffer(theBufferList().getBuffer(to_utf8(cmd.argument())));
			break;
		default:
			theLyXFunc().setLyXView(this);
			lyx::dispatch(cmd);
	}
}


Buffer const * LyXView::updateInset(Inset const * inset)
{
	WorkArea * work_area = currentWorkArea();
	if (!work_area)
		return 0;

	if (inset) {
		BOOST_ASSERT(work_area);
		work_area->scheduleRedraw();
	}
	return &work_area->bufferView().buffer();
}

} // namespace frontend
} // namespace lyx
