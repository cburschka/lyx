/**
 * \file LyXView.C
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
#include "Gui.h"
#include "Dialogs.h"
#include "Timeout.h"
#include "Toolbars.h"
#include "Menubar.h"
#include "WorkArea.h"

#include "buffer.h"
#include "bufferparams.h"
#include "BufferView.h"
#include "bufferview_funcs.h"
#include "cursor.h"
#include "debug.h"
#include "funcrequest.h"
#include "gettext.h"
#include "intl.h"
#include "lyx_cb.h"
#include "lyxfunc.h"
#include "lyxrc.h"
#include "lyxtext.h"
#include "MenuBackend.h"
#include "paragraph.h"

#include "controllers/ControlCommandBuffer.h"

#include "support/filetools.h" // OnlyFilename()

#include <boost/bind.hpp>

#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

using lyx::frontend::Gui;
using lyx::frontend::WorkArea;

using lyx::support::makeDisplayPath;
using lyx::support::onlyFilename;

using std::endl;
using std::string;

using lyx::frontend::ControlCommandBuffer;

string current_layout;

Gui & LyXView::gui()
{
	return owner_;
}


LyXView::LyXView(Gui & owner)
	: work_area_(0),
	  owner_(owner),
	  toolbars_(new Toolbars(*this)),
	  intl_(new Intl),
	  autosave_timeout_(new Timeout(5000)),
	  lyxfunc_(new LyXFunc(this)),
	  dialogs_(new Dialogs(*this)),
	  controlcommand_(new ControlCommandBuffer(*this))
{
	lyxerr[Debug::INIT] << "Initializing LyXFunc" << endl;
}

LyXView::~LyXView()
{
}


void LyXView::setWorkArea(WorkArea * work_area)
{
	work_area_ = work_area;
}


void LyXView::redrawWorkArea()
{
	work_area_->redraw();
	updateStatusBar();
}


WorkArea * LyXView::workArea()
{
	return work_area_;
}


void LyXView::init()
{
	updateLayoutChoice();
	updateMenubar();

	// Start autosave timer
	if (lyxrc.autosave) {
		autosave_timeout_->timeout.connect(boost::bind(&LyXView::autoSave, this));
		autosave_timeout_->setTimeout(lyxrc.autosave * 1000);
		autosave_timeout_->start();
	}

	intl_->initKeyMapper(lyxrc.use_kbmap);
}


Buffer * LyXView::buffer() const
{
	return work_area_->bufferView().buffer();
}


void LyXView::setBuffer(Buffer * b)
{
	work_area_->bufferView().setBuffer(b);
	updateMenubar();
	updateToolbars();
	updateLayoutChoice();
	updateWindowTitle();
	if (b)
		setLayout(work_area_->bufferView().firstLayout());
	redrawWorkArea();
}


bool LyXView::loadLyXFile(string const & filename, bool tolastfiles)
{
	bool loaded = work_area_->bufferView().loadLyXFile(filename, tolastfiles);
	updateMenubar();
	updateToolbars();
	updateLayoutChoice();
	updateWindowTitle();
	if (loaded)
		setLayout(work_area_->bufferView().firstLayout());
	redrawWorkArea();
	return loaded;
}

BufferView * LyXView::view() const
{
	return &work_area_->bufferView();
}


void LyXView::setLayout(string const & layout)
{
	toolbars_->setLayout(layout);
}


void LyXView::updateToolbars()
{
	bool const math = work_area_->bufferView().cursor().inMathed();
	bool const table =
		getLyXFunc().getStatus(FuncRequest(LFUN_LAYOUT_TABULAR)).enabled();
	toolbars_->update(math, table);
	// update redaonly status of open dialogs. This could also be in
	// updateMenubar(), but since updateToolbars() and updateMenubar()
	// are always called together it is only here.
	getDialogs().checkStatus();
}


void LyXView::updateMenubar()
{
	menubar_->update();
}


void LyXView::autoSave()
{
	lyxerr[Debug::INFO] << "Running autoSave()" << endl;

	if (view()->available()) {
		::autoSave(view());
	}
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
		current_layout = buffer()->params().getLyXTextClass().defaultLayoutName();
	}

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
	static string last_title = "LyX";
	string maximize_title = "LyX";
	string minimize_title = "LyX";

	if (view()->available()) {
		string const cur_title = buffer()->fileName();
		if (!cur_title.empty()) {
			maximize_title += ": " + makeDisplayPath(cur_title, 30);
			minimize_title = onlyFilename(cur_title);
			if (!buffer()->isClean()) {
				maximize_title += _(" (changed)");
				minimize_title += '*';
			}
			if (buffer()->isReadonly())
				maximize_title += _(" (read only)");
		}
	}

	if (maximize_title != last_title) {
		setWindowTitle(maximize_title, minimize_title);
		last_title = maximize_title;
	}
}


void LyXView::dispatch(FuncRequest const & cmd)
{
	getLyXFunc().dispatch(cmd);
}


Buffer const * const LyXView::updateInset(InsetBase const * inset) const
{
	Buffer const * buffer_ptr = 0;
	if (inset) {
		buffer_ptr = work_area_->bufferView().buffer();
		// No FitCursor:
		work_area_->bufferView().update(Update::Force);
	}
	return buffer_ptr;
}
