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

#include "Lexer.h"
#include "GuiApplication.h"
#include "GuiView.h"
#include "GuiWorkArea.h"
#include "qt_helpers.h"
#include "Language.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "BufferList.h"
#include "BufferView.h"
#include "Text.h"
#include "TextClass.h"
#include "Cursor.h"
#include "FuncRequest.h"
#include "LyX.h"
#include "lyxfind.h"

#include "frontends/alert.h"

#include "support/debug.h"
#include "support/filetools.h"
#include "support/FileName.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h"

#include <QCloseEvent>
#include <QLineEdit>
#include <QMenu>

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {


FindAndReplaceWidget::FindAndReplaceWidget(GuiView & view)
	: QTabWidget(&view), view_(view)
{
	setupUi(this);
	find_work_area_->setGuiView(view_);
	find_work_area_->init();
	find_work_area_->setFrameStyle(QFrame::StyledPanel);

	setFocusProxy(find_work_area_);
	replace_work_area_->setGuiView(view_);
	replace_work_area_->init();
	replace_work_area_->setFrameStyle(QFrame::StyledPanel);

	// We don't want two cursors blinking.
	find_work_area_->stopBlinkingCursor();
	replace_work_area_->stopBlinkingCursor();
}


void FindAndReplaceWidget::dockLocationChanged(Qt::DockWidgetArea area)
{
       if (area == Qt::RightDockWidgetArea || area == Qt::LeftDockWidgetArea) {
               dynamicLayoutBasic_->setDirection(QBoxLayout::TopToBottom);
               dynamicLayoutAdvanced_->setDirection(QBoxLayout::TopToBottom);
       } else {
               dynamicLayoutBasic_->setDirection(QBoxLayout::LeftToRight);
               dynamicLayoutAdvanced_->setDirection(QBoxLayout::LeftToRight);
       }
}


bool FindAndReplaceWidget::eventFilter(QObject * obj, QEvent * event)
{
	if (event->type() != QEvent::KeyPress
		  || (obj != find_work_area_ && obj != replace_work_area_))
		return QWidget::eventFilter(obj, event);

	QKeyEvent * e = static_cast<QKeyEvent *> (event);
	switch (e->key()) {
	case Qt::Key_Escape:
		if (e->modifiers() == Qt::NoModifier) {
			hideDialog();
			return true;
		}
		break;

	case Qt::Key_Enter:
	case Qt::Key_Return: {
		// with shift we (temporarily) change search/replace direction
		bool const searchback = searchbackCB->isChecked();
		if (e->modifiers() == Qt::ShiftModifier && !searchback)
			searchbackCB->setChecked(!searchback);

		if (obj == find_work_area_)
			on_findNextPB_clicked();
		else
			on_replacePB_clicked();
		// back to how it was
		searchbackCB->setChecked(searchback);
		return true;
	}

	case Qt::Key_Tab:
		if (e->modifiers() == Qt::NoModifier) {
			if (obj == find_work_area_){
				LYXERR(Debug::FIND, "Focusing replace WA");
				replace_work_area_->setFocus();
				LYXERR(Debug::FIND, "Selecting entire replace buffer");
				dispatch(FuncRequest(LFUN_BUFFER_BEGIN));
				dispatch(FuncRequest(LFUN_BUFFER_END_SELECT));
				return true;
			}
		}
		break;

	case Qt::Key_Backtab:
		if (obj == replace_work_area_) {
			LYXERR(Debug::FIND, "Focusing find WA");
			find_work_area_->setFocus();
			LYXERR(Debug::FIND, "Selecting entire find buffer");
			dispatch(FuncRequest(LFUN_BUFFER_BEGIN));
			dispatch(FuncRequest(LFUN_BUFFER_END_SELECT));
			return true;
		}
		break;

	default:
		break;
	}
	// standard event processing
	return QWidget::eventFilter(obj, event);
}


static vector<string> const & allManualsFiles() 
{
	static const char * files[] = {
		"Intro", "UserGuide", "Tutorial", "Additional",
		"EmbeddedObjects", "Math", "Customization", "Shortcuts",
		"LFUNs", "LaTeXConfig"
	};

	static vector<string> v;
	if (v.empty()) {
		FileName fname;
		for (size_t i = 0; i < sizeof(files) / sizeof(files[0]); ++i) {
			fname = i18nLibFileSearch("doc", files[i], "lyx");
			v.push_back(fname.absFileName());
		}
	}

	return v;
}


/** Switch buf to point to next document buffer.
 **
 ** Return true if restarted from master-document buffer.
 **/
static bool nextDocumentBuffer(Buffer * & buf) 
{
	ListOfBuffers const children = buf->allRelatives();
	LYXERR(Debug::FIND, "children.size()=" << children.size());
	ListOfBuffers::const_iterator it =
		find(children.begin(), children.end(), buf);
	LASSERT(it != children.end(), return false);
	++it;
	if (it == children.end()) {
		buf = *children.begin();
		return true;
	}
	buf = *it;
	return false;
}


/** Switch p_buf to point to previous document buffer.
 **
 ** Return true if restarted from last child buffer.
 **/
static bool prevDocumentBuffer(Buffer * & buf) 
{
	ListOfBuffers const children = buf->allRelatives();
	LYXERR(Debug::FIND, "children.size()=" << children.size());
	ListOfBuffers::const_iterator it =
		find(children.begin(), children.end(), buf);
	LASSERT(it != children.end(), return false)
	if (it == children.begin()) {
		it = children.end();
		--it;
		buf = *it;
		return true;
	}
	--it;
	buf = *it;
	return false;
}


/** Switch buf to point to next or previous buffer in search scope.
 **
 ** Return true if restarted from scratch.
 **/
static bool nextPrevBuffer(Buffer * & buf,
			     FindAndReplaceOptions const & opt)
{
	bool restarted = false;
	switch (opt.scope) {
	case FindAndReplaceOptions::S_BUFFER:
		restarted = true;
		break;
	case FindAndReplaceOptions::S_DOCUMENT:
		if (opt.forward)
			restarted = nextDocumentBuffer(buf);
		else
			restarted = prevDocumentBuffer(buf);
		break;
	case FindAndReplaceOptions::S_OPEN_BUFFERS:
		if (opt.forward) {
			buf = theBufferList().next(buf);
			restarted = (buf == *theBufferList().begin());
		} else {
			buf = theBufferList().previous(buf);
			restarted = (buf == *(theBufferList().end() - 1));
		}
		break;
	case FindAndReplaceOptions::S_ALL_MANUALS:
		vector<string> const & manuals = allManualsFiles();
		vector<string>::const_iterator it =
			find(manuals.begin(), manuals.end(), buf->absFileName());
		if (it == manuals.end())
			it = manuals.begin();
		else if (opt.forward) {
			++it;
			if (it == manuals.end()) {
				it = manuals.begin();
				restarted = true;
			}
		} else {
			if (it == manuals.begin()) {
				it = manuals.end();
				restarted = true;
			}
			--it;
		}
		FileName const & fname = FileName(*it);
		if (!theBufferList().exists(fname)) {
			guiApp->currentView()->setBusy(false);
			guiApp->currentView()->loadDocument(fname, false);
			guiApp->currentView()->setBusy(true);
		}
		buf = theBufferList().getBuffer(fname);
		break;
	}
	return restarted;
}


/** Find the finest question message to post to the user */
docstring getQuestionString(FindAndReplaceOptions const & opt)
{
	docstring scope;
	switch (opt.scope) {
	case FindAndReplaceOptions::S_BUFFER:
		scope = _("File");
		break;
	case FindAndReplaceOptions::S_DOCUMENT:
		scope = _("Master document");
		break;
	case FindAndReplaceOptions::S_OPEN_BUFFERS:
		scope = _("Open files");
		break;
	case FindAndReplaceOptions::S_ALL_MANUALS:
		scope = _("Manuals");
		break;
	}
	docstring message = opt.forward ?
		bformat(_("%1$s: the end was reached while searching forward.\n"
			  "Continue searching from the beginning?"),
			scope) : 
		bformat(_("%1$s: the beginning was reached while searching backward.\n"
			  "Continue searching from the end?"),
			scope);

	return message;
}


/// Return true if a match was found
bool FindAndReplaceWidget::findAndReplaceScope(FindAndReplaceOptions & opt, bool replace_all)
{
	BufferView * bv = view_.documentBufferView();
	if (!bv)
		return false;
	Buffer * buf = &bv->buffer();
	Buffer * buf_orig = &bv->buffer();
	DocIterator cur_orig(bv->cursor());
	int wrap_answer = -1;
	ostringstream oss;
	oss << opt;
	FuncRequest cmd(LFUN_WORD_FINDADV, from_utf8(oss.str()));

	view_.message(_("Advanced search in progress (press ESC to cancel) . . ."));
	theApp()->startLongOperation();
	view_.setBusy(true);
	if (opt.scope == FindAndReplaceOptions::S_ALL_MANUALS) {
		vector<string> const & v = allManualsFiles();
		if (std::find(v.begin(), v.end(), buf->absFileName()) == v.end()) {
			FileName const & fname = FileName(*v.begin());
			if (!theBufferList().exists(fname)) {
				guiApp->currentView()->setBusy(false);
				theApp()->stopLongOperation();
				guiApp->currentView()->loadDocument(fname, false);
				theApp()->startLongOperation();
				guiApp->currentView()->setBusy(true);
			}
			buf = theBufferList().getBuffer(fname);
			if (!buf) {
				view_.setBusy(false);
				return false;
			}

			lyx::dispatch(FuncRequest(LFUN_BUFFER_SWITCH,
						  buf->absFileName()));
			bv = view_.documentBufferView();
			bv->cursor().clear();
			bv->cursor().push_back(CursorSlice(buf->inset()));
		}
	}

	do {
		LYXERR(Debug::FIND, "Dispatching LFUN_WORD_FINDADV");
		dispatch(cmd);
		LYXERR(Debug::FIND, "dispatched");
		if (bv->cursor().result().dispatched()) {
			// New match found and selected (old selection replaced if needed)
			if (replace_all)
				continue;
			view_.setBusy(false);
			theApp()->stopLongOperation();
			return true;
		} else if (replace_all)
			bv->clearSelection();

		if (theApp()->longOperationCancelled()) {
			// Search aborted by user
			view_.message(_("Advanced search cancelled by user"));
			view_.setBusy(false);
			theApp()->stopLongOperation();
			return false;
		}

		// No match found in current buffer (however old selection might have been replaced)
		// select next buffer in scope, if any
		bool const prompt = nextPrevBuffer(buf, opt);
		if (!buf)
			break;
		if (prompt) {
			if (wrap_answer != -1)
				break;
			docstring q = getQuestionString(opt);
			view_.setBusy(false);
			theApp()->stopLongOperation();
			wrap_answer = frontend::Alert::prompt(
				_("Wrap search?"), q,
				0, 1, _("&Yes"), _("&No"));
			theApp()->startLongOperation();
			view_.setBusy(true);
			if (wrap_answer == 1)
				break;
		}
		if (buf != &view_.documentBufferView()->buffer())
			lyx::dispatch(FuncRequest(LFUN_BUFFER_SWITCH,
						  buf->absFileName()));
		bv = view_.documentBufferView();
		if (opt.forward) {
			bv->cursor().clear();
			bv->cursor().push_back(CursorSlice(buf->inset()));
		} else {
			//lyx::dispatch(FuncRequest(LFUN_BUFFER_END));
			bv->cursor().setCursor(doc_iterator_end(buf));
			bv->cursor().backwardPos();
			LYXERR(Debug::FIND, "findBackAdv5: cur: "
				<< bv->cursor());
		}
		bv->clearSelection();
	} while (wrap_answer != 1);
	if (buf_orig != &view_.documentBufferView()->buffer())
		lyx::dispatch(FuncRequest(LFUN_BUFFER_SWITCH,
					  buf_orig->absFileName()));
	bv = view_.documentBufferView();
	// This may happen after a replace occurred
	if (cur_orig.pos() > cur_orig.lastpos())
		cur_orig.pos() = cur_orig.lastpos();
	bv->cursor().setCursor(cur_orig);
	view_.setBusy(false);
	theApp()->stopLongOperation();
	return false;
}


/// Return true if a match was found
bool FindAndReplaceWidget::findAndReplace(
	bool casesensitive, bool matchword, bool backwards,
	bool expandmacros, bool ignoreformat, bool replace,
	bool keep_case, bool replace_all)
{
	Buffer & find_buf = find_work_area_->bufferView().buffer();
	docstring const & find_buf_name = find_buf.fileName().absoluteFilePath();

	if (find_buf.text().empty()) {
		view_.message(_("Nothing to search"));
		return false;
	}

	Buffer & repl_buf = replace_work_area_->bufferView().buffer();
	docstring const & repl_buf_name = replace ?
		repl_buf.fileName().absoluteFilePath() : docstring();

	FindAndReplaceOptions::SearchScope scope =
		FindAndReplaceOptions::S_BUFFER;
	if (CurrentDocument->isChecked())
		scope = FindAndReplaceOptions::S_BUFFER;
	else if (MasterDocument->isChecked())
		scope = FindAndReplaceOptions::S_DOCUMENT;
	else if (OpenDocuments->isChecked())
		scope = FindAndReplaceOptions::S_OPEN_BUFFERS;
	else if (AllManualsRB->isChecked())
		scope = FindAndReplaceOptions::S_ALL_MANUALS;
	else
		LATTEST(false);

	FindAndReplaceOptions::SearchRestriction restr =
		OnlyMaths->isChecked()
			? FindAndReplaceOptions::R_ONLY_MATHS
			: FindAndReplaceOptions::R_EVERYTHING;

	LYXERR(Debug::FIND, "FindAndReplaceOptions: "
	       << "find_buf_name=" << find_buf_name
	       << ", casesensitiv=" << casesensitive
	       << ", matchword=" << matchword
	       << ", backwards=" << backwards
	       << ", expandmacros=" << expandmacros
	       << ", ignoreformat=" << ignoreformat
	       << ", repl_buf_name" << repl_buf_name
	       << ", keep_case=" << keep_case
	       << ", scope=" << scope
	       << ", restr=" << restr);

	FindAndReplaceOptions opt(find_buf_name, casesensitive, matchword,
				  !backwards, expandmacros, ignoreformat,
				  repl_buf_name, keep_case, scope, restr);
	return findAndReplaceScope(opt, replace_all);
}


bool FindAndReplaceWidget::findAndReplace(bool backwards, bool replace, bool replace_all)
{
	if (! view_.currentMainWorkArea()) {
		view_.message(_("No open document(s) in which to search"));
		return false;
	}
	// Finalize macros that are being typed, both in main document and in search or replacement WAs
	if (view_.currentWorkArea()->bufferView().cursor().macroModeClose())
		view_.currentWorkArea()->bufferView().processUpdateFlags(Update::Force);
	if (view_.currentMainWorkArea()->bufferView().cursor().macroModeClose())
		view_.currentMainWorkArea()->bufferView().processUpdateFlags(Update::Force);

	// FIXME: create a Dialog::returnFocus()
	// or something instead of this:
	view_.setCurrentWorkArea(view_.currentMainWorkArea());
	return findAndReplace(caseCB->isChecked(),
		wordsCB->isChecked(),
		backwards,
		expandMacrosCB->isChecked(),
		ignoreFormatCB->isChecked(),
		replace,
		keepCaseCB->isChecked(),
		replace_all);
}


void FindAndReplaceWidget::hideDialog()
{
	dispatch(FuncRequest(LFUN_DIALOG_TOGGLE, "findreplaceadv"));
}


void FindAndReplaceWidget::on_findNextPB_clicked() 
{
	findAndReplace(searchbackCB->isChecked(), false);
	find_work_area_->setFocus();
}


void FindAndReplaceWidget::on_replacePB_clicked()
{
	findAndReplace(searchbackCB->isChecked(), true);
	replace_work_area_->setFocus();
}


void FindAndReplaceWidget::on_replaceallPB_clicked()
{
	findAndReplace(searchbackCB->isChecked(), true, true);
	replace_work_area_->setFocus();
}


/** Copy selected elements from bv's BufferParams to the dest_bv's one
 ** We don't want to copy'em all, e.g., not the default master **/
static void copy_params(BufferView const & bv, BufferView & dest_bv) {
	Buffer const & doc_buf = bv.buffer();
	BufferParams const & doc_bp = doc_buf.params();
	string const & lang = doc_bp.language->lang();
	string const & doc_class = doc_bp.documentClass().name();
	Buffer & dest_buf = dest_bv.buffer();
	dest_buf.params().setLanguage(lang);
	dest_buf.params().setBaseClass(doc_class);
	dest_bv.makeDocumentClass();
	dest_bv.cursor().current_font.setLanguage(doc_bp.language);
}


void FindAndReplaceWidget::showEvent(QShowEvent * /* ev */)
{
	LYXERR(Debug::DEBUG, "showEvent()" << endl);
	BufferView * bv = view_.documentBufferView();
	if (bv) {
		copy_params(*bv, find_work_area_->bufferView());
		copy_params(*bv, replace_work_area_->bufferView());
	}

	find_work_area_->installEventFilter(this);
	replace_work_area_->installEventFilter(this);

	view_.setCurrentWorkArea(find_work_area_);
	LYXERR(Debug::FIND, "Selecting entire find buffer");
	dispatch(FuncRequest(LFUN_BUFFER_BEGIN));
	dispatch(FuncRequest(LFUN_BUFFER_END_SELECT));
}


void FindAndReplaceWidget::hideEvent(QHideEvent *ev)
{
	replace_work_area_->removeEventFilter(this);
	find_work_area_->removeEventFilter(this);
	this->QWidget::hideEvent(ev);
}


bool FindAndReplaceWidget::initialiseParams(std::string const & /*params*/)
{
	return true;
}


void FindAndReplace::updateView()
{
	widget_->updateGUI();
}


FindAndReplace::FindAndReplace(GuiView & parent,
		Qt::DockWidgetArea area, Qt::WindowFlags flags)
	: DockView(parent, "findreplaceadv", qt_("Advanced Find and Replace"),
		   area, flags)
{
	widget_ = new FindAndReplaceWidget(parent);
	setWidget(widget_);
	setFocusProxy(widget_);

	connect(this, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
               widget_, SLOT(dockLocationChanged(Qt::DockWidgetArea)));
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


void FindAndReplaceWidget::updateGUI()
{
	bool replace_enabled = view_.documentBufferView()
		&& !view_.documentBufferView()->buffer().isReadonly();
	replace_work_area_->setEnabled(replace_enabled);
	replacePB->setEnabled(replace_enabled);
	replaceallPB->setEnabled(replace_enabled);
}


Dialog * createGuiSearchAdv(GuiView & lv)
{
	FindAndReplace * gui = new FindAndReplace(lv, Qt::RightDockWidgetArea);
#ifdef Q_OS_MAC
	// On Mac show and floating
	gui->setFloating(true);
#endif
	return gui;
}


} // namespace frontend
} // namespace lyx


#include "moc_FindAndReplace.cpp"
