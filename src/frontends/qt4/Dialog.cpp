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
#include "BufferParams.h"
#include "BufferView.h"
#include "Cursor.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "LyX.h"

#include "insets/Inset.h"

#include "support/debug.h"
#include "support/gettext.h"
#include "support/lassert.h"

#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QSettings>
#include <QString>
#include <QValidator>

#include <string>

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {

Dialog::Dialog(GuiView & lv, QString const & name, QString const & title)
	: name_(name), title_(title), lyxview_(lv)
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
	lyx::dispatch(fr);
}


void Dialog::updateDialog() const
{
	dispatch(FuncRequest(LFUN_DIALOG_UPDATE, fromqstr(name_)));
}


void Dialog::disconnect() const
{
	lyxview_.disconnectDialog(fromqstr(name_));
}


bool Dialog::isBufferAvailable() const
{
	return lyxview_.currentBufferView() != 0;
}


bool Dialog::isBufferReadonly() const
{
	if (!lyxview_.documentBufferView())
		return true;
	return lyxview_.documentBufferView()->buffer().isReadonly();
}


QString Dialog::bufferFilePath() const
{
	return toqstr(buffer().filePath());
}


KernelDocType Dialog::docType() const
{
	if (buffer().params().isLatex())
		return LATEX;
	if (buffer().params().isLiterate())
		return LITERATE;

	return DOCBOOK;
}


BufferView const * Dialog::bufferview() const
{
	return lyxview_.currentBufferView();
}


Buffer const & Dialog::buffer() const
{
	LAPPERR(lyxview_.currentBufferView());
	return lyxview_.currentBufferView()->buffer();
}


Buffer const & Dialog::documentBuffer() const
{
	LAPPERR(lyxview_.documentBufferView());
	return lyxview_.documentBufferView()->buffer();
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


void Dialog::prepareView()
{
	// Make sure the dialog controls are correctly enabled/disabled with
	// readonly status.
	checkStatus();

	QWidget * w = asQWidget();
	w->setWindowTitle(title_);

	QSize const hint = w->sizeHint();
	if (hint.height() >= 0 && hint.width() >= 0)
		w->setMinimumSize(hint);
}


void Dialog::showView()
{
	prepareView();

	QWidget * w = asQWidget();
	if (w->isVisible()) {
		w->raise();
		w->activateWindow();
	} else
		w->show();

	if (wantInitialFocus())
		w->setFocus();
	else {
		lyxview_.raise();
		lyxview_.activateWindow();
		lyxview_.setFocus();
	}
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


Inset const * Dialog::inset(InsetCode code) const
{
	// ins: the innermost inset of the type we look for
	//      that contains the cursor
	Inset * ins = bufferview()->cursor().innerInsetOfType(code);
	// next: a potential inset at cursor position
	Inset * next = bufferview()->cursor().nextInset();
	// Check if next is of the type we look for
	if (next)
		if (next->lyxCode() != code)
			next = 0;
	if (ins) {
		// prefer next if it is of the requested type (bug 8716)
		if (next)
			ins = next;
	} else
		// no containing inset of requested type
		// use next (which might also be 0)
		ins = next;
	return ins;
}


void Dialog::checkStatus()
{
	// buffer independent dialogs are always active.
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
		enableView(!readonly || canApplyToReadOnly());
		// refreshReadOnly() is too generous in _enabling_ widgets
		// update dialog to disable disabled widgets again

		updateView();
	} else
		enableView(false);
}


QString Dialog::sessionKey() const
{
	return "views/" + QString::number(lyxview_.id())
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
