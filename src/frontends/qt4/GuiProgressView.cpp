// -*- C++ -*-
/**
 * \file GuiProgressView.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Peter Kümmel
 * \author Pavel Sanda
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiProgressView.h"

#include "GuiApplication.h"
#include "GuiProgress.h"
#include "qt_helpers.h"

#include "FuncRequest.h"

#include "support/convert.h"
#include "support/debug.h"

#include <QCheckBox>
#include <QDebug>
#include <QSettings>
#include <QTime>

using namespace std;
using namespace lyx::support;

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


namespace{
typedef pair<int, QString> DebugMap;
typedef vector<DebugMap> DebugVector;

bool DebugSorter(DebugMap const & a, DebugMap const & b)
{
	return a.second < b.second;
}
}


GuiProgressView::GuiProgressView(GuiView & parent, Qt::DockWidgetArea area, 
	Qt::WindowFlags flags)
	: DockView(parent, "progress", qt_("Progress/Debug Messages"), area, flags)
{
	eol_last_ = true;
	widget_ = new ProgressViewWidget;
	widget_->setMinimumHeight(150);
	widget_->debugMessagesTW->setSizePolicy(QSizePolicy::Ignored,
						QSizePolicy::Expanding);
	widget_->adjustSize();
	setWidget(widget_);

	widget_->outTE->setFont(guiApp->typewriterSystemFont());
	widget_->tabWidget->widget(0)->setContentsMargins(-5, -7, 0, -7);

	connect(widget_->debugNoneRB, SIGNAL(clicked()),
		this, SLOT(debugSelectionChanged()));
	connect(widget_->debugSelectedRB, SIGNAL(clicked()),
		this, SLOT(debugSelectionChanged()));
	connect(widget_->debugAnyRB, SIGNAL(clicked()),
		this, SLOT(debugSelectionChanged()));
	widget_->debugMessagesTW->setEnabled(false);
	widget_->debugNoneRB->setChecked(true);

	// ignore Debug::NONE and Debug::ANY
	int const level_count = Debug::levelCount() - 1;
	DebugVector dmap;
	for (int i = 1 ; i < level_count; i++) {
		Debug::Type const level = Debug::value(i);
		QString const desc =
			toqstr(from_ascii(Debug::name(level) + " - "))
			+ qt_(Debug::description(level));
		dmap.push_back(DebugMap(level, desc));
	}
	sort(dmap.begin(), dmap.end(), DebugSorter);

	widget_->debugMessagesTW->setColumnCount(2);
	widget_->debugMessagesTW->headerItem()->setText(0, qt_("Debug Level"));
	widget_->debugMessagesTW->headerItem()->setText(1, qt_("Set"));

	DebugVector::const_iterator dit = dmap.begin();
	DebugVector::const_iterator const den = dmap.end();
	for (; dit != den; ++dit) {
		QTreeWidgetItem * item = new QTreeWidgetItem(widget_->debugMessagesTW);
		item->setText(0, dit->second);
		item->setData(0, Qt::UserRole, int(dit->first));
		item->setText(1, qt_("No"));
	}
	widget_->debugMessagesTW->resizeColumnToContents(0);
	widget_->debugMessagesTW->resizeColumnToContents(1);
	connect(widget_->debugMessagesTW,
		SIGNAL(itemActivated(QTreeWidgetItem *, int)),
		this, SLOT(debugMessageActivated(QTreeWidgetItem *, int)));
  
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
			this, SLOT(appendLyXErrText(QString const &)), Qt::QueuedConnection);
		connect(progress, SIGNAL(appendError(QString const &)),
			this, SLOT(appendText(QString const &)));
		connect(progress, SIGNAL(clearMessages()), this, SLOT(clearText()));
		progress->lyxerrConnect();
	}
}


void GuiProgressView::debugMessageActivated(QTreeWidgetItem * item, int)
{
	if (item == 0)
		return;

	QString const no = qt_("No");
	QString const yes = qt_("Yes");

	bool selected = (item->text(1) == yes);
	item->setText(1, selected ? no : yes);

	levelChanged();
}


void GuiProgressView::levelChanged()
{
	int level = Debug::NONE;
	QTreeWidgetItemIterator it(widget_->debugMessagesTW);
	while (*it) {
		if ((*it)->text(1) == qt_("Yes"))
			level |= (*it)->data(0, Qt::UserRole).toInt();
		++it;
	}
	dispatch(FuncRequest(LFUN_DEBUG_LEVEL_SET, convert<string>(level)));
}


void GuiProgressView::debugSelectionChanged()
{
	int level = Debug::NONE;
	if (widget_->debugAnyRB->isChecked())
		level = Debug::ANY;
	else if (widget_->debugSelectedRB->isChecked()) {
		widget_->debugMessagesTW->setEnabled(true);
		levelChanged();
		return;
	}
	QTreeWidgetItemIterator it(widget_->debugMessagesTW);
	while (*it) {
		(*it)->setText(1, level == Debug::NONE ?
				qt_("No") : qt_("Yes"));
		++it;
	}
	widget_->debugMessagesTW->setEnabled(false);
	dispatch(FuncRequest(LFUN_DEBUG_LEVEL_SET, convert<string>(level)));
}


void GuiProgressView::clearText()
{
	if (widget_->autoClearCB->isChecked()){
		widget_->outTE->clear();
		eol_last_ = true;
	}
}


void GuiProgressView::appendLyXErrText(QString const & text)
{
	widget_->outTE->moveCursor(QTextCursor::End);
	widget_->outTE->insertPlainText(text);
	widget_->outTE->ensureCursorVisible();
	eol_last_ = false;
	// Give the user a chance to disable debug messages because
	// showing Debug::ANY messages completely blocks the GUI.
	// Text is not always send as the whole line, so we must be
	// careful about eolns.
	// WARNING: processing events could cause crashes!
	// TODO: find a better solution
	if (text.endsWith("\n")) {
		eol_last_ = true;
		QApplication::processEvents();
	}
}


void GuiProgressView::appendText(QString const & text)
{
	if (text.isEmpty() || !widget_->sbarCB->isChecked())
		return;
	QString str = GuiProgress::currentTime();
	str += ": " + text;
	if (!eol_last_)
		str = "\n" + str;
	eol_last_ = text.endsWith("\n");

	widget_->outTE->moveCursor(QTextCursor::End);
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
	return new GuiProgressView(guiview, Qt::BottomDockWidgetArea);
}



} // namespace frontend
} // namespace lyx

#include "moc_GuiProgressView.cpp"
