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
#include "paragraph.h"
#include "frontends/LyXView.h"
#include "undo_funcs.h"
#include "buffer.h"
#include "bufferparams.h"
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

#include "insets/inseterror.h"
#include "insets/insetbibitem.h"
#include "insets/insetspecialchar.h"
#include "insets/insettext.h"
#include "insets/insetfloat.h"
#include "insets/insetwrap.h"

#include "support/LAssert.h"
#include "support/textutils.h"
#include "support/lstrings.h"

#include "BoostFormat.h"

using std::vector;
using std::copy;
using std::endl;
using std::find;
using std::pair;
using lyx::pos_type;


LyXText::LyXText(BufferView * bv)
	: height(0), width(0), anchor_row_(0), anchor_row_offset_(0),
	  inset_owner(0), the_locking_inset(0), need_break_row(0),
	  bv_owner(bv)
{
	refresh_row = 0;
	clearPaint();
}


LyXText::LyXText(BufferView * bv, InsetText * inset)
	: height(0), width(0), anchor_row_(0), anchor_row_offset_(0),
	  inset_owner(inset), the_locking_inset(0), need_break_row(0),
	  bv_owner(bv)
{
	refresh_row = 0;
	clearPaint();
}


void LyXText::init(BufferView * bview, bool reinit)
{
	if (reinit) {
		rowlist_.clear();
		need_break_row = 0;
		width = height = 0;
		copylayouttype.erase();
		top_y(0);
		clearPaint();
	} else if (firstRow())
		return;

	Paragraph * par = ownerParagraph();
	current_font = getFont(bview->buffer(), par, 0);

	while (par) {
		if (rowlist_.empty())
			insertParagraph(par, 0);
		else
			insertParagraph(par, lastRow());
		par = par->next();
	}
	setCursorIntern(firstRow()->par(), 0);
	selection.cursor = cursor;

	updateCounters();
}


namespace {

LyXFont const realizeFont(LyXFont const & font,
			  Buffer const * buf,
			  Paragraph * par)
{
	LyXTextClass const & tclass = buf->params.getLyXTextClass();
	LyXFont tmpfont(font);
	Paragraph::depth_type par_depth = par->getDepth();

	// Resolve against environment font information
	while (par && par_depth && !tmpfont.resolved()) {
		par = par->outerHook();
		if (par) {
			tmpfont.realize(par->layout()->font);
			par_depth = par->getDepth();
		}
	}

	tmpfont.realize(tclass.defaultfont());

	return tmpfont;
}

}


// Gets the fully instantiated font at a given position in a paragraph
// Basically the same routine as Paragraph::getFont() in paragraph.C.
// The difference is that this one is used for displaying, and thus we
// are allowed to make cosmetic improvements. For instance make footnotes
// smaller. (Asger)
// If position is -1, we get the layout font of the paragraph.
// If position is -2, we get the font of the manual label of the paragraph.
LyXFont const LyXText::getFont(Buffer const * buf, Paragraph * par,
			       pos_type pos) const
{
	lyx::Assert(pos >= 0);

	LyXLayout_ptr const & layout = par->layout();

	// We specialize the 95% common case:
	if (!par->getDepth()) {
		if (layout->labeltype == LABEL_MANUAL
		    && pos < par->beginningOfBody()) {
			// 1% goes here
			LyXFont f = par->getFontSettings(buf->params, pos);
			if (par->inInset())
				par->inInset()->getDrawFont(f);
			return f.realize(layout->reslabelfont);
		} else {
			LyXFont f = par->getFontSettings(buf->params, pos);
			if (par->inInset())
				par->inInset()->getDrawFont(f);
			return f.realize(layout->resfont);
		}
	}

	// The uncommon case need not be optimized as much

	LyXFont layoutfont;

	if (pos < par->beginningOfBody()) {
		// 1% goes here
		layoutfont = layout->labelfont;
	} else {
		// 99% goes here
		layoutfont = layout->font;
	}

	LyXFont tmpfont = par->getFontSettings(buf->params, pos);
	tmpfont.realize(layoutfont);

	if (par->inInset())
		par->inInset()->getDrawFont(tmpfont);

	return realizeFont(tmpfont, buf, par);
}


LyXFont const LyXText::getLayoutFont(Buffer const * buf, Paragraph * par) const
{
	LyXLayout_ptr const & layout = par->layout();

	if (!par->getDepth()) {
		return layout->resfont;
	}

	return realizeFont(layout->font, buf, par);
}


LyXFont const LyXText::getLabelFont(Buffer const * buf, Paragraph * par) const
{
	LyXLayout_ptr const & layout = par->layout();

	if (!par->getDepth()) {
		return layout->reslabelfont;
	}

	return realizeFont(layout->labelfont, buf, par);
}


void LyXText::setCharFont(Paragraph * par,
			  pos_type pos, LyXFont const & fnt,
			  bool toggleall)
{
	Buffer const * buf = bv()->buffer();
	LyXFont font = getFont(buf, par, pos);
	font.update(fnt, buf->params.language, toggleall);
	// Let the insets convert their font
	if (par->isInset(pos)) {
		Inset * inset = par->getInset(pos);
		if (isEditableInset(inset)) {
			UpdatableInset * uinset =
				static_cast<UpdatableInset *>(inset);
			uinset->setFont(bv(), fnt, toggleall, true);
		}
	}

	// Plug thru to version below:
	setCharFont(buf, par, pos, font);
}


void LyXText::setCharFont(Buffer const * buf, Paragraph * par,
			  pos_type pos, LyXFont const & fnt)
{
	LyXFont font(fnt);

	LyXTextClass const & tclass = buf->params.getLyXTextClass();
	LyXLayout_ptr const & layout = par->layout();

	// Get concrete layout font to reduce against
	LyXFont layoutfont;

	if (pos < par->beginningOfBody())
		layoutfont = layout->labelfont;
	else
		layoutfont = layout->font;

	// Realize against environment font information
	if (par->getDepth()) {
		Paragraph * tp = par;
		while (!layoutfont.resolved() && tp && tp->getDepth()) {
			tp = tp->outerHook();
			if (tp)
				layoutfont.realize(tp->layout()->font);
		}
	}

	layoutfont.realize(tclass.defaultfont());

	// Now, reduce font against full layout font
	font.reduce(layoutfont);

	par->setFont(pos, font);
}


// inserts a new row before the specified row, increments
// the touched counters
void LyXText::insertRow(Row * row, Paragraph * par,
			pos_type pos)
{
	Row * tmprow = new Row;
	tmprow->par(par);
	tmprow->pos(pos);

	if (!row) {
		rowlist_.insert(rowlist_.begin(), tmprow);
	} else {
		rowlist_.insert(row->next(), tmprow);
	}
}


// removes the row and reset the touched counters
void LyXText::removeRow(Row * row)
{
	lyx::Assert(row);

	Row * row_prev = row->previous();
	Row * row_next = row->next();
	int const row_height = row->height();

	/* FIXME: when we cache the bview, this should just
	 * become a postPaint(), I think */
	if (refresh_row == row) {
		refresh_row = row_prev ? row_prev : row_next;
		// what about refresh_y
	}

	if (anchor_row_ == row) {
		if (row_prev) {
			anchor_row_ = row_prev;
			anchor_row_offset_ += row_prev->height();
		} else {
			anchor_row_ = row_next;
			anchor_row_offset_ -= row_height;
		}
	}

	// the text becomes smaller
	height -= row_height;

	rowlist_.erase(row);
}


// remove all following rows of the paragraph of the specified row.
void LyXText::removeParagraph(Row * row)
{
	Paragraph * tmppar = row->par();
	row = row->next();

	Row * tmprow;
	while (row && row->par() == tmppar) {
		tmprow = row->next();
		removeRow(row);
		row = tmprow;
	}
}


void LyXText::insertParagraph(Paragraph * par, Row * row)
{
	// insert a new row, starting at position 0
	insertRow(row, par, 0);

	// and now append the whole paragraph before the new row
	if (!row) {
		appendParagraph(firstRow());
	} else {
		appendParagraph(row->next());
	}
}


Inset * LyXText::getInset() const
{
	if (cursor.pos() < cursor.par()->size()
		   && cursor.par()->isInset(cursor.pos())) {
		return cursor.par()->getInset(cursor.pos());
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
			bv()->unlockInset(static_cast<UpdatableInset *>(inset_owner->owner()));
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

	LyXFont layoutfont;
	for (pos_type pos = 0; pos < par.size(); ++pos) {
		if (pos < par.beginningOfBody())
			layoutfont = layout->labelfont;
		else
			layoutfont = layout->font;

		LyXFont tmpfont = par.getFontSettings(buf.params, pos);
		tmpfont.reduce(layoutfont);
		par.setFont(pos, tmpfont);
	}
}


Paragraph * LyXText::setLayout(LyXCursor & cur, LyXCursor & sstart_cur,
			       LyXCursor & send_cur,
			       string const & layout)
{
	Paragraph * endpar = send_cur.par()->next();
	Paragraph * undoendpar = endpar;

	if (endpar && endpar->getDepth()) {
		while (endpar && endpar->getDepth()) {
			endpar = endpar->next();
			undoendpar = endpar;
		}
	} else if (endpar) {
		endpar = endpar->next(); // because of parindents etc.
	}

	setUndo(bv(), Undo::EDIT, sstart_cur.par(), undoendpar);

	// ok we have a selection. This is always between sstart_cur
	// and sel_end cursor
	cur = sstart_cur;
	Paragraph * par = sstart_cur.par();
	Paragraph * epar = send_cur.par()->next();

	LyXLayout_ptr const & lyxlayout =
		bv()->buffer()->params.getLyXTextClass()[layout];

	do {
		par->applyLayout(lyxlayout);
		makeFontEntriesLayoutSpecific(*bv()->buffer(), *par);
		Paragraph * fppar = par;
		fppar->params().spaceTop(lyxlayout->fill_top ?
					 VSpace(VSpace::VFILL)
					 : VSpace(VSpace::NONE));
		fppar->params().spaceBottom(lyxlayout->fill_bottom ?
					    VSpace(VSpace::VFILL)
					    : VSpace(VSpace::NONE));
		if (lyxlayout->margintype == MARGIN_MANUAL)
			par->setLabelWidthString(lyxlayout->labelstring());
		cur.par(par);
		par = par->next();
	} while (par != epar);

	return endpar;
}


// set layout over selection and make a total rebreak of those paragraphs
void LyXText::setLayout(string const & layout)
{
	LyXCursor tmpcursor = cursor;  /* store the current cursor  */

	// if there is no selection just set the layout
	// of the current paragraph  */
	if (!selection.set()) {
		selection.start = cursor;  // dummy selection
		selection.end = cursor;
	}
	Paragraph * endpar = setLayout(cursor, selection.start,
				       selection.end, layout);
	redoParagraphs(selection.start, endpar);

	// we have to reset the selection, because the
	// geometry could have changed
	setCursor(selection.start.par(),
		  selection.start.pos(), false);
	selection.cursor = cursor;
	setCursor(selection.end.par(), selection.end.pos(), false);
	updateCounters();
	clearSelection();
	setSelection();
	setCursor(tmpcursor.par(), tmpcursor.pos(), true);
}


// increment depth over selection and
// make a total rebreak of those paragraphs
void  LyXText::incDepth()
{
	// If there is no selection, just use the current paragraph
	if (!selection.set()) {
		selection.start = cursor; // dummy selection
		selection.end = cursor;
	}

	// We end at the next paragraph with depth 0
	Paragraph * endpar = selection.end.par()->next();

	Paragraph * undoendpar = endpar;

	if (endpar && endpar->getDepth()) {
		while (endpar && endpar->getDepth()) {
			endpar = endpar->next();
			undoendpar = endpar;
		}
	} else if (endpar) {
		endpar = endpar->next(); // because of parindents etc.
	}

	setUndo(bv(), Undo::EDIT,
		selection.start.par(), undoendpar);

	LyXCursor tmpcursor = cursor; // store the current cursor

	// ok we have a selection. This is always between sel_start_cursor
	// and sel_end cursor
	cursor = selection.start;

	while (true) {
		// NOTE: you can't change the depth of a bibliography entry
		if (cursor.par()->layout()->labeltype != LABEL_BIBLIO) {
			Paragraph * prev = cursor.par()->previous();

			if (prev) {
				if (cursor.par()->getDepth()
				    < prev->getMaxDepthAfter()) {
					cursor.par()->params().depth(cursor.par()->getDepth() + 1);
				}
			}
		}
		if (cursor.par() == selection.end.par())
			break;
		cursor.par(cursor.par()->next());
	}

	redoParagraphs(selection.start, endpar);

	// we have to reset the selection, because the
	// geometry could have changed
	setCursor(selection.start.par(), selection.start.pos());
	selection.cursor = cursor;
	setCursor(selection.end.par(), selection.end.pos());
	updateCounters();
	clearSelection();
	setSelection();
	setCursor(tmpcursor.par(), tmpcursor.pos());
}


// decrement depth over selection and
// make a total rebreak of those paragraphs
void  LyXText::decDepth()
{
	// if there is no selection just set the layout
	// of the current paragraph
	if (!selection.set()) {
		selection.start = cursor; // dummy selection
		selection.end = cursor;
	}
	Paragraph * endpar = selection.end.par()->next();
	Paragraph * undoendpar = endpar;

	if (endpar && endpar->getDepth()) {
		while (endpar && endpar->getDepth()) {
			endpar = endpar->next();
			undoendpar = endpar;
		}
	} else if (endpar) {
		endpar = endpar->next(); // because of parindents etc.
	}

	setUndo(bv(), Undo::EDIT,
		selection.start.par(), undoendpar);

	LyXCursor tmpcursor = cursor; // store the current cursor

	// ok we have a selection. This is always between sel_start_cursor
	// and sel_end cursor
	cursor = selection.start;

	while (true) {
		if (cursor.par()->params().depth()) {
			cursor.par()->params()
				.depth(cursor.par()->params().depth() - 1);
		}
		if (cursor.par() == selection.end.par()) {
			break;
		}
		cursor.par(cursor.par()->next());
	}

	redoParagraphs(selection.start, endpar);

	// we have to reset the selection, because the
	// geometry could have changed
	setCursor(selection.start.par(),
		  selection.start.pos());
	selection.cursor = cursor;
	setCursor(selection.end.par(), selection.end.pos());
	updateCounters();
	clearSelection();
	setSelection();
	setCursor(tmpcursor.par(), tmpcursor.pos());
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

	setUndo(bv(), Undo::EDIT,
		selection.start.par(), selection.end.par()->next());
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
			cursor.par(cursor.par()->next());
		}
	}
	unFreezeUndo();

	redoParagraphs(selection.start, selection.end.par()->next());

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
	Row * tmprow = cursor.row();
	int y = cursor.y() - tmprow->baseline();

	setHeightOfRow(tmprow);

	while (tmprow->previous()
	       && tmprow->previous()->par() == tmprow->par()) {
		tmprow = tmprow->previous();
		y -= tmprow->height();
		setHeightOfRow(tmprow);
	}

	postPaint(y);

	setCursor(cursor.par(), cursor.pos(), false, cursor.boundary());
}


void LyXText::redoDrawingOfParagraph(LyXCursor const & cur)
{
	Row * tmprow = cur.row();

	int y = cur.y() - tmprow->baseline();
	setHeightOfRow(tmprow);

	while (tmprow->previous()
	       && tmprow->previous()->par() == tmprow->par())  {
		tmprow = tmprow->previous();
		y -= tmprow->height();
	}

	postPaint(y);
	setCursor(cur.par(), cur.pos());
}


// deletes and inserts again all paragaphs between the cursor
// and the specified par
// This function is needed after SetLayout and SetFont etc.
void LyXText::redoParagraphs(LyXCursor const & cur,
			     Paragraph const * endpar)
{
	Row * tmprow = cur.row();

	int y = cur.y() - tmprow->baseline();

	Paragraph * first_phys_par = 0;
	if (!tmprow->previous()) {
		// a trick/hack for UNDO
		// This is needed because in an UNDO/REDO we could have changed
		// the ownerParagrah() so the paragraph inside the row is NOT
		// my really first par anymore. Got it Lars ;) (Jug 20011206)
		first_phys_par = ownerParagraph();
		lyxerr << "ownerParagraph" << endl;

	} else {
		first_phys_par = tmprow->par();
		lyxerr << "tmprow->par()" << endl;

		// Find first row of this paragraph.
		while (tmprow->previous()
		       && tmprow->previous()->par() == first_phys_par)
		{
			tmprow = tmprow->previous();
			y -= tmprow->height();
		}
	}

	Row * prevrow = tmprow->previous();

	// Remove all the rows until we reach endpar
	Paragraph * tmppar = 0;
	if (tmprow->next())
		tmppar = tmprow->next()->par();
	while (tmprow->next() && tmppar != endpar) {
		removeRow(tmprow->next());
		if (tmprow->next()) {
			tmppar = tmprow->next()->par();
		} else {
			tmppar = 0;
		}
	}

	// Remove the first of the paragraphs rows.
	// This is because tmprow->previous() can be 0
	Row * tmprow2 = tmprow;
	tmprow = tmprow->previous();
	removeRow(tmprow2);

	// Reinsert the paragraphs.
	tmppar = first_phys_par;
	do {
		if (tmppar) {
			insertParagraph(tmppar, tmprow);
			if (!tmprow) {
				tmprow = firstRow();
			}
			while (tmprow->next()
			       && tmprow->next()->par() == tmppar) {
				tmprow = tmprow->next();
			}
			tmppar = tmppar->next();
		}
	} while (tmppar && tmppar != endpar);

	// this is because of layout changes
	if (prevrow) {
		setHeightOfRow(prevrow);
		const_cast<LyXText *>(this)->postPaint(y - prevrow->height());
	} else {
		setHeightOfRow(firstRow());
		const_cast<LyXText *>(this)->postPaint(0);
	}

	if (tmprow && tmprow->next())
		setHeightOfRow(tmprow->next());
	updateCounters();
}


void LyXText::fullRebreak()
{
	if (!firstRow()) {
		init(bv());
		return;
	}
	if (need_break_row) {
		breakAgain(need_break_row);
		need_break_row = 0;
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
	bool const lsel = selection.set();

	if (!selection.set()) {
		last_sel_cursor = selection.cursor;
		selection.start = selection.cursor;
		selection.end = selection.cursor;
	}

	selection.set(true);

	// first the toggling area
	if (cursor.y() < last_sel_cursor.y()
	    || (cursor.y() == last_sel_cursor.y()
		&& cursor.x() < last_sel_cursor.x())) {
		toggle_end_cursor = last_sel_cursor;
		toggle_cursor = cursor;
	} else {
		toggle_end_cursor = cursor;
		toggle_cursor = last_sel_cursor;
	}

	last_sel_cursor = cursor;

	// and now the whole selection

	if (selection.cursor.par() == cursor.par())
		if (selection.cursor.pos() < cursor.pos()) {
			selection.end = cursor;
			selection.start = selection.cursor;
		} else {
			selection.end = selection.cursor;
			selection.start = cursor;
		}
	else if (selection.cursor.y() < cursor.y() ||
		 (selection.cursor.y() == cursor.y()
		  && selection.cursor.x() < cursor.x())) {
		selection.end = cursor;
		selection.start = selection.cursor;
	}
	else {
		selection.end = selection.cursor;
		selection.start = cursor;
	}

	// a selection with no contents is not a selection
	if (selection.start.par() == selection.end.par() &&
	    selection.start.pos() == selection.end.pos())
		selection.set(false);

	if (inset_owner && (selection.set() || lsel))
		inset_owner->setUpdateStatus(bv(), InsetText::SELECTION);
}


string const LyXText::selectionAsString(Buffer const * buffer,
					bool label) const
{
	if (!selection.set()) return string();

	// should be const ...
	Paragraph * startpar(selection.start.par());
	Paragraph * endpar(selection.end.par());
	pos_type const startpos(selection.start.pos());
	pos_type const endpos(selection.end.pos());

	if (startpar == endpar) {
		return startpar->asString(buffer, startpos, endpos, label);
	}

	string result;

	// First paragraph in selection
	result += startpar->asString(buffer, startpos, startpar->size(), label) + "\n\n";

	// The paragraphs in between (if any)
	LyXCursor tmpcur(selection.start);
	tmpcur.par(tmpcur.par()->next());
	while (tmpcur.par() != endpar) {
		result += tmpcur.par()->asString(buffer, 0,
						 tmpcur.par()->size(),
						 label) + "\n\n";
		tmpcur.par(tmpcur.par()->next());
	}

	// Last paragraph in selection
	result += endpar->asString(buffer, 0, endpos, label);

	return result;
}


void LyXText::clearSelection()
{
	selection.set(false);
	selection.mark(false);
	last_sel_cursor = selection.end = selection.start = selection.cursor = cursor;
	// reset this in the bv_owner!
	if (bv_owner && bv_owner->text)
		bv_owner->text->xsel_cache.set(false);
}


void LyXText::cursorHome()
{
	setCursor(cursor.par(), cursor.row()->pos());
}


void LyXText::cursorEnd()
{
	if (!cursor.row()->next()
	    || cursor.row()->next()->par() != cursor.row()->par()) {
		setCursor(cursor.par(), cursor.row()->lastPos() + 1);
	} else {
		if (!cursor.par()->empty() &&
		    (cursor.par()->getChar(cursor.row()->lastPos()) == ' '
		     || cursor.par()->isNewline(cursor.row()->lastPos()))) {
			setCursor(cursor.par(), cursor.row()->lastPos());
		} else {
			setCursor(cursor.par(),
				  cursor.row()->lastPos() + 1);
		}
	}
}


void LyXText::cursorTop()
{
	while (cursor.par()->previous())
		cursor.par(cursor.par()->previous());
	setCursor(cursor.par(), 0);
}


void LyXText::cursorBottom()
{
	while (cursor.par()->next())
		cursor.par(cursor.par()->next());
	setCursor(cursor.par(), cursor.par()->size());
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
		? selectWordWhenUnderCursor(WHOLE_WORD_STRICT) : false;

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
		inset_owner->setUpdateStatus(bv(), InsetText::CURSOR_PAR);
}


string LyXText::getStringToIndex()
{
	// Try implicit word selection
	// If there is a change in the language the implicit word selection
	// is disabled.
	LyXCursor const reset_cursor = cursor;
	bool const implicitSelection = selectWordWhenUnderCursor(PREVIOUS_WORD);

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
	Paragraph * endpar = selection.end.par()->next();
	Paragraph * undoendpar = endpar;

	if (endpar && endpar->getDepth()) {
		while (endpar && endpar->getDepth()) {
			endpar = endpar->next();
			undoendpar = endpar;
		}
	}
	else if (endpar) {
		// because of parindents etc.
		endpar = endpar->next();
	}

	setUndo(bv(), Undo::EDIT, selection.start.par(), undoendpar);


	Paragraph * tmppar = selection.end.par();

	while (tmppar != selection.start.par()->previous()) {
		setCursor(tmppar, 0);
		postPaint(cursor.y() - cursor.row()->baseline());
		cursor.par()->params().lineTop(line_top);
		cursor.par()->params().lineBottom(line_bottom);
		cursor.par()->params().pagebreakTop(pagebreak_top);
		cursor.par()->params().pagebreakBottom(pagebreak_bottom);
		cursor.par()->params().spaceTop(space_top);
		cursor.par()->params().spaceBottom(space_bottom);
		cursor.par()->params().spacing(spacing);
		// does the layout allow the new alignment?
		LyXLayout_ptr const & layout = cursor.par()->layout();

		if (align == LYX_ALIGN_LAYOUT)
			align = layout->align;
		if (align & layout->alignpossible) {
			if (align == layout->align)
				cursor.par()->params().align(LYX_ALIGN_LAYOUT);
			else
				cursor.par()->params().align(align);
		}
		cursor.par()->setLabelWidthString(labelwidthstring);
		cursor.par()->params().noindent(noindent);
		tmppar = cursor.par()->previous();
	}

	redoParagraphs(selection.start, endpar);

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
void LyXText::setCounter(Buffer const * buf, Paragraph * par)
{
	LyXTextClass const & textclass = buf->params.getLyXTextClass();
	LyXLayout_ptr const & layout = par->layout();

	if (par->previous()) {

		par->params().appendix(par->previous()->params().appendix());
		if (!par->params().appendix() && par->params().startOfAppendix()) {
			par->params().appendix(true);
			textclass.counters().reset();
		}
		par->enumdepth = par->previous()->enumdepth;
		par->itemdepth = par->previous()->itemdepth;
	} else {
		par->params().appendix(par->params().startOfAppendix());
		par->enumdepth = 0;
		par->itemdepth = 0;
	}

	/* Maybe we have to increment the enumeration depth.
	 * BUT, enumeration in a footnote is considered in isolation from its
	 *	surrounding paragraph so don't increment if this is the
	 *	first line of the footnote
	 * AND, bibliographies can't have their depth changed ie. they
	 *	are always of depth 0
	 */
	if (par->previous()
	    && par->previous()->getDepth() < par->getDepth()
	    && par->previous()->layout()->labeltype == LABEL_COUNTER_ENUMI
	    && par->enumdepth < 3
	    && layout->labeltype != LABEL_BIBLIO) {
		par->enumdepth++;
	}

	// Maybe we have to decrement the enumeration depth, see note above
	if (par->previous()
	    && par->previous()->getDepth() > par->getDepth()
	    && layout->labeltype != LABEL_BIBLIO) {
		par->enumdepth = par->depthHook(par->getDepth())->enumdepth;
	}

	if (!par->params().labelString().empty()) {
		par->params().labelString(string());
	}

	if (layout->margintype == MARGIN_MANUAL) {
		if (par->params().labelWidthString().empty()) {
			par->setLabelWidthString(layout->labelstring());
		}
	} else {
		par->setLabelWidthString(string());
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
			if (!par->params().appendix()) {
				s << layout->labelstring();
			} else {
				s << layout->labelstring_appendix();
			}

			// Use of an integer is here less than elegant. For now.
			int head = textclass.maxcounter() - LABEL_COUNTER_CHAPTER;
			if (!par->params().appendix()) {
				numbertype = "sectioning";
			} else {
				numbertype = "appendix";
				if (par->isRightToLeftPar(buf->params))
					langtype = "hebrew";
				else
					langtype = "latin";
			}

			s << textclass.counters()
				.numberLabel(layout->latexname(),
					     numbertype, langtype, head);

			par->params().labelString(STRCONV(s.str()));

			// reset enum counters
			textclass.counters().reset("enum");
		} else if (layout->labeltype < LABEL_COUNTER_ENUMI) {
			textclass.counters().reset("enum");
		} else if (layout->labeltype == LABEL_COUNTER_ENUMI) {
			// FIXME
			// Yes I know this is a really, really! bad solution
			// (Lgb)
			string enumcounter("enum");

			switch (par->enumdepth) {
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
			par->params().labelString(STRCONV(s.str()));
		}
	} else if (layout->labeltype == LABEL_BIBLIO) {// ale970302
		textclass.counters().step("bibitem");
		int number = textclass.counters().value("bibitem");
		if (par->bibitem()) {
			par->bibitem()->setCounter(number);
			par->params().labelString(layout->labelstring());
		}
		// In biblio should't be following counters but...
	} else {
		string s = layout->labelstring();

		// the caption hack:
		if (layout->labeltype == LABEL_SENSITIVE) {
			Paragraph * tmppar = par;
			Inset * in = 0;
			bool isOK = false;
			while (tmppar && tmppar->inInset()
			       // the single '=' is intended below
			       && (in = tmppar->inInset()->owner())) {
				if (in->lyxCode() == Inset::FLOAT_CODE ||
				    in->lyxCode() == Inset::WRAP_CODE) {
					isOK = true;
					break;
				} else {
					tmppar = in->parOwner();
				}
			}

			if (isOK) {
				Floating const & fl
					= textclass.floats().getType(static_cast<InsetFloat*>(in)->type());

				textclass.counters().step(fl.type());

				// Doesn't work... yet.
#warning use boost.format
#if USE_BOOST_FORMAT
				s = boost::io::str(boost::format(_("%1$s #:")) % fl.name());
				// s << boost::format(_("%1$s %1$d:")
				//        % fl.name()
				//	  % buf->counters().value(fl.name());
#else
				ostringstream o;
				//o << fl.name() << ' ' << buf->counters().value(fl.name()) << ":";
				o << fl.name() << " #:";
				s = STRCONV(o.str());
#endif
			} else {
				// par->SetLayout(0);
				// s = layout->labelstring;
				s = _("Senseless: ");
			}
		}
		par->params().labelString(s);

		// reset the enumeration counter. They are always reset
		// when there is any other layout between
		// Just fall-through between the cases so that all
		// enum counters deeper than enumdepth is also reset.
		switch (par->enumdepth) {
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
	Row * row = firstRow();
	Paragraph * par = row->par();

	// CHECK if this is really needed. (Lgb)
	bv()->buffer()->params.getLyXTextClass().counters().reset();

	while (par) {
		while (row->par() != par)
			row = row->next();

		string const oldLabel = par->params().labelString();

		// setCounter can potentially change the labelString.
		setCounter(bv()->buffer(), par);

		string const & newLabel = par->params().labelString();

		if (oldLabel.empty() && !newLabel.empty()) {
			removeParagraph(row);
			appendParagraph(row);
		}

		par = par->next();
	}
}


void LyXText::insertInset(Inset * inset)
{
	if (!cursor.par()->insetAllowed(inset->lyxCode()))
		return;
	setUndo(bv(), Undo::FINISH, cursor.par(), cursor.par()->next());
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


void LyXText::copyEnvironmentType()
{
	copylayouttype = cursor.par()->layout()->name();
}


void LyXText::pasteEnvironmentType()
{
	// do nothing if there has been no previous copyEnvironmentType()
	if (!copylayouttype.empty())
		setLayout(copylayouttype);
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
	Paragraph * endpar = selection.end.par()->next();
	Paragraph * undoendpar = endpar;

	if (endpar && endpar->getDepth()) {
		while (endpar && endpar->getDepth()) {
			endpar = endpar->next();
			undoendpar = endpar;
		}
	} else if (endpar) {
		endpar = endpar->next(); // because of parindents etc.
	}

	setUndo(bv(), Undo::DELETE,
		selection.start.par(), undoendpar);

	// there are two cases: cut only within one paragraph or
	// more than one paragraph
	if (selection.start.par() == selection.end.par()) {
		// only within one paragraph
		endpar = selection.end.par();
		int pos = selection.end.pos();
		CutAndPaste::cutSelection(selection.start.par(), &endpar,
					  selection.start.pos(), pos,
					  bv()->buffer()->params.textclass,
					  doclear, realcut);
		selection.end.pos(pos);
	} else {
		endpar = selection.end.par();
		int pos = selection.end.pos();
		CutAndPaste::cutSelection(selection.start.par(), &endpar,
					  selection.start.pos(), pos,
					  bv()->buffer()->params.textclass,
					  doclear, realcut);
		cursor.par(endpar);
		selection.end.par(endpar);
		selection.end.pos(pos);
		cursor.pos(selection.end.pos());
	}
	endpar = endpar->next();

	// sometimes necessary
	if (doclear)
		selection.start.par()->stripLeadingSpaces();

	redoParagraphs(selection.start, endpar);

	// cutSelection can invalidate the cursor so we need to set
	// it anew. (Lgb)
	// we prefer the end for when tracking changes
	cursor = selection.end;

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

	CutAndPaste::copySelection(selection.start.par(), selection.end.par(),
				   selection.start.pos(), selection.end.pos(),
				   bv()->buffer()->params.textclass);
}


void LyXText::pasteSelection()
{
	// this does not make sense, if there is nothing to paste
	if (!CutAndPaste::checkPastePossible())
		return;

	setUndo(bv(), Undo::INSERT,
		cursor.par(), cursor.par()->next());

	Paragraph * endpar;
	Paragraph * actpar = cursor.par();
	int pos = cursor.pos();

	CutAndPaste::pasteSelection(&actpar, &endpar, pos,
				    bv()->buffer()->params.textclass);

	redoParagraphs(cursor, endpar);

	setCursor(cursor.par(), cursor.pos());
	clearSelection();

	selection.cursor = cursor;
	setCursor(actpar, pos);
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
	for (string::const_iterator cit = str.begin(); cit != str.end(); ++cit) {
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
	Paragraph * par = cursor.par();
	pos_type pos = cursor.pos();
	Paragraph * endpar = cursor.par()->next();

	setCursorParUndo(bv());

	// only to be sure, should not be neccessary
	clearSelection();

	bv()->buffer()->insertStringAsLines(par, pos, current_font, str);

	redoParagraphs(cursor, endpar);
	setCursor(cursor.par(), cursor.pos());
	selection.cursor = cursor;
	setCursor(par, pos);
	setSelection();
}


// turns double-CR to single CR, others where converted into one
// blank. Then InsertStringAsLines is called
void LyXText::insertStringAsParagraphs(string const & str)
{
	string linestr(str);
	bool newline_inserted = false;
	for (string::size_type i = 0; i < linestr.length(); ++i) {
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


void LyXText::checkParagraph(Paragraph * par,
			     pos_type pos)
{
	LyXCursor tmpcursor;

	int y = 0;
	pos_type z;
	Row * row = getRow(par, pos, y);

	// is there a break one row above
	if (row->previous() && row->previous()->par() == row->par()) {
		z = rowBreakPoint(*row->previous());
		if (z >= row->pos()) {
			// set the dimensions of the row above
			y -= row->previous()->height();
			postPaint(y);

			breakAgain(row->previous());

			// set the cursor again. Otherwise
			// dangling pointers are possible
			setCursor(cursor.par(), cursor.pos(),
				  false, cursor.boundary());
			selection.cursor = cursor;
			return;
		}
	}

	int const tmpheight = row->height();
	pos_type const tmplast = row->lastPos();

	breakAgain(row);
	if (row->height() == tmpheight && row->lastPos() == tmplast) {
		postRowPaint(row, y);
	} else {
		postPaint(y);
	}

	// check the special right address boxes
	if (par->layout()->margintype == MARGIN_RIGHT_ADDRESS_BOX) {
		tmpcursor.par(par);
		tmpcursor.row(row);
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

	Paragraph * par = ownerParagraph();
	do {
		pos = par->getPositionOfInset(inset);
		if (pos != -1) {
			checkParagraph(par, pos);
			return true;
		}
		par = par->next();
	} while (par);

	return false;
}


bool LyXText::setCursor(Paragraph * par,
			pos_type pos,
			bool setfont, bool boundary)
{
	LyXCursor old_cursor = cursor;
	setCursorIntern(par, pos, setfont, boundary);
	return deleteEmptyParagraphMechanism(old_cursor);
}


void LyXText::setCursor(LyXCursor & cur, Paragraph * par,
			pos_type pos, bool boundary)
{
	lyx::Assert(par);

	cur.par(par);
	cur.pos(pos);
	cur.boundary(boundary);

	// get the cursor y position in text
	int y = 0;
	Row * row = getRow(par, pos, y);
	Row * old_row = row;
	cur.irow(row);
	// if we are before the first char of this row and are still in the
	// same paragraph and there is a previous row then put the cursor on
	// the end of the previous row
	cur.iy(y + row->baseline());
	Inset * ins;
	if (row->previous() && pos &&
		row->previous()->par() == row->par() &&
	    pos < par->size() &&
		par->getChar(pos) == Paragraph::META_INSET &&
		(ins = par->getInset(pos)) && (ins->needFullRow() || ins->display()))
	{
		row = row->previous();
		y -= row->height();
	}

	cur.row(row);
	// y is now the beginning of the cursor row
	y += row->baseline();
	// y is now the cursor baseline
	cur.y(y);

	pos_type last = old_row->lastPrintablePos();

	// None of these should happen, but we're scaredy-cats
	if (pos > par->size()) {
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
	//if the cursor is in a visible row, anchor to it
	int topy = top_y();
	if (topy < y && y < topy + bv()->workHeight())
		anchor_row(row);
}


float LyXText::getCursorX(Row * row,
			  pos_type pos, pos_type last, bool boundary) const
{
	pos_type cursor_vpos = 0;
	float x;
	float fill_separator;
	float fill_hfill;
	float fill_label_hfill;
	// This call HAS to be here because of the BidiTables!!!
	prepareToPrint(row, x, fill_separator, fill_hfill,
		       fill_label_hfill);

	if (last < row->pos())
		cursor_vpos = row->pos();
	else if (pos > last && !boundary)
		cursor_vpos = (row->par()->isRightToLeftPar(bv()->buffer()->params))
			? row->pos() : last + 1;
	else if (pos > row->pos() &&
		 (pos > last || boundary))
		/// Place cursor after char at (logical) position pos - 1
		cursor_vpos = (bidi_level(pos - 1) % 2 == 0)
			? log2vis(pos - 1) + 1 : log2vis(pos - 1);
	else
		/// Place cursor before char at (logical) position pos
		cursor_vpos = (bidi_level(pos) % 2 == 0)
			? log2vis(pos) : log2vis(pos) + 1;

	pos_type body_pos = row->par()->beginningOfBody();
	if ((body_pos > 0) &&
	    ((body_pos-1 > last) ||
	     !row->par()->isLineSeparator(body_pos - 1)))
		body_pos = 0;

	for (pos_type vpos = row->pos(); vpos < cursor_vpos; ++vpos) {
		pos_type pos = vis2log(vpos);
		if (body_pos > 0 && pos == body_pos - 1) {
			x += fill_label_hfill +
				font_metrics::width(
					row->par()->layout()->labelsep,
					getLabelFont(bv()->buffer(),
						     row->par()));
			if (row->par()->isLineSeparator(body_pos - 1))
				x -= singleWidth(
						 row->par(), body_pos - 1);
		}
		if (row->hfillExpansion(pos)) {
			x += singleWidth(row->par(), pos);
			if (pos >= body_pos)
				x += fill_hfill;
			else
				x += fill_label_hfill;
		} else if (row->par()->isSeparator(pos)) {
			x += singleWidth(row->par(), pos);
			if (pos >= body_pos)
				x += fill_separator;
		} else
			x += singleWidth(row->par(), pos);
	}
	return x;
}


void LyXText::setCursorIntern(Paragraph * par,
			      pos_type pos, bool setfont, bool boundary)
{
	InsetText * it = static_cast<InsetText *>(par->inInset());
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

	setCursor(cursor, par, pos, boundary);
	if (setfont)
		setCurrentFont();
}


void LyXText::setCurrentFont()
{
	pos_type pos = cursor.pos();
	if (cursor.boundary() && pos > 0)
		--pos;

	if (pos > 0) {
		if (pos == cursor.par()->size())
			--pos;
		else // potentional bug... BUG (Lgb)
			if (cursor.par()->isSeparator(pos)) {
				if (pos > cursor.row()->pos() &&
				    bidi_level(pos) % 2 ==
				    bidi_level(pos - 1) % 2)
					--pos;
				else if (pos + 1 < cursor.par()->size())
					++pos;
			}
	}

	current_font =
		cursor.par()->getFontSettings(bv()->buffer()->params, pos);
	real_current_font = getFont(bv()->buffer(), cursor.par(), pos);

	if (cursor.pos() == cursor.par()->size() &&
	    isBoundary(bv()->buffer(), cursor.par(), cursor.pos()) &&
	    !cursor.boundary()) {
		Language const * lang =
			cursor.par()->getParLanguage(bv()->buffer()->params);
		current_font.setLanguage(lang);
		current_font.setNumber(LyXFont::OFF);
		real_current_font.setLanguage(lang);
		real_current_font.setNumber(LyXFont::OFF);
	}
}


// returns the column near the specified x-coordinate of the row
// x is set to the real beginning of this column
pos_type
LyXText::getColumnNearX(Row * row, int & x,
			bool & boundary) const
{
	float tmpx = 0.0;
	float fill_separator;
	float fill_hfill;
	float fill_label_hfill;

	prepareToPrint(row, tmpx, fill_separator,
		       fill_hfill, fill_label_hfill);

	pos_type vc = row->pos();
	pos_type last = row->lastPrintablePos();
	pos_type c = 0;

	LyXLayout_ptr const & layout = row->par()->layout();

	bool left_side = false;

	pos_type body_pos = row->par()->beginningOfBody();
	float last_tmpx = tmpx;

	if (body_pos > 0 &&
	    (body_pos - 1 > last ||
	     !row->par()->isLineSeparator(body_pos - 1)))
		body_pos = 0;

	// check for empty row
	if (!row->par()->size()) {
		x = int(tmpx);
		return 0;
	}

	while (vc <= last && tmpx <= x) {
		c = vis2log(vc);
		last_tmpx = tmpx;
		if (body_pos > 0 && c == body_pos-1) {
			tmpx += fill_label_hfill +
				font_metrics::width(layout->labelsep,
					       getLabelFont(bv()->buffer(), row->par()));
			if (row->par()->isLineSeparator(body_pos - 1))
				tmpx -= singleWidth(row->par(), body_pos-1);
		}

		if (row->hfillExpansion(c)) {
			tmpx += singleWidth(row->par(), c);
			if (c >= body_pos)
				tmpx += fill_hfill;
			else
				tmpx += fill_label_hfill;
		} else if (row->par()->isSeparator(c)) {
			tmpx += singleWidth(row->par(), c);
			if (c >= body_pos)
				tmpx+= fill_separator;
		} else {
			tmpx += singleWidth(row->par(), c);
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
	bool const lastrow = lyxrc.rtl_support // This is not needed, but gives
					 // some speedup if rtl_support=false
		&& (!row->next() || row->next()->par() != row->par());
	bool const rtl = (lastrow)
		? row->par()->isRightToLeftPar(bv()->buffer()->params)
		: false; // If lastrow is false, we don't need to compute
			 // the value of rtl.

	if (lastrow &&
		 ((rtl &&  left_side && vc == row->pos() && x < tmpx - 5) ||
		   (!rtl && !left_side && vc == last + 1   && x > tmpx + 5)))
		c = last + 1;
	else if (vc == row->pos()) {
		c = vis2log(vc);
		if (bidi_level(c) % 2 == 1)
			++c;
	} else {
		c = vis2log(vc - 1);
		bool const rtl = (bidi_level(c) % 2 == 1);
		if (left_side == rtl) {
			++c;
			boundary = isBoundary(bv()->buffer(), row->par(), c);
		}
	}

	if (row->pos() <= last && c > last
	    && row->par()->isNewline(last)) {
		if (bidi_level(last) % 2 == 0)
			tmpx -= singleWidth(row->par(), last);
		else
			tmpx += singleWidth(row->par(), last);
		c = last;
	}

	c -= row->pos();
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
	bool beforeFullRowInset(Row & row, LyXCursor & cur) {
		if (!row.next())
			return false;
		Row const & next = *row.next();

		if (next.pos() != cur.pos() || next.par() != cur.par())
			return false;
		if (!cur.par()->isInset(cur.pos()))
			return false;
		Inset const * inset = cur.par()->getInset(cur.pos());
		if (inset->needFullRow() || inset->display())
			return true;
		return false;
	}
}


void LyXText::setCursorFromCoordinates(LyXCursor & cur,
				       int x, int y)
{
	// Get the row first.

	Row * row = getRowNearY(y);
	bool bound = false;
	pos_type const column = getColumnNearX(row, x, bound);
	cur.par(row->par());
	cur.pos(row->pos() + column);
	cur.x(x);
	cur.y(y + row->baseline());
	cur.row(row);

	if (beforeFullRowInset(*row, cur)) {
		pos_type last = row->lastPrintablePos();
		float x = getCursorX(row->next(), cur.pos(), last, bound);
		cur.ix(int(x));
		cur.iy(y + row->height() + row->next()->baseline());
		cur.irow(row->next());
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
		    isBoundary(bv()->buffer(), cursor.par(), cursor.pos() + 1))
			setCursor(cursor.par(), cursor.pos() + 1, true, true);
	} else if (cursor.par()->previous()) { // steps into the above paragraph.
		Paragraph * par = cursor.par()->previous();
		setCursor(par, par->size());
	}
}


void LyXText::cursorRight(bool internal)
{
	if (!internal && cursor.boundary() &&
	    !cursor.par()->isNewline(cursor.pos()))
		setCursor(cursor.par(), cursor.pos(), true, false);
	else if (cursor.pos() < cursor.par()->size()) {
		setCursor(cursor.par(), cursor.pos() + 1, true, false);
		if (!internal &&
		    isBoundary(bv()->buffer(), cursor.par(), cursor.pos()))
			setCursor(cursor.par(), cursor.pos(), true, true);
	} else if (cursor.par()->next())
		setCursor(cursor.par()->next(), 0);
}


void LyXText::cursorUp(bool selecting)
{
#if 1
	int x = cursor.x_fix();
	int y = cursor.y() - cursor.row()->baseline() - 1;
	setCursorFromCoordinates(x, y);
	if (!selecting) {
		int topy = top_y();
		int y1 = cursor.iy() - topy;
		int y2 = y1;
		y -= topy;
		Inset * inset_hit = checkInsetHit(x, y1);
		if (inset_hit && isHighlyEditableInset(inset_hit)) {
			inset_hit->edit(bv(), x, y - (y2 - y1), mouse_button::none);
		}
	}
#else
	setCursorFromCoordinates(bv(), cursor.x_fix(),
				 cursor.y() - cursor.row()->baseline() - 1);
#endif
}


void LyXText::cursorDown(bool selecting)
{
#if 1
	int x = cursor.x_fix();
	int y = cursor.y() - cursor.row()->baseline() +
		cursor.row()->height() + 1;
	setCursorFromCoordinates(x, y);
	if (!selecting && cursor.row() == cursor.irow()) {
		int topy = top_y();
		int y1 = cursor.iy() - topy;
		int y2 = y1;
		y -= topy;
		Inset * inset_hit = checkInsetHit(x, y1);
		if (inset_hit && isHighlyEditableInset(inset_hit)) {
			inset_hit->edit(bv(), x, y - (y2 - y1), mouse_button::none);
		}
	}
#else
	setCursorFromCoordinates(bv(), cursor.x_fix(),
				 cursor.y() - cursor.row()->baseline()
				 + cursor.row()->height() + 1);
#endif
}


void LyXText::cursorUpParagraph()
{
	if (cursor.pos() > 0) {
		setCursor(cursor.par(), 0);
	}
	else if (cursor.par()->previous()) {
		setCursor(cursor.par()->previous(), 0);
	}
}


void LyXText::cursorDownParagraph()
{
	if (cursor.par()->next()) {
		setCursor(cursor.par()->next(), 0);
	} else {
		setCursor(cursor.par(), cursor.par()->size());
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
			redoParagraphs(old_cursor, old_cursor.par()->next());

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
	if (!old_cursor.par()->next() && !old_cursor.par()->previous())
		return false;

	// Do not delete empty paragraphs with keepempty set.
	if (old_cursor.par()->layout()->keepempty)
		return false;

	// only do our magic if we changed paragraph
	if (old_cursor.par() == cursor.par())
		return false;

	// record if we have deleted a paragraph
	// we can't possibly have deleted a paragraph before this point
	bool deleted = false;

	if ((old_cursor.par()->empty()
	     || (old_cursor.par()->size() == 1
		 && old_cursor.par()->isLineSeparator(0)))) {
		// ok, we will delete anything
		LyXCursor tmpcursor;

		deleted = true;

		if (old_cursor.row()->previous()) {
			const_cast<LyXText *>(this)->postPaint(old_cursor.y() - old_cursor.row()->baseline()
				  - old_cursor.row()->previous()->height());
			tmpcursor = cursor;
			cursor = old_cursor; // that undo can restore the right cursor position
			Paragraph * endpar = old_cursor.par()->next();
			if (endpar && endpar->getDepth()) {
				while (endpar && endpar->getDepth()) {
					endpar = endpar->next();
				}
			}
			setUndo(bv(), Undo::DELETE, old_cursor.par(), endpar);
			cursor = tmpcursor;

			// delete old row
			removeRow(old_cursor.row());
			if (ownerParagraph() == old_cursor.par()) {
				ownerParagraph(ownerParagraph()->next());
			}
			// delete old par
			delete old_cursor.par();

			/* Breakagain the next par. Needed because of
			 * the parindent that can occur or dissappear.
			 * The next row can change its height, if
			 * there is another layout before */
			if (refresh_row) {
				if (refresh_row->next()) {
					breakAgain(refresh_row->next());
					updateCounters();
				}
				setHeightOfRow(refresh_row);
			}
		} else {
			Row * nextrow = old_cursor.row()->next();
			const_cast<LyXText *>(this)->postPaint(
				old_cursor.y() - old_cursor.row()->baseline());

			tmpcursor = cursor;
			cursor = old_cursor; // that undo can restore the right cursor position
			Paragraph * endpar = old_cursor.par()->next();
			if (endpar && endpar->getDepth()) {
				while (endpar && endpar->getDepth()) {
					endpar = endpar->next();
				}
			}
			setUndo(bv(), Undo::DELETE, old_cursor.par(), endpar);
			cursor = tmpcursor;

			// delete old row
			removeRow(old_cursor.row());
			// delete old par
			if (ownerParagraph() == old_cursor.par()) {
				ownerParagraph(ownerParagraph()->next());
			}

			delete old_cursor.par();

			/* Breakagain the next par. Needed because of
			   the parindent that can occur or dissappear.
			   The next row can change its height, if
			   there is another layout before */
			if (nextrow) {
				breakAgain(nextrow);
				updateCounters();
			}
		}

		// correct cursor y
		setCursorIntern(cursor.par(), cursor.pos());

		if (selection.cursor.par()  == old_cursor.par()
		    && selection.cursor.pos() == old_cursor.pos()) {
			// correct selection
			selection.cursor = cursor;
		}
	}
	if (!deleted) {
		if (old_cursor.par()->stripLeadingSpaces()) {
			redoParagraphs(old_cursor,
				       old_cursor.par()->next());
			// correct cursor y
			setCursorIntern(cursor.par(), cursor.pos());
			selection.cursor = cursor;
		}
	}
	return deleted;
}


Paragraph * LyXText::ownerParagraph() const
{
	if (inset_owner) {
		return inset_owner->paragraph();
	}
	return &*(bv_owner->buffer()->paragraphs.begin());
}


void LyXText::ownerParagraph(Paragraph * p) const
{
	if (inset_owner) {
		inset_owner->paragraph(p);
	} else {
		bv_owner->buffer()->paragraphs.set(p);
	}
}


void LyXText::ownerParagraph(int id, Paragraph * p) const
{
	Paragraph * op = bv_owner->buffer()->getParFromID(id);
	if (op && op->inInset()) {
		static_cast<InsetText *>(op->inInset())->paragraph(p);
	} else {
		ownerParagraph(p);
	}
}


LyXText::refresh_status LyXText::refreshStatus() const
{
	return refresh_status_;
}


void LyXText::clearPaint()
{
	refresh_status_ = REFRESH_NONE;
	refresh_row = 0;
	refresh_y = 0;
}


void LyXText::postPaint(int start_y)
{
	refresh_status old = refresh_status_;

	refresh_status_ = REFRESH_AREA;
	refresh_row = 0;

	if (old != REFRESH_NONE && refresh_y < start_y)
		return;

	refresh_y = start_y;

	if (!inset_owner)
		return;

	// We are an inset's lyxtext. Tell the top-level lyxtext
	// it needs to update the row we're in.
	LyXText * t = bv()->text;
	t->postRowPaint(t->cursor.row(), t->cursor.y() - t->cursor.row()->baseline());
}


// FIXME: we should probably remove this y parameter,
// make refresh_y be 0, and use row->y etc.
void LyXText::postRowPaint(Row * row, int start_y)
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
	refresh_row = row;

	if (!inset_owner)
		return;

	// We are an inset's lyxtext. Tell the top-level lyxtext
	// it needs to update the row we're in.
	LyXText * t = bv()->text;
	t->postRowPaint(t->cursor.row(), t->cursor.y() - t->cursor.row()->baseline());
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
