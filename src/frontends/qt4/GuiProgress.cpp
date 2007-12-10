// -*- C++ -*-
/**
 * \file GuiProgress.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Peter Kümmel
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiProgress.h"

#include "qt_helpers.h"

#include "support/Systemcall.h"

#include <QApplication>


namespace lyx {
namespace frontend {


GuiProgress::GuiProgress(GuiView & parent, Qt::DockWidgetArea area, 
	Qt::WindowFlags flags) : DockView(parent, "latex-progress", area, flags)
{
	setWindowTitle(qt_("LaTeX Progress"));
	setWidget(&text_edit);
	support::Systemcall::registerProgressInterface(this);
}


void GuiProgress::appendMessage(QString const & msg)
{
	text_edit.append(msg);
	// QEventLoop::ExcludeUserInputEvents: 
	// don't allow user inputs while processing a document
	// if we allow it, we open will Pandora's Box of multithreading
	QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}


void GuiProgress::clearMessages()
{
	text_edit.clear();
}


Dialog * createGuiProgress(GuiView & lv)
{
	GuiView & guiview = static_cast<GuiView &>(lv);
#ifdef Q_WS_MACX
	// TODO where to show up on the Mac?
	//return new GuiProgress(guiview, Qt::RightDockWidgetArea, Qt::Drawer);
#else
	return new GuiProgress(guiview, Qt::BottomDockWidgetArea);
#endif
}

} // namespace frontend
} // namespace lyx


