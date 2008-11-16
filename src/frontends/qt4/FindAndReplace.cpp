/**
 * \file FindAndReplace.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Tommaso Cucinotta
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "FindAndReplace.h"


#include "GuiWorkArea.h"
#include "GuiView.h"
#include "qt_helpers.h"

#include "Application.h"
#include "BufferList.h"
#include "buffer_funcs.h"
#include "BufferParams.h"
#include "Cursor.h"
#include "FuncRequest.h"
#include "lyxfind.h"
#include "OutputParams.h"
#include "output_latex.h"
#include "TexRow.h"

#include "support/FileName.h"
#include "support/convert.h"
#include "support/debug.h"
#include "support/gettext.h"
#include "support/lassert.h"

#include <QLineEdit>
#include <QCloseEvent>

#include <iostream>

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {


FindAndReplace::FindAndReplace(GuiView & parent)
	: DockView(parent, "Find LyX", "Find LyX Dialog", Qt::RightDockWidgetArea),
	parent_view_(parent),
	delayedFocusTimer_(this)
{
	searchBuffer_ = theBufferList().newBuffer(
		support::FileName::tempName().absFilename() + "_searchadv.internal");
	LASSERT(searchBuffer_ != 0, /* */);
	searchBufferView_ = new BufferView(*searchBuffer_);
	searchBuffer_->setUnnamed(true);
	searchBuffer_->setFullyLoaded(true);

	setupUi(this);
	find_work_area_->setGuiView(parent_view_);
	find_work_area_->setBuffer(*searchBuffer_);
	find_work_area_->setUpdatesEnabled(false);
	find_work_area_->setDialogMode(true);
}


FindAndReplace::~FindAndReplace()
{
	// No need to destroy buffer and bufferview here, because it is done
	// in theBuffeerList() destruction loop at application exit
	LYXERR(Debug::DEBUG, "FindAndReplace::~FindAndReplace()");
}


bool FindAndReplace::eventFilter(QObject *obj, QEvent *event)
{
	LYXERR(Debug::DEBUG, "FindAndReplace::eventFilter()" << std::endl);
	if (obj == find_work_area_ && event->type() == QEvent::KeyPress) {
		QKeyEvent *e = static_cast<QKeyEvent *> (event);
		if (e->key() == Qt::Key_Escape && e->modifiers() == Qt::NoModifier) {
			on_closePB_clicked();
			return true;
		} else if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
			if (e->modifiers() == Qt::ShiftModifier) {
				on_findPrevPB_clicked();
				return true;
			} else if (e->modifiers() == Qt::NoModifier) {
				on_findNextPB_clicked();
				return true;
			}
		}
	}

	// standard event processing
	return QObject::eventFilter(obj, event);
}


void FindAndReplace::closeEvent(QCloseEvent * close_event)
{
	LYXERR(Debug::DEBUG, "FindAndReplace::closeEvent()");
	find_work_area_->removeEventFilter(this);
	disableSearchWorkArea();

	DockView::closeEvent(close_event);
}


void FindAndReplace::selectAll()
{
	dispatch(FuncRequest(LFUN_BUFFER_BEGIN));
	dispatch(FuncRequest(LFUN_BUFFER_END_SELECT));
	find_work_area_->redraw();
}


void FindAndReplace::findAdv(bool casesensitive,
		bool matchword, bool backwards,
		bool expandmacros, bool ignoreformat)
{
	docstring searchString;
	if (! ignoreformat) {
		OutputParams runparams(&searchBuffer_->params().encoding());
		odocstringstream os;
		runparams.nice = true;
		runparams.flavor = OutputParams::LATEX;
		runparams.linelen = 80; //lyxrc.plaintext_linelen;
		// No side effect of file copying and image conversion
		runparams.dryrun = true;
		searchBuffer_->texrow().reset();
//		latexParagraphs(searchBuffer_, searchBuffer_.paragraphs(), os, searchBuffer_.texrow(), runparams);
		for (ParagraphList::const_iterator pit = searchBuffer_->paragraphs().begin(); pit != searchBuffer_->paragraphs().end(); ++pit) {
			TeXOnePar(*searchBuffer_, searchBuffer_->text(), pit, os, searchBuffer_->texrow(), runparams);
			lyxerr << "searchString up to here: " << to_utf8(os.str()) << std::endl;
		}
		searchString = os.str();
	} else {
		for (ParIterator it = searchBuffer_->par_iterator_begin(); it != searchBuffer_->par_iterator_end(); ++it) {
			lyxerr << "Adding to search string: '" << to_utf8(it->asString(false)) << "'" << std::endl;
			searchString += it->asString(AS_STR_INSETS);
		}
	}
//	lyxerr << "Searching for '" << to_utf8(searchString) << "'" << std::endl;
	if (to_utf8(searchString).empty()) {
		searchBufferView_->message(_("Nothing to search"));
		return;
	}
	bool regexp = (to_utf8(searchString).find("\\regexp") != std::string::npos);
	FindAdvOptions opt(searchString, casesensitive, matchword, ! backwards, expandmacros, ignoreformat, regexp);
	std::cerr << "Dispatching LFUN_WORD_FINDADV" << std::endl;
	std::ostringstream oss;
	oss << opt;
	std::cerr << "Dispatching LFUN_WORD_FINDADV" << std::endl;
	dispatch(FuncRequest(LFUN_WORD_FINDADV, from_utf8(oss.str())));

	//	findAdv(&theApp()->currentView()->currentWorkArea()->bufferView(),
	// 			searchString, len, casesensitive, matchword, ! backwards, expandmacros);
}


void FindAndReplace::onDelayedFocus()
{
	LYXERR(Debug::DEBUG, "Delayed Focus");
	parent_view_.setCurrentWorkArea(find_work_area_);
	find_work_area_->setFocus();
}


void FindAndReplace::showEvent(QShowEvent *ev)
{
	LYXERR(Debug::DEBUG, "FindAndReplace::showEvent");
	parent_view_.setCurrentWorkArea(find_work_area_);
	selectAll();
	find_work_area_->redraw();
	find_work_area_->setFocus();
	find_work_area_->installEventFilter(this);
	connect(&delayedFocusTimer_, SIGNAL(timeout()), this, SLOT(onDelayedFocus()));
	delayedFocusTimer_.setSingleShot(true);
	delayedFocusTimer_.start(100);

	this->QWidget::showEvent(ev);
}


void FindAndReplace::disableSearchWorkArea()
{
	LYXERR(Debug::DEBUG, "FindAndReplace::disableSearchWorkArea()");
	// Ok, closing the window before 100ms may be impossible, however...
	delayedFocusTimer_.stop();
	if (parent_view_.currentWorkArea() == find_work_area_) {
		LASSERT(parent_view_.currentMainWorkArea(), /* */);
		parent_view_.setCurrentWorkArea(parent_view_.currentMainWorkArea());
	}
	find_work_area_->stopBlinkingCursor();
}


void FindAndReplace::hideEvent(QHideEvent *ev)
{
	LYXERR(Debug::DEBUG, "FindAndReplace::hideEvent");
	find_work_area_->removeEventFilter(this);
	disableSearchWorkArea();
	this->QWidget::hideEvent(ev);
}


void FindAndReplace::find(bool backwards)
{
	parent_view_.setCurrentWorkArea(parent_view_.currentMainWorkArea());
	findAdv(caseCB->isChecked(),
			wordsCB->isChecked(),
			backwards,
			expandMacrosCB->isChecked(),
			ignoreFormatCB->isChecked());
	parent_view_.currentMainWorkArea()->redraw();
	parent_view_.setCurrentWorkArea(find_work_area_);
	find_work_area_->setFocus();
}


void FindAndReplace::on_regexpInsertCombo_currentIndexChanged(int index)
{
	static char const * regexps[] = {
		".*", ".+", "[a-z]+", "[0-9]+"
	};
	//lyxerr << "Index: " << index << std::endl;
	if (index >= 1 && index < 1 + int(sizeof(regexps)/sizeof(regexps[0]))) {
		find_work_area_->setFocus();
		Cursor & cur = find_work_area_->bufferView().cursor();
		if (! cur.inRegexped())
			dispatch(FuncRequest(LFUN_REGEXP_MODE));
		dispatch(FuncRequest(LFUN_SELF_INSERT, regexps[index - 1]));
		regexpInsertCombo->setCurrentIndex(0);
	}
}


void FindAndReplace::on_closePB_clicked() {
	disableSearchWorkArea();
	LYXERR(Debug::DEBUG, "Dispatching dialog-hide findreplaceadv" << std::endl);
	parent_view_.dispatch(FuncRequest(LFUN_DIALOG_TOGGLE, "findreplaceadv"));
}


void FindAndReplace::on_findNextPB_clicked() {
	find(false);
}


void FindAndReplace::on_findPrevPB_clicked() {
	find(true);
}


void FindAndReplace::on_replacePB_clicked()
{
}


void FindAndReplace::on_replaceallPB_clicked()
{
}

Dialog * createGuiSearchAdv(GuiView & lv)
{
	return new FindAndReplace(lv);
}


} // namespace frontend
} // namespace lyx


#include "moc_FindAndReplace.cpp"
