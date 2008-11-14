/**
 * \file Action.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Action.h"

#include "FuncRequest.h"
#include "FuncStatus.h"
#include "GuiView.h"
#include "LyXFunc.h"
#include "qt_helpers.h"

#include "support/debug.h"
#include "support/lstrings.h"

namespace lyx {
namespace frontend {


Action::Action(GuiView * lyxView, QIcon const & icon,
	  QString const & text, FuncRequest const & func,
	  QString const & tooltip, QObject * parent)
	: QAction(parent), func_(func), lyxView_(lyxView)
{
	// only Qt/Mac handles that
	setMenuRole(NoRole);
	setIcon(icon);
	setText(text);
	setToolTip(tooltip);
	setStatusTip(tooltip);
	connect(this, SIGNAL(triggered()), this, SLOT(action()));
	update();
}


void Action::update()
{
	FuncStatus const status = getStatus(func_);

	if (status.onoff(true)) {
		setCheckable(true);
		setChecked(true);
	} else if (status.onoff(false)) {
		setCheckable(true);
		setChecked(false);
	} else {
		setCheckable(false);
	}

	setEnabled(status.enabled());
}


void Action::action()
{
	//LYXERR(Debug::ACTION, "calling LyXFunc::dispatch: func_: ");

	if (lyxView_)
		theLyXFunc().setLyXView(lyxView_);

	lyx::dispatch(func_);
	triggered(this);
}

} // namespace frontend
} // namespace lyx

#include "moc_Action.cpp"
