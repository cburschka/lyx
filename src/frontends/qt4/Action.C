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

#include "GuiView.h"
#include "qt_helpers.h"

#include "lyx_cb.h"
#include "lyxfunc.h"
#include "FuncStatus.h"
#include "debug.h"

#include "support/lstrings.h"

#include <boost/bind.hpp>

using lyx::docstring;

using std::string;
using std::endl;

namespace lyx {

namespace frontend {


Action::Action(GuiView & lyxView, docstring const & text,
		FuncRequest const & func, docstring const & tooltip)
	: QAction(&lyxView), func_(func), lyxView_(lyxView)
{
#if QT_VERSION >= 0x040200
	// only Qt/Mac handles that
	setMenuRole(NoRole);
#endif
	setText(toqstr(text));
	setToolTip(toqstr(tooltip));
	setStatusTip(toqstr(tooltip));
	connect(this, SIGNAL(triggered()), this, SLOT(action()));
	update();
}

Action::Action(GuiView & lyxView, string const & icon, docstring const & text,
		FuncRequest const & func, docstring const & tooltip)
		: QAction(&lyxView), func_(func), lyxView_(lyxView)
{
	setIcon(QPixmap(icon.c_str()));
	setText(toqstr(text));
	setToolTip(toqstr(tooltip));
	setStatusTip(toqstr(tooltip));
	connect(this, SIGNAL(triggered()), this, SLOT(action()));
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
//	LYXERR(Debug::ACTION) << "calling LyXFunc::dispatch: func_: " << func_ << endl;

	lyxView_.dispatch(func_);
	triggered(this);
}

} // namespace frontend
} // namespace lyx

#include "Action_moc.cpp"
