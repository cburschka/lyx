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
#include "Toolbars.h"
#include "Menubar.h"
#include "WorkArea.h"
#include "Gui.h"

#include "Buffer.h"
#include "buffer_funcs.h"
#include "BufferList.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "bufferview_funcs.h"
#include "Cursor.h"
#include "debug.h"
#include "ErrorList.h"
#include "FuncRequest.h"
#include "gettext.h"
#include "Intl.h"
#include "callback.h"
#include "LyX.h"
#include "LyXFunc.h"
#include "LyXRC.h"
#include "Text.h"
#include "MenuBackend.h"
#include "Paragraph.h"

#include "controllers/ControlCommandBuffer.h"

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

docstring current_layout;

LyXView::LyXView(int id)
	: toolbars_(new Toolbars(*this)),
	  autosave_timeout_(new Timeout(5000)),
	  dialogs_(new Dialogs(*this)),
	  controlcommand_(new ControlCommandBuffer(*this)), id_(id)
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
	disconnectBuffer();
	disconnectBufferView();
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
	} else
		//Disconnect the old buffer...there's no new one.
		disconnectBuffer();
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

	WorkArea * wa = addWorkArea(*newBuffer);

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
	if (errorsConnection_.connected())
		disconnectBuffer();

	bufferStructureChangedConnection_ =
		buf.getMasterBuffer()->structureChanged.connect(
			boost::bind(&LyXView::updateToc, this));

	errorsConnection_ =
		buf.errors.connect(
			boost::bind(&LyXView::showErrorList, this, _1));

	messageConnection_ =
		buf.message.connect(
			boost::bind(&LyXView::message, this, _1));

	busyConnection_ =
		buf.busy.connect(
			boost::bind(&LyXView::busy, this, _1));

	titleConnection_ =
		buf.updateTitles.connect(
			boost::bind(&LyXView::updateWindowTitle, this));

	timerConnection_ =
		buf.resetAutosaveTimers.connect(
			boost::bind(&LyXView::resetAutosaveTimer, this));

	readonlyConnection_ =
		buf.readonly.connect(
			boost::bind(&LyXView::showReadonly, this, _1));
}


void LyXView::disconnectBuffer()
{
	errorsConnection_.disconnect();
	bufferStructureChangedConnection_.disconnect();
	messageConnection_.disconnect();
	busyConnection_.disconnect();
	titleConnection_.disconnect();
	timerConnection_.disconnect();
	readonlyConnection_.disconnect();
	layout_changed_connection_.disconnect();
}


void LyXView::connectBufferView(BufferView & bv)
{
	message_connection_ = bv.message.connect(
			boost::bind(&LyXView::message, this, _1));
	show_dialog_connection_ = bv.showDialog.connect(
			boost::bind(&LyXView::showDialog, this, _1));
	show_dialog_with_data_connection_ = bv.showDialogWithData.connect(
			boost::bind(&LyXView::showDialogWithData, this, _1, _2));
	show_inset_dialog_connection_ = bv.showInsetDialog.connect(
			boost::bind(&LyXView::showInsetDialog, this, _1, _2, _3));
	update_dialog_connection_ = bv.updateDialog.connect(
			boost::bind(&LyXView::updateDialog, this, _1, _2));
	layout_changed_connection_ = bv.layoutChanged.connect(
			boost::bind(&Toolbars::setLayout, toolbars_.get(), _1));
}


void LyXView::disconnectBufferView()
{
	message_connection_.disconnect();
	show_dialog_connection_.disconnect();
	show_dialog_with_data_connection_.disconnect();
	show_inset_dialog_connection_.disconnect();
	update_dialog_connection_.disconnect();
}


void LyXView::showErrorList(string const & error_type)
{
	ErrorList & el = buffer()->errorList(error_type);
	if (!el.empty()) {
		getDialogs().show("errorlist", error_type);
	}
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


void LyXView::updateToolbars()
{
	WorkArea * wa = currentWorkArea();
	if (wa) {
		bool const math =
			wa->bufferView().cursor().inMathed();
		bool const table =
			lyx::getStatus(FuncRequest(LFUN_LAYOUT_TABULAR)).enabled();
		bool const review =
			lyx::getStatus(FuncRequest(LFUN_CHANGES_TRACK)).enabled() &&
			lyx::getStatus(FuncRequest(LFUN_CHANGES_TRACK)).onoff(true);

		toolbars_->update(math, table, review);
	} else
		toolbars_->update(false, false, false);

	// update redaonly status of open dialogs. This could also be in
	// updateMenubar(), but since updateToolbars() and updateMenubar()
	// are always called together it is only here.
	getDialogs().checkStatus();
}


ToolbarInfo * LyXView::getToolbarInfo(string const & name)
{
	return toolbars_->getToolbarInfo(name);
}


void LyXView::toggleToolbarState(string const & name, bool allowauto)
{
	// it is possible to get current toolbar status like this,...
	// but I decide to obey the order of ToolbarBackend::flags
	// and disregard real toolbar status.
	// toolbars_->saveToolbarInfo();
	//
	// toggle state on/off/auto
	toolbars_->toggleToolbarState(name, allowauto);
	// update toolbar
	updateToolbars();
}


void LyXView::updateMenubar()
{
	menubar_->update();
}


void LyXView::autoSave()
{
	LYXERR(Debug::INFO) << "Running autoSave()" << endl;

	if (buffer())
		lyx::autoSave(view());
}


void LyXView::resetAutosaveTimer()
{
	if (lyxrc.autosave)
		autosave_timeout_->restart();
}


void LyXView::updateLayoutChoice()
{
	// Don't show any layouts without a buffer
	if (!buffer()) {
		toolbars_->clearLayoutList();
		return;
	}

	// Update the layout display
	if (toolbars_->updateLayoutList(buffer()->params().textclass)) {
		current_layout = buffer()->params().getTextClass().defaultLayoutName();
	}

	docstring const & layout = currentWorkArea()->bufferView().cursor().
		innerParagraph().layout()->name();

	if (layout != current_layout) {
		toolbars_->setLayout(layout);
		current_layout = layout;
	}
}


void LyXView::updateWindowTitle()
{
	docstring maximize_title = lyx::from_ascii("LyX");
	docstring minimize_title = lyx::from_ascii("LyX");

	Buffer * buf = buffer();
	if (buf) {
		string const cur_title = buf->fileName();
		if (!cur_title.empty()) {
			maximize_title += ": " + makeDisplayPath(cur_title, 30);
			minimize_title = lyx::from_utf8(onlyFilename(cur_title));
			if (!buf->isClean()) {
				maximize_title += _(" (changed)");
				minimize_title += lyx::char_type('*');
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


Buffer const * const LyXView::updateInset(Inset const * inset)
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


void LyXView::openLayoutList()
{
	toolbars_->openLayoutList();
}


bool LyXView::isToolbarVisible(std::string const & id)
{
	return toolbars_->visible(id);
}

} // namespace frontend
} // namespace lyx
