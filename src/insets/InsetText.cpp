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
#include "InsetNewline.h"

#include "Buffer.h"
#include "buffer_funcs.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "CoordCache.h"
#include "CutAndPaste.h"
#include "Cursor.h"
#include "DispatchResult.h"
#include "ErrorList.h"
#include "FuncRequest.h"
#include "InsetList.h"
#include "Intl.h"
#include "lyxfind.h"
#include "Lexer.h"
#include "LyXRC.h"
#include "Text.h"
#include "MetricsInfo.h"
#include "OutputParams.h"
#include "output_docbook.h"
#include "output_latex.h"
#include "output_plaintext.h"
#include "Paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "ParIterator.h"
#include "Row.h"
#include "sgml.h"
#include "TextClass.h"
#include "TextMetrics.h"
#include "TexRow.h"

#include "frontends/alert.h"
#include "frontends/Painter.h"

#include "support/debug.h"
#include "support/gettext.h"
#include "support/lstrings.h"

#include <boost/bind.hpp>
#include <boost/assert.hpp>

using namespace std;
using namespace lyx::support;

using boost::bind;
using boost::ref;

namespace lyx {

using graphics::PreviewLoader;


class TextCompletionList : public Inset::CompletionList {
public:
	///
	TextCompletionList(Cursor const & cur)
	: buf_(cur.buffer()), it_(buf_.registeredWords().begin()), pos_(0) {}
	///
	virtual ~TextCompletionList() {}

	///
	virtual size_t size() const {
		return buf_.registeredWords().size();
	}
	///
	virtual docstring data(size_t idx) const {
		std::set<docstring>::const_iterator it
		= buf_.registeredWords().begin();
		for (size_t i = 0; i < idx; ++i)
			it++;
		return *it;
	}

private:
	Buffer const & buf_;
	std::set<docstring>::const_iterator const it_;
	size_t pos_;
};


/////////////////////////////////////////////////////////////////////

InsetText::InsetText(BufferParams const & bp)
	: drawFrame_(false), frame_color_(Color_insetframe)
{
	paragraphs().push_back(Paragraph());
	Paragraph & ourpar = paragraphs().back();
	if (useEmptyLayout())
		ourpar.layout(bp.getTextClass().emptyLayout());
	else
		ourpar.layout(bp.getTextClass().defaultLayout());
	ourpar.setInsetOwner(this);
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


InsetText::InsetText()
{}


void InsetText::setParagraphOwner()
{
	for_each(paragraphs().begin(), paragraphs().end(),
		 bind(&Paragraph::setInsetOwner, _1, this));
}


void InsetText::clear()
{
	ParagraphList & pars = paragraphs();

	// This is a gross hack...
	LayoutPtr old_layout = pars.begin()->layout();

	pars.clear();
	pars.push_back(Paragraph());
	pars.begin()->setInsetOwner(this);
	pars.begin()->layout(old_layout);
}


Inset * InsetText::clone() const
{
	return new InsetText(*this);
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


void InsetText::write(Buffer const & buf, ostream & os) const
{
	os << "Text\n";
	text_.write(buf, os);
}


void InsetText::read(Buffer const & buf, Lexer & lex)
{
	clear();

	// delete the initial paragraph
	Paragraph oldpar = *paragraphs().begin();
	paragraphs().clear();
	ErrorList errorList;
	bool res = text_.read(buf, lex, errorList, this);

	if (!res) {
		lex.printError("Missing \\end_inset at this point. "
					   "Read: `$$Token'");
	}

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
			pi.pain.fillRectangle(xframe, yframe, w, h, backgroundColor());
		if (drawFrame_)
			pi.pain.rectangle(xframe, yframe, w, h, frameColor());
	}
	tm.draw(pi, x + TEXT_TO_INSET_OFFSET, y);
}


docstring const InsetText::editMessage() const
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
	return text_.getStatus(cur, cmd, status);
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


int InsetText::latex(Buffer const & buf, odocstream & os,
		     OutputParams const & runparams) const
{
	TexRow texrow;
	latexParagraphs(buf, text_, os, texrow, runparams);
	return texrow.rows();
}


int InsetText::plaintext(Buffer const & buf, odocstream & os,
			 OutputParams const & runparams) const
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
		writePlaintextParagraph(buf, *it, oss, runparams, ref_printed);
		docstring const str = oss.str();
		os << str;
		// FIXME: len is not computed fully correctly; in principle,
		// we have to count the characters after the last '\n'
		len = str.size();
	}

	return len;
}


int InsetText::docbook(Buffer const & buf, odocstream & os,
		       OutputParams const & runparams) const
{
	docbookParagraphs(paragraphs(), buf, os, runparams);
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


void InsetText::appendParagraphs(Buffer * buffer, ParagraphList & plist)
{
	// There is little we can do here to keep track of changes.
	// As of 2006/10/20, appendParagraphs is used exclusively by
	// LyXTabular::setMultiColumn. In this context, the paragraph break
	// is lost irreversibly and the appended text doesn't really change

	ParagraphList & pl = paragraphs();

	ParagraphList::iterator pit = plist.begin();
	ParagraphList::iterator ins = pl.insert(pl.end(), *pit);
	++pit;
	mergeParagraph(buffer->params(), pl,
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


//FIXME: instead of this hack, which only works by chance,
// cells should have their own insetcell type, which returns CELL_CODE!
bool InsetText::neverIndent(Buffer const & buffer) const
{
	// this is only true for tabular cells
	return !text_.isMainText(buffer) && lyxCode() == TEXT_CODE;
}


ParagraphList const & InsetText::paragraphs() const
{
	return text_.paragraphs();
}


ParagraphList & InsetText::paragraphs()
{
	return text_.paragraphs();
}


void InsetText::updateLabels(Buffer const & buf, ParIterator const & it)
{
	ParIterator it2 = it;
	it2.forwardPos();
	BOOST_ASSERT(&it2.inset() == this && it2.pit() == 0);
	lyx::updateLabels(buf, it2);
}


bool InsetText::completionSupported(Cursor const & cur) const
{
	Cursor const & bvCur = cur.bv().cursor();
	if (&bvCur.inset() != this)
		return false;
	Paragraph const & par = cur.paragraph();
	return cur.pos() > 0
		&& !par.isLetter(cur.pos())
		&& par.isLetter(cur.pos() - 1);
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


Inset::CompletionListPtr InsetText::completionList(Cursor const & cur) const
{
	if (!completionSupported(cur))
		return CompletionListPtr();

	return CompletionListPtr(new TextCompletionList(cur));
}


docstring InsetText::previousWord(Buffer const & buffer, CursorSlice const & sl) const
{
	CursorSlice from = sl;
	CursorSlice to = sl;
	text_.getWord(from, to, PREVIOUS_WORD);
	if (sl == from || to == from)
		return docstring();
	
	Paragraph const & par = sl.paragraph();
	return par.asString(buffer, from.pos(), to.pos(), false);
}


docstring InsetText::completionPrefix(Cursor const & cur) const
{
	if (!completionSupported(cur))
		return docstring();
	
	return previousWord(cur.buffer(), cur.top());
}


bool InsetText::insertCompletion(Cursor & cur, docstring const & s,
				     bool finished)
{
	if (!completionSupported(cur))
		return false;

	BOOST_ASSERT(cur.bv().cursor() == cur);
	cur.insert(s);
	cur.bv().cursor() = cur;
	return true;
}


void InsetText::completionPosAndDim(Cursor const & cur, int & x, int & y, 
					Dimension & dim) const
{
	// get word in front of cursor
	docstring word = previousWord(cur.buffer(), cur.top());
	DocIterator wordStart = cur;
	wordStart.pos() -= word.length();
	
	// get position on screen of the word start
	Point lxy = cur.bv().getPos(wordStart, false);
	x = lxy.x_;
	y = lxy.y_;

	// Calculate dimensions of the word
	TextMetrics const & tm = cur.bv().textMetrics(&text_);
	dim = tm.rowHeight(cur.pit(), wordStart.pos(), cur.pos(), false);
	Point rxy = cur.bv().getPos(cur, cur.boundary());
	dim.wid = abs(rxy.x_ - x);
	x = (rxy.x_ < x) ? x - dim.wid : x; // for RTL
}


} // namespace lyx
