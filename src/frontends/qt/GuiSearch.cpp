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

#include "qt_helpers.h"
#include "support/filetools.h"
#include "support/debug.h"
#include "support/gettext.h"
#include "support/FileName.h"
#include "frontends/alert.h"
#include "frontends/Clipboard.h"

#include <QClipboard>
#include <QPainter>
#include <QLineEdit>
#include <QSettings>
#include <QShowEvent>
#include "QSizePolicy"
#if QT_VERSION >= 0x050000
#include <QSvgRenderer>
#endif

using namespace std;
using namespace lyx::support;

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

	// Use a FancyLineEdit due to the indicator icons
	findLE_ = new FancyLineEdit(this);
	findCO->setLineEdit(findLE_);

	// And a menu in minimal mode
	menu_ = new QMenu();
	act_casesense_ = new QAction(qt_("&Case sensitive[[search]]"), this);
	act_casesense_->setCheckable(true);
	act_wholewords_ = new QAction(qt_("Wh&ole words"), this);
	act_wholewords_->setCheckable(true);
	act_selection_ = new QAction(qt_("Selection onl&y"), this);
	act_selection_->setCheckable(true);
	act_immediate_ = new QAction(qt_("Search as yo&u type"), this);
	act_immediate_->setCheckable(true);
	act_wrap_ = new QAction(qt_("&Wrap"), this);
	act_wrap_->setCheckable(true);

	menu_->addAction(act_casesense_);
	menu_->addAction(act_wholewords_);
	menu_->addAction(act_selection_);
	menu_->addAction(act_immediate_);
	menu_->addAction(act_wrap_);
	findLE_->setButtonMenu(FancyLineEdit::Right, menu_);

	connect(act_casesense_, SIGNAL(triggered()), this, SLOT(caseSenseActTriggered()));
	connect(act_wholewords_, SIGNAL(triggered()), this, SLOT(wholeWordsActTriggered()));
	connect(act_selection_, SIGNAL(triggered()), this, SLOT(searchSelActTriggered()));
	connect(act_immediate_, SIGNAL(triggered()), this, SLOT(immediateActTriggered()));
	connect(act_wrap_, SIGNAL(triggered()), this, SLOT(wrapActTriggered()));

	findCO->setCompleter(nullptr);
	replaceCO->setCompleter(nullptr);

	replacePB->setEnabled(false);
	replacePrevPB->setEnabled(false);
	replaceallPB->setEnabled(false);
}


bool GuiSearchWidget::initialiseParams(std::string const & str)
{
	if (!str.empty())
		findCO->lineEdit()->setText(toqstr(str));
	return true;
}


void GuiSearchWidget::keyPressEvent(QKeyEvent * ev)
{
	KeySymbol sym;
	setKeySymbol(&sym, ev);

	// catch Return and Shift-Return
	if (ev->key() == Qt::Key_Return || ev->key() == Qt::Key_Enter) {
		doFind(ev->modifiers() == Qt::ShiftModifier);
		return;
	}
	if (ev->key() == Qt::Key_Escape) {
		dispatch(FuncRequest(LFUN_DIALOG_HIDE, "findreplace"));
		return;
	}

	// we catch the key sequences for forward and backwards search
	if (sym.isOK()) {
		KeyModifier mod = lyx::q_key_state(ev->modifiers());
		KeySequence keyseq(&theTopLevelKeymap(), &theTopLevelKeymap());
		FuncRequest fr = keyseq.addkey(sym, mod);
		if (fr == FuncRequest(LFUN_WORD_FIND_FORWARD)
		    || fr == FuncRequest(LFUN_WORD_FIND)) {
			doFind();
			return;
		}
		if (fr == FuncRequest(LFUN_WORD_FIND_BACKWARD)) {
			doFind(true);
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
		minimizePB->setToolTip(qt_("Show replace and option widgets"));
		// update menu items
		blockSignals(true);
		act_casesense_->setChecked(caseCB->isChecked());
		act_immediate_->setChecked(instantSearchCB->isChecked());
		act_selection_->setChecked(selectionCB->isChecked());
		act_wholewords_->setChecked(wordsCB->isChecked());
		act_wrap_->setChecked(wrapCB->isChecked());
		blockSignals(false);
	} else {
		minimizePB->setText(qt_("&Minimize"));
		minimizePB->setToolTip(qt_("Hide replace and option widgets"));
	}

	Q_EMIT needSizeUpdate();
	Q_EMIT needTitleBarUpdate();
	handleIndicators();
}


void GuiSearchWidget::handleIndicators()
{
	findLE_->setButtonVisible(FancyLineEdit::Right, minimized_);

	QString tip;

	if (minimized_) {
		int pms = 0;
		if (caseCB->isChecked())
			++pms;
		if (wordsCB->isChecked())
			++pms;
		if (selectionCB->isChecked())
			++pms;
		if (instantSearchCB->isChecked())
			++pms;
		if (wrapCB->isChecked())
			++pms;

		bool const dark_mode = guiApp && guiApp->isInDarkMode();
		qreal dpr = 1.0;
#if QT_VERSION >= 0x050000
		// Consider device/pixel ratio (HiDPI)
		if (guiApp && guiApp->currentView())
			dpr = guiApp->currentView()->devicePixelRatio();
#endif
		QString imagedir = "images/";
		QPixmap bpixmap = getPixmap("images/", "search-options", "svgz,png");
		QPixmap pm = bpixmap;

		if (pms > 0) {
			int const gap = 3;
			QPixmap scaled_pm = QPixmap(bpixmap.size() * dpr);
			pm = QPixmap(pms * scaled_pm.width() + ((pms - 1) * gap),
				     scaled_pm.height());
			pm.fill(Qt::transparent);
			QPainter painter(&pm);
			int x = 0;
			
			tip = qt_("Active options:");
			tip += "<ul>";
			if (caseCB->isChecked()) {
				tip += "<li>" + qt_("Case sensitive search");
				QPixmap spixmap = getPixmap("images/", "search-case-sensitive", "svgz,png");
#if QT_VERSION < 0x050000
				painter.drawPixmap(x, 0, spixmap);
#else
				// With Qt5, we render SVG directly for HiDPI scalability
				FileName fname = imageLibFileSearch(imagedir, "search-case-sensitive", "svgz,png");
				QString fpath = toqstr(fname.absFileName());
				if (!fpath.isEmpty()) {
					QSvgRenderer svgRenderer(fpath);
					if (svgRenderer.isValid())
						svgRenderer.render(&painter, QRectF(0, 0, spixmap.width() * dpr,
										    spixmap.height() * dpr));
				}
#endif
				x += (spixmap.width() * dpr) + gap;
			}
			if (wordsCB->isChecked()) {
				tip += "<li>" + qt_("Whole words only");
				QPixmap spixmap = getPixmap("images/", "search-whole-words", "svgz,png");
#if QT_VERSION < 0x050000
				painter.drawPixmap(x, 0, spixmap);
#else
				FileName fname = imageLibFileSearch(imagedir, "search-whole-words", "svgz,png");
				QString fpath = toqstr(fname.absFileName());
				if (!fpath.isEmpty()) {
					QSvgRenderer svgRenderer(fpath);
					if (svgRenderer.isValid())
						svgRenderer.render(&painter, QRectF(x, 0, spixmap.width() * dpr,
										    spixmap.height() * dpr));
				}
#endif
				x += (spixmap.width() * dpr) + gap;
			}
			if (selectionCB->isChecked()) {
				tip += "<li>" + qt_("Search only in selection");
				QPixmap spixmap = getPixmap("images/", "search-selection", "svgz,png");
#if QT_VERSION < 0x050000
				painter.drawPixmap(x, 0, spixmap);
#else
				FileName fname = imageLibFileSearch(imagedir, "search-selection", "svgz,png");
				QString fpath = toqstr(fname.absFileName());
				if (!fpath.isEmpty()) {
					QSvgRenderer svgRenderer(fpath);
					if (svgRenderer.isValid())
						svgRenderer.render(&painter, QRectF(x, 0, spixmap.width() * dpr,
										    spixmap.height() * dpr));
				}
#endif
				x += (spixmap.width() * dpr) + gap;
			}
			if (instantSearchCB->isChecked()) {
				tip += "<li>" + qt_("Search as you type");
				QPixmap spixmap = getPixmap("images/", "search-instant", "svgz,png");
#if QT_VERSION < 0x050000
				painter.drawPixmap(x, 0, spixmap);
#else
				FileName fname = imageLibFileSearch(imagedir, "search-instant", "svgz,png");
				QString fpath = toqstr(fname.absFileName());
				if (!fpath.isEmpty()) {
					QSvgRenderer svgRenderer(fpath);
					if (svgRenderer.isValid())
						svgRenderer.render(&painter, QRectF(x, 0, spixmap.width() * dpr,
										    spixmap.height() * dpr));
				}
#endif
				x += (spixmap.width() * dpr) + gap;
			}
			if (wrapCB->isChecked()) {
				tip += "<li>" + qt_("Wrap search");
				QPixmap spixmap = getPixmap("images/", "search-wrap", "svgz,png");
#if QT_VERSION < 0x050000
				painter.drawPixmap(x, 0, spixmap);
#else
				FileName fname = imageLibFileSearch(imagedir, "search-wrap", "svgz,png");
				QString fpath = toqstr(fname.absFileName());
				if (!fpath.isEmpty()) {
					QSvgRenderer svgRenderer(fpath);
					if (svgRenderer.isValid())
						svgRenderer.render(&painter, QRectF(x, 0, spixmap.width() * dpr,
										    spixmap.height() * dpr));
				}
#endif
				x += (spixmap.width() * dpr) + gap;
			}
			tip += "</ul>";
#if QT_VERSION >= 0x050000
			pm.setDevicePixelRatio(dpr);
#endif
			painter.end();
		} else {
			tip = qt_("Click here to change search options");
#if QT_VERSION >= 0x050000
			// With Qt5, we render SVG directly for HiDPI scalability
			FileName fname = imageLibFileSearch(imagedir, "search-options", "svgz,png");
			QString fpath = toqstr(fname.absFileName());
			if (!fpath.isEmpty()) {
				QSvgRenderer svgRenderer(fpath);
				if (svgRenderer.isValid()) {
					pm = QPixmap(bpixmap.size() * dpr);
					pm.fill(Qt::transparent);
					QPainter painter(&pm);
					svgRenderer.render(&painter);
					pm.setDevicePixelRatio(dpr);
				}
			}
#endif
		}
		if (dark_mode) {
			QImage img = pm.toImage();
			img.invertPixels();
			pm.convertFromImage(img);
		}
		findLE_->setButtonPixmap(FancyLineEdit::Right, pm);
	}
	findLE_->setButtonToolTip(FancyLineEdit::Right, tip);
}


void GuiSearchWidget::caseSenseActTriggered()
{
	caseCB->setChecked(act_casesense_->isChecked());
	handleIndicators();
}


void GuiSearchWidget::wholeWordsActTriggered()
{
	wordsCB->setChecked(act_wholewords_->isChecked());
	handleIndicators();
}


void GuiSearchWidget::searchSelActTriggered()
{
	selectionCB->setChecked(act_selection_->isChecked());
	handleIndicators();
}


void GuiSearchWidget::immediateActTriggered()
{
	instantSearchCB->setChecked(act_immediate_->isChecked());
	handleIndicators();
}


void GuiSearchWidget::wrapActTriggered()
{
	wrapCB->setChecked(act_wrap_->isChecked());
	handleIndicators();
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
		findCO->lineEdit()->selectAll();
		findCO->lineEdit()->insert(toqstr(search));
		findCO->lineEdit()->selectAll();
	}
}


void GuiSearchWidget::findChanged()
{
	bool const emptytext = findCO->currentText().isEmpty();
	findPB->setEnabled(!emptytext);
	findPrevPB->setEnabled(!emptytext);
	bool const replace = !emptytext && bv_ && !bv_->buffer().isReadonly();
	replacePB->setEnabled(replace);
	replacePrevPB->setEnabled(replace);
	replaceallPB->setEnabled(replace);
	if (instantSearchCB->isChecked())
		doFind(false, true);
}


void GuiSearchWidget::findClicked()
{
	doFind();
}


void GuiSearchWidget::findPrevClicked()
{
	doFind(true);
}


void GuiSearchWidget::replaceClicked()
{
	doReplace();
}


void GuiSearchWidget::replacePrevClicked()
{
	doReplace(true);
}


void GuiSearchWidget::replaceallClicked()
{
	replace(qstring_to_ucs4(findCO->currentText()),
		qstring_to_ucs4(replaceCO->currentText()),
		caseCB->isChecked(), wordsCB->isChecked(),
		true, true, true, selectionCB->isChecked());
	uniqueInsert(findCO, findCO->currentText());
	uniqueInsert(replaceCO, replaceCO->currentText());
}


void GuiSearchWidget::doFind(bool const backwards, bool const instant)
{
	docstring const needle = qstring_to_ucs4(findCO->currentText());
	find(needle, caseCB->isChecked(), wordsCB->isChecked(), !backwards,
	     instant, wrapCB->isChecked(), selectionCB->isChecked());
	uniqueInsert(findCO, findCO->currentText());
	if (!instant)
		findCO->lineEdit()->selectAll();
}


void GuiSearchWidget::find(docstring const & search, bool casesensitive,
			   bool matchword, bool forward, bool instant,
			   bool wrap, bool onlysel)
{
	docstring const sdata =
		find2string(search, casesensitive, matchword,
			    forward, wrap, instant, onlysel);

	dispatch(FuncRequest(LFUN_WORD_FIND, sdata));
}


void GuiSearchWidget::doReplace(bool const backwards)
{
	docstring const needle = qstring_to_ucs4(findCO->currentText());
	docstring const repl = qstring_to_ucs4(replaceCO->currentText());
	replace(needle, repl, caseCB->isChecked(), wordsCB->isChecked(),
		!backwards, false, wrapCB->isChecked(), selectionCB->isChecked());
	uniqueInsert(findCO, findCO->currentText());
	uniqueInsert(replaceCO, replaceCO->currentText());
}


void GuiSearchWidget::replace(docstring const & search, docstring const & replace,
			    bool casesensitive, bool matchword,
			    bool forward, bool all, bool wrap, bool onlysel)
{
	docstring const sdata =
		replace2string(replace, search, casesensitive,
			       matchword, all, forward, true, wrap, onlysel);

	dispatch(FuncRequest(LFUN_WORD_REPLACE, sdata));
}

void GuiSearchWidget::saveSession(QSettings & settings, QString const & session_key) const
{
	settings.setValue(session_key + "/casesensitive", caseCB->isChecked());
	settings.setValue(session_key + "/words", wordsCB->isChecked());
	settings.setValue(session_key + "/instant", instantSearchCB->isChecked());
	settings.setValue(session_key + "/wrap", wrapCB->isChecked());
	settings.setValue(session_key + "/selection", selectionCB->isChecked());
	settings.setValue(session_key + "/minimized", minimized_);
}


void GuiSearchWidget::restoreSession(QString const & session_key)
{
	QSettings settings;
	caseCB->setChecked(settings.value(session_key + "/casesensitive", false).toBool());
	act_casesense_->setChecked(settings.value(session_key + "/casesensitive", false).toBool());
	wordsCB->setChecked(settings.value(session_key + "/words", false).toBool());
	act_wholewords_->setChecked(settings.value(session_key + "/words", false).toBool());
	instantSearchCB->setChecked(settings.value(session_key + "/instant", false).toBool());
	act_immediate_->setChecked(settings.value(session_key + "/instant", false).toBool());
	wrapCB->setChecked(settings.value(session_key + "/wrap", false).toBool());
	act_wrap_->setChecked(settings.value(session_key + "/wrap", false).toBool());
	selectionCB->setChecked(settings.value(session_key + "/selection", false).toBool());
	act_selection_->setChecked(settings.value(session_key + "/selection", false).toBool());
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
	setFocusProxy(widget_);

	connect(widget_, SIGNAL(needTitleBarUpdate()), this, SLOT(updateTitle()));
	connect(widget_, SIGNAL(needSizeUpdate()), this, SLOT(updateSize()));
}

void GuiSearch::onBufferViewChanged()
{
	widget_->setEnabled(static_cast<bool>(bufferview()));
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
