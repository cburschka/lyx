/**
 * \file InsetText.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetText.h"

#include "insets/InsetOptArg.h"

#include "buffer_funcs.h"
#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "CompletionList.h"
#include "CoordCache.h"
#include "Cursor.h"
#include "CutAndPaste.h"
#include "DispatchResult.h"
#include "ErrorList.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "InsetList.h"
#include "Intl.h"
#include "Lexer.h"
#include "lyxfind.h"
#include "LyXRC.h"
#include "MetricsInfo.h"
#include "output_docbook.h"
#include "output_latex.h"
#include "OutputParams.h"
#include "output_plaintext.h"
#include "paragraph_funcs.h"
#include "Paragraph.h"
#include "ParagraphParameters.h"
#include "ParIterator.h"
#include "Row.h"
#include "sgml.h"
#include "TexRow.h"
#include "TextClass.h"
#include "Text.h"
#include "TextMetrics.h"
#include "TocBackend.h"

#include "frontends/alert.h"
#include "frontends/Painter.h"

#include "support/debug.h"
#include "support/gettext.h"
#include "support/lstrings.h"

#include <boost/bind.hpp>
#include "support/lassert.h"

using namespace std;
using namespace lyx::support;

using boost::bind;
using boost::ref;

namespace lyx {

using graphics::PreviewLoader;


/////////////////////////////////////////////////////////////////////

InsetText::InsetText(Buffer const & buf)
	: drawFrame_(false), frame_color_(Color_insetframe)
{
	setBuffer(const_cast<Buffer &>(buf));
	initParagraphs();
}


InsetText::InsetText(InsetText const & in)
	: Inset(in), text_()
{
	text_.autoBreakRows_ = in.text_.autoBreakRows_;
	drawFrame_ = in.drawFrame_;
	frame_color_ = in.frame_color_;
	text_.paragraphs() = in.text_.paragraphs();
	setParagraphOwner();
}


void InsetText::setBuffer(Buffer & buf)
{
	ParagraphList::iterator end = paragraphs().end();
	for (ParagraphList::iterator it = paragraphs().begin(); it != end; ++it)
		it->setBuffer(buf);
	Inset::setBuffer(buf);
}


void InsetText::initParagraphs()
{
	LASSERT(paragraphs().empty(), /**/);
	paragraphs().push_back(Paragraph());
	Paragraph & ourpar = paragraphs().back();
	ourpar.setInsetOwner(this);
	ourpar.setDefaultLayout(buffer_->params().documentClass());
}


void InsetText::setParagraphOwner()
{
	for_each(paragraphs().begin(), paragraphs().end(),
		 bind(&Paragraph::setInsetOwner, _1, this));
}


void InsetText::clear()
{
	ParagraphList & pars = paragraphs();
	LASSERT(!pars.empty(), /**/);

	// This is a gross hack...
	Layout const & old_layout = pars.begin()->layout();

	pars.clear();
	pars.push_back(Paragraph());
	pars.begin()->setInsetOwner(this);
	pars.begin()->setLayout(old_layout);
}


Dimension const InsetText::dimension(BufferView const & bv) const
{
	TextMetrics const & tm = bv.textMetrics(&text_);
	Dimension dim = tm.dimension();
	dim.wid += 2 * TEXT_TO_INSET_OFFSET;
	dim.des += TEXT_TO_INSET_OFFSET;
	dim.asc += TEXT_TO_INSET_OFFSET;
	return dim;
}


void InsetText::write(ostream & os) const
{
	os << "Text\n";
	text_.write(buffer(), os);
}


void InsetText::read(Lexer & lex)
{
	clear();

	// delete the initial paragraph
	Paragraph oldpar = *paragraphs().begin();
	paragraphs().clear();
	ErrorList errorList;
	lex.setContext("InsetText::read");
	bool res = text_.read(buffer(), lex, errorList, this);

	if (!res)
		lex.printError("Missing \\end_inset at this point. ");

	// sanity check
	// ensure we have at least one paragraph.
	if (paragraphs().empty())
		paragraphs().push_back(oldpar);
}


void InsetText::metrics(MetricsInfo & mi, Dimension & dim) const
{
	TextMetrics & tm = mi.base.bv->textMetrics(&text_);

	//lyxerr << "InsetText::metrics: width: " << mi.base.textwidth << endl;

	// Hand font through to contained lyxtext:
	tm.font_.fontInfo() = mi.base.font;
	mi.base.textwidth -= 2 * TEXT_TO_INSET_OFFSET;
	if (hasFixedWidth())
		tm.metrics(mi, dim, mi.base.textwidth);
	else
		tm.metrics(mi, dim);
	mi.base.textwidth += 2 * TEXT_TO_INSET_OFFSET;
	dim.asc += TEXT_TO_INSET_OFFSET;
	dim.des += TEXT_TO_INSET_OFFSET;
	dim.wid += 2 * TEXT_TO_INSET_OFFSET;
}


void InsetText::draw(PainterInfo & pi, int x, int y) const
{
	TextMetrics & tm = pi.base.bv->textMetrics(&text_);

	if (drawFrame_ || pi.full_repaint) {
		int const w = tm.width() + TEXT_TO_INSET_OFFSET;
		int const yframe = y - TEXT_TO_INSET_OFFSET - tm.ascent();
		int const h = tm.height() + 2 * TEXT_TO_INSET_OFFSET;
		int const xframe = x + TEXT_TO_INSET_OFFSET / 2;
		if (pi.full_repaint)
			pi.pain.fillRectangle(xframe, yframe, w, h,
				pi.backgroundColor(this));

		if (drawFrame_)
			pi.pain.rectangle(xframe, yframe, w, h, frameColor());
	}
	ColorCode const old_color = pi.background_color;
	pi.background_color = pi.backgroundColor(this, false);

	tm.draw(pi, x + TEXT_TO_INSET_OFFSET, y);

	pi.background_color = old_color;
}


docstring InsetText::editMessage() const
{
	return _("Opened Text Inset");
}


void InsetText::edit(Cursor & cur, bool front, EntryDirection entry_from)
{
	pit_type const pit = front ? 0 : paragraphs().size() - 1;
	pos_type pos = front ? 0 : paragraphs().back().size();

	// if visual information is not to be ignored, move to extreme right/left
	if (entry_from != ENTRY_DIRECTION_IGNORE) {
		Cursor temp_cur = cur;
		temp_cur.pit() = pit;
		temp_cur.pos() = pos;
		temp_cur.posVisToRowExtremity(entry_from == ENTRY_DIRECTION_LEFT);
		pos = temp_cur.pos();
	}

	text_.setCursor(cur.top(), pit, pos);
	cur.clearSelection();
	cur.finishUndo();
}


Inset * InsetText::editXY(Cursor & cur, int x, int y)
{
	return cur.bv().textMetrics(&text_).editXY(cur, x, y);
}


void InsetText::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	LYXERR(Debug::ACTION, "InsetText::doDispatch()"
		<< " [ cmd.action = " << cmd.action << ']');
	text_.dispatch(cur, cmd);
}


bool InsetText::getStatus(Cursor & cur, FuncRequest const & cmd,
	FuncStatus & status) const
{
	switch (cmd.action) {
	case LFUN_LAYOUT:
		status.setEnabled(!forcePlainLayout());
		return true;

	case LFUN_LAYOUT_PARAGRAPH:
	case LFUN_PARAGRAPH_PARAMS:
	case LFUN_PARAGRAPH_PARAMS_APPLY:
	case LFUN_PARAGRAPH_SPACING:
	case LFUN_PARAGRAPH_UPDATE:
		status.setEnabled(allowParagraphCustomization());
		return true;
	default:
		return text_.getStatus(cur, cmd, status);
	}
}


void InsetText::setChange(Change const & change)
{
	ParagraphList::iterator pit = paragraphs().begin();
	ParagraphList::iterator end = paragraphs().end();
	for (; pit != end; ++pit) {
		pit->setChange(change);
	}
}


void InsetText::acceptChanges(BufferParams const & bparams)
{
	text_.acceptChanges(bparams);
}


void InsetText::rejectChanges(BufferParams const & bparams)
{
	text_.rejectChanges(bparams);
}


int InsetText::latex(odocstream & os, OutputParams const & runparams) const
{
	TexRow texrow;
	latexParagraphs(buffer(), text_, os, texrow, runparams);
	return texrow.rows();
}


int InsetText::plaintext(odocstream & os, OutputParams const & runparams) const
{
	ParagraphList::const_iterator beg = paragraphs().begin();
	ParagraphList::const_iterator end = paragraphs().end();
	ParagraphList::const_iterator it = beg;
	bool ref_printed = false;
	int len = 0;
	for (; it != end; ++it) {
		if (it != beg) {
			os << '\n';
			if (runparams.linelen > 0)
				os << '\n';
		}
		odocstringstream oss;
		writePlaintextParagraph(buffer(), *it, oss, runparams, ref_printed);
		docstring const str = oss.str();
		os << str;
		// FIXME: len is not computed fully correctly; in principle,
		// we have to count the characters after the last '\n'
		len = str.size();
	}

	return len;
}


int InsetText::docbook(odocstream & os, OutputParams const & runparams) const
{
	docbookParagraphs(paragraphs(), buffer(), os, runparams);
	return 0;
}


void InsetText::validate(LaTeXFeatures & features) const
{
	for_each(paragraphs().begin(), paragraphs().end(),
		 bind(&Paragraph::validate, _1, ref(features)));
}


void InsetText::cursorPos(BufferView const & bv,
		CursorSlice const & sl, bool boundary, int & x, int & y) const
{
	x = bv.textMetrics(&text_).cursorX(sl, boundary) + TEXT_TO_INSET_OFFSET;
	y = bv.textMetrics(&text_).cursorY(sl, boundary);
}


bool InsetText::showInsetDialog(BufferView *) const
{
	return false;
}


void InsetText::setText(docstring const & data, Font const & font, bool trackChanges)
{
	clear();
	Paragraph & first = paragraphs().front();
	for (unsigned int i = 0; i < data.length(); ++i)
		first.insertChar(i, data[i], font, trackChanges);
}


void InsetText::setAutoBreakRows(bool flag)
{
	if (flag == text_.autoBreakRows_)
		return;

	text_.autoBreakRows_ = flag;
	if (flag)
		return;

	// remove previously existing newlines
	ParagraphList::iterator it = paragraphs().begin();
	ParagraphList::iterator end = paragraphs().end();
	for (; it != end; ++it)
		for (int i = 0; i < it->size(); ++i)
			if (it->isNewline(i))
				// do not track the change, because the user
				// is not allowed to revert/reject it
				it->eraseChar(i, false);
}


void InsetText::setDrawFrame(bool flag)
{
	drawFrame_ = flag;
}


ColorCode InsetText::frameColor() const
{
	return frame_color_;
}


void InsetText::setFrameColor(ColorCode col)
{
	frame_color_ = col;
}


void InsetText::appendParagraphs(ParagraphList & plist)
{
	// There is little we can do here to keep track of changes.
	// As of 2006/10/20, appendParagraphs is used exclusively by
	// LyXTabular::setMultiColumn. In this context, the paragraph break
	// is lost irreversibly and the appended text doesn't really change

	ParagraphList & pl = paragraphs();

	ParagraphList::iterator pit = plist.begin();
	ParagraphList::iterator ins = pl.insert(pl.end(), *pit);
	++pit;
	mergeParagraph(buffer().params(), pl,
		       distance(pl.begin(), ins) - 1);

	for_each(pit, plist.end(),
		 bind(&ParagraphList::push_back, ref(pl), _1));
}


void InsetText::addPreview(PreviewLoader & loader) const
{
	ParagraphList::const_iterator pit = paragraphs().begin();
	ParagraphList::const_iterator pend = paragraphs().end();

	for (; pit != pend; ++pit) {
		InsetList::const_iterator it  = pit->insetList().begin();
		InsetList::const_iterator end = pit->insetList().end();
		for (; it != end; ++it)
			it->inset->addPreview(loader);
	}
}


ParagraphList const & InsetText::paragraphs() const
{
	return text_.paragraphs();
}


ParagraphList & InsetText::paragraphs()
{
	return text_.paragraphs();
}


void InsetText::updateLabels(ParIterator const & it)
{
	ParIterator it2 = it;
	it2.forwardPos();
	LASSERT(&it2.inset() == this && it2.pit() == 0, /**/);
	if (producesOutput())
		lyx::updateLabels(buffer(), it2);
	else {
		DocumentClass const & tclass = buffer().params().documentClass();
		Counters const savecnt = tclass.counters();
		lyx::updateLabels(buffer(), it2);
		tclass.counters() = savecnt;
	}
}


void InsetText::addToToc(DocIterator const & cdit)
{
	DocIterator dit = cdit;
	dit.push_back(CursorSlice(*this));
	Toc & toc = buffer().tocBackend().toc("tableofcontents");

	BufferParams const & bufparams = buffer_->params();
	const int min_toclevel = bufparams.documentClass().min_toclevel();

	// For each paragraph, traverse its insets and let them add
	// their toc items
	ParagraphList & pars = paragraphs();
	pit_type pend = paragraphs().size();
	for (pit_type pit = 0; pit != pend; ++pit) {
		Paragraph const & par = pars[pit];
		dit.pit() = pit;
		// the string that goes to the toc (could be the optarg)
		docstring tocstring;
		InsetList::const_iterator it  = par.insetList().begin();
		InsetList::const_iterator end = par.insetList().end();
		for (; it != end; ++it) {
			Inset & inset = *it->inset;
			dit.pos() = it->pos;
			//lyxerr << (void*)&inset << " code: " << inset.lyxCode() << std::endl;
			inset.addToToc(dit);
			switch (inset.lyxCode()) {
			case OPTARG_CODE: {
				if (!tocstring.empty())
					break;
				dit.pos() = 0;
				Paragraph const & insetpar =
					*static_cast<InsetOptArg&>(inset).paragraphs().begin();
				if (!par.labelString().empty())
					tocstring = par.labelString() + ' ';
				tocstring += insetpar.asString(AS_STR_INSETS);
				break;
			}
			default:
				break;
			}
		}
		/// now the toc entry for the paragraph
		int const toclevel = par.layout().toclevel;
		if (toclevel != Layout::NOT_IN_TOC && toclevel >= min_toclevel) {
			dit.pos() = 0;
			// insert this into the table of contents
			if (tocstring.empty())
				tocstring = par.asString(AS_STR_LABEL | AS_STR_INSETS);
			toc.push_back(TocItem(dit, toclevel - min_toclevel, tocstring));
		}
		
		// And now the list of changes.
		par.addChangesToToc(dit, buffer());
	}
}


bool InsetText::notifyCursorLeaves(Cursor const & old, Cursor & cur)
{
	if (cur.buffer().isClean())
		return Inset::notifyCursorLeaves(old, cur);
	
	// find text inset in old cursor
	Cursor insetCur = old;
	int scriptSlice	= insetCur.find(this);
	LASSERT(scriptSlice != -1, /**/);
	insetCur.cutOff(scriptSlice);
	LASSERT(&insetCur.inset() == this, /**/);
	
	// update the old paragraph's words
	insetCur.paragraph().updateWords(insetCur.top());
	
	return Inset::notifyCursorLeaves(old, cur);
}


bool InsetText::completionSupported(Cursor const & cur) const
{
	Cursor const & bvCur = cur.bv().cursor();
	if (&bvCur.inset() != this)
		return false;
	return text_.completionSupported(cur);
}


bool InsetText::inlineCompletionSupported(Cursor const & cur) const
{
	return completionSupported(cur);
}


bool InsetText::automaticInlineCompletion() const
{
	return lyxrc.completion_inline_text;
}


bool InsetText::automaticPopupCompletion() const
{
	return lyxrc.completion_popup_text;
}


bool InsetText::showCompletionCursor() const
{
	return lyxrc.completion_cursor_text;
}


CompletionList const * InsetText::createCompletionList(Cursor const & cur) const
{
	return completionSupported(cur) ? text_.createCompletionList(cur) : 0;
}


docstring InsetText::completionPrefix(Cursor const & cur) const
{
	if (!completionSupported(cur))
		return docstring();
	return text_.completionPrefix(cur);
}


bool InsetText::insertCompletion(Cursor & cur, docstring const & s,
	bool finished)
{
	if (!completionSupported(cur))
		return false;

	return text_.insertCompletion(cur, s, finished);
}


void InsetText::completionPosAndDim(Cursor const & cur, int & x, int & y, 
	Dimension & dim) const
{
	TextMetrics const & tm = cur.bv().textMetrics(&text_);
	tm.completionPosAndDim(cur, x, y, dim);
}


docstring InsetText::contextMenu(BufferView const &, int, int) const
{
	return from_ascii("context-edit");
}


} // namespace lyx
