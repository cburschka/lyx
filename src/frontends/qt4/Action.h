// -*- C++ -*-
/**
 * \file Action.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef ACTION_H
#define ACTION_H

#include "frontends/LyXView.h"
#include "funcrequest.h"

#include "support/docstring.h"

#include <QAction>

//class FuncRequest;

//class string;

namespace lyx {
namespace frontend {

class GuiView;

/**
 * Action - Qt interface with LyX' FuncRequest.
 *
 * Action can be used in LyX menubar and/or toolbars.
 */
class Action: public QAction {
	Q_OBJECT
public:

	Action(GuiView & lyxView, lyx::docstring const & text,
		FuncRequest const & func, lyx::docstring const & tooltip = lyx::docstring());

	Action(GuiView & lyxView, std::string const & icon, lyx::docstring const & text,
		FuncRequest const & func, lyx::docstring const & tooltip = lyx::docstring());

	void update();

//	void setAction(FuncRequest const & func);

Q_SIGNALS:
	/// the current action is triggered
	void triggered(QAction *);

private Q_SLOTS:
	void action();

private:
	FuncRequest const & func_ ;
	GuiView & lyxView_;
};


} // namespace frontend
} // namespace lyx

#endif // ACTION_H
