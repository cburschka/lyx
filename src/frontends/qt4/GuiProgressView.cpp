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
#include <QDebug>

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {


struct LevelButton : QCheckBox
{
	LevelButton(const QString& name) : QCheckBox(name) {}
	Debug::Type level;
	
	void setCheckStatusSilent(Qt::CheckState state) {
		blockSignals(true);
		setCheckState(state);
		blockSignals(false);	
	}
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
#if QT_VERSION < 0x040400
	widget_->scrollArea->setWidget(widget_->scrollAreaWidgetContents);
#endif
	widget_->setMinimumHeight(150);
	widget_->adjustSize();
	setWidget(widget_);

	QFont font(guiApp->typewriterFontName());
	font.setKerning(false);
	font.setFixedPitch(true);
	font.setStyleHint(QFont::TypeWriter);
	widget_->outTE->setFont(font);
	widget_->tabWidget->widget(0)->setContentsMargins(-5, -7, 0, -7);

	toggle_button = new LevelButton("Toggle ALL messages");
	toggle_button->level = Debug::ANY;
	toggle_button->setTristate(true);
	toggle_button->setCheckState(Qt::PartiallyChecked);
	widget_->settingsLayout->addWidget(toggle_button);
	connect(toggle_button, SIGNAL(stateChanged(int)), this, SLOT(tristateChanged(int)));

	// ignore Debug::NONE and Debug::ANY
	int const level_count = Debug::levelCount() - 1;
	for (int i = 1 ; i < level_count; i++) {
		Debug::Type const level = Debug::value(i);
		LevelButton * box = new LevelButton(toqstr(Debug::description(level)));
		box->level = level;
		widget_->settingsLayout->addWidget(box, (i + 3) % 10, (i + 3) / 10);
		box->setChecked(false);
		level_buttons << box;
		connect(box, SIGNAL(stateChanged(int)), this, SLOT(levelChanged()));
	}
	widget_->settingsLayout->activate();
	
	GuiProgress * progress =
		dynamic_cast<GuiProgress *>(ProgressInterface::instance());

	if (progress) {
		connect(progress, SIGNAL(processStarted(QString const &)),
			this, SLOT(appendText(QString const &)));
		//connect(progress, SIGNAL(processFinished(QString const &)),
		//	this, SLOT(appendText(QString const &)));
		connect(progress, SIGNAL(appendMessage(QString const &)),
			this, SLOT(appendText(QString const &)));
		connect(progress, SIGNAL(appendLyXErrMessage(QString const &)),
			this, SLOT(appendLyXErrText(QString const &)));
		connect(progress, SIGNAL(appendError(QString const &)),
			this, SLOT(appendText(QString const &)));
		connect(progress, SIGNAL(clearMessages()), this, SLOT(clearText()));
		progress->lyxerrConnect();
	}
}


void GuiProgressView::levelChanged()
{
	int level = Debug::NONE;
	checked_buttons.clear();
	Q_FOREACH(LevelButton* button, level_buttons) {
		if (button->isChecked()) {
			level |= button->level;
			checked_buttons << button;
		}
	}
	dispatch(FuncRequest(LFUN_DEBUG_LEVEL_SET, convert<string>(level)));
	
	toggle_button->setCheckStatusSilent (Qt::PartiallyChecked);
}


void GuiProgressView::tristateChanged(int state)
{
	if (state != Qt::PartiallyChecked) {
		Q_FOREACH(LevelButton* button, level_buttons) {
			button->setCheckStatusSilent(toggle_button->checkState());
		}
		int level = (state == Qt::Checked ? Debug::ANY : Debug::NONE);
		dispatch(FuncRequest(LFUN_DEBUG_LEVEL_SET, convert<string>(level)));
	} else {
		Q_FOREACH(LevelButton* button, checked_buttons) {
			button->setCheckStatusSilent(Qt::Checked);
		}
		levelChanged();
	}
}


void GuiProgressView::clearText()
{
	if (widget_->autoClearCB->isChecked())
		widget_->outTE->clear();
}


void GuiProgressView::appendLyXErrText(QString const & text)
{
	widget_->outTE->insertPlainText(text);
	widget_->outTE->ensureCursorVisible();

	// Give the user a chance to disable debug messages because
	// showing Debug::ANY messages completely blocks the GUI.
	// Text is not always send as the whole line, so we must be
	// careful about eolns.
	if (text.endsWith("\n"))
		QApplication::processEvents();
}


void GuiProgressView::appendText(QString const & text)
{
	if (text.isEmpty() || !widget_->sbarCB->isChecked())
		return;
	QString str = QTime::currentTime().toString();
	str += ": " + text;
	if (!text.endsWith("\n"))
		str += "\n";

	widget_->outTE->insertPlainText(str);
	widget_->outTE->ensureCursorVisible();
}


void GuiProgressView::saveSession() const
{
	Dialog::saveSession();
	QSettings settings;
	settings.setValue(
		sessionKey() + "/autoclear", widget_->autoClearCB->isChecked());
	settings.setValue(
		sessionKey() + "/statusbarmsgs", widget_->sbarCB->isChecked());
}


void GuiProgressView::restoreSession()
{
	DockView::restoreSession();
	QSettings settings;
	widget_->autoClearCB->setChecked(
		settings.value(sessionKey() + "/autoclear", true).toBool());
	widget_->sbarCB->setChecked(
		settings.value(sessionKey() + "/statusbarmsgs", true).toBool());
}


void GuiProgressView::showEvent(QShowEvent*)
{
	ProgressInterface::instance()->lyxerrConnect();
}


void GuiProgressView::hideEvent(QHideEvent*)
{
	ProgressInterface::instance()->lyxerrDisconnect();
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
