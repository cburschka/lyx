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
#include "Dialogs.h"
#include "Timeout.h"
#include "Toolbars.h"
#include "Menubar.h"

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

#include <sys/time.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

using lyx::support::MakeDisplayPath;
using lyx::support::OnlyFilename;

using std::endl;
using std::string;

using lyx::frontend::ControlCommandBuffer;

string current_layout;


LyXView::LyXView()
	: toolbars_(new Toolbars(*this)),
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

	intl_->InitKeyMapper(lyxrc.use_kbmap);
}


Buffer * LyXView::buffer() const
{
	return bufferview_->buffer();
}


boost::shared_ptr<BufferView> const & LyXView::view() const
{
	return bufferview_;
}


void LyXView::setLayout(string const & layout)
{
	toolbars_->setLayout(layout);
}


void LyXView::updateToolbars()
{
	bool const math = bufferview_->cursor().inMathed();
	bool const table =
		getLyXFunc().getStatus(FuncRequest(LFUN_LAYOUT_TABULAR)).enabled();
	toolbars_->update(math, table);
}


void LyXView::updateMenubar()
{
	menubar_->update();
}


void LyXView::autoSave()
{
	lyxerr[Debug::INFO] << "Running autoSave()" << endl;

	if (view()->available()) {
		::AutoSave(view().get());
	}
}


void LyXView::resetAutosaveTimer()
{
	if (lyxrc.autosave)
		autosave_timeout_->restart();
}


void LyXView::updateLayoutChoice()
{
	// don't show any layouts without a buffer
	if (!view()->buffer()) {
		toolbars_->clearLayoutList();
		return;
	}

	// update the layout display
	if (toolbars_->updateLayoutList(buffer()->params().textclass)) {
		current_layout = buffer()->params().getLyXTextClass().defaultLayoutName();
	}

	if (bufferview_->cursor().inMathed())
		return;

	string const & layout =
		bufferview_->cursor().paragraph().layout()->name();

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
			maximize_title += ": " + MakeDisplayPath(cur_title, 30);
			minimize_title = OnlyFilename(cur_title);
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
		buffer_ptr = bufferview_->buffer();
		bufferview_->update();
	}
	return buffer_ptr;
}
