// -*- C++ -*-
/**
 * \file GuiProgressView.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Peter Kümmel
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiProgressView.h"

#include "qt_helpers.h"

#include "support/debug.h"

#include <QTime>


namespace lyx {
namespace frontend {


ProgressViewWidget::ProgressViewWidget()
{
	setupUi(this);

}


GuiProgressView::~GuiProgressView()
{
	delete widget_;
}


GuiProgressView::GuiProgressView(GuiView & parent, Qt::DockWidgetArea area, 
	Qt::WindowFlags flags) : DockView(parent, "progress", "Debug/Progress window", area, flags)
{
	widget_ = new ProgressViewWidget();
	setWidget(widget_);

	GuiProgress* progress = dynamic_cast<GuiProgress*>(support::ProgressInterface::instance());

	if (progress) {
		connect(progress, SIGNAL(processStarted(QString const &)), this, SLOT(appendText(QString const &)));
		//connect(progress, SIGNAL(processFinished(QString const &)), this, SLOT(appendText(QString const &)));
		connect(progress, SIGNAL(appendMessage(QString const &)), this, SLOT(appendText(QString const &)));
		connect(progress, SIGNAL(appendError(QString const &)), this, SLOT(appendText(QString const &)));
		connect(progress, SIGNAL(clearMessages()), this, SLOT(clearText()));
	}
}


void GuiProgressView::clearText()
{
	widget_->outTE->clear();
}


void GuiProgressView::appendText(QString const & text)
{
	if (text.isEmpty())
		return;
	QString time = QTime::currentTime().toString();
	widget_->outTE->insertPlainText(time + ": " + text.trimmed() + "\n");
	widget_->outTE->ensureCursorVisible();
}



Dialog * createGuiProgressView(GuiView & guiview)
{
#ifdef Q_WS_MACX
	return new GuiProgressView(guiview, Qt::RightDockWidgetArea, Qt::Drawer);
#else
	return new GuiProgressView(guiview, Qt::BottomDockWidgetArea);
#endif
}



} // namespace frontend
} // namespace lyx

#include "moc_GuiProgressView.cpp"
