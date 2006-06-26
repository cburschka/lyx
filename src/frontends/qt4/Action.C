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

#include "BufferView.h"
#include "lyx_cb.h"
#include "lyxfunc.h"
#include "FuncStatus.h"

#include "debug.h"

#include <boost/bind.hpp>

#include "frontends/LyXView.h"
#include "qt_helpers.h"

#include "Action.h"

#include "support/lstrings.h"

using std::string;
using std::endl;

namespace lyx {

namespace frontend {

namespace {

int const statusbar_timer_value = 3000;

} // namespace anon

Action::Action(LyXView & lyxView, string const & text,
		FuncRequest const & func, string const & tooltip)
		: QAction(this), lyxView_(lyxView), func_(func)
{
	setText(tr(toqstr(text)));
	setToolTip(tr(toqstr(tooltip)));
	setStatusTip(toqstr(tooltip));
	connect(this, SIGNAL(triggered()), this, SLOT(action()));
	this->setCheckable(true);
}

Action::Action(LyXView & lyxView, string const & icon, string const & text,
		FuncRequest const & func, string const & tooltip)
		: QAction(this), lyxView_(lyxView), func_(func)
{
	setIcon(QPixmap(icon.c_str()));
	setText(tr(toqstr(text)));
	setToolTip(tr(toqstr(tooltip)));
	setStatusTip(toqstr(tooltip));
	connect(this, SIGNAL(triggered()), this, SLOT(action()));
	this->setCheckable(true);
}

/*
void Action::setAction(FuncRequest const & func)
{
	func_=func;
}
*/

void Action::update()
{
	FuncStatus const status = lyxView_.getLyXFunc().getStatus(func_);

	this->setChecked(status.onoff(true));
	this->setEnabled(status.enabled());
}


void Action::action()
{
//	lyxerr[Debug::ACTION] << "calling LyXFunc::dispatch: func_: " << func_ << endl;

	lyxView_.getLyXFunc().dispatch(func_);
}

} // namespace frontend
} // namespace lyx

#include "Action_moc.cpp"
