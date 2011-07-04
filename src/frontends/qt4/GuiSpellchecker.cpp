/**
 * \file GuiSpellchecker.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Edwin Leuven
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiSpellchecker.h"
#include "GuiApplication.h"

#include "qt_helpers.h"

#include "ui_SpellcheckerUi.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "buffer_funcs.h"
#include "Cursor.h"
#include "Text.h"
#include "CutAndPaste.h"
#include "FuncRequest.h"
#include "Language.h"
#include "LyX.h"
#include "LyXRC.h"
#include "lyxfind.h"
#include "Paragraph.h"
#include "WordLangTuple.h"

#include "support/debug.h"
#include "support/docstring.h"
#include "support/docstring_list.h"
#include "support/ExceptionMessage.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/textutils.h"

#include <QKeyEvent>
#include <QListWidgetItem>
#include <QMessageBox>

#include "SpellChecker.h"

#include "frontends/alert.h"

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {


struct SpellcheckerWidget::Private
{
	Private(SpellcheckerWidget * parent, DockView * dv)
		: p(parent), dv_(dv), incheck_(false), wrap_around_(false) {}
	/// update from controller
	void updateSuggestions(docstring_list & words);
	/// move to next position after current word
	void forward();
	/// check text until next misspelled/unknown word
	void check();
	///
	bool continueFromBeginning();
	///
	void setLanguage(Language const * lang);
	/// test and set guard flag
	bool inCheck() {
		if (incheck_)
			return true;
		incheck_ = true;
		return false;
	}
	void canCheck() { incheck_ = false; }
	/// check for wrap around of current position
	bool isWrapAround(DocIterator cursor) const;
	///
	Ui::SpellcheckerUi ui;
	///
	SpellcheckerWidget * p;
	///
	GuiView * gv_;
	///
	DockView * dv_;
	/// current word being checked and lang code
	WordLangTuple word_;
	///
	DocIterator start_;
	///
	bool incheck_;
	///
	bool wrap_around_;
};


SpellcheckerWidget::SpellcheckerWidget(GuiView * gv, DockView * dv, QWidget * parent)
	: QTabWidget(parent), d(new Private(this, dv))
{
	d->ui.setupUi(this);
	d->gv_ = gv;

	connect(d->ui.suggestionsLW, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
		this, SLOT(on_replacePB_clicked()));

	// language
	QAbstractItemModel * language_model = guiApp->languageModel();
	// FIXME: it would be nice if sorting was enabled/disabled via a checkbox.
	language_model->sort(0);
	d->ui.languageCO->setModel(language_model);
	d->ui.languageCO->setModelColumn(1);

	d->ui.wordED->setReadOnly(true);

	d->ui.suggestionsLW->installEventFilter(this);
}


SpellcheckerWidget::~SpellcheckerWidget()
{
	delete d;
}


bool SpellcheckerWidget::eventFilter(QObject *obj, QEvent *event)
{
	if (obj == d->ui.suggestionsLW && event->type() == QEvent::KeyPress) {
		QKeyEvent *e = static_cast<QKeyEvent *> (event);
		if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
			if (d->ui.suggestionsLW->currentItem()) {
				on_suggestionsLW_itemClicked(d->ui.suggestionsLW->currentItem());
				on_replacePB_clicked();
			}
			return true;
		} else if (e->key() == Qt::Key_Right) {
			if (d->ui.suggestionsLW->currentItem())
				on_suggestionsLW_itemClicked(d->ui.suggestionsLW->currentItem());
			return true;
		}
	}
	// standard event processing
	return QWidget::eventFilter(obj, event);
}


void SpellcheckerWidget::on_suggestionsLW_itemClicked(QListWidgetItem * item)
{
	if (d->ui.replaceCO->count() != 0)
		d->ui.replaceCO->setItemText(0, item->text());
	else
		d->ui.replaceCO->addItem(item->text());

	d->ui.replaceCO->setCurrentIndex(0);
}


void SpellcheckerWidget::on_replaceCO_highlighted(const QString & str)
{
	QListWidget * lw = d->ui.suggestionsLW;
	if (lw->currentItem() && lw->currentItem()->text() == str)
		return;

	for (int i = 0; i != lw->count(); ++i) {
		if (lw->item(i)->text() == str) {
			lw->setCurrentRow(i);
			break;
		}
	}
}


void SpellcheckerWidget::updateView()
{
	BufferView * bv = d->gv_->documentBufferView();
	setEnabled(bv != 0);
	if (bv && hasFocus() && d->start_.empty()) {
		d->start_ = bv->cursor();
		d->check();
	}
}


bool SpellcheckerWidget::Private::continueFromBeginning()
{
	QMessageBox::StandardButton const answer = QMessageBox::question(p,
		qt_("Spell Checker"),
		qt_("We reached the end of the document, would you like to "
			"continue from the beginning?"),
		QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
	if (answer == QMessageBox::No) {
		dv_->hide();
		return false;
	}
	dispatch(FuncRequest(LFUN_BUFFER_BEGIN));
	wrap_around_ = true;
	return true;
}

bool SpellcheckerWidget::Private::isWrapAround(DocIterator cursor) const
{
	return wrap_around_ && start_.buffer() == cursor.buffer() && start_ < cursor;
}


void SpellcheckerWidget::Private::forward()
{
	BufferView * bv = gv_->documentBufferView();
	DocIterator from = bv->cursor();

	dispatch(FuncRequest(LFUN_ESCAPE));
	dispatch(FuncRequest(LFUN_CHAR_FORWARD));
	if (bv->cursor().depth() <= 1 && bv->cursor().atLastPos()) {
		continueFromBeginning();
		return;
	}
	if (from == bv->cursor()) {
		//FIXME we must be at the end of a cell
		dispatch(FuncRequest(LFUN_CHAR_FORWARD));
 	}
	if (isWrapAround(bv->cursor())) {
		dv_->hide();
	}
}


void SpellcheckerWidget::on_languageCO_activated(int index)
{
	string const lang =
		fromqstr(d->ui.languageCO->itemData(index).toString());
	if (!d->word_.lang() || d->word_.lang()->lang() == lang)
		// nothing changed
		return;
	dispatch(FuncRequest(LFUN_LANGUAGE, lang));
	d->check();
}


bool SpellcheckerWidget::initialiseParams(std::string const &)
{
	BufferView * bv = d->gv_->documentBufferView();
	if (bv == 0)
		return false;
	std::set<Language const *> languages = 
	bv->buffer().masterBuffer()->getLanguages();
	if (!languages.empty())
		d->setLanguage(*languages.begin());
	d->start_ = DocIterator();
	d->wrap_around_ = false;
	d->incheck_ = false;
	return true;
}


void SpellcheckerWidget::on_ignoreAllPB_clicked()
{
	/// ignore all occurrences of word
	if (d->inCheck())
		return;
	LYXERR(Debug::GUI, "Spellchecker: ignore all button");
	if (d->word_.lang() && !d->word_.word().empty())
		theSpellChecker()->accept(d->word_);
	d->forward();
	d->check();
	d->canCheck();
}


void SpellcheckerWidget::on_addPB_clicked()
{
	/// insert word in personal dictionary
	if (d->inCheck())
		return;
	LYXERR(Debug::GUI, "Spellchecker: add word button");
	theSpellChecker()->insert(d->word_);
	d->forward();
	d->check();
	d->canCheck();
}


void SpellcheckerWidget::on_ignorePB_clicked()
{
	/// ignore this occurrence of word
	if (d->inCheck())
		return;
	LYXERR(Debug::GUI, "Spellchecker: ignore button");
	d->forward();
	d->check();
	d->canCheck();
}


void SpellcheckerWidget::on_findNextPB_clicked()
{
	if (d->inCheck())
		return;
	docstring const textfield = qstring_to_ucs4(d->ui.wordED->text());
	docstring const datastring = find2string(textfield,
				true, true, true);
	LYXERR(Debug::GUI, "Spellchecker: find next (" << textfield << ")");
	dispatch(FuncRequest(LFUN_WORD_FIND, datastring));
	d->canCheck();
}


void SpellcheckerWidget::on_replacePB_clicked()
{
	if (d->inCheck())
		return;
	docstring const textfield = qstring_to_ucs4(d->ui.wordED->text());
	docstring const replacement = qstring_to_ucs4(d->ui.replaceCO->currentText());
	docstring const datastring = replace2string(replacement, textfield,
		true, true, false, false);

	LYXERR(Debug::GUI, "Replace (" << replacement << ")");
	dispatch(FuncRequest(LFUN_WORD_REPLACE, datastring));
	d->forward();
	d->check();
	d->canCheck();
}


void SpellcheckerWidget::on_replaceAllPB_clicked()
{
	if (d->inCheck())
		return;
	docstring const textfield = qstring_to_ucs4(d->ui.wordED->text());
	docstring const replacement = qstring_to_ucs4(d->ui.replaceCO->currentText());
	docstring const datastring = replace2string(replacement, textfield,
		true, true, true, true);

	LYXERR(Debug::GUI, "Replace all (" << replacement << ")");
	dispatch(FuncRequest(LFUN_WORD_REPLACE, datastring));
	d->forward();
	d->check(); // continue spellchecking
	d->canCheck();
}


void SpellcheckerWidget::Private::updateSuggestions(docstring_list & words)
{
	QString const suggestion = toqstr(word_.word());
	ui.wordED->setText(suggestion);
	QListWidget * lw = ui.suggestionsLW;
	lw->clear();

	if (words.empty()) {
		p->on_suggestionsLW_itemClicked(new QListWidgetItem(suggestion));
		return;
	}
	for (size_t i = 0; i != words.size(); ++i)
		lw->addItem(toqstr(words[i]));

	p->on_suggestionsLW_itemClicked(lw->item(0));
	lw->setCurrentRow(0);
}


void SpellcheckerWidget::Private::setLanguage(Language const * lang)
{
	int const pos = ui.languageCO->findData(toqstr(lang->lang()));
	if (pos != -1)
		ui.languageCO->setCurrentIndex(pos);
}


void SpellcheckerWidget::Private::check()
{
	BufferView * bv = gv_->documentBufferView();
	if (!bv || bv->buffer().text().empty())
		return;

	DocIterator from = bv->cursor();
	DocIterator to;
	WordLangTuple word_lang;
	docstring_list suggestions;

	LYXERR(Debug::GUI, "Spellchecker: start check at " << from);
	int progress;
	try {
		progress = bv->buffer().spellCheck(from, to, word_lang, suggestions);
	} catch (ExceptionMessage const & message) {
		if (message.type_ == WarningException) {
			Alert::warning(message.title_, message.details_);
			return;
		}
		throw message;
	}

	// end of document
	if (from == doc_iterator_end(&bv->buffer())) {
		if (wrap_around_ || start_ == doc_iterator_begin(&bv->buffer())) {
			dv_->hide();
			return;
		}
		if (continueFromBeginning())
			check();
		return;
	}

	if (isWrapAround(from)) {
		dv_->hide();
		return;
	}

	word_ = word_lang;

	// set suggestions
	updateSuggestions(suggestions);
	// set language
	setLanguage(word_lang.lang());

	// FIXME LFUN
	// If we used a LFUN, dispatch would do all of this for us
	int const size = to.pos() - from.pos();
	bv->putSelectionAt(from, size, false);
	bv->processUpdateFlags(Update::Force | Update::FitCursor);	
}


GuiSpellchecker::GuiSpellchecker(GuiView & parent,
		Qt::DockWidgetArea area, Qt::WindowFlags flags)
	: DockView(parent, "spellchecker", qt_("Spellchecker"),
		   area, flags)
{
	widget_ = new SpellcheckerWidget(&parent, this);
	setWidget(widget_);
	setFocusProxy(widget_);
}


GuiSpellchecker::~GuiSpellchecker()
{
	setFocusProxy(0);
	delete widget_;
}


void GuiSpellchecker::updateView()
{
	widget_->updateView();
}


Dialog * createGuiSpellchecker(GuiView & lv) 
{ 
	GuiSpellchecker * gui = new GuiSpellchecker(lv, Qt::RightDockWidgetArea);
#ifdef Q_WS_MACX
	gui->setFloating(true);
#endif
	return gui;
}


} // namespace frontend
} // namespace lyx

#include "moc_GuiSpellchecker.cpp"
