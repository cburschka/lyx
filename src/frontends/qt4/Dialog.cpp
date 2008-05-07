/**
 * \file Dialog.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Dialog.h"

#include "GuiView.h"
#include "qt_helpers.h"

#include "Buffer.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "LyXFunc.h"

#include "support/debug.h"
#include "support/lassert.h"

#include <QSettings>
#include <QString>

#include <string>

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {


Dialog::Dialog(GuiView & lv, QString const & name, QString const & title)
	: name_(name), title_(title), lyxview_(&lv)
{}


Dialog::~Dialog()
{}


bool Dialog::canApply() const
{
	FuncRequest const fr(getLfun(), fromqstr(name_));
	FuncStatus const fs(getStatus(fr));
	return fs.enabled();
}


void Dialog::dispatch(FuncRequest const & fr) const
{
	theLyXFunc().setLyXView(lyxview_);
	lyx::dispatch(fr);
}


void Dialog::updateDialog() const
{
	dispatch(FuncRequest(LFUN_DIALOG_UPDATE, fromqstr(name_)));
}


void Dialog::disconnect() const
{
	lyxview_->disconnectDialog(fromqstr(name_));
}


bool Dialog::isBufferAvailable() const
{
	return lyxview_->buffer() != 0;
}


bool Dialog::isBufferReadonly() const
{
	if (!lyxview_->buffer())
		return true;
	return lyxview_->buffer()->isReadonly();
}


QString Dialog::bufferFilepath() const
{
	return toqstr(buffer().filePath());
}


KernelDocType Dialog::docType() const
{
	if (buffer().isLatex())
		return LATEX;
	if (buffer().isLiterate())
		return LITERATE;

	return DOCBOOK;
}


BufferView * Dialog::bufferview()
{
	return lyxview_->view();
}


BufferView const * Dialog::bufferview() const
{
	return lyxview_->view();
}


Buffer & Dialog::buffer()
{
	LASSERT(lyxview_->buffer(), /**/);
	return *lyxview_->buffer();
}


Buffer const & Dialog::buffer() const
{
	LASSERT(lyxview_->buffer(), /**/);
	return *lyxview_->buffer();
}


void Dialog::showData(string const & data)
{
	if (isBufferDependent() && !isBufferAvailable())
		return;

	if (!initialiseParams(data)) {
		LYXERR0("Dialog \"" << name()
			<< "\" failed to translate the data string passed to show()");
		return;
	}

	showView();
}


void Dialog::apply()
{
	if (isBufferDependent()) {
		if (!isBufferAvailable() ||
		    (isBufferReadonly() && !canApplyToReadOnly()))
			return;
	}

	applyView();
	dispatchParams();

	if (disconnectOnApply() && !isClosing()) {
		disconnect();
		initialiseParams(string());
		updateView();
	}
}


void Dialog::showView()
{
	// Make sure the dialog controls are correctly enabled/disabled with
	// readonly status.
	checkStatus();
	if (exitEarly())
		return;

	QWidget * w = asQWidget();
	w->setWindowTitle(title_);

	QSize const hint = w->sizeHint();
	if (hint.height() >= 0 && hint.width() >= 0)
		w->setMinimumSize(hint);

	if (w->isVisible()) {
		w->raise();
		w->activateWindow();
	} else
		w->show();

	w->setFocus();
}


void Dialog::hideView()
{
	QWidget * w = asQWidget();
	if (!w->isVisible())
		return;
	clearParams();
	disconnect();
	w->hide();
}


bool Dialog::isVisibleView() const
{
	return asQWidget()->isVisible();
}


void Dialog::checkStatus()
{
	// buffer independant dialogs are always active.
	// This check allows us leave canApply unimplemented for some dialogs.
	if (!isBufferDependent()) {
		updateView();
		return;
	}

	// deactivate the dialog if we have no buffer
	if (!isBufferAvailable()) {
		enableView(false);
		return;
	}

	// check whether this dialog may be active
	if (canApply()) {
		bool const readonly = isBufferReadonly();
		enableView(!readonly);
		// refreshReadOnly() is too generous in _enabling_ widgets
		// update dialog to disable disabled widgets again

		if (!readonly || canApplyToReadOnly())
			updateView();

	} else
		enableView(false);
}


QString Dialog::sessionKey() const
{
	return "view-" + QString::number(lyxview_->id())
		+ "/" + name();
}


void Dialog::saveSession() const
{
	QSettings settings;
	settings.setValue(sessionKey() + "/geometry", asQWidget()->saveGeometry());
}


void Dialog::restoreSession()
{
	QSettings settings;
	asQWidget()->restoreGeometry(
		settings.value(sessionKey() + "/geometry").toByteArray());
}

} // namespace frontend
} // namespace lyx
