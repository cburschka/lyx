/**
 * \file insettext.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "insettext.h"
#include "insetnewline.h"

#include "buffer.h"
#include "bufferparams.h"
#include "BufferView.h"
#include "CutAndPaste.h"
#include "cursor.h"
#include "debug.h"
#include "dispatchresult.h"
#include "errorlist.h"
#include "funcrequest.h"
#include "gettext.h"
#include "intl.h"
#include "LColor.h"
#include "lyxfind.h"
#include "lyxlex.h"
#include "lyxrc.h"
#include "lyxtext.h"
#include "metricsinfo.h"
#include "output_docbook.h"
#include "output_latex.h"
#include "output_linuxdoc.h"
#include "output_plaintext.h"
#include "paragraph.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "rowpainter.h"
#include "lyxrow.h"
#include "sgml.h"
#include "texrow.h"
#include "undo.h"

#include "frontends/Alert.h"
#include "frontends/font_metrics.h"
#include "frontends/LyXView.h"
#include "frontends/Painter.h"

#include "support/lyxalgo.h" // lyx::count

#include <boost/bind.hpp>

using bv_funcs::replaceSelection;

using lyx::pos_type;

using lyx::graphics::PreviewLoader;

using lyx::support::isStrUnsignedInt;
using lyx::support::strToUnsignedInt;

using std::endl;
using std::for_each;
using std::max;
using std::string;
using std::auto_ptr;
using std::ostream;
using std::vector;


InsetText::InsetText(BufferParams const & bp)
	: autoBreakRows_(false), drawFrame_(NEVER),
	  frame_color_(LColor::insetframe), text_(0, true)
{
	paragraphs().push_back(Paragraph());
	paragraphs().begin()->layout(bp.getLyXTextClass().defaultLayout());
	if (bp.tracking_changes)
		paragraphs().begin()->trackChanges();
	init();
}


InsetText::InsetText(InsetText const & in)
	: UpdatableInset(in), text_(in.text_.bv_owner, true)
{
	// this is ugly...
	operator=(in);
}


void InsetText::operator=(InsetText const & in)
{
	UpdatableInset::operator=(in);
	autoBreakRows_ = in.autoBreakRows_;
	drawFrame_ = in.drawFrame_;
	frame_color_ = in.frame_color_;
	text_ = LyXText(in.text_.bv_owner, true);
	text_.paragraphs() = in.text_.paragraphs();
	init();
}


void InsetText::init()
{
	ParagraphList::iterator pit = paragraphs().begin();
	ParagraphList::iterator end = paragraphs().end();
	for (; pit != end; ++pit)
		pit->setInsetOwner(this);
	old_par = -1;
	in_insetAllowed = false;
}


void InsetText::clear(bool just_mark_erased)
{
	ParagraphList & pars = paragraphs();
	if (just_mark_erased) {
		ParagraphList::iterator it = pars.begin();
		ParagraphList::iterator end = pars.end();
		for (; it != end; ++it)
			it->markErased();
		return;
	}

	// This is a gross hack...
	LyXLayout_ptr old_layout = pars.begin()->layout();

	pars.clear();
	pars.push_back(Paragraph());
	pars.begin()->setInsetOwner(this);
	pars.begin()->layout(old_layout);
}


auto_ptr<InsetBase> InsetText::clone() const
{
	return auto_ptr<InsetBase>(new InsetText(*this));
}


void InsetText::write(Buffer const & buf, ostream & os) const
{
	os << "Text\n";
	text_.write(buf, os);
}


void InsetText::read(Buffer const & buf, LyXLex & lex)
{
	clear(false);

#warning John, look here. Doesnt make much sense.
	if (buf.params().tracking_changes)
		paragraphs().begin()->trackChanges();

	// delete the initial paragraph
	Paragraph oldpar = *paragraphs().begin();
	paragraphs().clear();
	bool res = text_.read(buf, lex);
	init();

	if (!res) {
		lex.printError("Missing \\end_inset at this point. "
					   "Read: `$$Token'");
	}

	// sanity check
	// ensure we have at least one par.
	if (paragraphs().empty())
		paragraphs().push_back(oldpar);
}


void InsetText::metrics(MetricsInfo & mi, Dimension & dim) const
{
	//lyxerr << "InsetText::metrics: width: " << mi.base.textwidth << endl;
	setViewCache(mi.base.bv);
	mi.base.textwidth -= 2 * TEXT_TO_INSET_OFFSET;
	text_.metrics(mi, dim);
	dim.asc += TEXT_TO_INSET_OFFSET;
	dim.des += TEXT_TO_INSET_OFFSET;
	dim.wid += 2 * TEXT_TO_INSET_OFFSET;
	mi.base.textwidth += 2 * TEXT_TO_INSET_OFFSET;
	dim_ = dim;
	font_ = mi.base.font;
	text_.font_ = mi.base.font;
}


void InsetText::draw(PainterInfo & pi, int x, int y) const
{
	// update our idea of where we are
	setPosCache(pi, x, y);

	// repaint the background if needed
	x += TEXT_TO_INSET_OFFSET;
	if (backgroundColor() != LColor::background)
		clearInset(pi.pain, x, y);

	BufferView * bv = pi.base.bv;
	bv->hideCursor();

	if (!owner())
		x += scroll();
	y += bv->top_y() - text_.ascent();

	text_.draw(pi, x, y);

	if (drawFrame_ == ALWAYS || drawFrame_ == LOCKED)
		drawFrame(pi.pain, xo_);
}


void InsetText::drawSelection(PainterInfo & pi, int x, int y) const
{
	text_.drawSelection(pi, x, y);
}


void InsetText::drawFrame(Painter & pain, int x) const
{
	int const ttoD2 = TEXT_TO_INSET_OFFSET / 2;
	int const frame_x = x + ttoD2;
	int const frame_y = yo_ - dim_.asc + ttoD2;
	int const frame_w = dim_.wid - TEXT_TO_INSET_OFFSET;
	int const frame_h = dim_.asc + dim_.des - TEXT_TO_INSET_OFFSET;
	pain.rectangle(frame_x, frame_y, frame_w, frame_h, frameColor());
}


void InsetText::updateLocal(LCursor & cur)
{
	if (!autoBreakRows_ && paragraphs().size() > 1) {
		// collapseParagraphs
		while (paragraphs().size() > 1) {
			ParagraphList::iterator const first = paragraphs().begin();
			ParagraphList::iterator second = first;
			++second;
			size_t const first_par_size = first->size();

			if (!first->empty() &&
					!second->empty() &&
					!first->isSeparator(first_par_size - 1)) {
				first->insertChar(first_par_size, ' ');
			}

			cur.clearSelection();
			mergeParagraph(cur.bv().buffer()->params(), paragraphs(), first);
		}
	}

	if (!cur.selection())
		cur.resetAnchor();

	LyXView * lv = cur.bv().owner();
	lv->view_state_changed();
	lv->updateMenubar();
	lv->updateToolbar();
	if (old_par != cur.par()) {
		lv->setLayout(text_.getPar(cur.par())->layout()->name());
		old_par = cur.par();
	}
}


string const InsetText::editMessage() const
{
	return _("Opened Text Inset");
}


void InsetText::sanitizeEmptyText(BufferView & bv)
{
	if (paragraphs().size() == 1
	    && paragraphs().begin()->empty()
	    && bv.getParentLanguage(this) != text_.current_font.language()) {
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setLanguage(bv.getParentLanguage(this));
		text_.setFont(font, false);
	}
}


extern CursorBase theTempCursor;


void InsetText::edit(LCursor & cur, bool left)
{
	//lyxerr << "InsetText: edit left/right" << endl;
	old_par = -1;
	setViewCache(&cur.bv());
	int const par = left ? 0 : paragraphs().size() - 1;
	int const pos = left ? 0 : paragraphs().back().size();
	text_.setCursor(cur.current(), par, pos);
	cur.clearSelection();
	finishUndo();
	sanitizeEmptyText(cur.bv());
	updateLocal(cur);
	cur.bv().updateParagraphDialog();
}


void InsetText::edit(LCursor & cur, int x, int y)
{
	lyxerr << "InsetText::edit xy" << endl;
	old_par = -1;
	text_.edit(cur, x, y);
	//sanitizeEmptyText(cur.bv());
	//updateLocal(cur);
	//cur.bv().updateParagraphDialog();
}


DispatchResult InsetText::priv_dispatch(LCursor & cur, FuncRequest const & cmd)
{
	//lyxerr << "InsetText::priv_dispatch (begin), act: "
	//      << cmd.action << " " << endl;

	setViewCache(&cur.bv());

	bool was_empty = paragraphs().begin()->empty() && paragraphs().size() == 1;
	DispatchResult result = text_.dispatch(cur, cmd);

	// If the action has deleted all text in the inset, we need
	// to change the language to the language of the surronding
	// text.
	// Why this cleverness? (Andre')
	if (!was_empty && paragraphs().begin()->empty() &&
	    paragraphs().size() == 1) {
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setLanguage(cur.bv().getParentLanguage(this));
		text_.setFont(font, false);
	}

	//lyxerr << "InsetText::priv_dispatch (end)" << endl;
	return result;
}


int InsetText::latex(Buffer const & buf, ostream & os,
		     OutputParams const & runparams) const
{
	TexRow texrow;
	latexParagraphs(buf, paragraphs(), os, texrow, runparams);
	return texrow.rows();
}


int InsetText::plaintext(Buffer const & buf, ostream & os,
		     OutputParams const & runparams) const
{
	ParagraphList::const_iterator beg = paragraphs().begin();
	ParagraphList::const_iterator end = paragraphs().end();
	ParagraphList::const_iterator it = beg;
	for (; it != end; ++it)
		asciiParagraph(buf, *it, os, runparams, it == beg);

	//FIXME: Give the total numbers of lines
	return 0;
}


int InsetText::linuxdoc(Buffer const & buf, ostream & os,
			OutputParams const & runparams) const
{
	linuxdocParagraphs(buf, paragraphs(), os, runparams);
	return 0;
}


int InsetText::docbook(Buffer const & buf, ostream & os,
		       OutputParams const & runparams) const
{
	docbookParagraphs(buf, paragraphs(), os, runparams);
	return 0;
}


void InsetText::validate(LaTeXFeatures & features) const
{
	for_each(paragraphs().begin(), paragraphs().end(),
		 boost::bind(&Paragraph::validate, _1, boost::ref(features)));
}


void InsetText::getCursorPos(CursorSlice const & cur, int & x, int & y) const
{
	x = text_.cursorX(cur);
	y = text_.cursorY(cur);
}


bool InsetText::insetAllowed(InsetOld::Code code) const
{
	// in_insetAllowed is a really gross hack,
	// to allow us to call the owner's insetAllowed
	// without stack overflow, which can happen
	// when the owner uses InsetCollapsable::insetAllowed()
	bool ret = true;
	if (in_insetAllowed)
		return ret;
	in_insetAllowed = true;
	if (owner())
		ret = owner()->insetAllowed(code);
	in_insetAllowed = false;
	return ret;
}


bool InsetText::showInsetDialog(BufferView *) const
{
	return false;
}


void InsetText::getLabelList(Buffer const & buffer,
			     std::vector<string> & list) const
{
	ParagraphList::const_iterator pit = paragraphs().begin();
	ParagraphList::const_iterator pend = paragraphs().end();
	for (; pit != pend; ++pit) {
		InsetList::const_iterator beg = pit->insetlist.begin();
		InsetList::const_iterator end = pit->insetlist.end();
		for (; beg != end; ++beg)
			beg->inset->getLabelList(buffer, list);
	}
}


void InsetText::markNew(bool track_changes)
{
	ParagraphList::iterator pit = paragraphs().begin();
	ParagraphList::iterator end = paragraphs().end();
	for (; pit != end; ++pit) {
		if (track_changes) {
			pit->trackChanges();
		} else {
			// no-op when not tracking
			pit->cleanChanges();
		}
	}
}


void InsetText::setText(string const & data, LyXFont const & font)
{
	clear(false);
	for (unsigned int i = 0; i < data.length(); ++i)
		paragraphs().begin()->insertChar(i, data[i], font);
}


void InsetText::setAutoBreakRows(bool flag)
{
	if (flag != autoBreakRows_) {
		autoBreakRows_ = flag;
		if (!flag)
			removeNewlines();
	}
}


void InsetText::setDrawFrame(DrawFrame how)
{
	drawFrame_ = how;
}


LColor_color InsetText::frameColor() const
{
	return LColor::color(frame_color_);
}


void InsetText::setFrameColor(LColor_color col)
{
	frame_color_ = col;
}


void InsetText::setViewCache(BufferView const * bv) const
{
	if (bv && bv != text_.bv_owner) {
		//lyxerr << "setting view cache from "
		//	<< text_.bv_owner << " to " << bv << "\n";
		text_.bv_owner = const_cast<BufferView *>(bv);
	}
}


void InsetText::removeNewlines()
{
	ParagraphList::iterator it = paragraphs().begin();
	ParagraphList::iterator end = paragraphs().end();
	for (; it != end; ++it)
		for (int i = 0; i < it->size(); ++i)
			if (it->isNewline(i))
				it->erase(i);
}


int InsetText::scroll(bool /*recursive*/) const
{
	return UpdatableInset::scroll(false);
}


void InsetText::clearInset(Painter & pain, int x, int y) const
{
	int w = dim_.wid;
	int h = dim_.asc + dim_.des;
	int ty = y - dim_.asc;

	if (ty < 0) {
		h += ty;
		ty = 0;
	}
	if (ty + h > pain.paperHeight())
		h = pain.paperHeight();
	if (xo_ + w > pain.paperWidth())
		w = pain.paperWidth();
	pain.fillRectangle(x + 1, ty + 1, w - 3, h - 1, backgroundColor());
}


LyXText * InsetText::getText(int i) const
{
	return (i == 0) ? const_cast<LyXText*>(&text_) : 0;
}


void InsetText::appendParagraphs(Buffer * buffer, ParagraphList & plist)
{
#warning FIXME Check if Changes stuff needs changing here. (Lgb)
// And it probably does. You have to take a look at this John. (Lgb)
#warning John, have a look here. (Lgb)
	ParagraphList::iterator pit = plist.begin();
	ParagraphList::iterator ins = paragraphs().insert(paragraphs().end(), *pit);
	++pit;
	mergeParagraph(buffer->params(), paragraphs(), boost::prior(ins));

	ParagraphList::iterator pend = plist.end();
	for (; pit != pend; ++pit)
		paragraphs().push_back(*pit);
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


ParagraphList & InsetText::paragraphs() const
{
	return const_cast<ParagraphList &>(text_.paragraphs());
}
