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
	: UpdatableInset(),
	  paragraphs(1),
	  autoBreakRows_(false),
	  drawFrame_(NEVER),
	  frame_color_(LColor::insetframe),
	  text_(0, this, true, paragraphs)
{
	textwidth_ = 0; // broken
	paragraphs.begin()->layout(bp.getLyXTextClass().defaultLayout());
	if (bp.tracking_changes)
		paragraphs.begin()->trackChanges();
	init();
}


InsetText::InsetText(InsetText const & in)
	: UpdatableInset(in),
	  text_(in.text_.bv_owner, this, true, paragraphs)
{
	// this is ugly...
	operator=(in);
}


void InsetText::operator=(InsetText const & in)
{
	UpdatableInset::operator=(in);
	paragraphs = in.paragraphs;
	autoBreakRows_ = in.autoBreakRows_;
	drawFrame_ = in.drawFrame_;
	frame_color_ = in.frame_color_;
	textwidth_ = in.textwidth_;
	text_ = LyXText(in.text_.bv_owner, this, true, paragraphs);
	init();
}


void InsetText::init()
{
	ParagraphList::iterator pit = paragraphs.begin();
	ParagraphList::iterator end = paragraphs.end();
	for (; pit != end; ++pit)
		pit->setInsetOwner(this);
	text_.paragraphs_ = &paragraphs;
	old_par = -1;
	in_insetAllowed = false;
}


void InsetText::clear(bool just_mark_erased)
{
	if (just_mark_erased) {
		ParagraphList::iterator it = paragraphs.begin();
		ParagraphList::iterator end = paragraphs.end();
		for (; it != end; ++it)
			it->markErased();
		return;
	}

	// This is a gross hack...
	LyXLayout_ptr old_layout = paragraphs.begin()->layout();

	paragraphs.clear();
	paragraphs.push_back(Paragraph());
	paragraphs.begin()->setInsetOwner(this);
	paragraphs.begin()->layout(old_layout);
}


auto_ptr<InsetBase> InsetText::clone() const
{
	return auto_ptr<InsetBase>(new InsetText(*this));
}


void InsetText::write(Buffer const & buf, ostream & os) const
{
	os << "Text\n";
	writeParagraphData(buf, os);
}


void InsetText::writeParagraphData(Buffer const & buf, ostream & os) const
{
	ParagraphList::const_iterator it = paragraphs.begin();
	ParagraphList::const_iterator end = paragraphs.end();
	Paragraph::depth_type dth = 0;
	for (; it != end; ++it) {
		it->write(buf, os, buf.params(), dth);
	}
}


void InsetText::read(Buffer const & buf, LyXLex & lex)
{
	string token;
	Paragraph::depth_type depth = 0;

	clear(false);

#warning John, look here. Doesnt make much sense.
	if (buf.params().tracking_changes)
		paragraphs.begin()->trackChanges();

	// delete the initial paragraph
	Paragraph oldpar = *paragraphs.begin();
	paragraphs.clear();
	ParagraphList::iterator pit = paragraphs.begin();

	while (lex.isOK()) {
		lex.nextToken();
		token = lex.getString();
		if (token.empty())
			continue;
		if (token == "\\end_inset") {
			break;
		}

		if (token == "\\end_document") {
			lex.printError("\\end_document read in inset! Error in document!");
			return;
		}

		// FIXME: ugly.
		const_cast<Buffer&>(buf).readParagraph(lex, token, paragraphs, pit, depth);
	}

	pit = paragraphs.begin();
	ParagraphList::iterator const end = paragraphs.end();
	for (; pit != end; ++pit)
		pit->setInsetOwner(this);

	if (token != "\\end_inset") {
		lex.printError("Missing \\end_inset at this point. "
					   "Read: `$$Token'");
	}

	// sanity check
	// ensure we have at least one par.
	if (paragraphs.empty())
		paragraphs.push_back(oldpar);
}


void InsetText::metrics(MetricsInfo & mi, Dimension & dim) const
{
	//lyxerr << "InsetText::metrics: width: " << mi.base.textwidth << endl;
	textwidth_ = max(40, mi.base.textwidth - 30);
	BufferView * bv = mi.base.bv;
	setViewCache(bv);
	text_.metrics(mi, dim);
	dim.asc += TEXT_TO_INSET_OFFSET;
	dim.des += TEXT_TO_INSET_OFFSET;
	dim.wid += 2 * TEXT_TO_INSET_OFFSET;
	dim_ = dim;
}


int InsetText::textWidth() const
{
	return textwidth_;
}


void InsetText::draw(PainterInfo & pi, int x, int y) const
{
	// update our idea of where we are. Clearly, we should
	// not have to know this information.
	xo_ = x;
	yo_ = y;

	int const start_x = x;

	BufferView * bv = pi.base.bv;
	Painter & pain = pi.pain;

	// repaint the background if needed
	if (backgroundColor() != LColor::background)
		clearInset(bv, start_x + TEXT_TO_INSET_OFFSET, y);

	bv->hideCursor();

	if (!owner())
		x += scroll();

	x += TEXT_TO_INSET_OFFSET;
	y += bv->top_y() - text_.firstRow()->ascent_of_text();

	text_.xo_ = x;
	text_.yo_ = y;
	paintTextInset(*bv, text_, x, y);

	if (drawFrame_ == ALWAYS || drawFrame_ == LOCKED)
		drawFrame(pain, start_x);
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


void InsetText::updateLocal(BufferView * bv, bool /*mark_dirty*/)
{
	if (!bv)
		return;

	if (!autoBreakRows_ && paragraphs.size() > 1)
		collapseParagraphs(bv);

	if (!text_.selection.set())
		text_.selection.cursor = text_.cursor;

	bv->owner()->view_state_changed();
	bv->owner()->updateMenubar();
	bv->owner()->updateToolbar();
	if (old_par != text_.cursor.par()) {
		bv->owner()->setLayout(cpar()->layout()->name());
		old_par = text_.cursor.par();
	}
}


string const InsetText::editMessage() const
{
	return _("Opened Text Inset");
}


void InsetText::sanitizeEmptyText(BufferView * bv)
{
	if (paragraphs.size() == 1
			&& paragraphs.begin()->empty()
			&& bv->getParentLanguage(this) != text_.current_font.language()) {
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setLanguage(bv->getParentLanguage(this));
		setFont(bv, font, false);
	}
}


extern LCursor theTempCursor;


void InsetText::edit(BufferView * bv, bool left)
{
	lyxerr << "InsetText: edit left/right" << endl;
	setViewCache(bv);

	old_par = -1;

	if (left)
		text_.setCursorIntern(0, 0);
	else
		text_.setCursor(paragraphs.size() - 1, paragraphs.back().size());

	sanitizeEmptyText(bv);
	updateLocal(bv, false);
	bv->updateParagraphDialog();
}


void InsetText::edit(BufferView * bv, int x, int y)
{
	lyxerr << "InsetText::edit xy" << endl;
	old_par = -1;
	sanitizeEmptyText(bv);
	text_.setCursorFromCoordinates(x - text_.xo_, y + bv->top_y()
				       - text_.yo_);
	text_.clearSelection();
	finishUndo();

	updateLocal(bv, false);
	bv->updateParagraphDialog();
}


DispatchResult InsetText::priv_dispatch(FuncRequest const & cmd,
	idx_type &, pos_type &)
{
	lyxerr << "InsetText::priv_dispatch (begin), act: "
	       << cmd.action << " " << endl;

	BufferView * bv = cmd.view();
	setViewCache(bv);

	DispatchResult result;
	result.dispatched(true);

	bool was_empty = paragraphs.begin()->empty() && paragraphs.size() == 1;

	switch (cmd.action) {
	case LFUN_MOUSE_PRESS:
		bv->cursor() = theTempCursor;
		// fall through
	default:
		result = text_.dispatch(cmd);
		break;
	}

	// If the action has deleted all text in the inset, we need
	// to change the language to the language of the surronding
	// text.
	if (!was_empty && paragraphs.begin()->empty() &&
	    paragraphs.size() == 1) {
		LyXFont font(LyXFont::ALL_IGNORE);
		font.setLanguage(bv->getParentLanguage(this));
		setFont(bv, font, false);
	}

	lyxerr << "InsetText::priv_dispatch (end)" << endl;
	return result;
}


int InsetText::latex(Buffer const & buf, ostream & os,
		     OutputParams const & runparams) const
{
	TexRow texrow;
	latexParagraphs(buf, paragraphs, os, texrow, runparams);
	return texrow.rows();
}


int InsetText::plaintext(Buffer const & buf, ostream & os,
		     OutputParams const & runparams) const
{
	ParagraphList::const_iterator beg = paragraphs.begin();
	ParagraphList::const_iterator end = paragraphs.end();
	ParagraphList::const_iterator it = beg;
	for (; it != end; ++it)
		asciiParagraph(buf, *it, os, runparams, it == beg);

	//FIXME: Give the total numbers of lines
	return 0;
}


int InsetText::linuxdoc(Buffer const & buf, ostream & os,
			OutputParams const & runparams) const
{
	linuxdocParagraphs(buf, paragraphs, os, runparams);
	return 0;
}


int InsetText::docbook(Buffer const & buf, ostream & os,
		       OutputParams const & runparams) const
{
	docbookParagraphs(buf, paragraphs, os, runparams);
	return 0;
}


void InsetText::validate(LaTeXFeatures & features) const
{
	for_each(paragraphs.begin(), paragraphs.end(),
		 boost::bind(&Paragraph::validate, _1, boost::ref(features)));
}


void InsetText::getCursorPos(int & x, int & y) const
{
	x = text_.cursor.x() + TEXT_TO_INSET_OFFSET;
	y = text_.cursor.y() - dim_.asc + TEXT_TO_INSET_OFFSET;
}


int InsetText::insetInInsetY() const
{
	return 0;
}


bool InsetText::insertInset(BufferView * bv, InsetOld * inset)
{
	inset->setOwner(this);
	text_.insertInset(inset);
	updateLocal(bv, true);
	return true;
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
	ParagraphList::const_iterator pit = paragraphs.begin();
	ParagraphList::const_iterator pend = paragraphs.end();
	for (; pit != pend; ++pit) {
		InsetList::const_iterator beg = pit->insetlist.begin();
		InsetList::const_iterator end = pit->insetlist.end();
		for (; beg != end; ++beg)
			beg->inset->getLabelList(buffer, list);
	}
}


void InsetText::setFont(BufferView * bv, LyXFont const & font, bool toggleall,
			bool selectall)
{
	if ((paragraphs.size() == 1 && paragraphs.begin()->empty())
	    || cpar()->empty()) {
		text_.setFont(font, toggleall);
		return;
	}

	if (text_.selection.set())
		text_.recUndo(text_.cursor.par());

	if (selectall) {
		text_.cursorTop();
		text_.selection.cursor = text_.cursor;
		text_.cursorBottom();
		text_.setSelection();
	}

	text_.toggleFree(font, toggleall);

	if (selectall)
		text_.clearSelection();

	updateLocal(bv, true);
}


void InsetText::markNew(bool track_changes)
{
	ParagraphList::iterator pit = paragraphs.begin();
	ParagraphList::iterator end = paragraphs.end();
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
		paragraphs.begin()->insertChar(i, data[i], font);
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


pos_type InsetText::cpos() const
{
	return text_.cursor.pos();
}


ParagraphList::iterator InsetText::cpar() const
{
	return text_.cursorPar();
}


RowList::iterator InsetText::crow() const
{
	return cpar()->getRow(cpos());
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
	ParagraphList::iterator it = paragraphs.begin();
	ParagraphList::iterator end = paragraphs.end();
	for (; it != end; ++it)
		for (int i = 0; i < it->size(); ++i)
			if (it->isNewline(i))
				it->erase(i);
}


int InsetText::scroll(bool /*recursive*/) const
{
	return UpdatableInset::scroll(false);
}


void InsetText::clearSelection(BufferView *)
{
	text_.clearSelection();
}


void InsetText::clearInset(BufferView * bv, int start_x, int baseline) const
{
	Painter & pain = bv->painter();
	int w = dim_.wid;
	int h = dim_.asc + dim_.des;
	int ty = baseline - dim_.asc;

	if (ty < 0) {
		h += ty;
		ty = 0;
	}
	if (ty + h > pain.paperHeight())
		h = pain.paperHeight();
	if (xo_ + w > pain.paperWidth())
		w = pain.paperWidth();
	pain.fillRectangle(start_x + 1, ty + 1, w - 3, h - 1, backgroundColor());
}


ParagraphList * InsetText::getParagraphs(int i) const
{
	return (i == 0) ? const_cast<ParagraphList*>(&paragraphs) : 0;
}


LyXText * InsetText::getText(int i) const
{
	return (i == 0) ? const_cast<LyXText*>(&text_) : 0;
}


bool InsetText::checkInsertChar(LyXFont & font)
{
	return owner() ? owner()->checkInsertChar(font) : true;
}


void InsetText::collapseParagraphs(BufferView * bv)
{
	while (paragraphs.size() > 1) {
		ParagraphList::iterator const first = paragraphs.begin();
		ParagraphList::iterator second = first;
		++second;
		size_t const first_par_size = first->size();

		if (!first->empty() &&
		    !second->empty() &&
		    !first->isSeparator(first_par_size - 1)) {
			first->insertChar(first_par_size, ' ');
		}

#warning probably broken
		if (text_.selection.set()) {
			if (text_.selection.start.par() == 1) {
				text_.selection.start.par(1);
				text_.selection.start.pos(text_.selection.start.pos() + first_par_size);
			}
			if (text_.selection.end.par() == 2) {
				text_.selection.end.par(1);
				text_.selection.end.pos(text_.selection.end.pos() + first_par_size);
			}
		}

		mergeParagraph(bv->buffer()->params(), paragraphs, first);
	}
}


void InsetText::getDrawFont(LyXFont & font) const
{
	if (owner())
		owner()->getDrawFont(font);
}


void InsetText::appendParagraphs(Buffer * buffer, ParagraphList & plist)
{
#warning FIXME Check if Changes stuff needs changing here. (Lgb)
// And it probably does. You have to take a look at this John. (Lgb)
#warning John, have a look here. (Lgb)
	ParagraphList::iterator pit = plist.begin();
	ParagraphList::iterator ins = paragraphs.insert(paragraphs.end(), *pit);
	++pit;
	mergeParagraph(buffer->params(), paragraphs, boost::prior(ins));

	ParagraphList::iterator pend = plist.end();
	for (; pit != pend; ++pit)
		paragraphs.push_back(*pit);
}


void InsetText::addPreview(PreviewLoader & loader) const
{
	ParagraphList::const_iterator pit = paragraphs.begin();
	ParagraphList::const_iterator pend = paragraphs.end();

	for (; pit != pend; ++pit) {
		InsetList::const_iterator it  = pit->insetlist.begin();
		InsetList::const_iterator end = pit->insetlist.end();
		for (; it != end; ++it)
			it->inset->addPreview(loader);
	}
}
