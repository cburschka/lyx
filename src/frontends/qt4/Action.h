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
#include "FuncRequest.h"

#include "support/docstring.h"

#include <QAction>

namespace lyx {
namespace frontend {

class GuiViewBase;

/**
 * Action - Qt interface with LyX' FuncRequest.
 *
 * Action can be used in LyX menubar and/or toolbars.
 */
class Action : public QAction {
	Q_OBJECT
public:

	Action(GuiViewBase & lyxView, docstring const & text,
		FuncRequest const & func, docstring const & tooltip = docstring());

	Action(GuiViewBase & lyxView, std::string const & icon, docstring const & text,
		FuncRequest const & func, docstring const & tooltip = docstring());

	void update();

//	void setAction(FuncRequest const & func);

Q_SIGNALS:
	/// the current action is triggered
	void triggered(QAction *);

private Q_SLOTS:
	void action();

private:
	FuncRequest const & func_ ;
	GuiViewBase & lyxView_;
};


} // namespace frontend
} // namespace lyx

#endif // ACTION_H
