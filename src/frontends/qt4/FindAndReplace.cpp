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

#include "GuiApplication.h"
#include "qt_helpers.h"
#include "GuiView.h"
#include "GuiWorkArea.h"

#include "buffer_funcs.h"
#include "BufferParams.h"
#include "Cursor.h"
#include "FuncRequest.h"
#include "lyxfind.h"
#include "OutputParams.h"
#include "output_latex.h"
#include "TexRow.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/FileName.h"
#include "support/gettext.h"
#include "support/lassert.h"

#include <QCloseEvent>
#include <QLineEdit>

#include <iostream>

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {


FindAndReplaceWidget::FindAndReplaceWidget(GuiView & view)
	:	view_(view)
{
	setupUi(this);
	find_work_area_->setGuiView(view_);
	find_work_area_->init();
	setFocusProxy(find_work_area_);
	replace_work_area_->setGuiView(view_);
	replace_work_area_->init();
	// We don't want two cursors blinking.
	replace_work_area_->stopBlinkingCursor();
}


bool FindAndReplaceWidget::eventFilter(QObject *obj, QEvent *event)
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
	return QWidget::eventFilter(obj, event);
}

static docstring buffer_to_latex(Buffer & buffer) {
	OutputParams runparams(&buffer.params().encoding());
	odocstringstream os;
	runparams.nice = true;
	runparams.flavor = OutputParams::LATEX;
	runparams.linelen = 80; //lyxrc.plaintext_linelen;
	// No side effect of file copying and image conversion
	runparams.dryrun = true;
	buffer.texrow().reset();
	ParagraphList::const_iterator pit = buffer.paragraphs().begin();
	ParagraphList::const_iterator const end = buffer.paragraphs().end();
	for (; pit != end; ++pit) {
		TeXOnePar(buffer, buffer.text(), pit, os, buffer.texrow(), runparams);
		LYXERR(Debug::DEBUG, "searchString up to here: " << os.str());
	}
	return os.str();
}

void FindAndReplaceWidget::findAndReplace(
	bool casesensitive, bool matchword, bool backwards,
	bool expandmacros, bool ignoreformat, bool replace)
{
	Buffer & buffer = find_work_area_->bufferView().buffer();
	docstring searchString;
	if (!ignoreformat) {
		searchString = buffer_to_latex(buffer);
	} else {
		ParIterator it = buffer.par_iterator_begin();
		ParIterator end = buffer.par_iterator_end();
		OutputParams runparams(&buffer.params().encoding());
		odocstringstream os;
		runparams.nice = true;
		runparams.flavor = OutputParams::LATEX;
		runparams.linelen = 100000; //lyxrc.plaintext_linelen;
		runparams.dryrun = true;
		for (; it != end; ++it) {
			LYXERR(Debug::DEBUG, "Adding to search string: '" << it->asString(false) << "'");
			searchString += it->stringify(pos_type(0), it->size(), AS_STR_INSETS, runparams);
		}
	}
//	lyxerr << "Searching for '" << to_utf8(searchString) << "'" << std::endl;
	if (to_utf8(searchString).empty()) {
		buffer.message(_("Nothing to search"));
		return;
	}
	bool const regexp = to_utf8(searchString).find("\\regexp") != std::string::npos;
	docstring replaceString;
	if (replace) {
		Buffer & replace_buffer = replace_work_area_->bufferView().buffer();
		replaceString = buffer_to_latex(replace_buffer);
	} else {
		replaceString = from_utf8(LYX_FR_NULL_STRING);
	}
	FindAndReplaceOptions opt(searchString, casesensitive, matchword, ! backwards,
		expandmacros, ignoreformat, regexp, replaceString);
	LYXERR(Debug::DEBUG, "Dispatching LFUN_WORD_FINDADV" << std::endl);
	std::ostringstream oss;
	oss << opt;
	LYXERR(Debug::DEBUG, "Dispatching LFUN_WORD_FINDADV" << std::endl);
	dispatch(FuncRequest(LFUN_WORD_FINDADV, from_utf8(oss.str())));

	//	findAdv(&theApp()->currentView()->currentWorkArea()->bufferView(),
	// 			searchString, len, casesensitive, matchword, ! backwards, expandmacros);
}


void FindAndReplaceWidget::findAndReplace(bool backwards, bool replace)
{
	// FIXME: create a Dialog::returnFocus() or something instead of this:
	view_.setCurrentWorkArea(view_.currentMainWorkArea());
	// FIXME: This should be an LFUN.
	findAndReplace(caseCB->isChecked(),
		wordsCB->isChecked(),
		backwards,
		expandMacrosCB->isChecked(),
		ignoreFormatCB->isChecked(),
		replace);
	view_.currentMainWorkArea()->redraw();
	find_work_area_->setFocus();
}


void FindAndReplaceWidget::on_regexpInsertCombo_currentIndexChanged(int index)
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


void FindAndReplaceWidget::on_closePB_clicked()
{
	dispatch(FuncRequest(LFUN_DIALOG_TOGGLE, "findreplaceadv"));
}


void FindAndReplaceWidget::on_findNextPB_clicked() {
	findAndReplace(false, false);
}


void FindAndReplaceWidget::on_findPrevPB_clicked() {
	findAndReplace(true, false);
}


void FindAndReplaceWidget::on_replaceNextPB_clicked()
{
	findAndReplace(false, true);
}


void FindAndReplaceWidget::on_replacePrevPB_clicked()
{
	findAndReplace(true, true);
}


void FindAndReplaceWidget::on_replaceallPB_clicked()
{
}


void FindAndReplaceWidget::showEvent(QShowEvent *ev)
{
	replace_work_area_->setEnabled(true);
	replace_work_area_->redraw();
	find_work_area_->setFocus();
	dispatch(FuncRequest(LFUN_BUFFER_BEGIN));
	dispatch(FuncRequest(LFUN_BUFFER_END_SELECT));
	find_work_area_->redraw();
	find_work_area_->installEventFilter(this);
}


void FindAndReplaceWidget::hideEvent(QHideEvent *ev)
{
	find_work_area_->removeEventFilter(this);
	this->QWidget::hideEvent(ev);
}


bool FindAndReplaceWidget::initialiseParams(std::string const & params)
{
	find_work_area_->redraw();
	replace_work_area_->setEnabled(true);
	replace_work_area_->redraw();
	find_work_area_->setFocus();
	dispatch(FuncRequest(LFUN_BUFFER_BEGIN));
	dispatch(FuncRequest(LFUN_BUFFER_END_SELECT));
	return true;
}


FindAndReplace::FindAndReplace(GuiView & parent,
		Qt::DockWidgetArea area, Qt::WindowFlags flags)
	: DockView(parent, "Find LyX", qt_("Find LyX Dialog"), area, flags)
{
	widget_ = new FindAndReplaceWidget(parent);
	setWidget(widget_);
	setFocusProxy(widget_);
}


FindAndReplace::~FindAndReplace()
{
	setFocusProxy(0);
	delete widget_;
}


bool FindAndReplace::initialiseParams(std::string const & params)
{
	return widget_->initialiseParams(params);
}


Dialog * createGuiSearchAdv(GuiView & lv)
{
	return new FindAndReplace(lv, Qt::RightDockWidgetArea);
}


} // namespace frontend
} // namespace lyx


#include "moc_FindAndReplace.cpp"
