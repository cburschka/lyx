/**
 * \file Action.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Action.h"

#include "lyx_cb.h"
#include "lyxfunc.h"
#include "FuncStatus.h"
#include "debug.h"

#include "frontends/LyXView.h"

#include "qt_helpers.h"

#include "support/lstrings.h"

#include <boost/bind.hpp>

using lyx::docstring;

using std::string;
using std::endl;

namespace lyx {

namespace frontend {

namespace {

int const statusbar_timer_value = 3000;

} // namespace anon

Action::Action(LyXView & lyxView, docstring const & text,
		FuncRequest const & func, docstring const & tooltip)
	: QAction(this), func_(func), lyxView_(lyxView)
{
	setText(toqstr(text));
	setToolTip(toqstr(tooltip));
	setStatusTip(toqstr(tooltip));
	Q_CONNECT_1(QAction, this, triggered, bool,
				Action, this, action, void);
	update();
}

Action::Action(LyXView & lyxView, string const & icon, docstring const & text,
		FuncRequest const & func, docstring const & tooltip)
		: QAction(this), func_(func), lyxView_(lyxView)
{
	setIcon(QPixmap(icon.c_str()));
	setText(toqstr(text));
	setToolTip(toqstr(tooltip));
	setStatusTip(toqstr(tooltip));
	Q_CONNECT_1(QAction, this, triggered, bool,
				Action, this, action, void);
	update();
}

/*
void Action::setAction(FuncRequest const & func)
{
	func_=func;
}
*/

void Action::update()
{
	FuncStatus const status = lyx::getStatus(func_);

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
//	lyxerr[Debug::ACTION] << "calling LyXFunc::dispatch: func_: " << func_ << endl;

	lyxView_.dispatch(func_);
}

} // namespace frontend
} // namespace lyx

#include "Action_moc.cpp"
