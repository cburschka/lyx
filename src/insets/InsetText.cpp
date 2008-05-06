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
#include "BufferParams.h"
#include "BufferView.h"
#include "CoordCache.h"
#include "CutAndPaste.h"
#include "Cursor.h"
#include "debug.h"
#include "DispatchResult.h"
#include "ErrorList.h"
#include "FuncRequest.h"
#include "gettext.h"
#include "Intl.h"
#include "Color.h"
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
#include "rowpainter.h"
#include "Row.h"
#include "sgml.h"
#include "TexRow.h"
#include "Undo.h"

#include "frontends/alert.h"
#include "frontends/Painter.h"

#include "support/lyxalgo.h" // count

#include <boost/bind.hpp>
#include <boost/current_function.hpp>
#include <boost/signal.hpp>

#include <sstream>


namespace lyx {

using graphics::PreviewLoader;

using support::isStrUnsignedInt;

using boost::bind;
using boost::ref;

using std::endl;
using std::for_each;
using std::max;
using std::string;
using std::auto_ptr;
using std::ostream;
using std::vector;


InsetText::InsetText(BufferParams const & bp)
	: drawFrame_(false), frame_color_(Color::insetframe), fixed_width_(false)
{
	paragraphs().push_back(Paragraph());
	paragraphs().back().layout(bp.getTextClass().defaultLayout());
	// Dispose of the infamous L-shaped cursor.
	text_.current_font.setLanguage(bp.language);
	text_.real_current_font.setLanguage(bp.language);
	init();
}


InsetText::InsetText(InsetText const & in)
	: Inset(in), fixed_width_(fixed_width_), text_()
{
	text_.autoBreakRows_ = in.text_.autoBreakRows_;
	drawFrame_ = in.drawFrame_;
	frame_color_ = in.frame_color_;
	text_.paragraphs() = in.text_.paragraphs();
	// Hand current buffer language down to "cloned" textinsets
	// e.g. tabular cells
	text_.current_font = in.text_.current_font;
	text_.real_current_font = in.text_.real_current_font;
	init();
}


InsetText::InsetText()
{}


void InsetText::init()
{
	for_each(paragraphs().begin(), paragraphs().end(),
		 bind(&Paragraph::setInsetOwner, _1, this));
}


void InsetText::clear()
{
	ParagraphList & pars = paragraphs();

	// This is a gross hack...
	Layout_ptr old_layout = pars.begin()->layout();

	pars.clear();
	pars.push_back(Paragraph());
	pars.begin()->setInsetOwner(this);
	pars.begin()->layout(old_layout);
}


auto_ptr<Inset> InsetText::doClone() const
{
	return auto_ptr<Inset>(new InsetText(*this));
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
	bool res = text_.read(buf, lex, errorList);
	init();

	if (!res) {
		lex.printError("Missing \\end_inset at this point. "
					   "Read: `$$Token'");
	}

	// sanity check
	// ensure we have at least one paragraph.
	if (paragraphs().empty())
		paragraphs().push_back(oldpar);
}


bool InsetText::metrics(MetricsInfo & mi, Dimension & dim) const
{
	TextMetrics & tm = mi.base.bv->textMetrics(&text_);

	//lyxerr << "InsetText::metrics: width: " << mi.base.textwidth << endl;
	mi.base.textwidth -= 2 * TEXT_TO_INSET_OFFSET;
	font_ = mi.base.font;
	// Hand font through to contained lyxtext:
	text_.font_ = mi.base.font;
	// Expand the inset if
	fixed_width_ = text_.paragraphs().size() > 1;
	if (hasFixedWidth())
		tm.metrics(mi, dim, mi.base.textwidth);
	else
		tm.metrics(mi, dim);
	fixed_width_ |= tm.parMetrics(0).rows().size() > 1;
	dim.asc += TEXT_TO_INSET_OFFSET;
	dim.des += TEXT_TO_INSET_OFFSET;
	dim.wid += 2 * TEXT_TO_INSET_OFFSET;
	mi.base.textwidth += 2 * TEXT_TO_INSET_OFFSET;
	bool const changed = dim_ != dim;
	dim_ = dim;
	return changed;
}


void InsetText::draw(PainterInfo & pi, int x, int y) const
{
	// update our idea of where we are
	setPosCache(pi, x, y);

	TextMetrics & tm = pi.base.bv->textMetrics(&text_);

	text_.background_color_ = backgroundColor();
	text_.draw(pi, x + TEXT_TO_INSET_OFFSET, y);

	if (drawFrame_) {
		int w = hasFixedWidth() ? tm.maxWidth() : tm.width();
		w += 2 * TEXT_TO_INSET_OFFSET;
		int const a = tm.ascent() + TEXT_TO_INSET_OFFSET;
		int const h = a + tm.descent() + TEXT_TO_INSET_OFFSET;
		pi.pain.rectangle(x, y - a, w, h, frameColor());
	}
}


void InsetText::drawSelection(PainterInfo & pi, int x, int y) const
{
	TextMetrics & tm = pi.base.bv->textMetrics(&text_);

	int w = hasFixedWidth() ? tm.maxWidth() : tm.width();
	w += 2 * TEXT_TO_INSET_OFFSET;

	int const a = tm.ascent() + TEXT_TO_INSET_OFFSET;
	int const h = a + tm.descent() + TEXT_TO_INSET_OFFSET;
	pi.pain.fillRectangle(x, y - a, w, h, backgroundColor());
	text_.drawSelection(pi, x + TEXT_TO_INSET_OFFSET, y);
}


docstring const InsetText::editMessage() const
{
	return _("Opened Text Inset");
}


void InsetText::edit(Cursor & cur, bool left)
{
	//lyxerr << "InsetText: edit left/right" << endl;
	int const pit = left ? 0 : paragraphs().size() - 1;
	int const pos = left ? 0 : paragraphs().back().size();
	text_.setCursor(cur.top(), pit, pos);
	cur.clearSelection();
	finishUndo();
}


Inset * InsetText::editXY(Cursor & cur, int x, int y)
{
	return text_.editXY(cur, x, y);
}


void InsetText::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	LYXERR(Debug::ACTION) << BOOST_CURRENT_FUNCTION
			     << " [ cmd.action = "
			     << cmd.action << ']' << endl;
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
	x = text_.cursorX(bv, sl, boundary) + TEXT_TO_INSET_OFFSET;
	y = text_.cursorY(bv, sl, boundary);
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


Color_color InsetText::frameColor() const
{
	return Color::color(frame_color_);
}


void InsetText::setFrameColor(Color_color col)
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
		       std::distance(pl.begin(), ins) - 1);

	for_each(pit, plist.end(),
		 bind(&ParagraphList::push_back, ref(pl), _1));
}


void InsetText::addPreview(PreviewLoader & loader) const
{
	ParagraphList::const_iterator pit = paragraphs().begin();
	ParagraphList::const_iterator pend = paragraphs().end();

	for (; pit != pend; ++pit) {
		InsetList::const_iterator it  = pit->insetlist.begin();
		InsetList::const_iterator end = pit->insetlist.end();
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


} // namespace lyx
