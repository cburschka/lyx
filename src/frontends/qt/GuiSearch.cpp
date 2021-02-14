/**
 * \file GuiSearch.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Edwin Leuven
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiApplication.h"
#include "GuiSearch.h"

#include "lyxfind.h"
#include "qt_helpers.h"
#include "FuncRequest.h"
#include "LyX.h"
#include "BufferView.h"
#include "Buffer.h"
#include "Cursor.h"
#include "FuncRequest.h"
#include "KeyMap.h"
#include "GuiKeySymbol.h"
#include "GuiView.h"

#include "support/debug.h"
#include "support/gettext.h"
#include "frontends/alert.h"
#include "frontends/Clipboard.h"

#include <QClipboard>
#include <QLineEdit>
#include <QSettings>
#include <QShowEvent>
#include "QSizePolicy"

using namespace std;

using lyx::KeySymbol;

namespace lyx {
namespace frontend {

static void uniqueInsert(QComboBox * box, QString const & text)
{
	for (int i = box->count(); --i >= 0; )
		if (box->itemText(i) == text)
			return;

	box->insertItem(0, text);
}


GuiSearchWidget::GuiSearchWidget(QWidget * parent)
	:	QWidget(parent)
{
	setupUi(this);

	// fix height to minimum
	setFixedHeight(sizeHint().height());

	// align items in grid on top
	gridLayout->setAlignment(Qt::AlignTop);

	connect(findPB, SIGNAL(clicked()), this, SLOT(findClicked()));
	connect(findPrevPB, SIGNAL(clicked()), this, SLOT(findPrevClicked()));
	connect(minimizePB, SIGNAL(clicked()), this, SLOT(minimizeClicked()));
	connect(replacePB, SIGNAL(clicked()), this, SLOT(replaceClicked()));
	connect(replacePrevPB, SIGNAL(clicked()), this, SLOT(replacePrevClicked()));
	connect(replaceallPB, SIGNAL(clicked()), this, SLOT(replaceallClicked()));
	connect(findCO, SIGNAL(editTextChanged(QString)),
		this, SLOT(findChanged()));
	if(qApp->clipboard()->supportsFindBuffer()) {
		connect(qApp->clipboard(), SIGNAL(findBufferChanged()),
			this, SLOT(findBufferChanged()));
		findBufferChanged();
	}

	setFocusProxy(findCO);

	findCO->setCompleter(nullptr);
	replaceCO->setCompleter(nullptr);

	replacePB->setEnabled(false);
	replacePrevPB->setEnabled(false);
	replaceallPB->setEnabled(false);
}


void GuiSearchWidget::keyPressEvent(QKeyEvent * ev)
{
	KeySymbol sym;
	setKeySymbol(&sym, ev);

	// catch Return and Shift-Return
	if (ev->key() == Qt::Key_Return) {
		findClicked(ev->modifiers() == Qt::ShiftModifier);
		return;
	}

	// we catch the key sequences for forward and backwards search
	if (sym.isOK()) {
		KeyModifier mod = lyx::q_key_state(ev->modifiers());
		KeySequence keyseq(&theTopLevelKeymap(), &theTopLevelKeymap());
		FuncRequest fr = keyseq.addkey(sym, mod);
		if (fr == FuncRequest(LFUN_WORD_FIND_FORWARD) || fr == FuncRequest(LFUN_WORD_FIND)) {
			findClicked();
			return;
		}
		if (fr == FuncRequest(LFUN_WORD_FIND_BACKWARD)) {
			findClicked(true);
			return;
		}
		if (fr == FuncRequest(LFUN_DIALOG_TOGGLE, "findreplace")) {
			dispatch(fr);
			return;
		}
	}
	QWidget::keyPressEvent(ev);
}


void GuiSearchWidget::minimizeClicked(bool const toggle)
{
	if (toggle)
		minimized_ = !minimized_;

	replaceLA->setHidden(minimized_);
	replaceCO->setHidden(minimized_);
	replacePB->setHidden(minimized_);
	replacePrevPB->setHidden(minimized_);
	replaceallPB->setHidden(minimized_);
	CBFrame->setHidden(minimized_);
	if (minimized_) {
		minimizePB->setText(qt_("Ex&pand"));
		minimizePB->setToolTip("Show replace and option widgets");
	} else {
		minimizePB->setText(qt_("&Minimize"));
		minimizePB->setToolTip("Hide replace and option widgets");
	}

	Q_EMIT needSizeUpdate();
	Q_EMIT needTitleBarUpdate();
}


void GuiSearchWidget::showEvent(QShowEvent * e)
{
	findChanged();
	findPB->setFocus();
	findCO->lineEdit()->selectAll();
	QWidget::showEvent(e);
}


void GuiSearchWidget::findBufferChanged()
{
	docstring search = theClipboard().getFindBuffer();
	if (!search.empty()) {
		LYXERR(Debug::CLIPBOARD, "from findbuffer: " << search);
		findCO->setCurrentText(toqstr(search));
	}
}


void GuiSearchWidget::findChanged()
{
	findPB->setEnabled(!findCO->currentText().isEmpty());
	findPrevPB->setEnabled(!findCO->currentText().isEmpty());
	bool const replace = !findCO->currentText().isEmpty()
			&& bv_ && !bv_->buffer().isReadonly();
	replacePB->setEnabled(replace);
	replacePrevPB->setEnabled(replace);
	replaceallPB->setEnabled(replace);
}


void GuiSearchWidget::findClicked(bool const backwards)
{
	docstring const needle = qstring_to_ucs4(findCO->currentText());
	find(needle, caseCB->isChecked(), wordsCB->isChecked(), !backwards);
	uniqueInsert(findCO, findCO->currentText());
	findCO->lineEdit()->selectAll();
}


void GuiSearchWidget::findPrevClicked()
{
	findClicked(true);
}


void GuiSearchWidget::replaceClicked(bool const backwards)
{
	docstring const needle = qstring_to_ucs4(findCO->currentText());
	docstring const repl = qstring_to_ucs4(replaceCO->currentText());
	replace(needle, repl, caseCB->isChecked(), wordsCB->isChecked(),
		!backwards, false);
	uniqueInsert(findCO, findCO->currentText());
	uniqueInsert(replaceCO, replaceCO->currentText());
}


void GuiSearchWidget::replacePrevClicked()
{
	replaceClicked(true);
}


void GuiSearchWidget::replaceallClicked()
{
	replace(qstring_to_ucs4(findCO->currentText()),
		qstring_to_ucs4(replaceCO->currentText()),
		caseCB->isChecked(), wordsCB->isChecked(), true, true);
	uniqueInsert(findCO, findCO->currentText());
	uniqueInsert(replaceCO, replaceCO->currentText());
}


void GuiSearchWidget::find(docstring const & search, bool casesensitive,
			 bool matchword, bool forward)
{
	docstring const sdata =
		find2string(search, casesensitive, matchword, forward);
	dispatch(FuncRequest(LFUN_WORD_FIND, sdata));
}


void GuiSearchWidget::replace(docstring const & search, docstring const & replace,
			    bool casesensitive, bool matchword,
			    bool forward, bool all)
{
	docstring const sdata =
		replace2string(replace, search, casesensitive,
				     matchword, all, forward);
	dispatch(FuncRequest(LFUN_WORD_REPLACE, sdata));
}

void GuiSearchWidget::saveSession(QSettings & settings, QString const & session_key) const
{
	settings.setValue(session_key + "/casesensitive", caseCB->isChecked());
	settings.setValue(session_key + "/words", wordsCB->isChecked());
	settings.setValue(session_key + "/minimized", minimized_);
}


void GuiSearchWidget::restoreSession(QString const & session_key)
{
	QSettings settings;
	caseCB->setChecked(settings.value(session_key + "/casesensitive", false).toBool());
	wordsCB->setChecked(settings.value(session_key + "/words", false).toBool());
	minimized_ = settings.value(session_key + "/minimized", false).toBool();
	// initialize hidings
	minimizeClicked(false);
}


GuiSearch::GuiSearch(GuiView & parent, Qt::DockWidgetArea area, Qt::WindowFlags flags)
	: DockView(parent, "findreplace", qt_("Search and Replace"), area, flags),
	  widget_(new GuiSearchWidget(this))
{
	setWidget(widget_);
	widget_->setBufferView(bufferview());
	setFocusProxy(widget_->findCO);

	connect(widget_, SIGNAL(needTitleBarUpdate()), this, SLOT(updateTitle()));
	connect(widget_, SIGNAL(needSizeUpdate()), this, SLOT(updateSize()));
}

void GuiSearch::onBufferViewChanged()
{
	widget_->setEnabled((bool)bufferview());
	widget_->setBufferView(bufferview());
}


void GuiSearch::updateView()
{
	updateTitle();
	updateSize();
}


void GuiSearch::saveSession(QSettings & settings) const
{
	Dialog::saveSession(settings);
	widget_->saveSession(settings, sessionKey());
}


void GuiSearch::restoreSession()
{
	DockView::restoreSession();
	widget_->restoreSession(sessionKey());
}


void GuiSearch::updateTitle()
{
	if (widget_->isMinimized()) {
		// remove title bar
		setTitleBarWidget(new QWidget());
		titleBarWidget()->hide();
	} else
		// restore title bar
		setTitleBarWidget(nullptr);
}


void GuiSearch::updateSize()
{
	widget_->setFixedHeight(widget_->sizeHint().height());
	if (widget_->isMinimized())
		setFixedHeight(widget_->sizeHint().height());
	else {
		// undo setFixedHeight
		setMaximumHeight(QWIDGETSIZE_MAX);
		setMinimumHeight(0);
	}
	update();
}


} // namespace frontend
} // namespace lyx


#include "moc_GuiSearch.cpp"
