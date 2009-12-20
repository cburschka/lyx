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
#include <QDebug>


namespace lyx {
namespace frontend {



GuiProgress::GuiProgress(GuiView & parent, Qt::DockWidgetArea area, 
	Qt::WindowFlags flags) : DockView(parent, "progress", "External tools", area, flags)
{
	setWindowTitle(qt_("Tool monitoring"));
	setWidget(&text_edit);
	text_edit.setReadOnly(true);

	connect(this, SIGNAL(processStarted(QString const &)), SLOT(doProcessStarted(QString const &)));
	connect(this, SIGNAL(processFinished(QString const &)), SLOT(doProcessFinished(QString const &)));
	connect(this, SIGNAL(appendMessage(QString const &)), SLOT(doAppendMessage(QString const &)));
	connect(this, SIGNAL(appendError(QString const &)), SLOT(doAppendError(QString const &)));
	connect(this, SIGNAL(clearMessages()), SLOT(doClearMessages()));
}


void GuiProgress::doProcessStarted(QString const & cmd)
{
	appendText("Process started : " + cmd + "\n");
}


void GuiProgress::doProcessFinished(QString const & cmd)
{
	appendText("Process finished: " + cmd + "\n");
}


void GuiProgress::doAppendMessage(QString const & msg)
{
	// No good messages from the processes
	//appendText(msg);
}


void GuiProgress::doAppendError(QString const & msg)
{
	appendText(msg);
}


void GuiProgress::doClearMessages()
{
	text_edit.clear();
}


void GuiProgress::appendText(QString const & text)
{
	text_edit.insertPlainText(text);
	text_edit.ensureCursorVisible();
}


void GuiProgress::showEvent(QShowEvent*)
{
	support::ProgressInterface::setInstance(this);
}


void GuiProgress::hideEvent(QHideEvent*)
{
	support::ProgressInterface::setInstance(0);
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

#include "moc_GuiProgress.cpp"