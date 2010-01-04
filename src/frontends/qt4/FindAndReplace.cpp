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
#include "BufferList.h"
#include "Cursor.h"
#include "FuncRequest.h"
#include "lyxfind.h"
#include "OutputParams.h"
#include "output_latex.h"
#include "TexRow.h"
#include "alert.h"

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
#if QT_VERSION < 0x040400
	scrollArea->setWidget(scrollAreaWidgetContents);
#endif
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
		} else if (e->key() == Qt::Key_Tab && e->modifiers() == Qt::NoModifier) {
			LYXERR(Debug::FIND, "Focusing replace WA");
			replace_work_area_->setFocus();
			return true;
		}
	}
	if (obj == replace_work_area_ && event->type() == QEvent::KeyPress) {
		QKeyEvent *e = static_cast<QKeyEvent *> (event);
		if (e->key() == Qt::Key_Escape && e->modifiers() == Qt::NoModifier) {
			on_closePB_clicked();
			return true;
		} else if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
			if (e->modifiers() == Qt::ShiftModifier) {
				on_replacePrevPB_clicked();
				return true;
			} else if (e->modifiers() == Qt::NoModifier) {
				on_replaceNextPB_clicked();
				return true;
			}
		} else if (e->key() == Qt::Key_Backtab) {
			LYXERR(Debug::FIND, "Focusing find WA");
			find_work_area_->setFocus();
			return true;
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
		LYXERR(Debug::FIND, "searchString up to here: " << os.str());
	}
	return os.str();
}


/** Switch p_buf to point to next document buffer.
 **
 ** Return true if restarted from master-document buffer.
 **
 ** @note
 ** Not using p_buf->allRelatives() here, because I'm not sure
 ** whether or not the returned order is independent of p_buf.
 **/
static bool next_document_buffer(Buffer * & p_buf) {
	Buffer *p_master = p_buf;
	Buffer *p_old;
	do {
		p_old = p_master;
		p_master = const_cast<Buffer *>(p_master->masterBuffer());
		LYXERR(Debug::FIND, "p_old=" << p_old << ", p_master=" << p_master);
	} while (p_master != p_old);
	LASSERT(p_master != NULL, /**/);
	vector<Buffer *> v_children;
	/* Root master added as first buffer in the vector */
	v_children.push_back(p_master);
	p_master->getChildren(v_children, true);
	LYXERR(Debug::FIND, "v_children.size()=" << v_children.size());
	vector<Buffer *>::const_iterator it = find(v_children.begin(), v_children.end(), p_buf);
	LASSERT(it != v_children.end(), /**/)
	++it;
	if (it == v_children.end()) {
		p_buf = *v_children.begin();
		return true;
	}
	p_buf = *it;
	return false;
}


/** Switch p_buf to point to previous document buffer.
 **
 ** Return true if restarted from last child buffer.
 **
 ** @note
 ** Not using p_buf->allRelatives() here, because I'm not sure
 ** whether or not the returned order is independent of p_buf.
 **/
static bool prev_document_buffer(Buffer * & p_buf) {
	Buffer *p_master = p_buf;
	Buffer *p_old;
	do {
		p_old = p_master;
		p_master = const_cast<Buffer *>(p_master->masterBuffer());
		LYXERR(Debug::FIND, "p_old=" << p_old << ", p_master=" << p_master);
	} while (p_master != p_old);
	LASSERT(p_master != NULL, /**/);
	vector<Buffer *> v_children;
	/* Root master added as first buffer in the vector */
	v_children.push_back(p_master);
	p_master->getChildren(v_children, true);
	LYXERR(Debug::FIND, "v_children.size()=" << v_children.size());
	vector<Buffer *>::const_iterator it = find(v_children.begin(), v_children.end(), p_buf);
	LASSERT(it != v_children.end(), /**/)
	if (it == v_children.begin()) {
		it = v_children.end();
		--it;
		p_buf = *it;
		return true;
	}
	--it;
	p_buf = *it;
	return false;
}


/** Switch buf to point to next or previous buffer in search scope.
 **
 ** Return true if restarted from scratch.
 **/
static bool next_prev_buffer(Buffer * & buf, FindAndReplaceOptions const & opt) {
	bool restarted = false;
	switch (opt.scope) {
	case FindAndReplaceOptions::S_BUFFER:
		restarted = true;
		break;
	case FindAndReplaceOptions::S_DOCUMENT:
		if (opt.forward)
			restarted = next_document_buffer(buf);
		else
			restarted = prev_document_buffer(buf);
		break;
	case FindAndReplaceOptions::S_OPEN_BUFFERS:
		if (opt.forward) {
			buf = theBufferList().next(buf);
			restarted = buf == *theBufferList().begin();
		} else {
			buf = theBufferList().previous(buf);
			restarted = buf == *(theBufferList().end() - 1);
		}
		break;
	}
	return restarted;
}


/** Find the finest question message to post to the user */
docstring question_string(FindAndReplaceOptions const & opt)
{
	docstring cur_pos = opt.forward ? _("End") : _("Begin");
	docstring new_pos = opt.forward ? _("begin") : _("end");
	docstring scope;
	switch (opt.scope) {
	case FindAndReplaceOptions::S_BUFFER:
		scope = _("file");
		break;
	case FindAndReplaceOptions::S_DOCUMENT:
		scope = _("master document");
		break;
	case FindAndReplaceOptions::S_OPEN_BUFFERS:
		scope = _("open files");
		break;
	}
	docstring dir = opt.forward ? _("forward") : _("backwards");
	return cur_pos + _(" of ") + scope
		+ _(" reached while searching ") + dir + ".\n"
		+ "\n"
		+ _("Continue searching from ") + new_pos + " ?";
}


void FindAndReplaceWidget::findAndReplaceScope(FindAndReplaceOptions & opt) {
	int wrap_answer = -1;
	ostringstream oss;
	oss << opt;
	FuncRequest cmd(LFUN_WORD_FINDADV, from_utf8(oss.str()));
	BufferView * bv = view_.documentBufferView();
	Buffer * buf = &bv->buffer();

	Buffer * buf_orig = &bv->buffer();
	Cursor cur_orig(bv->cursor());

	do {
		LYXERR(Debug::FIND, "Dispatching LFUN_WORD_FINDADV");
		dispatch(cmd);
		if (bv->cursor().result().dispatched()) {
			// Match found, selected and replaced if needed
			return;
		}

		// No match found in current buffer: select next buffer in scope, if any
		bool prompt = next_prev_buffer(buf, opt);
		if (prompt) {
			if (wrap_answer != -1)
				break;
			docstring q = question_string(opt);
			wrap_answer = frontend::Alert::prompt(
				_("Wrap search?"), q,
				0, 1, _("&Yes"), _("&No"));
			if (wrap_answer == 1)
				break;
		}
		lyx::dispatch(FuncRequest(LFUN_BUFFER_SWITCH,
					  buf->absFileName()));
		bv = view_.documentBufferView();
		if (opt.forward) {
			bv->cursor().clear();
			bv->cursor().push_back(CursorSlice(buf->inset()));
		} else {
			lyx::dispatch(FuncRequest(LFUN_BUFFER_END));
			bv->cursor().setCursor(doc_iterator_end(buf));
			bv->cursor().backwardPos();
			LYXERR(Debug::FIND, "findBackAdv5: cur: " << bv->cursor());
		}
		bv->clearSelection();
	} while (wrap_answer != 1);
	lyx::dispatch(FuncRequest(LFUN_BUFFER_SWITCH,
				  buf_orig->absFileName()));
	bv = view_.documentBufferView();
	bv->cursor() = cur_orig;
}


void FindAndReplaceWidget::findAndReplace(
	bool casesensitive, bool matchword, bool backwards,
	bool expandmacros, bool ignoreformat, bool replace,
	bool keep_case)
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
			LYXERR(Debug::FIND, "Adding to search string: '" << it->asString(false) << "'");
			searchString += it->stringify(pos_type(0), it->size(), AS_STR_INSETS, runparams);
		}
	}
	if (to_utf8(searchString).empty()) {
		buffer.message(_("Nothing to search"));
		return;
	}
	bool const regexp = to_utf8(searchString).find("\\regexp") != std::string::npos;
	docstring replaceString;
	if (replace) {
		Buffer & repl_buffer = replace_work_area_->bufferView().buffer();
		ostringstream oss;
		repl_buffer.write(oss);
		replaceString = from_utf8(oss.str()); //buffer_to_latex(replace_buffer);
	} else {
		replaceString = from_utf8(LYX_FR_NULL_STRING);
	}
	FindAndReplaceOptions::SearchScope scope = FindAndReplaceOptions::S_BUFFER;
	if (CurrentDocument->isChecked())
		scope = FindAndReplaceOptions::S_BUFFER;
	else if (MasterDocument->isChecked())
		scope = FindAndReplaceOptions::S_DOCUMENT;
	else if (OpenDocuments->isChecked())
		scope = FindAndReplaceOptions::S_OPEN_BUFFERS;
	else
		LASSERT(false, /**/);
	LYXERR(Debug::FIND, "FindAndReplaceOptions: "
	       << "searchstring=" << searchString
	       << ", casesensitiv=" << casesensitive
	       << ", matchword=" << matchword
	       << ", backwards=" << backwards
	       << ", expandmacros=" << expandmacros
	       << ", ignoreformat=" << ignoreformat
	       << ", regexp=" << regexp
	       << ", replaceString" << replaceString
	       << ", keep_case=" << keep_case
	       << ", scope=" << scope);
	FindAndReplaceOptions opt(searchString, casesensitive, matchword, ! backwards,
		expandmacros, ignoreformat, regexp, replaceString, keep_case, scope);
	findAndReplaceScope(opt);
}


void FindAndReplaceWidget::findAndReplace(bool backwards, bool replace)
{
	if (! view_.currentMainWorkArea()) {
		view_.message(_("No open document(s) in which to search"));
		return;
	}
	// FIXME: create a Dialog::returnFocus() or something instead of this:
	view_.setCurrentWorkArea(view_.currentMainWorkArea());
	findAndReplace(caseCB->isChecked(),
		wordsCB->isChecked(),
		backwards,
		expandMacrosCB->isChecked(),
		ignoreFormatCB->isChecked(),
		replace,
		keepCaseCB->isChecked());
}


void FindAndReplaceWidget::on_regexpInsertCombo_currentIndexChanged(int index)
{
	static char const * regexps[] = {
		".*", ".+", "[a-z]+", "[0-9]+", ""
	};
	LYXERR(Debug::FIND, "Index: " << index);
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
	find_work_area_->setFocus();
}


void FindAndReplaceWidget::on_findPrevPB_clicked() {
	findAndReplace(true, false);
	find_work_area_->setFocus();
}


void FindAndReplaceWidget::on_replaceNextPB_clicked()
{
	findAndReplace(false, true);
	replace_work_area_->setFocus();
}


void FindAndReplaceWidget::on_replacePrevPB_clicked()
{
	findAndReplace(true, true);
	replace_work_area_->setFocus();
}


void FindAndReplaceWidget::on_replaceallPB_clicked()
{
	replace_work_area_->setFocus();
}


void FindAndReplaceWidget::showEvent(QShowEvent * /* ev */)
{
	view_.setCurrentWorkArea(find_work_area_);
	LYXERR(Debug::FIND, "Selecting entire find buffer");
	dispatch(FuncRequest(LFUN_BUFFER_BEGIN));
	dispatch(FuncRequest(LFUN_BUFFER_END_SELECT));
	find_work_area_->installEventFilter(this);
	replace_work_area_->installEventFilter(this);
}


void FindAndReplaceWidget::hideEvent(QHideEvent *ev)
{
	replace_work_area_->removeEventFilter(this);
	find_work_area_->removeEventFilter(this);
	this->QWidget::hideEvent(ev);
}


bool FindAndReplaceWidget::initialiseParams(std::string const & /* params */)
{
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
