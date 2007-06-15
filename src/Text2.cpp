/**
 * \file text2.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author Lars Gullik Bjønnes
 * \author Alfredo Braunstein
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 * \author John Levon
 * \author André Pönitz
 * \author Allan Rae
 * \author Stefan Schimanski
 * \author Dekel Tsur
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Text.h"

#include "Buffer.h"
#include "buffer_funcs.h"
#include "BufferList.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "bufferview_funcs.h"
#include "Bullet.h"
#include "CoordCache.h"
#include "Cursor.h"
#include "CutAndPaste.h"
#include "debug.h"
#include "DispatchResult.h"
#include "ErrorList.h"
#include "FuncRequest.h"
#include "gettext.h"
#include "Language.h"
#include "Color.h"
#include "LyXFunc.h"
#include "LyXRC.h"
#include "Row.h"
#include "Paragraph.h"
#include "TextMetrics.h"
#include "paragraph_funcs.h"
#include "ParagraphParameters.h"
#include "ParIterator.h"
#include "Server.h"
#include "ServerSocket.h"
#include "Undo.h"
#include "VSpace.h"

#include "frontends/FontMetrics.h"

#include "insets/InsetEnvironment.h"

#include "mathed/InsetMathHull.h"

#include "support/textutils.h"

#include <boost/current_function.hpp>

#include <sstream>


namespace lyx {

using std::endl;
using std::ostringstream;
using std::string;
using std::max;
using std::min;


Text::Text()
	: current_font(Font::ALL_INHERIT),
	  background_color_(Color::background),
	  autoBreakRows_(false)
{}


bool Text::isMainText(Buffer const & buffer) const
{
	return &buffer.text() == this;
}


//takes screen x,y coordinates
Inset * Text::checkInsetHit(BufferView & bv, int x, int y)
{
	pit_type pit = getPitNearY(bv, y);
	BOOST_ASSERT(pit != -1);

	Paragraph const & par = pars_[pit];

	LYXERR(Debug::DEBUG)
		<< BOOST_CURRENT_FUNCTION
		<< ": x: " << x
		<< " y: " << y
		<< "  pit: " << pit
		<< endl;
	InsetList::const_iterator iit = par.insetlist.begin();
	InsetList::const_iterator iend = par.insetlist.end();
	for (; iit != iend; ++iit) {
		Inset * inset = iit->inset;
#if 1
		LYXERR(Debug::DEBUG)
			<< BOOST_CURRENT_FUNCTION
			<< ": examining inset " << inset << endl;

		if (bv.coordCache().getInsets().has(inset))
			LYXERR(Debug::DEBUG)
				<< BOOST_CURRENT_FUNCTION
				<< ": xo: " << inset->xo(bv) << "..."
				<< inset->xo(bv) + inset->width()
				<< " yo: " << inset->yo(bv) - inset->ascent()
				<< "..."
				<< inset->yo(bv) + inset->descent()
				<< endl;
		else
			LYXERR(Debug::DEBUG)
				<< BOOST_CURRENT_FUNCTION
				<< ": inset has no cached position" << endl;
#endif
		if (inset->covers(bv, x, y)) {
			LYXERR(Debug::DEBUG)
				<< BOOST_CURRENT_FUNCTION
				<< ": Hit inset: " << inset << endl;
			return inset;
		}
	}
	LYXERR(Debug::DEBUG)
		<< BOOST_CURRENT_FUNCTION
		<< ": No inset hit. " << endl;
	return 0;
}



// Gets the fully instantiated font at a given position in a paragraph
// Basically the same routine as Paragraph::getFont() in Paragraph.cpp.
// The difference is that this one is used for displaying, and thus we
// are allowed to make cosmetic improvements. For instance make footnotes
// smaller. (Asger)
Font Text::getFont(Buffer const & buffer, Paragraph const & par,
		pos_type const pos) const
{
	BOOST_ASSERT(pos >= 0);

	Layout_ptr const & layout = par.layout();
#ifdef WITH_WARNINGS
#warning broken?
#endif
	BufferParams const & params = buffer.params();
	pos_type const body_pos = par.beginOfBody();

	// We specialize the 95% common case:
	if (!par.getDepth()) {
		Font f = par.getFontSettings(params, pos);
		if (!isMainText(buffer))
			applyOuterFont(buffer, f);
		Font lf;
		Font rlf;
		if (layout->labeltype == LABEL_MANUAL && pos < body_pos) {
			lf = layout->labelfont;
			rlf = layout->reslabelfont;
		} else {
			lf = layout->font;
			rlf = layout->resfont;
		}
		// In case the default family has been customized
		if (lf.family() == Font::INHERIT_FAMILY)
			rlf.setFamily(params.getFont().family());
		return f.realize(rlf);
	}

	// The uncommon case need not be optimized as much
	Font layoutfont;
	if (pos < body_pos)
		layoutfont = layout->labelfont;
	else
		layoutfont = layout->font;

	Font font = par.getFontSettings(params, pos);
	font.realize(layoutfont);

	if (!isMainText(buffer))
		applyOuterFont(buffer, font);

	// Find the pit value belonging to paragraph. This will not break
	// even if pars_ would not be a vector anymore.
	// Performance appears acceptable.

	pit_type pit = pars_.size();
	for (pit_type it = 0; it < pit; ++it)
		if (&pars_[it] == &par) {
			pit = it;
			break;
		}
	// Realize against environment font information
	// NOTE: the cast to pit_type should be removed when pit_type
	// changes to a unsigned integer.
	if (pit < pit_type(pars_.size()))
		font.realize(outerFont(pit, pars_));

	// Realize with the fonts of lesser depth.
	font.realize(params.getFont());

	return font;
}

// There are currently two font mechanisms in LyX:
// 1. The font attributes in a lyxtext, and
// 2. The inset-specific font properties, defined in an inset's
// metrics() and draw() methods and handed down the inset chain through
// the pi/mi parameters, and stored locally in a lyxtext in font_.
// This is where the two are integrated in the final fully realized
// font.
void Text::applyOuterFont(Buffer const & buffer, Font & font) const {
	Font lf(font_);
	lf.reduce(buffer.params().getFont());
	lf.realize(font);
	lf.setLanguage(font.language());
	font = lf;
}


Font Text::getLayoutFont(Buffer const & buffer, pit_type const pit) const
{
	Layout_ptr const & layout = pars_[pit].layout();

	if (!pars_[pit].getDepth())  {
		Font lf = layout->resfont;
		// In case the default family has been customized
		if (layout->font.family() == Font::INHERIT_FAMILY)
			lf.setFamily(buffer.params().getFont().family());
		return lf;
	}

	Font font = layout->font;
	// Realize with the fonts of lesser depth.
	//font.realize(outerFont(pit, paragraphs()));
	font.realize(buffer.params().getFont());

	return font;
}


Font Text::getLabelFont(Buffer const & buffer, Paragraph const & par) const
{
	Layout_ptr const & layout = par.layout();

	if (!par.getDepth()) {
		Font lf = layout->reslabelfont;
		// In case the default family has been customized
		if (layout->labelfont.family() == Font::INHERIT_FAMILY)
			lf.setFamily(buffer.params().getFont().family());
		return lf;
	}

	Font font = layout->labelfont;
	// Realize with the fonts of lesser depth.
	font.realize(buffer.params().getFont());

	return font;
}


void Text::setCharFont(Buffer const & buffer, pit_type pit,
		pos_type pos, Font const & fnt)
{
	Font font = fnt;
	Layout_ptr const & layout = pars_[pit].layout();

	// Get concrete layout font to reduce against
	Font layoutfont;

	if (pos < pars_[pit].beginOfBody())
		layoutfont = layout->labelfont;
	else
		layoutfont = layout->font;

	// Realize against environment font information
	if (pars_[pit].getDepth()) {
		pit_type tp = pit;
		while (!layoutfont.resolved() &&
		       tp != pit_type(paragraphs().size()) &&
		       pars_[tp].getDepth()) {
			tp = outerHook(tp, paragraphs());
			if (tp != pit_type(paragraphs().size()))
				layoutfont.realize(pars_[tp].layout()->font);
		}
	}

	// Inside inset, apply the inset's font attributes if any
	// (charstyle!)
	if (!isMainText(buffer))
		layoutfont.realize(font_);

	layoutfont.realize(buffer.params().getFont());

	// Now, reduce font against full layout font
	font.reduce(layoutfont);

	pars_[pit].setFont(pos, font);
}


void Text::setInsetFont(Buffer const & buffer, pit_type pit,
		pos_type pos, Font const & font, bool toggleall)
{
	BOOST_ASSERT(pars_[pit].isInset(pos) &&
		     pars_[pit].getInset(pos)->noFontChange());

	Inset * const inset = pars_[pit].getInset(pos);
	DocIterator dit = doc_iterator_begin(*inset);
	// start of the last cell
	DocIterator end = dit;
	end.idx() = end.lastidx();

	while (true) {
		Text * text = dit.text();
		Inset * cell = dit.realInset();
		if (text && cell) {
			DocIterator cellbegin = doc_iterator_begin(*cell);
			// last position of the cell
			DocIterator cellend = cellbegin;
			cellend.pit() = cellend.lastpit();
			cellend.pos() = cellend.lastpos();
			text->setFont(buffer, cellbegin, cellend, font, toggleall);
		}
		if (dit == end)
			break;
		dit.forwardIdx();
	}
}


// return past-the-last paragraph influenced by a layout change on pit
pit_type Text::undoSpan(pit_type pit)
{
	pit_type end = paragraphs().size();
	pit_type nextpit = pit + 1;
	if (nextpit == end)
		return nextpit;
	//because of parindents
	if (!pars_[pit].getDepth())
		return boost::next(nextpit);
	//because of depth constrains
	for (; nextpit != end; ++pit, ++nextpit) {
		if (!pars_[pit].getDepth())
			break;
	}
	return nextpit;
}


void Text::setLayout(Buffer const & buffer, pit_type start, pit_type end,
		string const & layout)
{
	BOOST_ASSERT(start != end);

	BufferParams const & bufparams = buffer.params();
	Layout_ptr const & lyxlayout = bufparams.getTextClass()[layout];

	for (pit_type pit = start; pit != end; ++pit) {
		Paragraph & par = pars_[pit];
		par.applyLayout(lyxlayout);
		if (lyxlayout->margintype == MARGIN_MANUAL)
			par.setLabelWidthString(par.translateIfPossible(
				lyxlayout->labelstring(), buffer.params()));
	}
}


// set layout over selection and make a total rebreak of those paragraphs
void Text::setLayout(Cursor & cur, string const & layout)
{
	BOOST_ASSERT(this == cur.text());
	// special handling of new environment insets
	BufferView & bv = cur.bv();
	BufferParams const & params = bv.buffer()->params();
	Layout_ptr const & lyxlayout = params.getTextClass()[layout];
	if (lyxlayout->is_environment) {
		// move everything in a new environment inset
		LYXERR(Debug::DEBUG) << "setting layout " << layout << endl;
		lyx::dispatch(FuncRequest(LFUN_LINE_BEGIN));
		lyx::dispatch(FuncRequest(LFUN_LINE_END_SELECT));
		lyx::dispatch(FuncRequest(LFUN_CUT));
		Inset * inset = new InsetEnvironment(params, layout);
		insertInset(cur, inset);
		//inset->edit(cur, true);
		//lyx::dispatch(FuncRequest(LFUN_PASTE));
		return;
	}

	pit_type start = cur.selBegin().pit();
	pit_type end = cur.selEnd().pit() + 1;
	pit_type undopit = undoSpan(end - 1);
	recUndo(cur, start, undopit - 1);
	setLayout(cur.buffer(), start, end, layout);
	updateLabels(cur.buffer());
}


static bool changeDepthAllowed(Text::DEPTH_CHANGE type,
			Paragraph const & par, int max_depth)
{
	if (par.layout()->labeltype == LABEL_BIBLIO)
		return false;
	int const depth = par.params().depth();
	if (type == Text::INC_DEPTH && depth < max_depth)
		return true;
	if (type == Text::DEC_DEPTH && depth > 0)
		return true;
	return false;
}


bool Text::changeDepthAllowed(Cursor & cur, DEPTH_CHANGE type) const
{
	BOOST_ASSERT(this == cur.text());
	// this happens when selecting several cells in tabular (bug 2630)
	if (cur.selBegin().idx() != cur.selEnd().idx())
		return false;

	pit_type const beg = cur.selBegin().pit();
	pit_type const end = cur.selEnd().pit() + 1;
	int max_depth = (beg != 0 ? pars_[beg - 1].getMaxDepthAfter() : 0);

	for (pit_type pit = beg; pit != end; ++pit) {
		if (lyx::changeDepthAllowed(type, pars_[pit], max_depth))
			return true;
		max_depth = pars_[pit].getMaxDepthAfter();
	}
	return false;
}


void Text::changeDepth(Cursor & cur, DEPTH_CHANGE type)
{
	BOOST_ASSERT(this == cur.text());
	pit_type const beg = cur.selBegin().pit();
	pit_type const end = cur.selEnd().pit() + 1;
	recordUndoSelection(cur);
	int max_depth = (beg != 0 ? pars_[beg - 1].getMaxDepthAfter() : 0);

	for (pit_type pit = beg; pit != end; ++pit) {
		Paragraph & par = pars_[pit];
		if (lyx::changeDepthAllowed(type, par, max_depth)) {
			int const depth = par.params().depth();
			if (type == INC_DEPTH)
				par.params().depth(depth + 1);
			else
				par.params().depth(depth - 1);
		}
		max_depth = par.getMaxDepthAfter();
	}
	// this handles the counter labels, and also fixes up
	// depth values for follow-on (child) paragraphs
	updateLabels(cur.buffer());
}


void Text::setFont(Cursor & cur, Font const & font, bool toggleall)
{
	BOOST_ASSERT(this == cur.text());
	// Set the current_font
	// Determine basis font
	Font layoutfont;
	pit_type pit = cur.pit();
	if (cur.pos() < pars_[pit].beginOfBody())
		layoutfont = getLabelFont(cur.buffer(), pars_[pit]);
	else
		layoutfont = getLayoutFont(cur.buffer(), pit);

	// Update current font
	real_current_font.update(font,
					cur.buffer().params().language,
					toggleall);

	// Reduce to implicit settings
	current_font = real_current_font;
	current_font.reduce(layoutfont);
	// And resolve it completely
	real_current_font.realize(layoutfont);

	// if there is no selection that's all we need to do
	if (!cur.selection())
		return;

	// Ok, we have a selection.
	recordUndoSelection(cur);

	setFont(cur.buffer(), cur.selectionBegin(), cur.selectionEnd(), font,
		toggleall);
}


void Text::setFont(Buffer const & buffer, DocIterator const & begin,
		DocIterator const & end, Font const & font,
		bool toggleall)
{
	// Don't use forwardChar here as ditend might have
	// pos() == lastpos() and forwardChar would miss it.
	// Can't use forwardPos either as this descends into
	// nested insets.
	Language const * language = buffer.params().language;
	for (DocIterator dit = begin; dit != end; dit.forwardPosNoDescend()) {
		if (dit.pos() != dit.lastpos()) {
			pit_type const pit = dit.pit();
			pos_type const pos = dit.pos();
			if (pars_[pit].isInset(pos) &&
			    pars_[pit].getInset(pos)->noFontChange())
				// We need to propagate the font change to all
				// text cells of the inset (bug 1973).
				// FIXME: This should change, see documentation
				// of noFontChange in Inset.h
				setInsetFont(buffer, pit, pos, font, toggleall);
			Font f = getFont(buffer, dit.paragraph(), pos);
			f.update(font, language, toggleall);
			setCharFont(buffer, pit, pos, f);
		}
	}
}


// the cursor set functions have a special mechanism. When they
// realize you left an empty paragraph, they will delete it.

bool Text::cursorHome(Cursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	ParagraphMetrics const & pm = cur.bv().parMetrics(this, cur.pit());
	Row const & row = pm.getRow(cur.pos(),cur.boundary());
	return setCursor(cur, cur.pit(), row.pos());
}


bool Text::cursorEnd(Cursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	// if not on the last row of the par, put the cursor before
	// the final space exept if I have a spanning inset or one string
	// is so long that we force a break.
	pos_type end = cur.textRow().endpos();
	if (end == 0)
		// empty text, end-1 is no valid position
		return false;
	bool boundary = false;
	if (end != cur.lastpos()) {
		if (!cur.paragraph().isLineSeparator(end-1)
		    && !cur.paragraph().isNewline(end-1))
			boundary = true;
		else
			--end;
	}
	return setCursor(cur, cur.pit(), end, true, boundary);
}


bool Text::cursorTop(Cursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	return setCursor(cur, 0, 0);
}


bool Text::cursorBottom(Cursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	return setCursor(cur, cur.lastpit(), boost::prior(paragraphs().end())->size());
}


void Text::toggleFree(Cursor & cur, Font const & font, bool toggleall)
{
	BOOST_ASSERT(this == cur.text());
	// If the mask is completely neutral, tell user
	if (font == Font(Font::ALL_IGNORE)) {
		// Could only happen with user style
		cur.message(_("No font change defined."));
		return;
	}

	// Try implicit word selection
	// If there is a change in the language the implicit word selection
	// is disabled.
	CursorSlice resetCursor = cur.top();
	bool implicitSelection =
		font.language() == ignore_language
		&& font.number() == Font::IGNORE
		&& selectWordWhenUnderCursor(cur, WHOLE_WORD_STRICT);

	// Set font
	setFont(cur, font, toggleall);

	// Implicit selections are cleared afterwards
	// and cursor is set to the original position.
	if (implicitSelection) {
		cur.clearSelection();
		cur.top() = resetCursor;
		cur.resetAnchor();
	}
}


docstring Text::getStringToIndex(Cursor const & cur)
{
	BOOST_ASSERT(this == cur.text());

	docstring idxstring;
	if (cur.selection())
		idxstring = cur.selectionAsString(false);
	else {
		// Try implicit word selection. If there is a change
		// in the language the implicit word selection is
		// disabled.
		Cursor tmpcur = cur;
		selectWord(tmpcur, PREVIOUS_WORD);

		if (!tmpcur.selection())
			cur.message(_("Nothing to index!"));
		else if (tmpcur.selBegin().pit() != tmpcur.selEnd().pit())
			cur.message(_("Cannot index more than one paragraph!"));
		else
			idxstring = tmpcur.selectionAsString(false);
	}

	return idxstring;
}


void Text::setParagraph(Cursor & cur,
			   Spacing const & spacing, LyXAlignment align,
			   docstring const & labelwidthstring, bool noindent)
{
	BOOST_ASSERT(cur.text());
	// make sure that the depth behind the selection are restored, too
	pit_type undopit = undoSpan(cur.selEnd().pit());
	recUndo(cur, cur.selBegin().pit(), undopit - 1);

	for (pit_type pit = cur.selBegin().pit(), end = cur.selEnd().pit();
	     pit <= end; ++pit) {
		Paragraph & par = pars_[pit];
		ParagraphParameters & params = par.params();
		params.spacing(spacing);

		// does the layout allow the new alignment?
		Layout_ptr const & layout = par.layout();

		if (align == LYX_ALIGN_LAYOUT)
			align = layout->align;
		if (align & layout->alignpossible) {
			if (align == layout->align)
				params.align(LYX_ALIGN_LAYOUT);
			else
				params.align(align);
		}
		par.setLabelWidthString(labelwidthstring);
		params.noindent(noindent);
	}
}


// this really should just insert the inset and not move the cursor.
void Text::insertInset(Cursor & cur, Inset * inset)
{
	BOOST_ASSERT(this == cur.text());
	BOOST_ASSERT(inset);
	cur.paragraph().insertInset(cur.pos(), inset, current_font,
				    Change(cur.buffer().params().trackChanges ?
					   Change::INSERTED : Change::UNCHANGED));
}


// needed to insert the selection
void Text::insertStringAsLines(Cursor & cur, docstring const & str)
{
	cur.buffer().insertStringAsLines(pars_, cur.pit(), cur.pos(),
					 current_font, str, autoBreakRows_);
}


// turn double CR to single CR, others are converted into one
// blank. Then insertStringAsLines is called
void Text::insertStringAsParagraphs(Cursor & cur, docstring const & str)
{
	docstring linestr = str;
	bool newline_inserted = false;

	for (string::size_type i = 0, siz = linestr.size(); i < siz; ++i) {
		if (linestr[i] == '\n') {
			if (newline_inserted) {
				// we know that \r will be ignored by
				// insertStringAsLines. Of course, it is a dirty
				// trick, but it works...
				linestr[i - 1] = '\r';
				linestr[i] = '\n';
			} else {
				linestr[i] = ' ';
				newline_inserted = true;
			}
		} else if (isPrintable(linestr[i])) {
			newline_inserted = false;
		}
	}
	insertStringAsLines(cur, linestr);
}


bool Text::setCursor(Cursor & cur, pit_type par, pos_type pos,
			bool setfont, bool boundary)
{
	Cursor old = cur;
	setCursorIntern(cur, par, pos, setfont, boundary);
	return cur.bv().checkDepm(cur, old);
}


void Text::setCursor(CursorSlice & cur, pit_type par, pos_type pos)
{
	BOOST_ASSERT(par != int(paragraphs().size()));
	cur.pit() = par;
	cur.pos() = pos;

	// now some strict checking
	Paragraph & para = getPar(par);

	// None of these should happen, but we're scaredy-cats
	if (pos < 0) {
		lyxerr << "dont like -1" << endl;
		BOOST_ASSERT(false);
	}

	if (pos > para.size()) {
		lyxerr << "dont like 1, pos: " << pos
		       << " size: " << para.size()
		       << " par: " << par << endl;
		BOOST_ASSERT(false);
	}
}


void Text::setCursorIntern(Cursor & cur,
			      pit_type par, pos_type pos, bool setfont, bool boundary)
{
	BOOST_ASSERT(this == cur.text());
	cur.boundary(boundary);
	setCursor(cur.top(), par, pos);
	if (setfont)
		setCurrentFont(cur);
}


void Text::setCurrentFont(Cursor & cur)
{
	BOOST_ASSERT(this == cur.text());
	pos_type pos = cur.pos();
	Paragraph & par = cur.paragraph();

	// are we behind previous char in fact? -> go to that char
	if (pos > 0 && cur.boundary())
		--pos;

	// find position to take the font from
	if (pos != 0) {
		// paragraph end? -> font of last char
		if (pos == cur.lastpos())
			--pos;
		// on space? -> look at the words in front of space
		else if (pos > 0 && par.isSeparator(pos))	{
			// abc| def -> font of c
			// abc |[WERBEH], i.e. boundary==true -> font of c
			// abc [WERBEH]| def, font of the space
			if (!isRTLBoundary(cur.buffer(), par, pos))
				--pos;
		}
	}

	// get font
	BufferParams const & bufparams = cur.buffer().params();
	current_font = par.getFontSettings(bufparams, pos);
	real_current_font = getFont(cur.buffer(), par, pos);

	// special case for paragraph end
	if (cur.pos() == cur.lastpos()
	    && isRTLBoundary(cur.buffer(), par, cur.pos())
	    && !cur.boundary()) {
		Language const * lang = par.getParLanguage(bufparams);
		current_font.setLanguage(lang);
		current_font.setNumber(Font::OFF);
		real_current_font.setLanguage(lang);
		real_current_font.setNumber(Font::OFF);
	}
}

// y is screen coordinate
pit_type Text::getPitNearY(BufferView & bv, int y) const
{
	BOOST_ASSERT(!paragraphs().empty());
	BOOST_ASSERT(bv.coordCache().getParPos().find(this) != bv.coordCache().getParPos().end());
	CoordCache::InnerParPosCache const & cc = bv.coordCache().getParPos().find(this)->second;
	LYXERR(Debug::DEBUG)
		<< BOOST_CURRENT_FUNCTION
		<< ": y: " << y << " cache size: " << cc.size()
		<< endl;

	// look for highest numbered paragraph with y coordinate less than given y
	pit_type pit = 0;
	int yy = -1;
	CoordCache::InnerParPosCache::const_iterator it = cc.begin();
	CoordCache::InnerParPosCache::const_iterator et = cc.end();
	CoordCache::InnerParPosCache::const_iterator last = et; last--;

	TextMetrics & tm = bv.textMetrics(this);
	ParagraphMetrics const & pm = tm.parMetrics(it->first);

	// If we are off-screen (before the visible part)
	if (y < 0
		// and even before the first paragraph in the cache.
		&& y < it->second.y_ - int(pm.ascent())) {
		//  and we are not at the first paragraph in the inset.
		if (it->first == 0)
			return 0;
		// then this is the paragraph we are looking for.
		pit = it->first - 1;
		// rebreak it and update the CoordCache.
		tm.redoParagraph(pit);
		bv.coordCache().parPos()[this][pit] =
			Point(0, it->second.y_ - pm.descent());
		return pit;
	}

	ParagraphMetrics const & pm_last = bv.parMetrics(this, last->first);

	// If we are off-screen (after the visible part)
	if (y > bv.workHeight()
		// and even after the first paragraph in the cache.
		&& y >= last->second.y_ + int(pm_last.descent())) {
		pit = last->first + 1;
		//  and we are not at the last paragraph in the inset.
		if (pit == int(pars_.size()))
			return last->first;
		// then this is the paragraph we are looking for.
		// rebreak it and update the CoordCache.
		tm.redoParagraph(pit);
		bv.coordCache().parPos()[this][pit] =
			Point(0, last->second.y_ + pm_last.ascent());
		return pit;
	}

	for (; it != et; ++it) {
		LYXERR(Debug::DEBUG)
			<< BOOST_CURRENT_FUNCTION
			<< "  examining: pit: " << it->first
			<< " y: " << it->second.y_
			<< endl;

		ParagraphMetrics const & pm = bv.parMetrics(this, it->first);

		if (it->first >= pit && int(it->second.y_) - int(pm.ascent()) <= y) {
			pit = it->first;
			yy = it->second.y_;
		}
	}

	LYXERR(Debug::DEBUG)
		<< BOOST_CURRENT_FUNCTION
		<< ": found best y: " << yy << " for pit: " << pit
		<< endl;

	return pit;
}


Row const & Text::getRowNearY(BufferView const & bv, int y, pit_type pit) const
{
	ParagraphMetrics const & pm = bv.parMetrics(this, pit);

	int yy = bv.coordCache().get(this, pit).y_ - pm.ascent();
	BOOST_ASSERT(!pm.rows().empty());
	RowList::const_iterator rit = pm.rows().begin();
	RowList::const_iterator const rlast = boost::prior(pm.rows().end());
	for (; rit != rlast; yy += rit->height(), ++rit)
		if (yy + rit->height() > y)
			break;
	return *rit;
}


// x,y are absolute screen coordinates
// sets cursor recursively descending into nested editable insets
Inset * Text::editXY(Cursor & cur, int x, int y)
{
	if (lyxerr.debugging(Debug::WORKAREA)) {
		lyxerr << "Text::editXY(cur, " << x << ", " << y << ")" << std::endl;
		cur.bv().coordCache().dump();
	}
	pit_type pit = getPitNearY(cur.bv(), y);
	BOOST_ASSERT(pit != -1);

	Row const & row = getRowNearY(cur.bv(), y, pit);
	bool bound = false;

	TextMetrics const & tm = cur.bv().textMetrics(this);
	int xx = x; // is modified by getColumnNearX
	pos_type const pos = row.pos()
		+ tm.getColumnNearX(pit, row, xx, bound);
	cur.pit() = pit;
	cur.pos() = pos;
	cur.boundary(bound);
	cur.setTargetX(x);

	// try to descend into nested insets
	Inset * inset = checkInsetHit(cur.bv(), x, y);
	//lyxerr << "inset " << inset << " hit at x: " << x << " y: " << y << endl;
	if (!inset) {
		// Either we deconst editXY or better we move current_font
		// and real_current_font to Cursor
		setCurrentFont(cur);
		return 0;
	}

	Inset * insetBefore = pos? pars_[pit].getInset(pos - 1): 0;
	//Inset * insetBehind = pars_[pit].getInset(pos);

	// This should be just before or just behind the
	// cursor position set above.
	BOOST_ASSERT((pos != 0 && inset == insetBefore)
		|| inset == pars_[pit].getInset(pos));

	// Make sure the cursor points to the position before
	// this inset.
	if (inset == insetBefore) {
		--cur.pos();
		cur.boundary(false);
	}

	// Try to descend recursively inside the inset.
	inset = inset->editXY(cur, x, y);

	if (cur.top().text() == this)
		setCurrentFont(cur);
	return inset;
}


bool Text::checkAndActivateInset(Cursor & cur, bool front)
{
	if (cur.selection())
		return false;
	if (front && cur.pos() == cur.lastpos())
		return false;
	if (!front && cur.pos() == 0)
		return false;
	Inset * inset = front ? cur.nextInset() : cur.prevInset();
	if (!isHighlyEditableInset(inset))
		return false;
	/*
	 * Apparently, when entering an inset we are expected to be positioned
	 * *before* it in the containing paragraph, regardless of the direction
	 * from which we are entering. Otherwise, cursor placement goes awry,
	 * and when we exit from the beginning, we'll be placed *after* the
	 * inset.
	 */
	if (!front)
		--cur.pos();
	inset->edit(cur, front);
	return true;
}


bool Text::cursorLeft(Cursor & cur)
{
	// Tell BufferView to test for FitCursor in any case!
	cur.updateFlags(Update::FitCursor);

	// not at paragraph start?
	if (cur.pos() > 0) {
		// if on right side of boundary (i.e. not at paragraph end, but line end)
		// -> skip it, i.e. set boundary to true, i.e. go only logically left
		// there are some exceptions to ignore this: lineseps, newlines, spaces
#if 0
		// some effectless debug code to see the values in the debugger
		bool bound = cur.boundary();
		int rowpos = cur.textRow().pos();
		int pos = cur.pos();
		bool sep = cur.paragraph().isSeparator(cur.pos() - 1);
		bool newline = cur.paragraph().isNewline(cur.pos() - 1);
		bool linesep = cur.paragraph().isLineSeparator(cur.pos() - 1);
#endif
		if (!cur.boundary() &&
				cur.textRow().pos() == cur.pos() &&
				!cur.paragraph().isLineSeparator(cur.pos() - 1) &&
				!cur.paragraph().isNewline(cur.pos() - 1) &&
				!cur.paragraph().isSeparator(cur.pos() - 1)) {
			return setCursor(cur, cur.pit(), cur.pos(), true, true);
		}
		
		// go left and try to enter inset
		if (checkAndActivateInset(cur, false))
			return false;
		
		// normal character left
		return setCursor(cur, cur.pit(), cur.pos() - 1, true, false);
	}

	// move to the previous paragraph or do nothing
	if (cur.pit() > 0)
		return setCursor(cur, cur.pit() - 1, getPar(cur.pit() - 1).size());
	return false;
}


bool Text::cursorRight(Cursor & cur)
{
	// Tell BufferView to test for FitCursor in any case!
	cur.updateFlags(Update::FitCursor);

	// not at paragraph end?
	if (cur.pos() != cur.lastpos()) {
		// in front of editable inset, i.e. jump into it?
		if (checkAndActivateInset(cur, true))
			return false;

		// if left of boundary -> just jump to right side
	  // but for RTL boundaries don't, because: abc|DDEEFFghi -> abcDDEEF|Fghi
	  if (cur.boundary() && 
				!isRTLBoundary(cur.buffer(), cur.paragraph(), cur.pos()))
			return setCursor(cur, cur.pit(), cur.pos(), true, false);

		// next position is left of boundary, 
		// but go to next line for special cases like space, newline, linesep
#if 0
		// some effectless debug code to see the values in the debugger
		int endpos = cur.textRow().endpos();
		int lastpos = cur.lastpos();
		int pos = cur.pos();
		bool linesep = cur.paragraph().isLineSeparator(cur.pos());
		bool newline = cur.paragraph().isNewline(cur.pos());
		bool sep = cur.paragraph().isSeparator(cur.pos());
		if (cur.pos() != cur.lastpos()) {
			bool linesep2 = cur.paragraph().isLineSeparator(cur.pos()+1);
			bool newline2 = cur.paragraph().isNewline(cur.pos()+1);
			bool sep2 = cur.paragraph().isSeparator(cur.pos()+1);
		}
#endif
		if (cur.textRow().endpos() == cur.pos() + 1 &&
		    cur.textRow().endpos() != cur.lastpos() &&
				!cur.paragraph().isNewline(cur.pos()) &&
				!cur.paragraph().isLineSeparator(cur.pos()) &&
				!cur.paragraph().isSeparator(cur.pos())) {
			return setCursor(cur, cur.pit(), cur.pos() + 1, true, true);
		}
		
		// in front of RTL boundary? Stay on this side of the boundary because:
		//   ab|cDDEEFFghi -> abc|DDEEFFghi
		if (isRTLBoundary(cur.buffer(), cur.paragraph(), cur.pos() + 1))
			return setCursor(cur, cur.pit(), cur.pos() + 1, true, true);
		
		// move right
		return setCursor(cur, cur.pit(), cur.pos() + 1, true, false);
	}

	// move to next paragraph
	if (cur.pit() != cur.lastpit())
		return setCursor(cur, cur.pit() + 1, 0);
	return false;
}


bool Text::cursorUpParagraph(Cursor & cur)
{
	bool updated = false;
	if (cur.pos() > 0)
		updated = setCursor(cur, cur.pit(), 0);
	else if (cur.pit() != 0)
		updated = setCursor(cur, cur.pit() - 1, 0);
	return updated;
}


bool Text::cursorDownParagraph(Cursor & cur)
{
	bool updated = false;
	if (cur.pit() != cur.lastpit())
		updated = setCursor(cur, cur.pit() + 1, 0);
	else
		updated = setCursor(cur, cur.pit(), cur.lastpos());
	return updated;
}


// fix the cursor `cur' after a characters has been deleted at `where'
// position. Called by deleteEmptyParagraphMechanism
void Text::fixCursorAfterDelete(CursorSlice & cur, CursorSlice const & where)
{
	// Do nothing if cursor is not in the paragraph where the
	// deletion occured,
	if (cur.pit() != where.pit())
		return;

	// If cursor position is after the deletion place update it
	if (cur.pos() > where.pos())
		--cur.pos();

	// Check also if we don't want to set the cursor on a spot behind the
	// pagragraph because we erased the last character.
	if (cur.pos() > cur.lastpos())
		cur.pos() = cur.lastpos();
}


bool Text::deleteEmptyParagraphMechanism(Cursor & cur,
		Cursor & old, bool & need_anchor_change)
{
	//LYXERR(Debug::DEBUG) << "DEPM: cur:\n" << cur << "old:\n" << old << endl;

	Paragraph & oldpar = old.paragraph();

	// We allow all kinds of "mumbo-jumbo" when freespacing.
	if (oldpar.isFreeSpacing())
		return false;

	/* Ok I'll put some comments here about what is missing.
	   There are still some small problems that can lead to
	   double spaces stored in the document file or space at
	   the beginning of paragraphs(). This happens if you have
	   the cursor between to spaces and then save. Or if you
	   cut and paste and the selection have a space at the
	   beginning and then save right after the paste. (Lgb)
	*/

	// If old.pos() == 0 and old.pos()(1) == LineSeparator
	// delete the LineSeparator.
	// MISSING

	// If old.pos() == 1 and old.pos()(0) == LineSeparator
	// delete the LineSeparator.
	// MISSING

	bool const same_inset = &old.inset() == &cur.inset();
	bool const same_par = same_inset && old.pit() == cur.pit();
	bool const same_par_pos = same_par && old.pos() == cur.pos();

	// If the chars around the old cursor were spaces, delete one of them.
	if (!same_par_pos) {
		// Only if the cursor has really moved.
		if (old.pos() > 0
		    && old.pos() < oldpar.size()
		    && oldpar.isLineSeparator(old.pos())
		    && oldpar.isLineSeparator(old.pos() - 1)
		    && !oldpar.isDeleted(old.pos() - 1)) {
			oldpar.eraseChar(old.pos() - 1, cur.buffer().params().trackChanges);
#ifdef WITH_WARNINGS
#warning This will not work anymore when we have multiple views of the same buffer
// In this case, we will have to correct also the cursors held by
// other bufferviews. It will probably be easier to do that in a more
// automated way in CursorSlice code. (JMarc 26/09/2001)
#endif
			// correct all cursor parts
			if (same_par) {
				fixCursorAfterDelete(cur.top(), old.top());
				need_anchor_change = true;
			}
			return true;
		}
	}

	// only do our magic if we changed paragraph
	if (same_par)
		return false;

	// don't delete anything if this is the ONLY paragraph!
	if (old.lastpit() == 0)
		return false;

	// Do not delete empty paragraphs with keepempty set.
	if (oldpar.allowEmpty())
		return false;

	if (oldpar.empty() || (oldpar.size() == 1 && oldpar.isLineSeparator(0))) {
		// Delete old par.
		recordUndo(old, Undo::ATOMIC,
			   max(old.pit() - 1, pit_type(0)),
			   min(old.pit() + 1, old.lastpit()));
		ParagraphList & plist = old.text()->paragraphs();
		plist.erase(boost::next(plist.begin(), old.pit()));

		// see #warning above
		if (cur.depth() >= old.depth()) {
			CursorSlice & curslice = cur[old.depth() - 1];
			if (&curslice.inset() == &old.inset()
			    && curslice.pit() > old.pit()) {
				--curslice.pit();
				// since a paragraph has been deleted, all the
				// insets after `old' have been copied and
				// their address has changed. Therefore we
				// need to `regenerate' cur. (JMarc)
				cur.updateInsets(&(cur.bottom().inset()));
				need_anchor_change = true;
			}
		}
		return true;
	}

	if (oldpar.stripLeadingSpaces(cur.buffer().params().trackChanges)) {
		need_anchor_change = true;
		// We return true here because the Paragraph contents changed and
		// we need a redraw before further action is processed.
		return true;
	}

	return false;
}


void Text::deleteEmptyParagraphMechanism(pit_type first, pit_type last, bool trackChanges)
{
	BOOST_ASSERT(first >= 0 && first <= last && last < (int) pars_.size());

	for (pit_type pit = first; pit <= last; ++pit) {
		Paragraph & par = pars_[pit];

		// We allow all kinds of "mumbo-jumbo" when freespacing.
		if (par.isFreeSpacing())
			continue;

		for (pos_type pos = 1; pos < par.size(); ++pos) {
			if (par.isLineSeparator(pos) && par.isLineSeparator(pos - 1)
			    && !par.isDeleted(pos - 1)) {
				if (par.eraseChar(pos - 1, trackChanges)) {
					--pos;
				}
			}
		}

		// don't delete anything if this is the only remaining paragraph within the given range
		// note: Text::acceptOrRejectChanges() sets the cursor to 'first' after calling DEPM
		if (first == last)
			continue;

		// don't delete empty paragraphs with keepempty set
		if (par.allowEmpty())
			continue;

		if (par.empty() || (par.size() == 1 && par.isLineSeparator(0))) {
			pars_.erase(boost::next(pars_.begin(), pit));
			--pit;
			--last;
			continue;
		}

		par.stripLeadingSpaces(trackChanges);
	}
}


void Text::recUndo(Cursor & cur, pit_type first, pit_type last) const
{
	recordUndo(cur, Undo::ATOMIC, first, last);
}


void Text::recUndo(Cursor & cur, pit_type par) const
{
	recordUndo(cur, Undo::ATOMIC, par, par);
}

} // namespace lyx
