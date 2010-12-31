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

#include "buffer_funcs.h"
#include "BufferParams.h"
#include "BufferList.h"
#include "Cursor.h"
#include "FuncRequest.h"
#include "lyxfind.h"
#include "output_latex.h"
#include "OutputParams.h"
#include "TexRow.h"

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

#include <iostream>

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {


/// Apply to buf the parameters supplied through bp
static void ApplyParams(Buffer &buf, BufferParams const & bp) {
	ostringstream ss;
	ss << "\\begin_header\n";
	bp.writeFile(ss);
	ss << "\\end_header\n";
	istringstream iss(ss.str());
	Lexer lex;
	lex.setStream(iss);
	int unknown_tokens = buf.readHeader(lex);
	LASSERT(unknown_tokens == 0, /* */);
}


FindAndReplaceWidget::FindAndReplaceWidget(GuiView & view)
	:	view_(view)
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
	replace_work_area_->stopBlinkingCursor();
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
		break;
		}
	case Qt::Key_Tab:
		if (e->modifiers() == Qt::NoModifier) {
			if (obj == find_work_area_){
				LYXERR(Debug::FIND, "Focusing replace WA");
				replace_work_area_->setFocus();
				return true;
			}
		}
		break;

	case Qt::Key_Backtab:
		if (obj == replace_work_area_) {
			LYXERR(Debug::FIND, "Focusing find WA");
			find_work_area_->setFocus();
			return true;
		}
		break;

	default:
		break;
	}
	// standard event processing
	return QWidget::eventFilter(obj, event);
}


static docstring buffer_to_latex(Buffer & buffer) 
{
	OutputParams runparams(&buffer.params().encoding());
	odocstringstream os;
	runparams.nice = true;
	runparams.flavor = OutputParams::LATEX;
	runparams.linelen = 80; //lyxrc.plaintext_linelen;
	// No side effect of file copying and image conversion
	runparams.dryrun = true;
	buffer.texrow().reset();
	pit_type const endpit = buffer.paragraphs().size();
	for (pit_type pit = 0; pit != endpit; ++pit) {
		TeXOnePar(buffer, buffer.text(),
			  pit, os, buffer.texrow(), runparams);
		LYXERR(Debug::FIND, "searchString up to here: "
			<< os.str());
	}
	return os.str();
}


static vector<string> const & allManualsFiles() 
{
	static vector<string> v;
	static const char * files[] = {
		"Intro", "UserGuide", "Tutorial", "Additional",
		"EmbeddedObjects", "Math", "Customization", "Shortcuts",
		"LFUNs", "LaTeXConfig"
	};
	if (v.empty()) {
		FileName fname;
		for (size_t i = 0; i < sizeof(files) / sizeof(files[0]); ++i) {
			fname = i18nLibFileSearch("doc", files[i], "lyx");
			v.push_back(fname.absFileName());
		}
	}
	return v;
}


/** Switch p_buf to point to next document buffer.
 **
 ** Return true if restarted from master-document buffer.
 **/
static bool nextDocumentBuffer(Buffer * & buf) 
{
	ListOfBuffers const children = buf->allRelatives();
	LYXERR(Debug::FIND, "children.size()=" << children.size());
	ListOfBuffers::const_iterator it =
		find(children.begin(), children.end(), buf);
	LASSERT(it != children.end(), /**/)
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
	LASSERT(it != children.end(), /**/)
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


void FindAndReplaceWidget::findAndReplaceScope(FindAndReplaceOptions & opt)
{
	int wrap_answer = -1;
	ostringstream oss;
	oss << opt;
	FuncRequest cmd(LFUN_WORD_FINDADV, from_utf8(oss.str()));
	BufferView * bv = view_.documentBufferView();
	Buffer * buf = &bv->buffer();

	Buffer * buf_orig = &bv->buffer();
	DocIterator cur_orig(bv->cursor());

	if (opt.scope == FindAndReplaceOptions::S_ALL_MANUALS) {
		vector<string> const & v = allManualsFiles();
		if (std::find(v.begin(), v.end(), buf->absFileName()) == v.end()) {
			FileName const & fname = FileName(*v.begin());
			if (!theBufferList().exists(fname)) {
				guiApp->currentView()->setBusy(false);
				guiApp->currentView()->loadDocument(fname, false);
				guiApp->currentView()->setBusy(true);
			}
			buf = theBufferList().getBuffer(fname);
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
			return;
		}

		// No match found in current buffer (however old selection might have been replaced)
		// select next buffer in scope, if any
		bool prompt = nextPrevBuffer(buf, opt);
		if (prompt) {
			if (wrap_answer != -1)
				break;
			docstring q = getQuestionString(opt);
			wrap_answer = frontend::Alert::prompt(
				_("Wrap search?"), q,
				0, 1, _("&Yes"), _("&No"));
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
			LYXERR(Debug::FIND, "Adding to search string: '"
				<< it->asString(false)
				<< "'");
			searchString +=
				it->stringify(pos_type(0), it->size(),
					      AS_STR_INSETS, runparams);
		}
	}
	if (to_utf8(searchString).empty()) {
		buffer.message(_("Nothing to search"));
		return;
	}
	bool const regexp =
		to_utf8(searchString).find("\\regexp") != std::string::npos;
	docstring replaceString;
	if (replace) {
		Buffer & repl_buffer =
			replace_work_area_->bufferView().buffer();
		ostringstream oss;
		repl_buffer.write(oss);
		//buffer_to_latex(replace_buffer);
		replaceString = from_utf8(oss.str());
	} else {
		replaceString = from_utf8(LYX_FR_NULL_STRING);
	}
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
	FindAndReplaceOptions opt(searchString, casesensitive, matchword,
				  !backwards, expandmacros, ignoreformat,
				  regexp, replaceString, keep_case, scope);
	view_.setBusy(true);
	findAndReplaceScope(opt);
	view_.setBusy(false);
}


void FindAndReplaceWidget::findAndReplace(bool backwards, bool replace)
{
	if (! view_.currentMainWorkArea()) {
		view_.message(_("No open document(s) in which to search"));
		return;
	}
	// Finalize macros that are being typed, both in main document and in search or replacement WAs
	if (view_.currentWorkArea()->bufferView().cursor().macroModeClose())
		view_.currentWorkArea()->bufferView().processUpdateFlags(Update::Force);
	if (view_.currentMainWorkArea()->bufferView().cursor().macroModeClose())
		view_.currentMainWorkArea()->bufferView().processUpdateFlags(Update::Force);

	// FIXME: create a Dialog::returnFocus()
	// or something instead of this:
	view_.setCurrentWorkArea(view_.currentMainWorkArea());
	findAndReplace(caseCB->isChecked(),
		wordsCB->isChecked(),
		backwards,
		expandMacrosCB->isChecked(),
		ignoreFormatCB->isChecked(),
		replace,
		keepCaseCB->isChecked());
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
	replace_work_area_->setFocus();
}


void FindAndReplaceWidget::showEvent(QShowEvent * /* ev */)
{
	Buffer & doc_buf = view_.documentBufferView()->buffer();
	BufferParams & doc_bp = doc_buf.params();
	Buffer & find_buf = find_work_area_->bufferView().buffer();
	LYXERR(Debug::FIND, "Applying document params to find buffer");
	ApplyParams(find_buf, doc_bp);
	Buffer & replace_buf = replace_work_area_->bufferView().buffer();
	LYXERR(Debug::FIND, "Applying document params to replace buffer");
	ApplyParams(replace_buf, doc_bp);

	string lang = doc_bp.language->lang();
	LYXERR(Debug::FIND, "Setting current editing language to " << lang << endl);
	FuncRequest cmd(LFUN_LANGUAGE, lang);
	find_buf.text().dispatch(find_work_area_->bufferView().cursor(), cmd);
	replace_buf.text().dispatch(replace_work_area_->bufferView().cursor(), cmd);

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


bool FindAndReplaceWidget::initialiseParams(std::string const & /*params*/)
{
	return true;
}


FindAndReplace::FindAndReplace(GuiView & parent,
		Qt::DockWidgetArea area, Qt::WindowFlags flags)
	: DockView(parent, "Find LyX", qt_("Advanced Find and Replace"),
		   area, flags)
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
	FindAndReplace * gui = new FindAndReplace(lv, Qt::RightDockWidgetArea);
#ifdef Q_WS_MACX
	// On Mac show and floating
	gui->setFloating(true);
#endif
	return gui;
}


} // namespace frontend
} // namespace lyx


#include "moc_FindAndReplace.cpp"
