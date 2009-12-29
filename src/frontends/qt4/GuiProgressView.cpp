// -*- C++ -*-
/**
 * \file GuiProgressView.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Peter Kümmel
 * \author Pavel Sanda
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiProgressView.h"
#include "GuiApplication.h"

#include "qt_helpers.h"

#include "support/debug.h"

#include <QSettings>
#include <QTime>
#include <QGroupBox>
#include <QRadioButton>
#include <QButtonGroup>

namespace lyx {
namespace frontend {


struct LevelButton : QRadioButton
{
	LevelButton(const QString& name) : QRadioButton(name) {}
	Debug::Type level;
};


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

	QFont font(guiApp->typewriterFontName());
	font.setKerning(false);
	font.setFixedPitch(true);
	font.setStyleHint(QFont::TypeWriter);
	widget_->outTE->setFont(font);

	QButtonGroup* button_group = new QButtonGroup(this);
	const std::vector<Debug::Type> levels = Debug::levels();
	for (unsigned int i = 0; i < levels.size(); i++) {
		LevelButton * box = new LevelButton(toqstr(Debug::description(levels[i])));
		box->level = levels[i];
		widget_->settingsLayout->addWidget(box);
		button_group->addButton(box);
	}
	connect(button_group, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(levelChanged(QAbstractButton*)));
	// TODO settings
	button_group->buttons().front()->setChecked(true);
	
	GuiProgress* progress = dynamic_cast<GuiProgress*>(support::ProgressInterface::instance());

	if (progress) {
		connect(progress, SIGNAL(processStarted(QString const &)), this, SLOT(appendText(QString const &)));
		//connect(progress, SIGNAL(processFinished(QString const &)), this, SLOT(appendText(QString const &)));
		connect(progress, SIGNAL(appendMessage(QString const &)), this, SLOT(appendText(QString const &)));
		connect(progress, SIGNAL(appendError(QString const &)), this, SLOT(appendText(QString const &)));
		connect(progress, SIGNAL(clearMessages()), this, SLOT(clearText()));
		progress->lyxerrConnect();
	}
}


void GuiProgressView::levelChanged(QAbstractButton* b)
{
	LevelButton* lb = dynamic_cast<LevelButton*>(b);
	if (lb)
		lyxerr.level(lb->level);
}


void GuiProgressView::clearText()
{
	if (widget_->autoClearCB->isChecked())
		widget_->outTE->clear();
}


void GuiProgressView::appendText(QString const & text)
{
	if (text.isEmpty())
		return;
	QString time = QTime::currentTime().toString();
	if (text.endsWith("\n"))
		widget_->outTE->insertPlainText(time + ": " + text);
	else
		widget_->outTE->insertPlainText(text);

	widget_->outTE->ensureCursorVisible();
}


void GuiProgressView::saveSession() const
{
	Dialog::saveSession();
	QSettings settings;
	settings.setValue(
		sessionKey() + "/autoclear", widget_->autoClearCB->isChecked());
}


void GuiProgressView::restoreSession()
{
	DockView::restoreSession();
	QSettings settings;
	widget_->autoClearCB->setChecked(
		settings.value(sessionKey() + "/autoclear", true).toBool());
}


void GuiProgressView::showEvent(QShowEvent*)
{
	support::ProgressInterface::instance()->lyxerrConnect();
}


void GuiProgressView::hideEvent(QHideEvent*)
{
	support::ProgressInterface::instance()->lyxerrDisconnect();
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
