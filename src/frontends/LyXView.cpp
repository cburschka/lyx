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
#include "Timeout.h"
#include "Toolbars.h"
#include "Menubar.h"
#include "WorkArea.h"
#include "Gui.h"

#include "Buffer.h"
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
#include "LyXFunc.h"
#include "LyXRC.h"
#include "Text.h"
#include "MenuBackend.h"
#include "Paragraph.h"

#include "controllers/ControlCommandBuffer.h"

#include "support/lstrings.h"
#include "support/filetools.h" // OnlyFilename()

#include <boost/bind.hpp>


namespace lyx {

#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

using frontend::WorkArea;

using support::bformat;
using support::FileName;
using support::makeDisplayPath;
using support::onlyFilename;

using std::endl;
using std::string;

using lyx::frontend::ControlCommandBuffer;

string current_layout;


LyXView::LyXView(int id)
	: work_area_(0),
	  toolbars_(new Toolbars(*this)),
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
}


// FIXME, there's only one WorkArea per LyXView possible for now.
void LyXView::setWorkArea(WorkArea * work_area)
{
	BOOST_ASSERT(work_area);
	work_area_ = work_area;
	work_area_ids_.clear();
	work_area_ids_.push_back(work_area_->id());
}


// FIXME, there's only one WorkArea per LyXView possible for now.
WorkArea const * LyXView::currentWorkArea() const
{
	return work_area_;
}


// FIXME, there's only one WorkArea per LyXView possible for now.
WorkArea * LyXView::currentWorkArea()
{
	return work_area_;
}


Buffer * LyXView::buffer() const
{
	BOOST_ASSERT(work_area_);
	return work_area_->bufferView().buffer();
}


void LyXView::setBuffer(Buffer * b)
{
	busy(true);

	BOOST_ASSERT(work_area_);
	if (work_area_->bufferView().buffer())
		disconnectBuffer();

	if (!b && theBufferList().empty())
		getDialogs().hideBufferDependent();

	work_area_->bufferView().setBuffer(b);
	// Make sure the TOC is updated before anything else.
	updateToc();

	if (work_area_->bufferView().buffer()) {
		// Buffer-dependent dialogs should be updated or
		// hidden. This should go here because some dialogs (eg ToC)
		// require bv_->text.
		getDialogs().updateBufferDependent(true);
		connectBuffer(*work_area_->bufferView().buffer());
	}

	if (quitting)
		return;

	updateMenubar();
	updateToolbars();
	updateLayoutChoice();
	updateWindowTitle();
	updateStatusBar();
	updateTab();
	busy(false);
	work_area_->redraw();
}


bool LyXView::loadLyXFile(FileName const & filename, bool tolastfiles)
{
	busy(true);

	BOOST_ASSERT(work_area_);
	bool const hadBuffer = work_area_->bufferView().buffer();
	if (hadBuffer)
		disconnectBuffer();

	bool const loaded =
		work_area_->bufferView().loadLyXFile(filename, tolastfiles);

	updateToc();
	updateMenubar();
	updateToolbars();
	updateLayoutChoice();
	updateWindowTitle();
	updateTab();
	if (loaded) {
		connectBuffer(*work_area_->bufferView().buffer());
		showErrorList("Parse");
	} else if (hadBuffer)
		//Need to reconnect the buffer if the load failed
		connectBuffer(*work_area_->bufferView().buffer());
	updateStatusBar();
	busy(false);
	work_area_->redraw();
	return loaded;
}


void LyXView::connectBuffer(Buffer & buf)
{
	if (errorsConnection_.connected())
		disconnectBuffer();

	BOOST_ASSERT(work_area_);
	bufferChangedConnection_ =
		buf.changed.connect(
			boost::bind(&WorkArea::redraw, work_area_));

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

	closingConnection_ =
		buf.closing.connect(
			boost::bind(&LyXView::setBuffer, this, (Buffer *)0));
}


void LyXView::disconnectBuffer()
{
	errorsConnection_.disconnect();
	bufferChangedConnection_.disconnect();
	bufferStructureChangedConnection_.disconnect();
	messageConnection_.disconnect();
	busyConnection_.disconnect();
	titleConnection_.disconnect();
	timerConnection_.disconnect();
	readonlyConnection_.disconnect();
	closingConnection_.disconnect();
	layout_changed_connection_.disconnect();
}


void LyXView::connectBufferView(BufferView & bv)
{
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


BufferView * LyXView::view() const
{
	BOOST_ASSERT(work_area_);
	return &work_area_->bufferView();
}


void LyXView::updateToc()
{
	updateDialog("toc", "");
}


void LyXView::updateToolbars()
{
	BOOST_ASSERT(work_area_);
	bool const math =
		work_area_->bufferView().cursor().inMathed();
	bool const table =
		lyx::getStatus(FuncRequest(LFUN_LAYOUT_TABULAR)).enabled();
	bool const review =
		lyx::getStatus(FuncRequest(LFUN_CHANGES_TRACK)).enabled() &&
		lyx::getStatus(FuncRequest(LFUN_CHANGES_TRACK)).onoff(true);

	toolbars_->update(math, table, review);
	// update redaonly status of open dialogs. This could also be in
	// updateMenubar(), but since updateToolbars() and updateMenubar()
	// are always called together it is only here.
	getDialogs().checkStatus();
}


ToolbarInfo::Flags LyXView::getToolbarState(string const & name)
{
	return toolbars_->getToolbarState(name);
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

	if (view()->buffer())
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
	if (!view()->buffer()) {
		toolbars_->clearLayoutList();
		return;
	}

	// Update the layout display
	if (toolbars_->updateLayoutList(buffer()->params().textclass)) {
		current_layout = buffer()->params().getTextClass().defaultLayoutName();
	}

	BOOST_ASSERT(work_area_);
	if (work_area_->bufferView().cursor().inMathed())
		return;

	string const & layout =
		work_area_->bufferView().cursor().paragraph().layout()->name();

	if (layout != current_layout) {
		toolbars_->setLayout(layout);
		current_layout = layout;
	}
}


void LyXView::updateWindowTitle()
{
	docstring maximize_title = lyx::from_ascii("LyX");
	docstring minimize_title = lyx::from_ascii("LyX");

	if (view()->buffer()) {
		string const cur_title = buffer()->fileName();
		if (!cur_title.empty()) {
			maximize_title += ": " + makeDisplayPath(cur_title, 30);
			minimize_title = lyx::from_utf8(onlyFilename(cur_title));
			if (!buffer()->isClean()) {
				maximize_title += _(" (changed)");
				minimize_title += lyx::char_type('*');
			}
			if (buffer()->isReadonly())
				maximize_title += _(" (read only)");
		}
	}

	setWindowTitle(maximize_title, minimize_title);
	updateTab();
}


void LyXView::dispatch(FuncRequest const & cmd)
{
	theLyXFunc().setLyXView(this);
	lyx::dispatch(cmd);
}


Buffer const * const LyXView::updateInset(Inset const * inset) const
{
	Buffer const * buffer_ptr = 0;
	if (inset) {
		BOOST_ASSERT(work_area_);
		work_area_->scheduleRedraw();

		buffer_ptr = work_area_->bufferView().buffer();
	}
	return buffer_ptr;
}


} // namespace lyx
