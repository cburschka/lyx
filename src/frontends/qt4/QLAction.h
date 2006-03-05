// -*- C++ -*-
/**
 * \file QLAction.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QLACTION_H
#define QLACTION_H

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
 * QLAction - Qt interface with LyX' FuncRequest.
 *
 * QLAction can be used in LyX menubar and/or toolbars.
 */
class QLAction: public QAction {
	Q_OBJECT
public:

	QLAction(LyXView & lyxView, std::string const & text,
		FuncRequest const & func, std::string const & tooltip="");

	QLAction(LyXView & lyxView, std::string const & icon, std::string const & text,
		FuncRequest const & func, std::string const & tooltip="");

	void update();

//	void setAction(FuncRequest const & func);

private slots:
	void action();

private:
	FuncRequest const & func_ ;
	//FuncRequest func_ ;
	LyXView & lyxView_;
};


} // namespace frontend
} // namespace lyx

#endif // QLACTION_H
