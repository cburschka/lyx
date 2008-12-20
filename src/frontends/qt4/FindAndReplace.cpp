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

FindAndReplace::FindAndReplace(GuiView & parent)
	: DockView(parent, "Find LyX", "Find LyX Dialog", Qt::RightDockWidgetArea)
{
	setupUi(this);
	find_work_area_->setGuiView(parent);
	find_work_area_->init();
	setFocusProxy(find_work_area_);
	replace_work_area_->setGuiView(parent);
	replace_work_area_->init();
	// We don't want two cursors blinking.
	replace_work_area_->stopBlinkingCursor();
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


void FindAndReplace::findAdv(bool casesensitive,
		bool matchword, bool backwards,
		bool expandmacros, bool ignoreformat)
{
	Buffer & buffer = find_work_area_->bufferView().buffer();
	docstring searchString;
	if (!ignoreformat) {
		OutputParams runparams(&buffer.params().encoding());
		odocstringstream os;
		runparams.nice = true;
		runparams.flavor = OutputParams::LATEX;
		runparams.linelen = 80; //lyxrc.plaintext_linelen;
		// No side effect of file copying and image conversion
		runparams.dryrun = true;
		buffer.texrow().reset();
//		latexParagraphs(buffer, buffer.paragraphs(), os, buffer.texrow(), runparams);
		ParagraphList::const_iterator pit = buffer.paragraphs().begin();
		ParagraphList::const_iterator const end = buffer.paragraphs().end();
		for (; pit != end; ++pit) {
			TeXOnePar(buffer, buffer.text(), pit, os, buffer.texrow(), runparams);
			LYXERR0("searchString up to here: " << os.str());
		}
		searchString = os.str();
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
			LYXERR0("Adding to search string: '" << it->asString(false) << "'");
			searchString += it->stringify(pos_type(0), it->size(), AS_STR_INSETS, runparams);
		}
	}
//	lyxerr << "Searching for '" << to_utf8(searchString) << "'" << std::endl;
	if (to_utf8(searchString).empty()) {
		buffer.message(_("Nothing to search"));
		return;
	}
	bool const regexp = to_utf8(searchString).find("\\regexp") != std::string::npos;
	FindAdvOptions opt(searchString, casesensitive, matchword, ! backwards,
		expandmacros, ignoreformat, regexp);
	std::cerr << "Dispatching LFUN_WORD_FINDADV" << std::endl;
	std::ostringstream oss;
	oss << opt;
	std::cerr << "Dispatching LFUN_WORD_FINDADV" << std::endl;
	dispatch(FuncRequest(LFUN_WORD_FINDADV, from_utf8(oss.str())));

	//	findAdv(&theApp()->currentView()->currentWorkArea()->bufferView(),
	// 			searchString, len, casesensitive, matchword, ! backwards, expandmacros);
}

bool FindAndReplace::initialiseParams(std::string const &)
{
	find_work_area_->redraw();
	replace_work_area_->setEnabled(true);
	replace_work_area_->redraw();
	find_work_area_->setFocus();
	dispatch(FuncRequest(LFUN_BUFFER_BEGIN));
	dispatch(FuncRequest(LFUN_BUFFER_END_SELECT));
	return true;
}


void FindAndReplace::find(bool backwards)
{
	// FIXME: create a Dialog::returnFocus() or something instead of this:
	GuiView & gv = const_cast<GuiView &>(lyxview());
	gv.setCurrentWorkArea(gv.currentMainWorkArea());
	// FIXME: This should be an LFUN.
	findAdv(caseCB->isChecked(),
			wordsCB->isChecked(),
			backwards,
			expandMacrosCB->isChecked(),
			ignoreFormatCB->isChecked());
	gv.currentMainWorkArea()->redraw();
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


void FindAndReplace::on_closePB_clicked()
{
	dispatch(FuncRequest(LFUN_DIALOG_TOGGLE, "findreplaceadv"));
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
