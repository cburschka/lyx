/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#include "lyxtext.h"
#include "LString.h"
#include "Lsstream.h"
#include "paragraph.h"
#include "funcrequest.h"
#include "frontends/LyXView.h"
#include "undo_funcs.h"
#include "buffer.h"
#include "buffer_funcs.h"
#include "bufferparams.h"
#include "errorlist.h"
#include "gettext.h"
#include "BufferView.h"
#include "CutAndPaste.h"
#include "frontends/Painter.h"
#include "frontends/font_metrics.h"
#include "debug.h"
#include "lyxrc.h"
#include "lyxrow.h"
#include "FloatList.h"
#include "language.h"
#include "ParagraphParameters.h"
#include "counters.h"
#include "lyxrow_funcs.h"
#include "paragraph_funcs.h"

#include "insets/insetbibitem.h"
#include "insets/insetenv.h"
#include "insets/insetfloat.h"
#include "insets/insetwrap.h"

#include "support/LAssert.h"
#include "support/textutils.h"
#include "support/lstrings.h"

#include <boost/tuple/tuple.hpp>

using namespace lyx::support;

using std::vector;
using std::copy;
using std::endl;
using std::find;
using std::pair;
using lyx::pos_type;


LyXText::LyXText(BufferView * bv)
	: height(0), width(0), anchor_row_offset_(0),
	  inset_owner(0), the_locking_inset(0), bv_owner(bv)
{
	anchor_row_ = rows().end();
	need_break_row = rows().end();
	refresh_row = rows().end();

	clearPaint();
}


LyXText::LyXText(BufferView * bv, InsetText * inset)
	: height(0), width(0), anchor_row_offset_(0),
	  inset_owner(inset), the_locking_inset(0), bv_owner(bv)
{
	anchor_row_ = rows().end();
	need_break_row = rows().end();
	refresh_row = rows().end();

	clearPaint();
}


void LyXText::init(BufferView * bview)
{
	bv_owner = bview;

	rowlist_.clear();
	need_break_row = rows().end();
	width = height = 0;
	clearPaint();

	anchor_row_ = rows().end();
	anchor_row_offset_ = 0;

	ParagraphList::iterator pit = ownerParagraphs().begin();
	ParagraphList::iterator end = ownerParagraphs().end();

	current_font = getFont(bview->buffer(), pit, 0);

	for (; pit != end; ++pit)
		insertParagraph(pit, rowlist_.end());

	setCursorIntern(rowlist_.begin()->par(), 0);
	selection.cursor = cursor;

	updateCounters();
}


// Gets the fully instantiated font at a given position in a paragraph
// Basically the same routine as Paragraph::getFont() in paragraph.C.
// The difference is that this one is used for displaying, and thus we
// are allowed to make cosmetic improvements. For instance make footnotes
// smaller. (Asger)
// If position is -1, we get the layout font of the paragraph.
// If position is -2, we get the font of the manual label of the paragraph.
LyXFont const LyXText::getFont(Buffer const * buf, ParagraphList::iterator pit,
			       pos_type pos) const
{
	Assert(pos >= 0);

	LyXLayout_ptr const & layout = pit->layout();

	// We specialize the 95% common case:
	if (!pit->getDepth()) {
		if (layout->labeltype == LABEL_MANUAL
		    && pos < pit->beginningOfBody()) {
			// 1% goes here
			LyXFont f = pit->getFontSettings(buf->params, pos);
			if (pit->inInset())
				pit->inInset()->getDrawFont(f);
			return f.realize(layout->reslabelfont);
		} else {
			LyXFont f = pit->getFontSettings(buf->params, pos);
			if (pit->inInset())
				pit->inInset()->getDrawFont(f);
			return f.realize(layout->resfont);
		}
	}

	// The uncommon case need not be optimized as much

	LyXFont layoutfont;

	if (pos < pit->beginningOfBody()) {
		// 1% goes here
		layoutfont = layout->labelfont;
	} else {
		// 99% goes here
		layoutfont = layout->font;
	}

	LyXFont tmpfont = pit->getFontSettings(buf->params, pos);
	tmpfont.realize(layoutfont);

	if (pit->inInset())
		pit->inInset()->getDrawFont(tmpfont);

	// Realize with the fonts of lesser depth.
	tmpfont.realize(outerFont(pit, ownerParagraphs()));

	return realizeFont(tmpfont, buf->params);
}


LyXFont const LyXText::getLayoutFont(Buffer const * buf,
				     ParagraphList::iterator pit) const
{
	LyXLayout_ptr const & layout = pit->layout();

	if (!pit->getDepth()) {
		return layout->resfont;
	}

	LyXFont font(layout->font);
	// Realize with the fonts of lesser depth.
	font.realize(outerFont(pit, ownerParagraphs()));

	return realizeFont(font, buf->params);
}


LyXFont const LyXText::getLabelFont(Buffer const * buf,
				    ParagraphList::iterator pit) const
{
	LyXLayout_ptr const & layout = pit->layout();

	if (!pit->getDepth()) {
		return layout->reslabelfont;
	}

	LyXFont font(layout->labelfont);
	// Realize with the fonts of lesser depth.
	font.realize(outerFont(pit, ownerParagraphs()));

	return realizeFont(layout->labelfont, buf->params);
}


void LyXText::setCharFont(ParagraphList::iterator pit,
			  pos_type pos, LyXFont const & fnt,
			  bool toggleall)
{
	Buffer const * buf = bv()->buffer();
	LyXFont font = getFont(buf, pit, pos);
	font.update(fnt, buf->params.language, toggleall);
	// Let the insets convert their font
	if (pit->isInset(pos)) {
		Inset * inset = pit->getInset(pos);
		if (isEditableInset(inset)) {
			UpdatableInset * uinset =
				static_cast<UpdatableInset *>(inset);
			uinset->setFont(bv(), fnt, toggleall, true);
		}
	}

	// Plug thru to version below:
	setCharFont(buf, pit, pos, font);
}


void LyXText::setCharFont(Buffer const * buf, ParagraphList::iterator pit,
			  pos_type pos, LyXFont const & fnt)
{
	LyXFont font(fnt);

	LyXTextClass const & tclass = buf->params.getLyXTextClass();
	LyXLayout_ptr const & layout = pit->layout();

	// Get concrete layout font to reduce against
	LyXFont layoutfont;

	if (pos < pit->beginningOfBody())
		layoutfont = layout->labelfont;
	else
		layoutfont = layout->font;

	// Realize against environment font information
	if (pit->getDepth()) {
		ParagraphList::iterator tp = pit;
		while (!layoutfont.resolved() &&
		       tp != ownerParagraphs().end() &&
		       tp->getDepth()) {
			tp = outerHook(tp, ownerParagraphs());
			if (tp != ownerParagraphs().end())
				layoutfont.realize(tp->layout()->font);
		}
	}

	layoutfont.realize(tclass.defaultfont());

	// Now, reduce font against full layout font
	font.reduce(layoutfont);

	pit->setFont(pos, font);
}


// removes the row and reset the touched counters
void LyXText::removeRow(RowList::iterator rit)
{
	/* FIXME: when we cache the bview, this should just
	 * become a postPaint(), I think */
	if (refresh_row == rit) {
		if (rit == rows().begin())
			refresh_row = boost::next(rit);
		else
			refresh_row = boost::prior(rit);

		// what about refresh_y
	}

	if (anchor_row_ == rit) {
		if (rit != rows().begin()) {
			anchor_row_ = boost::prior(rit);
			anchor_row_offset_ += anchor_row_->height();
		} else {
			anchor_row_ = boost::next(rit);
			anchor_row_offset_ -= rit->height();
		}
	}

	// the text becomes smaller
	height -= rit->height();

	rowlist_.erase(rit);
}


// remove all following rows of the paragraph of the specified row.
void LyXText::removeParagraph(RowList::iterator rit)
{
	ParagraphList::iterator tmppit = rit->par();
	++rit;

	while (rit != rows().end() && rit->par() == tmppit) {
		RowList::iterator tmprit = boost::next(rit);
		removeRow(rit);
		rit = tmprit;
	}
}


void LyXText::insertParagraph(ParagraphList::iterator pit,
			      RowList::iterator rowit)
{
	// insert a new row, starting at position 0
	Row newrow(pit, 0);
	RowList::iterator rit = rowlist_.insert(rowit, newrow);

	// and now append the whole paragraph before the new row
	appendParagraph(rit);
}


Inset * LyXText::getInset() const
{
	ParagraphList::iterator pit = cursor.par();
	pos_type const pos = cursor.pos();

	if (pos < pit->size() && pit->isInset(pos)) {
		return pit->getInset(pos);
	}
	return 0;
}


void LyXText::toggleInset()
{
	Inset * inset = getInset();
	// is there an editable inset at cursor position?
	if (!isEditableInset(inset)) {
		// No, try to see if we are inside a collapsable inset
		if (inset_owner && inset_owner->owner()
		    && inset_owner->owner()->isOpen()) {
			bv()->unlockInset(inset_owner->owner());
			inset_owner->owner()->close(bv());
			bv()->getLyXText()->cursorRight(bv());
		}
		return;
	}
	//bv()->owner()->message(inset->editMessage());

	// do we want to keep this?? (JMarc)
	if (!isHighlyEditableInset(inset))
		setCursorParUndo(bv());

	if (inset->isOpen()) {
		inset->close(bv());
	} else {
		inset->open(bv());
	}

	bv()->updateInset(inset);
}


/* used in setlayout */
// Asger is not sure we want to do this...
void LyXText::makeFontEntriesLayoutSpecific(Buffer const & buf,
					    Paragraph & par)
{
	LyXLayout_ptr const & layout = par.layout();
	pos_type const psize = par.size();

	LyXFont layoutfont;
	for (pos_type pos = 0; pos < psize; ++pos) {
		if (pos < par.beginningOfBody())
			layoutfont = layout->labelfont;
		else
			layoutfont = layout->font;

		LyXFont tmpfont = par.getFontSettings(buf.params, pos);
		tmpfont.reduce(layoutfont);
		par.setFont(pos, tmpfont);
	}
}


ParagraphList::iterator
LyXText::setLayout(LyXCursor & cur, LyXCursor & sstart_cur,
		   LyXCursor & send_cur,
		   string const & layout)
{
	ParagraphList::iterator endpit = boost::next(send_cur.par());
	ParagraphList::iterator undoendpit = endpit;
	ParagraphList::iterator pars_end = ownerParagraphs().end();

	if (endpit != pars_end && endpit->getDepth()) {
		while (endpit != pars_end && endpit->getDepth()) {
			++endpit;
			undoendpit = endpit;
		}
	} else if (endpit != pars_end) {
		// because of parindents etc.
		++endpit;
	}

	setUndo(bv(), Undo::EDIT, sstart_cur.par(), boost::prior(undoendpit));

	// ok we have a selection. This is always between sstart_cur
	// and sel_end cursor
	cur = sstart_cur;
	ParagraphList::iterator pit = sstart_cur.par();
	ParagraphList::iterator epit = boost::next(send_cur.par());

	LyXLayout_ptr const & lyxlayout =
		bv()->buffer()->params.getLyXTextClass()[layout];

	do {
		pit->applyLayout(lyxlayout);
		makeFontEntriesLayoutSpecific(*bv()->buffer(), *pit);
		ParagraphList::iterator fppit = pit;
		fppit->params().spaceTop(lyxlayout->fill_top ?
					 VSpace(VSpace::VFILL)
					 : VSpace(VSpace::NONE));
		fppit->params().spaceBottom(lyxlayout->fill_bottom ?
					    VSpace(VSpace::VFILL)
					    : VSpace(VSpace::NONE));
		if (lyxlayout->margintype == MARGIN_MANUAL)
			pit->setLabelWidthString(lyxlayout->labelstring());
		cur.par(pit);
		++pit;
	} while (pit != epit);

	return endpit;
}


// set layout over selection and make a total rebreak of those paragraphs
void LyXText::setLayout(string const & layout)
{
	LyXCursor tmpcursor = cursor;  // store the current cursor

	// if there is no selection just set the layout
	// of the current paragraph
	if (!selection.set()) {
		selection.start = cursor;  // dummy selection
		selection.end = cursor;
	}

	// special handling of new environment insets
	BufferParams const & params = bv()->buffer()->params;
	LyXLayout_ptr const & lyxlayout = params.getLyXTextClass()[layout];
	if (lyxlayout->is_environment) {
		// move everything in a new environment inset
		lyxerr << "setting layout " << layout << endl;
		bv()->owner()->dispatch(FuncRequest(LFUN_HOME));
		bv()->owner()->dispatch(FuncRequest(LFUN_ENDSEL));
		bv()->owner()->dispatch(FuncRequest(LFUN_CUT));
		Inset * inset = new InsetEnvironment(params, layout);
		if (bv()->insertInset(inset)) {
			//inset->edit(bv());
			//bv()->owner()->dispatch(FuncRequest(LFUN_PASTE));
		}
		else
			delete inset;
		return;
	}

	ParagraphList::iterator endpit = setLayout(cursor, selection.start,
						   selection.end, layout);
	redoParagraphs(selection.start, endpit);

	// we have to reset the selection, because the
	// geometry could have changed
	setCursor(selection.start.par(), selection.start.pos(), false);
	selection.cursor = cursor;
	setCursor(selection.end.par(), selection.end.pos(), false);
	updateCounters();
	clearSelection();
	setSelection();
	setCursor(tmpcursor.par(), tmpcursor.pos(), true);
}


bool LyXText::changeDepth(bv_funcs::DEPTH_CHANGE type, bool test_only)
{
	ParagraphList::iterator pit(cursor.par());
	ParagraphList::iterator end(cursor.par());
	ParagraphList::iterator start = pit;

	if (selection.set()) {
		pit = selection.start.par();
		end = selection.end.par();
		start = pit;
	}

	ParagraphList::iterator pastend = boost::next(end);

	if (!test_only)
		setUndo(bv(), Undo::EDIT, start, end);

	bool changed = false;

	int prev_after_depth = 0;
#warning parlist ... could be nicer ?
	if (start != ownerParagraphs().begin()) {
		prev_after_depth = boost::prior(start)->getMaxDepthAfter();
	}

	while (true) {
		int const depth = pit->params().depth();
		if (type == bv_funcs::INC_DEPTH) {
			if (depth < prev_after_depth
			    && pit->layout()->labeltype != LABEL_BIBLIO) {
				changed = true;
				if (!test_only) {
					pit->params().depth(depth + 1);
				}

			}
		} else if (depth) {
			changed = true;
			if (!test_only)
				pit->params().depth(depth - 1);
		}

		prev_after_depth = pit->getMaxDepthAfter();

		if (pit == end) {
			break;
		}

		++pit;
	}

	if (test_only)
		return changed;

	// Wow, redoParagraphs is stupid.
	LyXCursor tmpcursor;
	setCursor(tmpcursor, start, 0);

	redoParagraphs(tmpcursor, pastend);

	// We need to actually move the text->cursor. I don't
	// understand why ...
	tmpcursor = cursor;

	// we have to reset the visual selection because the
	// geometry could have changed
	if (selection.set()) {
		setCursor(selection.start.par(), selection.start.pos());
		selection.cursor = cursor;
		setCursor(selection.end.par(), selection.end.pos());
	}

	// this handles the counter labels, and also fixes up
	// depth values for follow-on (child) paragraphs
	updateCounters();

	setSelection();
	setCursor(tmpcursor.par(), tmpcursor.pos());

	return changed;
}


// set font over selection and make a total rebreak of those paragraphs
void LyXText::setFont(LyXFont const & font, bool toggleall)
{
	// if there is no selection just set the current_font
	if (!selection.set()) {
		// Determine basis font
		LyXFont layoutfont;
		if (cursor.pos() < cursor.par()->beginningOfBody()) {
			layoutfont = getLabelFont(bv()->buffer(),
						  cursor.par());
		} else {
			layoutfont = getLayoutFont(bv()->buffer(),
						   cursor.par());
		}
		// Update current font
		real_current_font.update(font,
					 bv()->buffer()->params.language,
					 toggleall);

		// Reduce to implicit settings
		current_font = real_current_font;
		current_font.reduce(layoutfont);
		// And resolve it completely
		real_current_font.realize(layoutfont);

		return;
	}

	LyXCursor tmpcursor = cursor; // store the current cursor

	// ok we have a selection. This is always between sel_start_cursor
	// and sel_end cursor

	setUndo(bv(), Undo::EDIT, selection.start.par(), selection.end.par());
	freezeUndo();
	cursor = selection.start;
	while (cursor.par() != selection.end.par() ||
	       cursor.pos() < selection.end.pos())
	{
		if (cursor.pos() < cursor.par()->size()) {
			// an open footnote should behave like a closed one
			setCharFont(cursor.par(), cursor.pos(),
				    font, toggleall);
			cursor.pos(cursor.pos() + 1);
		} else {
			cursor.pos(0);
			cursor.par(boost::next(cursor.par()));
		}
	}
	unFreezeUndo();

	redoParagraphs(selection.start, boost::next(selection.end.par()));

	// we have to reset the selection, because the
	// geometry could have changed, but we keep
	// it for user convenience
	setCursor(selection.start.par(), selection.start.pos());
	selection.cursor = cursor;
	setCursor(selection.end.par(), selection.end.pos());
	setSelection();
	setCursor(tmpcursor.par(), tmpcursor.pos(), true,
		  tmpcursor.boundary());
}


void LyXText::redoHeightOfParagraph()
{
	RowList::iterator tmprow = cursorRow();
	int y = cursor.y() - tmprow->baseline();

	setHeightOfRow(tmprow);

	while (tmprow != rows().begin()
	       && boost::prior(tmprow)->par() == tmprow->par()) {
		--tmprow;
		y -= tmprow->height();
		setHeightOfRow(tmprow);
	}

	postPaint(y);

	setCursor(cursor.par(), cursor.pos(), false, cursor.boundary());
}


void LyXText::redoDrawingOfParagraph(LyXCursor const & cur)
{
	RowList::iterator tmprow = getRow(cur);

	int y = cur.y() - tmprow->baseline();
	setHeightOfRow(tmprow);

	while (tmprow != rows().begin()
	       && boost::prior(tmprow)->par() == tmprow->par())  {
		--tmprow;
		y -= tmprow->height();
	}

	postPaint(y);
	setCursor(cur.par(), cur.pos());
}


// deletes and inserts again all paragraphs between the cursor
// and the specified par
// This function is needed after SetLayout and SetFont etc.
void LyXText::redoParagraphs(LyXCursor const & cur,
			     ParagraphList::iterator endpit)
{
	RowList::iterator tmprit = getRow(cur);
	int y = cur.y() - tmprit->baseline();

	ParagraphList::iterator first_phys_pit;
	RowList::iterator prevrit;
	if (tmprit == rows().begin()) {
		// A trick/hack for UNDO.
		// This is needed because in an UNDO/REDO we could have
		// changed the ownerParagraph() so the paragraph inside
		// the row is NOT my really first par anymore.
		// Got it Lars ;) (Jug 20011206)
		first_phys_pit = ownerParagraphs().begin();
		prevrit = rows().end();
	} else {
		first_phys_pit = tmprit->par();
		while (tmprit != rows().begin()
		       && boost::prior(tmprit)->par() == first_phys_pit)
		{
			--tmprit;
			y -= tmprit->height();
		}
		prevrit = boost::prior(tmprit);
	}

	// remove it
	while (tmprit != rows().end() && tmprit->par() != endpit) {
		RowList::iterator tmprit2 = tmprit++;
		removeRow(tmprit2);
	}

	// Reinsert the paragraphs.
	ParagraphList::iterator tmppit = first_phys_pit;

	while (tmppit != ownerParagraphs().end()) {
		insertParagraph(tmppit, tmprit);
		while (tmprit != rows().end()
		       && tmprit->par() == tmppit) {
			++tmprit;
		}
		++tmppit;
		if (tmppit == endpit)
			break;
	}
	if (prevrit != rows().end()) {
		setHeightOfRow(prevrit);
		postPaint(y - prevrit->height());
	} else {
		setHeightOfRow(rows().begin());
		postPaint(0);
	}
	if (tmprit != rows().end())
		setHeightOfRow(tmprit);

	updateCounters();
}


void LyXText::fullRebreak()
{
	init(bv());
	setCursorIntern(cursor.par(), cursor.pos());
}


void LyXText::partialRebreak()
{
	if (rows().empty()) {
		init(bv());
		return;
	}

	RowList::iterator rows_end = rows().end();

	if (need_break_row != rows_end) {
		breakAgain(need_break_row);
		need_break_row = rows_end;
		return;
	}
}


// important for the screen


// the cursor set functions have a special mechanism. When they
// realize, that you left an empty paragraph, they will delete it.
// They also delete the corresponding row

// need the selection cursor:
void LyXText::setSelection()
{
	bool const lsel = TextCursor::setSelection();

	if (inset_owner && (selection.set() || lsel))
		inset_owner->setUpdateStatus(InsetText::SELECTION);
}



void LyXText::clearSelection()
{
	TextCursor::clearSelection();

	// reset this in the bv_owner!
	if (bv_owner && bv_owner->text)
		bv_owner->text->xsel_cache.set(false);
}


void LyXText::cursorHome()
{
	setCursor(cursor.par(), cursorRow()->pos());
}


void LyXText::cursorEnd()
{
	if (cursor.par()->empty())
		return;

	RowList::iterator rit = cursorRow();
	RowList::iterator next_rit = boost::next(rit);
	ParagraphList::iterator pit = rit->par();
	pos_type last_pos = lastPos(*this, rit);

	if (next_rit == rows().end() || next_rit->par() != pit) {
		++last_pos;
	} else {
		if (pit->empty() ||
		    (pit->getChar(last_pos) != ' ' && !pit->isNewline(last_pos))) {
			++last_pos;
		}
	}

	setCursor(pit, last_pos);
}


void LyXText::cursorTop()
{
	setCursor(ownerParagraphs().begin(), 0);
}


void LyXText::cursorBottom()
{
	ParagraphList::iterator lastpit =
		boost::prior(ownerParagraphs().end());
	setCursor(lastpit, lastpit->size());
}


void LyXText::toggleFree(LyXFont const & font, bool toggleall)
{
	// If the mask is completely neutral, tell user
	if (font == LyXFont(LyXFont::ALL_IGNORE)) {
		// Could only happen with user style
		bv()->owner()->message(_("No font change defined. Use Character under the Layout menu to define font change."));
		return;
	}

	// Try implicit word selection
	// If there is a change in the language the implicit word selection
	// is disabled.
	LyXCursor resetCursor = cursor;
	bool implicitSelection = (font.language() == ignore_language
				  && font.number() == LyXFont::IGNORE)
		? selectWordWhenUnderCursor(lyx::WHOLE_WORD_STRICT) : false;

	// Set font
	setFont(font, toggleall);

	// Implicit selections are cleared afterwards
	//and cursor is set to the original position.
	if (implicitSelection) {
		clearSelection();
		cursor = resetCursor;
		setCursor(cursor.par(), cursor.pos());
		selection.cursor = cursor;
	}
	if (inset_owner)
		inset_owner->setUpdateStatus(InsetText::CURSOR_PAR);
}


string LyXText::getStringToIndex()
{
	// Try implicit word selection
	// If there is a change in the language the implicit word selection
	// is disabled.
	LyXCursor const reset_cursor = cursor;
	bool const implicitSelection =
		selectWordWhenUnderCursor(lyx::PREVIOUS_WORD);

	string idxstring;
	if (!selection.set())
		bv()->owner()->message(_("Nothing to index!"));
	else if (selection.start.par() != selection.end.par())
		bv()->owner()->message(_("Cannot index more than one paragraph!"));
	else
		idxstring = selectionAsString(bv()->buffer(), false);

	// Reset cursors to their original position.
	cursor = reset_cursor;
	setCursor(cursor.par(), cursor.pos());
	selection.cursor = cursor;

	// Clear the implicit selection.
	if (implicitSelection)
		clearSelection();

	return idxstring;
}


// the DTP switches for paragraphs. LyX will store them in the first
// physicla paragraph. When a paragraph is broken, the top settings rest,
// the bottom settings are given to the new one. So I can make shure,
// they do not duplicate themself and you cannnot make dirty things with
// them!

void LyXText::setParagraph(bool line_top, bool line_bottom,
			   bool pagebreak_top, bool pagebreak_bottom,
			   VSpace const & space_top,
			   VSpace const & space_bottom,
			   Spacing const & spacing,
			   LyXAlignment align,
			   string const & labelwidthstring,
			   bool noindent)
{
	LyXCursor tmpcursor = cursor;
	if (!selection.set()) {
		selection.start = cursor;
		selection.end = cursor;
	}

	// make sure that the depth behind the selection are restored, too
	ParagraphList::iterator endpit = boost::next(selection.end.par());
	ParagraphList::iterator undoendpit = endpit;
	ParagraphList::iterator pars_end = ownerParagraphs().end();

	if (endpit != pars_end && endpit->getDepth()) {
		while (endpit != pars_end && endpit->getDepth()) {
			++endpit;
			undoendpit = endpit;
		}
	} else if (endpit != pars_end) {
		// because of parindents etc.
		++endpit;
	}

	setUndo(bv(), Undo::EDIT, selection.start.par(),
		boost::prior(undoendpit));


	ParagraphList::iterator tmppit = selection.end.par();

	while (tmppit != boost::prior(selection.start.par())) {
		setCursor(tmppit, 0);
		postPaint(cursor.y() - cursorRow()->baseline());

		ParagraphList::iterator pit = cursor.par();
		ParagraphParameters & params = pit->params();

		params.lineTop(line_top);
		params.lineBottom(line_bottom);
		params.pagebreakTop(pagebreak_top);
		params.pagebreakBottom(pagebreak_bottom);
		params.spaceTop(space_top);
		params.spaceBottom(space_bottom);
		params.spacing(spacing);
		// does the layout allow the new alignment?
		LyXLayout_ptr const & layout = pit->layout();

		if (align == LYX_ALIGN_LAYOUT)
			align = layout->align;
		if (align & layout->alignpossible) {
			if (align == layout->align)
				params.align(LYX_ALIGN_LAYOUT);
			else
				params.align(align);
		}
		pit->setLabelWidthString(labelwidthstring);
		params.noindent(noindent);
		tmppit = boost::prior(pit);
	}

	redoParagraphs(selection.start, endpit);

	clearSelection();
	setCursor(selection.start.par(), selection.start.pos());
	selection.cursor = cursor;
	setCursor(selection.end.par(), selection.end.pos());
	setSelection();
	setCursor(tmpcursor.par(), tmpcursor.pos());
	if (inset_owner)
		bv()->updateInset(inset_owner);
}


// set the counter of a paragraph. This includes the labels
void LyXText::setCounter(Buffer const * buf, ParagraphList::iterator pit)
{
	LyXTextClass const & textclass = buf->params.getLyXTextClass();
	LyXLayout_ptr const & layout = pit->layout();

	if (pit != ownerParagraphs().begin()) {

		pit->params().appendix(boost::prior(pit)->params().appendix());
		if (!pit->params().appendix() &&
		    pit->params().startOfAppendix()) {
			pit->params().appendix(true);
			textclass.counters().reset();
		}
		pit->enumdepth = boost::prior(pit)->enumdepth;
		pit->itemdepth = boost::prior(pit)->itemdepth;
	} else {
		pit->params().appendix(pit->params().startOfAppendix());
		pit->enumdepth = 0;
		pit->itemdepth = 0;
	}

	/* Maybe we have to increment the enumeration depth.
	 * BUT, enumeration in a footnote is considered in isolation from its
	 *	surrounding paragraph so don't increment if this is the
	 *	first line of the footnote
	 * AND, bibliographies can't have their depth changed ie. they
	 *	are always of depth 0
	 */
	if (pit != ownerParagraphs().begin()
	    && boost::prior(pit)->getDepth() < pit->getDepth()
	    && boost::prior(pit)->layout()->labeltype == LABEL_COUNTER_ENUMI
	    && pit->enumdepth < 3
	    && layout->labeltype != LABEL_BIBLIO) {
		pit->enumdepth++;
	}

	// Maybe we have to decrement the enumeration depth, see note above
	if (pit != ownerParagraphs().begin()
	    && boost::prior(pit)->getDepth() > pit->getDepth()
	    && layout->labeltype != LABEL_BIBLIO) {
		pit->enumdepth = depthHook(pit, ownerParagraphs(),
					   pit->getDepth())->enumdepth;
	}

	if (!pit->params().labelString().empty()) {
		pit->params().labelString(string());
	}

	if (layout->margintype == MARGIN_MANUAL) {
		if (pit->params().labelWidthString().empty()) {
			pit->setLabelWidthString(layout->labelstring());
		}
	} else {
		pit->setLabelWidthString(string());
	}

	// is it a layout that has an automatic label?
	if (layout->labeltype >= LABEL_COUNTER_CHAPTER) {
		int const i = layout->labeltype - LABEL_COUNTER_CHAPTER;

		ostringstream s;

		if (i >= 0 && i <= buf->params.secnumdepth) {
			string numbertype;
			string langtype;

			textclass.counters().step(layout->latexname());

			// Is there a label? Useful for Chapter layout
			if (!pit->params().appendix()) {
				s << buf->B_(layout->labelstring());
			} else {
				s << buf->B_(layout->labelstring_appendix());
			}

			// Use of an integer is here less than elegant. For now.
			int head = textclass.maxcounter() - LABEL_COUNTER_CHAPTER;
			if (!pit->params().appendix()) {
				numbertype = "sectioning";
			} else {
				numbertype = "appendix";
				if (pit->isRightToLeftPar(buf->params))
					langtype = "hebrew";
				else
					langtype = "latin";
			}

			s << " "
			  << textclass.counters()
				.numberLabel(layout->latexname(),
					     numbertype, langtype, head);

			pit->params().labelString(STRCONV(s.str()));

			// reset enum counters
			textclass.counters().reset("enum");
		} else if (layout->labeltype < LABEL_COUNTER_ENUMI) {
			textclass.counters().reset("enum");
		} else if (layout->labeltype == LABEL_COUNTER_ENUMI) {
			// FIXME
			// Yes I know this is a really, really! bad solution
			// (Lgb)
			string enumcounter("enum");

			switch (pit->enumdepth) {
			case 2:
				enumcounter += 'i';
			case 1:
				enumcounter += 'i';
			case 0:
				enumcounter += 'i';
				break;
			case 3:
				enumcounter += "iv";
				break;
			default:
				// not a valid enumdepth...
				break;
			}

			textclass.counters().step(enumcounter);

			s << textclass.counters()
				.numberLabel(enumcounter, "enumeration");
			pit->params().labelString(STRCONV(s.str()));
		}
	} else if (layout->labeltype == LABEL_BIBLIO) {// ale970302
		textclass.counters().step("bibitem");
		int number = textclass.counters().value("bibitem");
		if (pit->bibitem()) {
			pit->bibitem()->setCounter(number);
			pit->params().labelString(layout->labelstring());
		}
		// In biblio should't be following counters but...
	} else {
		string s = buf->B_(layout->labelstring());

		// the caption hack:
		if (layout->labeltype == LABEL_SENSITIVE) {
			ParagraphList::iterator tmppit = pit;
			Inset * in = 0;
			bool isOK = false;
			while (tmppit != ownerParagraphs().end() &&
			       tmppit->inInset()
			       // the single '=' is intended below
			       && (in = tmppit->inInset()->owner())) {
				if (in->lyxCode() == Inset::FLOAT_CODE ||
				    in->lyxCode() == Inset::WRAP_CODE) {
					isOK = true;
					break;
				} else {
					tmppit = std::find(ownerParagraphs().begin(), ownerParagraphs().end(), *in->parOwner());
				}
			}

			if (isOK) {
				string type;

				if (in->lyxCode() == Inset::FLOAT_CODE)
					type = static_cast<InsetFloat*>(in)->params().type;
				else if (in->lyxCode() == Inset::WRAP_CODE)
					type = static_cast<InsetWrap*>(in)->params().type;
				else
					Assert(0);

				Floating const & fl = textclass.floats().getType(type);

				textclass.counters().step(fl.type());

				// Doesn't work... yet.
				s = bformat(_("%1$s #:"), buf->B_(fl.name()));
			} else {
				// par->SetLayout(0);
				// s = layout->labelstring;
				s = _("Senseless: ");
			}
		}
		pit->params().labelString(s);

		// reset the enumeration counter. They are always reset
		// when there is any other layout between
		// Just fall-through between the cases so that all
		// enum counters deeper than enumdepth is also reset.
		switch (pit->enumdepth) {
		case 0:
			textclass.counters().reset("enumi");
		case 1:
			textclass.counters().reset("enumii");
		case 2:
			textclass.counters().reset("enumiii");
		case 3:
			textclass.counters().reset("enumiv");
		}
	}
}


// Updates all counters. Paragraphs with changed label string will be rebroken
void LyXText::updateCounters()
{
	RowList::iterator rowit = rows().begin();
	ParagraphList::iterator pit = rowit->par();

	// CHECK if this is really needed. (Lgb)
	bv()->buffer()->params.getLyXTextClass().counters().reset();

	ParagraphList::iterator beg = ownerParagraphs().begin();
	ParagraphList::iterator end = ownerParagraphs().end();
	for (; pit != end; ++pit) {
		while (rowit->par() != pit)
			++rowit;

		string const oldLabel = pit->params().labelString();

		size_t maxdepth = 0;
		if (pit != beg)
			maxdepth = boost::prior(pit)->getMaxDepthAfter();

		if (pit->params().depth() > maxdepth)
			pit->params().depth(maxdepth);

		// setCounter can potentially change the labelString.
		setCounter(bv()->buffer(), pit);

		string const & newLabel = pit->params().labelString();

		if (oldLabel.empty() && !newLabel.empty()) {
			removeParagraph(rowit);
			appendParagraph(rowit);
		}
	}
}


void LyXText::insertInset(Inset * inset)
{
	if (!cursor.par()->insetAllowed(inset->lyxCode()))
		return;
	setUndo(bv(), Undo::FINISH, cursor.par());
	freezeUndo();
	cursor.par()->insertInset(cursor.pos(), inset);
	// Just to rebreak and refresh correctly.
	// The character will not be inserted a second time
	insertChar(Paragraph::META_INSET);
	// If we enter a highly editable inset the cursor should be to before
	// the inset. This couldn't happen before as Undo was not handled inside
	// inset now after the Undo LyX tries to call inset->Edit(...) again
	// and cannot do this as the cursor is behind the inset and GetInset
	// does not return the inset!
	if (isHighlyEditableInset(inset)) {
		cursorLeft(true);
	}
	unFreezeUndo();
}


void LyXText::cutSelection(bool doclear, bool realcut)
{
	// Stuff what we got on the clipboard. Even if there is no selection.

	// There is a problem with having the stuffing here in that the
	// larger the selection the slower LyX will get. This can be
	// solved by running the line below only when the selection has
	// finished. The solution used currently just works, to make it
	// faster we need to be more clever and probably also have more
	// calls to stuffClipboard. (Lgb)
	bv()->stuffClipboard(selectionAsString(bv()->buffer(), true));

	// This doesn't make sense, if there is no selection
	if (!selection.set())
		return;

	// OK, we have a selection. This is always between selection.start
	// and selection.end

	// make sure that the depth behind the selection are restored, too
	ParagraphList::iterator endpit = boost::next(selection.end.par());
	ParagraphList::iterator undoendpit = endpit;
	ParagraphList::iterator pars_end = ownerParagraphs().end();

	if (endpit != pars_end && endpit->getDepth()) {
		while (endpit != pars_end && endpit->getDepth()) {
			++endpit;
			undoendpit = endpit;
		}
	} else if (endpit != pars_end) {
		// because of parindents etc.
		++endpit;
	}

	setUndo(bv(), Undo::DELETE, selection.start.par(),
		boost::prior(undoendpit));


	endpit = selection.end.par();
	int endpos = selection.end.pos();

	boost::tie(endpit, endpos) = realcut ?
		CutAndPaste::cutSelection(bv()->buffer()->params,
					  ownerParagraphs(),
					  selection.start.par(), endpit,
					  selection.start.pos(), endpos,
					  bv()->buffer()->params.textclass,
					  doclear)
		: CutAndPaste::eraseSelection(bv()->buffer()->params,
					      ownerParagraphs(),
					      selection.start.par(), endpit,
					      selection.start.pos(), endpos,
					      doclear);
	// sometimes necessary
	if (doclear)
		selection.start.par()->stripLeadingSpaces();

	redoParagraphs(selection.start, boost::next(endpit));
#warning FIXME latent bug
	// endpit will be invalidated on redoParagraphs once ParagraphList
	// becomes a std::list? There are maybe other places on which this
	// can happend? (Ab)
	// cutSelection can invalidate the cursor so we need to set
	// it anew. (Lgb)
	// we prefer the end for when tracking changes
	cursor.pos(endpos);
	cursor.par(endpit);

	// need a valid cursor. (Lgb)
	clearSelection();

	setCursor(cursor.par(), cursor.pos());
	selection.cursor = cursor;
	updateCounters();
}


void LyXText::copySelection()
{
	// stuff the selection onto the X clipboard, from an explicit copy request
	bv()->stuffClipboard(selectionAsString(bv()->buffer(), true));

	// this doesnt make sense, if there is no selection
	if (!selection.set())
		return;

	// ok we have a selection. This is always between selection.start
	// and sel_end cursor

	// copy behind a space if there is one
	while (selection.start.par()->size() > selection.start.pos()
	       && selection.start.par()->isLineSeparator(selection.start.pos())
	       && (selection.start.par() != selection.end.par()
		   || selection.start.pos() < selection.end.pos()))
		selection.start.pos(selection.start.pos() + 1);

	CutAndPaste::copySelection(selection.start.par(),
				   selection.end.par(),
				   selection.start.pos(), selection.end.pos(),
				   bv()->buffer()->params.textclass);
}


void LyXText::pasteSelection(size_t sel_index)
{
	// this does not make sense, if there is nothing to paste
	if (!CutAndPaste::checkPastePossible())
		return;

	setUndo(bv(), Undo::INSERT, cursor.par());

	ParagraphList::iterator endpit;
	PitPosPair ppp;

	ErrorList el;

	boost::tie(ppp, endpit) =
		CutAndPaste::pasteSelection(*bv()->buffer(),
					    ownerParagraphs(),
					    cursor.par(), cursor.pos(),
					    bv()->buffer()->params.textclass,
					    sel_index, el);
	bufferErrors(*bv()->buffer(), el);
	bv()->showErrorList(_("Paste"));

	redoParagraphs(cursor, endpit);

	setCursor(cursor.par(), cursor.pos());
	clearSelection();

	selection.cursor = cursor;
	setCursor(ppp.first, ppp.second);
	setSelection();
	updateCounters();
}


void LyXText::setSelectionRange(lyx::pos_type length)
{
	if (!length)
		return;

	selection.cursor = cursor;
	while (length--)
		cursorRight(bv());
	setSelection();
}


// simple replacing. The font of the first selected character is used
void LyXText::replaceSelectionWithString(string const & str)
{
	setCursorParUndo(bv());
	freezeUndo();

	if (!selection.set()) { // create a dummy selection
		selection.end = cursor;
		selection.start = cursor;
	}

	// Get font setting before we cut
	pos_type pos = selection.end.pos();
	LyXFont const font = selection.start.par()
		->getFontSettings(bv()->buffer()->params,
				  selection.start.pos());

	// Insert the new string
	string::const_iterator cit = str.begin();
	string::const_iterator end = str.end();
	for (; cit != end; ++cit) {
		selection.end.par()->insertChar(pos, (*cit), font);
		++pos;
	}

	// Cut the selection
	cutSelection(true, false);

	unFreezeUndo();
}


// needed to insert the selection
void LyXText::insertStringAsLines(string const & str)
{
	ParagraphList::iterator pit = cursor.par();
	pos_type pos = cursor.pos();
	ParagraphList::iterator endpit = boost::next(cursor.par());

	setCursorParUndo(bv());

	// only to be sure, should not be neccessary
	clearSelection();

	bv()->buffer()->insertStringAsLines(pit, pos, current_font, str);

	redoParagraphs(cursor, endpit);
	setCursor(cursor.par(), cursor.pos());
	selection.cursor = cursor;
	setCursor(pit, pos);
	setSelection();
}


// turns double-CR to single CR, others where converted into one
// blank. Then InsertStringAsLines is called
void LyXText::insertStringAsParagraphs(string const & str)
{
	string linestr(str);
	bool newline_inserted = false;
	string::size_type const siz = linestr.length();

	for (string::size_type i = 0; i < siz; ++i) {
		if (linestr[i] == '\n') {
			if (newline_inserted) {
				// we know that \r will be ignored by
				// InsertStringA. Of course, it is a dirty
				// trick, but it works...
				linestr[i - 1] = '\r';
				linestr[i] = '\n';
			} else {
				linestr[i] = ' ';
				newline_inserted = true;
			}
		} else if (IsPrintable(linestr[i])) {
			newline_inserted = false;
		}
	}
	insertStringAsLines(linestr);
}


void LyXText::checkParagraph(ParagraphList::iterator pit, pos_type pos)
{
	LyXCursor tmpcursor;

	int y = 0;
	pos_type z;
	RowList::iterator row = getRow(pit, pos, y);
	RowList::iterator beg = rows().begin();

	// is there a break one row above
	if (row != beg
	    && boost::prior(row)->par() == row->par()) {
		z = rowBreakPoint(*boost::prior(row));
		if (z >= row->pos()) {
			// set the dimensions of the row above
			y -= boost::prior(row)->height();
			postPaint(y);

			breakAgain(boost::prior(row));

			// set the cursor again. Otherwise
			// dangling pointers are possible
			setCursor(cursor.par(), cursor.pos(),
				  false, cursor.boundary());
			selection.cursor = cursor;
			return;
		}
	}

	int const tmpheight = row->height();
	pos_type const tmplast = lastPos(*this, row);

	breakAgain(row);
	if (row->height() == tmpheight && lastPos(*this, row) == tmplast) {
		postRowPaint(row, y);
	} else {
		postPaint(y);
	}

	// check the special right address boxes
	if (pit->layout()->margintype == MARGIN_RIGHT_ADDRESS_BOX) {
		tmpcursor.par(pit);
		tmpcursor.y(y);
		tmpcursor.x(0);
		tmpcursor.x_fix(0);
		tmpcursor.pos(pos);
		redoDrawingOfParagraph(tmpcursor);
	}

	// set the cursor again. Otherwise dangling pointers are possible
	// also set the selection

	if (selection.set()) {
		tmpcursor = cursor;
		setCursorIntern(selection.cursor.par(), selection.cursor.pos(),
				false, selection.cursor.boundary());
		selection.cursor = cursor;
		setCursorIntern(selection.start.par(),
				selection.start.pos(),
				false, selection.start.boundary());
		selection.start = cursor;
		setCursorIntern(selection.end.par(),
				selection.end.pos(),
				false, selection.end.boundary());
		selection.end = cursor;
		setCursorIntern(last_sel_cursor.par(),
				last_sel_cursor.pos(),
				false, last_sel_cursor.boundary());
		last_sel_cursor = cursor;
		cursor = tmpcursor;
	}
	setCursorIntern(cursor.par(), cursor.pos(),
			false, cursor.boundary());
}


// returns false if inset wasn't found
bool LyXText::updateInset(Inset * inset)
{
	// first check the current paragraph
	int pos = cursor.par()->getPositionOfInset(inset);
	if (pos != -1) {
		checkParagraph(cursor.par(), pos);
		return true;
	}

	// check every paragraph

	ParagraphList::iterator par = ownerParagraphs().begin();
	ParagraphList::iterator end = ownerParagraphs().end();
	for (; par != end; ++par) {
		pos = par->getPositionOfInset(inset);
		if (pos != -1) {
			checkParagraph(par, pos);
			return true;
		}
	};

	return false;
}


bool LyXText::setCursor(ParagraphList::iterator pit,
			pos_type pos,
			bool setfont, bool boundary)
{
	LyXCursor old_cursor = cursor;
	setCursorIntern(pit, pos, setfont, boundary);
	return deleteEmptyParagraphMechanism(old_cursor);
}


void LyXText::setCursor(LyXCursor & cur, ParagraphList::iterator pit,
			pos_type pos, bool boundary)
{
	Assert(pit != ownerParagraphs().end());

	cur.par(pit);
	cur.pos(pos);
	cur.boundary(boundary);

	// get the cursor y position in text
	int y = 0;
	RowList::iterator row = getRow(pit, pos, y);
	RowList::iterator beg = rows().begin();

	RowList::iterator old_row = row;
	cur.irow(row);
	// if we are before the first char of this row and are still in the
	// same paragraph and there is a previous row then put the cursor on
	// the end of the previous row
	cur.iy(y + row->baseline());
	if (row != beg &&
	    pos &&
	    boost::prior(row)->par() == row->par() &&
	    pos < pit->size() &&
	    pit->getChar(pos) == Paragraph::META_INSET) {
		Inset * ins = pit->getInset(pos);
		if (ins && (ins->needFullRow() || ins->display())) {
			--row;
			y -= row->height();
		}
	}

	// y is now the beginning of the cursor row
	y += row->baseline();
	// y is now the cursor baseline
	cur.y(y);

	pos_type last = lastPrintablePos(*this, old_row);

	// None of these should happen, but we're scaredy-cats
	if (pos > pit->size()) {
		lyxerr << "dont like 1 please report" << endl;
		pos = 0;
		cur.pos(0);
	} else if (pos > last + 1) {
		lyxerr << "dont like 2 please report" << endl;
		// This shouldn't happen.
		pos = last + 1;
		cur.pos(pos);
	} else if (pos < row->pos()) {
		lyxerr << "dont like 3 please report" << endl;
		pos = row->pos();
		cur.pos(pos);
	}

	// now get the cursors x position
	float x = getCursorX(row, pos, last, boundary);
	cur.x(int(x));
	cur.x_fix(cur.x());
	if (old_row != row) {
		x = getCursorX(old_row, pos, last, boundary);
		cur.ix(int(x));
	} else
		cur.ix(cur.x());
/* We take out this for the time being because 1) the redraw code is not
   prepared to this yet and 2) because some good policy has yet to be decided
   while editting: for instance how to act on rows being created/deleted
   because of DEPM.
*/
#if 0
	//if the cursor is in a visible row, anchor to it
	int topy = top_y();
	if (topy < y && y < topy + bv()->workHeight())
		anchor_row(row);
#endif
}


float LyXText::getCursorX(RowList::iterator rit,
			  pos_type pos, pos_type last, bool boundary) const
{
	pos_type cursor_vpos = 0;
	float x;
	float fill_separator;
	float fill_hfill;
	float fill_label_hfill;
	// This call HAS to be here because of the BidiTables!!!
	prepareToPrint(rit, x, fill_separator, fill_hfill,
		       fill_label_hfill);

	ParagraphList::iterator rit_par = rit->par();
	pos_type const rit_pos = rit->pos();

	if (last < rit_pos)
		cursor_vpos = rit_pos;
	else if (pos > last && !boundary)
		cursor_vpos = (rit_par->isRightToLeftPar(bv()->buffer()->params))
			? rit_pos : last + 1;
	else if (pos > rit_pos && (pos > last || boundary))
		/// Place cursor after char at (logical) position pos - 1
		cursor_vpos = (bidi_level(pos - 1) % 2 == 0)
			? log2vis(pos - 1) + 1 : log2vis(pos - 1);
	else
		/// Place cursor before char at (logical) position pos
		cursor_vpos = (bidi_level(pos) % 2 == 0)
			? log2vis(pos) : log2vis(pos) + 1;

	pos_type body_pos = rit_par->beginningOfBody();
	if ((body_pos > 0) &&
	    ((body_pos - 1 > last) || !rit_par->isLineSeparator(body_pos - 1)))
		body_pos = 0;

	for (pos_type vpos = rit_pos; vpos < cursor_vpos; ++vpos) {
		pos_type pos = vis2log(vpos);
		if (body_pos > 0 && pos == body_pos - 1) {
			x += fill_label_hfill +
				font_metrics::width(
					rit_par->layout()->labelsep,
					getLabelFont(bv()->buffer(), rit_par));
			if (rit_par->isLineSeparator(body_pos - 1))
				x -= singleWidth(rit_par, body_pos - 1);
		}

		if (hfillExpansion(*this, rit, pos)) {
			x += singleWidth(rit_par, pos);
			if (pos >= body_pos)
				x += fill_hfill;
			else
				x += fill_label_hfill;
		} else if (rit_par->isSeparator(pos)) {
			x += singleWidth(rit_par, pos);
			if (pos >= body_pos)
				x += fill_separator;
		} else
			x += singleWidth(rit_par, pos);
	}
	return x;
}


void LyXText::setCursorIntern(ParagraphList::iterator pit,
			      pos_type pos, bool setfont, bool boundary)
{
	UpdatableInset * it = pit->inInset();
	if (it) {
		if (it != inset_owner) {
			lyxerr[Debug::INSETS] << "InsetText   is " << it
					      << endl
					      << "inset_owner is "
					      << inset_owner << endl;
#ifdef WITH_WARNINGS
#warning I believe this code is wrong. (Lgb)
#warning Jürgen, have a look at this. (Lgb)
#warning Hmmm, I guess you are right but we
#warning should verify when this is needed
#endif
			// Jürgen, would you like to have a look?
			// I guess we need to move the outer cursor
			// and open and lock the inset (bla bla bla)
			// stuff I don't know... so can you have a look?
			// (Lgb)
			// I moved the lyxerr stuff in here so we can see if
			// this is actually really needed and where!
			// (Jug)
			// it->getLyXText(bv())->setCursorIntern(bv(), par, pos, setfont, boundary);
			return;
		}
	}

	setCursor(cursor, pit, pos, boundary);
	if (setfont)
		setCurrentFont();
}


void LyXText::setCurrentFont()
{
	pos_type pos = cursor.pos();
	ParagraphList::iterator pit = cursor.par();

	if (cursor.boundary() && pos > 0)
		--pos;

	if (pos > 0) {
		if (pos == pit->size())
			--pos;
		else // potentional bug... BUG (Lgb)
			if (pit->isSeparator(pos)) {
				if (pos > cursorRow()->pos() &&
				    bidi_level(pos) % 2 ==
				    bidi_level(pos - 1) % 2)
					--pos;
				else if (pos + 1 < pit->size())
					++pos;
			}
	}

	current_font =
		pit->getFontSettings(bv()->buffer()->params, pos);
	real_current_font = getFont(bv()->buffer(), pit, pos);

	if (cursor.pos() == pit->size() &&
	    isBoundary(bv()->buffer(), *pit, cursor.pos()) &&
	    !cursor.boundary()) {
		Language const * lang =
			pit->getParLanguage(bv()->buffer()->params);
		current_font.setLanguage(lang);
		current_font.setNumber(LyXFont::OFF);
		real_current_font.setLanguage(lang);
		real_current_font.setNumber(LyXFont::OFF);
	}
}


// returns the column near the specified x-coordinate of the row
// x is set to the real beginning of this column
pos_type
LyXText::getColumnNearX(RowList::iterator rit, int & x, bool & boundary) const
{
	float tmpx = 0.0;
	float fill_separator;
	float fill_hfill;
	float fill_label_hfill;

	prepareToPrint(rit, tmpx, fill_separator,
		       fill_hfill, fill_label_hfill);

	pos_type vc = rit->pos();
	pos_type last = lastPrintablePos(*this, rit);
	pos_type c = 0;

	ParagraphList::iterator rit_par = rit->par();
	LyXLayout_ptr const & layout = rit->par()->layout();

	bool left_side = false;

	pos_type body_pos = rit_par->beginningOfBody();
	float last_tmpx = tmpx;

	if (body_pos > 0 &&
	    (body_pos - 1 > last ||
	     !rit_par->isLineSeparator(body_pos - 1)))
		body_pos = 0;

	// check for empty row
	if (!rit_par->size()) {
		x = int(tmpx);
		return 0;
	}

	while (vc <= last && tmpx <= x) {
		c = vis2log(vc);
		last_tmpx = tmpx;
		if (body_pos > 0 && c == body_pos - 1) {
			tmpx += fill_label_hfill +
				font_metrics::width(layout->labelsep,
					       getLabelFont(bv()->buffer(), rit_par));
			if (rit_par->isLineSeparator(body_pos - 1))
				tmpx -= singleWidth(rit_par, body_pos - 1);
		}

		if (hfillExpansion(*this, rit, c)) {
			tmpx += singleWidth(rit_par, c);
			if (c >= body_pos)
				tmpx += fill_hfill;
			else
				tmpx += fill_label_hfill;
		} else if (rit_par->isSeparator(c)) {
			tmpx += singleWidth(rit_par, c);
			if (c >= body_pos)
				tmpx+= fill_separator;
		} else {
			tmpx += singleWidth(rit_par, c);
		}
		++vc;
	}

	if ((tmpx + last_tmpx) / 2 > x) {
		tmpx = last_tmpx;
		left_side = true;
	}

	if (vc > last + 1)  // This shouldn't happen.
		vc = last + 1;

	boundary = false;
	// This (rtl_support test) is not needed, but gives
	// some speedup if rtl_support=false
	RowList::iterator next_rit = boost::next(rit);

	bool const lastrow = lyxrc.rtl_support &&
		(next_rit == rowlist_.end() ||
		 next_rit->par() != rit_par);

	// If lastrow is false, we don't need to compute
	// the value of rtl.
	bool const rtl = (lastrow)
		? rit_par->isRightToLeftPar(bv()->buffer()->params)
		: false;
	if (lastrow &&
		 ((rtl &&  left_side && vc == rit->pos() && x < tmpx - 5) ||
		   (!rtl && !left_side && vc == last + 1   && x > tmpx + 5)))
		c = last + 1;
	else if (vc == rit->pos()) {
		c = vis2log(vc);
		if (bidi_level(c) % 2 == 1)
			++c;
	} else {
		c = vis2log(vc - 1);
		bool const rtl = (bidi_level(c) % 2 == 1);
		if (left_side == rtl) {
			++c;
			boundary = isBoundary(bv()->buffer(), *rit_par, c);
		}
	}

	if (rit->pos() <= last && c > last
	    && rit_par->isNewline(last)) {
		if (bidi_level(last) % 2 == 0)
			tmpx -= singleWidth(rit_par, last);
		else
			tmpx += singleWidth(rit_par, last);
		c = last;
	}

	c -= rit->pos();
	x = int(tmpx);
	return c;
}


void LyXText::setCursorFromCoordinates(int x, int y)
{
	LyXCursor old_cursor = cursor;

	setCursorFromCoordinates(cursor, x, y);
	setCurrentFont();
	deleteEmptyParagraphMechanism(old_cursor);
}


namespace {

	/**
	 * return true if the cursor given is at the end of a row,
	 * and the next row is filled by an inset that spans an entire
	 * row.
	 */
	bool beforeFullRowInset(LyXText & lt, LyXCursor const & cur)
	{
		RowList::iterator row = lt.getRow(cur);
		if (boost::next(row) == lt.rows().end())
			return false;

		Row const & next = *boost::next(row);

		if (next.pos() != cur.pos() || next.par() != cur.par())
			return false;

		if (cur.pos() == cur.par()->size()
		    || !cur.par()->isInset(cur.pos()))
			return false;

		Inset const * inset = cur.par()->getInset(cur.pos());
		if (inset->needFullRow() || inset->display())
			return true;

		return false;
	}
}


void LyXText::setCursorFromCoordinates(LyXCursor & cur, int x, int y)
{
	// Get the row first.

	RowList::iterator row = getRowNearY(y);
	bool bound = false;
	pos_type const column = getColumnNearX(row, x, bound);
	cur.par(row->par());
	cur.pos(row->pos() + column);
	cur.x(x);
	cur.y(y + row->baseline());

	if (beforeFullRowInset(*this, cur)) {
		pos_type const last = lastPrintablePos(*this, row);
		RowList::iterator next_row = boost::next(row);

		float x = getCursorX(next_row, cur.pos(), last, bound);
		cur.ix(int(x));
		cur.iy(y + row->height() + next_row->baseline());
		cur.irow(next_row);
	} else {
		cur.iy(cur.y());
		cur.ix(cur.x());
		cur.irow(row);
	}
	cur.boundary(bound);
}


void LyXText::cursorLeft(bool internal)
{
	if (cursor.pos() > 0) {
		bool boundary = cursor.boundary();
		setCursor(cursor.par(), cursor.pos() - 1, true, false);
		if (!internal && !boundary &&
		    isBoundary(bv()->buffer(), *cursor.par(), cursor.pos() + 1))
			setCursor(cursor.par(), cursor.pos() + 1, true, true);
	} else if (cursor.par() != ownerParagraphs().begin()) { // steps into the above paragraph.
		ParagraphList::iterator pit = boost::prior(cursor.par());
		setCursor(pit, pit->size());
	}
}


void LyXText::cursorRight(bool internal)
{
	bool const at_end = (cursor.pos() == cursor.par()->size());
	bool const at_newline = !at_end &&
		cursor.par()->isNewline(cursor.pos());

	if (!internal && cursor.boundary() && !at_newline)
		setCursor(cursor.par(), cursor.pos(), true, false);
	else if (!at_end) {
		setCursor(cursor.par(), cursor.pos() + 1, true, false);
		if (!internal &&
		    isBoundary(bv()->buffer(), *cursor.par(), cursor.pos()))
			setCursor(cursor.par(), cursor.pos(), true, true);
	} else if (boost::next(cursor.par()) != ownerParagraphs().end())
		setCursor(boost::next(cursor.par()), 0);
}


void LyXText::cursorUp(bool selecting)
{
#if 1
	int x = cursor.x_fix();
	int y = cursor.y() - cursorRow()->baseline() - 1;
	setCursorFromCoordinates(x, y);
	if (!selecting) {
		int topy = top_y();
		int y1 = cursor.iy() - topy;
		int y2 = y1;
		y -= topy;
		Inset * inset_hit = checkInsetHit(x, y1);
		if (inset_hit && isHighlyEditableInset(inset_hit)) {
			inset_hit->localDispatch(
				FuncRequest(bv(), LFUN_INSET_EDIT, x, y - (y2 - y1), mouse_button::none));
		}
	}
#else
	setCursorFromCoordinates(bv(), cursor.x_fix(),
				 cursor.y() - cursorRow()->baseline() - 1);
#endif
}


void LyXText::cursorDown(bool selecting)
{
#if 1
	int x = cursor.x_fix();
	int y = cursor.y() - cursorRow()->baseline() +
		cursorRow()->height() + 1;
	setCursorFromCoordinates(x, y);
	if (!selecting && cursorRow() == cursor.irow()) {
		int topy = top_y();
		int y1 = cursor.iy() - topy;
		int y2 = y1;
		y -= topy;
		Inset * inset_hit = checkInsetHit(x, y1);
		if (inset_hit && isHighlyEditableInset(inset_hit)) {
			FuncRequest cmd(bv(), LFUN_INSET_EDIT, x, y - (y2 - y1), mouse_button::none);
			inset_hit->localDispatch(cmd);
		}
	}
#else
	setCursorFromCoordinates(bv(), cursor.x_fix(),
				 cursor.y() - cursorRow()->baseline()
				 + cursorRow()->height() + 1);
#endif
}


void LyXText::cursorUpParagraph()
{
	if (cursor.pos() > 0) {
		setCursor(cursor.par(), 0);
	}
	else if (cursor.par() != ownerParagraphs().begin()) {
		setCursor(boost::prior(cursor.par()), 0);
	}
}


void LyXText::cursorDownParagraph()
{
	ParagraphList::iterator par = cursor.par();
	ParagraphList::iterator next_par = boost::next(par);

	if (next_par != ownerParagraphs().end()) {
		setCursor(next_par, 0);
	} else {
		setCursor(par, par->size());
	}
}

// fix the cursor `cur' after a characters has been deleted at `where'
// position. Called by deleteEmptyParagraphMechanism
void LyXText::fixCursorAfterDelete(LyXCursor & cur,
				   LyXCursor const & where)
{
	// if cursor is not in the paragraph where the delete occured,
	// do nothing
	if (cur.par() != where.par())
		return;

	// if cursor position is after the place where the delete occured,
	// update it
	if (cur.pos() > where.pos())
		cur.pos(cur.pos()-1);

	// check also if we don't want to set the cursor on a spot behind the
	// pagragraph because we erased the last character.
	if (cur.pos() > cur.par()->size())
		cur.pos(cur.par()->size());

	// recompute row et al. for this cursor
	setCursor(cur, cur.par(), cur.pos(), cur.boundary());
}


bool LyXText::deleteEmptyParagraphMechanism(LyXCursor const & old_cursor)
{
	// Would be wrong to delete anything if we have a selection.
	if (selection.set())
		return false;

	// We allow all kinds of "mumbo-jumbo" when freespacing.
	if (old_cursor.par()->layout()->free_spacing
	    || old_cursor.par()->isFreeSpacing()) {
		return false;
	}

	/* Ok I'll put some comments here about what is missing.
	   I have fixed BackSpace (and thus Delete) to not delete
	   double-spaces automagically. I have also changed Cut,
	   Copy and Paste to hopefully do some sensible things.
	   There are still some small problems that can lead to
	   double spaces stored in the document file or space at
	   the beginning of paragraphs. This happens if you have
	   the cursor betwenn to spaces and then save. Or if you
	   cut and paste and the selection have a space at the
	   beginning and then save right after the paste. I am
	   sure none of these are very hard to fix, but I will
	   put out 1.1.4pre2 with FIX_DOUBLE_SPACE defined so
	   that I can get some feedback. (Lgb)
	*/

	// If old_cursor.pos() == 0 and old_cursor.pos()(1) == LineSeparator
	// delete the LineSeparator.
	// MISSING

	// If old_cursor.pos() == 1 and old_cursor.pos()(0) == LineSeparator
	// delete the LineSeparator.
	// MISSING

	// If the pos around the old_cursor were spaces, delete one of them.
	if (old_cursor.par() != cursor.par()
	    || old_cursor.pos() != cursor.pos()) {
		// Only if the cursor has really moved

		if (old_cursor.pos() > 0
		    && old_cursor.pos() < old_cursor.par()->size()
		    && old_cursor.par()->isLineSeparator(old_cursor.pos())
		    && old_cursor.par()->isLineSeparator(old_cursor.pos() - 1)) {
			old_cursor.par()->erase(old_cursor.pos() - 1);
			redoParagraphs(old_cursor, boost::next(old_cursor.par()));

#ifdef WITH_WARNINGS
#warning This will not work anymore when we have multiple views of the same buffer
// In this case, we will have to correct also the cursors held by
// other bufferviews. It will probably be easier to do that in a more
// automated way in LyXCursor code. (JMarc 26/09/2001)
#endif
			// correct all cursors held by the LyXText
			fixCursorAfterDelete(cursor, old_cursor);
			fixCursorAfterDelete(selection.cursor,
					     old_cursor);
			fixCursorAfterDelete(selection.start,
					     old_cursor);
			fixCursorAfterDelete(selection.end, old_cursor);
			fixCursorAfterDelete(last_sel_cursor,
					     old_cursor);
			fixCursorAfterDelete(toggle_cursor, old_cursor);
			fixCursorAfterDelete(toggle_end_cursor,
					     old_cursor);
			return false;
		}
	}

	// don't delete anything if this is the ONLY paragraph!
	if (ownerParagraphs().size() == 1)
		return false;

	// Do not delete empty paragraphs with keepempty set.
	if (old_cursor.par()->allowEmpty())
		return false;

	// only do our magic if we changed paragraph
	if (old_cursor.par() == cursor.par())
		return false;

	// record if we have deleted a paragraph
	// we can't possibly have deleted a paragraph before this point
	bool deleted = false;

	if (old_cursor.par()->empty() ||
	    (old_cursor.par()->size() == 1 &&
	     old_cursor.par()->isLineSeparator(0))) {
		// ok, we will delete anything
		LyXCursor tmpcursor;

		deleted = true;

		bool selection_position_was_oldcursor_position = (
			selection.cursor.par()  == old_cursor.par()
			&& selection.cursor.pos() == old_cursor.pos());

		if (getRow(old_cursor) != rows().begin()) {
			RowList::iterator
				prevrow = boost::prior(getRow(old_cursor));
			postPaint(old_cursor.y() - getRow(old_cursor)->baseline() - prevrow->height());
			tmpcursor = cursor;
			cursor = old_cursor; // that undo can restore the right cursor position
			#warning FIXME. --end() iterator is usable here
			ParagraphList::iterator endpit = boost::next(old_cursor.par());
			while (endpit != ownerParagraphs().end() &&
			       endpit->getDepth()) {
				++endpit;
			}

			setUndo(bv(), Undo::DELETE, old_cursor.par(),
				boost::prior(endpit));
			cursor = tmpcursor;

			// delete old row
			removeRow(getRow(old_cursor));
			// delete old par
			ownerParagraphs().erase(old_cursor.par());

			/* Breakagain the next par. Needed because of
			 * the parindent that can occur or dissappear.
			 * The next row can change its height, if
			 * there is another layout before */
			RowList::iterator tmprit = boost::next(prevrow);
			if (tmprit != rows().end()) {
				breakAgain(tmprit);
				updateCounters();
			}
			setHeightOfRow(prevrow);
		} else {
			RowList::iterator nextrow = boost::next(getRow(old_cursor));
			postPaint(old_cursor.y() - getRow(old_cursor)->baseline());

			tmpcursor = cursor;
			cursor = old_cursor; // that undo can restore the right cursor position
#warning FIXME. --end() iterator is usable here
			ParagraphList::iterator endpit = boost::next(old_cursor.par());
			while (endpit != ownerParagraphs().end() &&
			       endpit->getDepth()) {
				++endpit;
			}

			setUndo(bv(), Undo::DELETE, old_cursor.par(), boost::prior(endpit));
			cursor = tmpcursor;

			// delete old row
			removeRow(getRow(old_cursor));
			// delete old par
			ownerParagraphs().erase(old_cursor.par());

			/* Breakagain the next par. Needed because of
			   the parindent that can occur or dissappear.
			   The next row can change its height, if
			   there is another layout before */
			if (nextrow != rows().end()) {
				breakAgain(nextrow);
				updateCounters();
			}
		}

		// correct cursor y
		setCursorIntern(cursor.par(), cursor.pos());

		if (selection_position_was_oldcursor_position) {
			// correct selection
			selection.cursor = cursor;
		}
	}
	if (!deleted) {
		if (old_cursor.par()->stripLeadingSpaces()) {
			redoParagraphs(old_cursor, boost::next(old_cursor.par()));
			// correct cursor y
			setCursorIntern(cursor.par(), cursor.pos());
			selection.cursor = cursor;
		}
	}
	return deleted;
}


ParagraphList & LyXText::ownerParagraphs() const
{
	if (inset_owner) {
		return inset_owner->paragraphs;
	}
	return bv_owner->buffer()->paragraphs;
}


LyXText::refresh_status LyXText::refreshStatus() const
{
	return refresh_status_;
}


void LyXText::clearPaint()
{
	refresh_status_ = REFRESH_NONE;
	refresh_row = rows().end();
	refresh_y = 0;
}


void LyXText::postPaint(int start_y)
{
	refresh_status old = refresh_status_;

	refresh_status_ = REFRESH_AREA;
	refresh_row = rows().end();

	if (old != REFRESH_NONE && refresh_y < start_y)
		return;

	refresh_y = start_y;

	if (!inset_owner)
		return;

	// We are an inset's lyxtext. Tell the top-level lyxtext
	// it needs to update the row we're in.
	LyXText * t = bv()->text;
	t->postRowPaint(t->cursorRow(), t->cursor.y() - t->cursorRow()->baseline());
}


// FIXME: we should probably remove this y parameter,
// make refresh_y be 0, and use row->y etc.
void LyXText::postRowPaint(RowList::iterator rit, int start_y)
{
	if (refresh_status_ != REFRESH_NONE && refresh_y < start_y) {
		refresh_status_ = REFRESH_AREA;
		return;
	} else {
		refresh_y = start_y;
	}

	if (refresh_status_ == REFRESH_AREA)
		return;

	refresh_status_ = REFRESH_ROW;
	refresh_row = rit;

	if (!inset_owner)
		return;

	// We are an inset's lyxtext. Tell the top-level lyxtext
	// it needs to update the row we're in.
	LyXText * t = bv()->text;
	t->postRowPaint(t->cursorRow(), t->cursor.y() - t->cursorRow()->baseline());
}


bool LyXText::isInInset() const
{
	// Sub-level has non-null bv owner and
	// non-null inset owner.
	return inset_owner != 0 && bv_owner != 0;
}


int defaultRowHeight()
{
	LyXFont const font(LyXFont::ALL_SANE);
	return int(font_metrics::maxAscent(font)
		 + font_metrics::maxDescent(font) * 1.5);
}
