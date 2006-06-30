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

// Must be here because of moc.
#include <config.h>

#include "frontends/LyXView.h"
#include "funcrequest.h"

#include <QAction>

//class FuncRequest;

//class string;

namespace lyx {
namespace frontend {

/**
 * Action - Qt interface with LyX' FuncRequest.
 *
 * Action can be used in LyX menubar and/or toolbars.
 */
class Action: public QAction {
	Q_OBJECT
public:

	Action(LyXView & lyxView, std::string const & text,
		FuncRequest const & func, std::string const & tooltip="");

	Action(LyXView & lyxView, std::string const & icon, std::string const & text,
		FuncRequest const & func, std::string const & tooltip="");

	void update();

//	void setAction(FuncRequest const & func);

private Q_SLOTS:
	void action();

private:
	FuncRequest const & func_ ;
	//FuncRequest func_ ;
	LyXView & lyxView_;
};


} // namespace frontend
} // namespace lyx

#endif // ACTION_H
