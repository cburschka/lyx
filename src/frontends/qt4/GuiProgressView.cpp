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
#include "FuncRequest.h"

#include "support/debug.h"
#include "support/convert.h"

#include <QSettings>
#include <QTime>

#include <QCheckBox>


namespace lyx {
namespace frontend {


struct LevelButton : QCheckBox
{
	LevelButton(const QString& name) : QCheckBox(name) {}
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
	widget_->tabWidget->widget(0)->setContentsMargins(-5,-7,0,-7);


	const int levelCount = Debug::levelCount();
	for (int i = 1; i <= levelCount; i++) {
		const Debug::Type level = Debug::value(i);
		LevelButton * box = new LevelButton(toqstr(Debug::description(level)));
		box->level = level;
		widget_->settingsLayout->addWidget(box,i%10,i/10);
		// TODO settings
		box->setChecked(false);
		level_buttons << box;
		connect(box, SIGNAL(stateChanged(int)), this, SLOT(levelChanged()));
	}

	
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


void GuiProgressView::levelChanged()
{
	int level = Debug::NONE;
	Q_FOREACH(const LevelButton* button, level_buttons) {
		if (button->isChecked()) {
			// Debug::NONE overwrites other levels
			if (button->level == Debug::NONE) {
				level = Debug::NONE;
				break;
			} else {
				level |= button->level;
			}
		}
	}
	dispatch(FuncRequest(LFUN_DEBUG_LEVEL_SET, convert<std::string>(level)));
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
