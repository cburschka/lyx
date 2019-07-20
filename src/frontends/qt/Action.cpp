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

// DispatchResult.h is needed by the windows compiler because lyx::dispatch
// returns a DispatchResult const reference. Gcc does not complain. Weird...
#include "DispatchResult.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "LyX.h"

#include "qt_helpers.h"

#include "support/debug.h"
#include "support/lstrings.h"

using namespace std;


namespace lyx {
namespace frontend {


Action::Action(FuncRequest func, QIcon const & icon, QString const & text,
               QString const & tooltip, QObject * parent)
	: QAction(parent), func_(make_shared<FuncRequest>(move(func)))
{
	init(icon, text, tooltip);
}


Action::Action(shared_ptr<FuncRequest const> func,
               QIcon const & icon, QString const & text,
               QString const & tooltip, QObject * parent)
	: QAction(parent), func_(func)
{
	init(icon, text, tooltip);
}


void Action::init(QIcon const & icon, QString const & text,
                  QString const & tooltip)
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
	FuncStatus const status = getStatus(*func_);

	if (status.onOff(true)) {
		setCheckable(true);
		setChecked(true);
	} else if (status.onOff(false)) {
		setCheckable(true);
		setChecked(false);
	} else {
		setCheckable(false);
	}

	setEnabled(status.enabled());
}


void Action::action()
{
	//LYXERR(Debug::ACTION, "calling lyx::dispatch: func_: ");

	lyx::dispatch(*func_);
	triggered(this);
}

} // namespace frontend
} // namespace lyx

#include "moc_Action.cpp"
