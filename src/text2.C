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

#ifdef __GNUG__
#pragma implementation "lyxtext.h"
#endif

#include "lyxtext.h"
#include "LString.h"
#include "paragraph.h"
#include "lyxtextclasslist.h"
#include "LyXView.h"
#include "undo_funcs.h"
#include "buffer.h"
#include "bufferparams.h"
#include "gettext.h"
#include "BufferView.h"
#include "LyXView.h"
#include "CutAndPaste.h"
#include "Painter.h"
#include "font.h"
#include "debug.h"
#include "lyxrc.h"
#include "lyxrow.h"
#include "FloatList.h"
#include "language.h"
#include "ParagraphParameters.h"

#include "insets/inseterror.h"
#include "insets/insetbib.h"
#include "insets/insetspecialchar.h"
#include "insets/insettext.h"
#include "insets/insetfloat.h"

#include "support/LAssert.h"
#include "support/textutils.h"
#include "support/lstrings.h"

using std::vector;
using std::copy;
using std::find;
using std::endl;
using std::find;
using std::pair;
using lyx::pos_type;


LyXText::LyXText(BufferView * bv)
	: number_of_rows(0), height(0), width(0), first_y(0),
	  bv_owner(bv), inset_owner(0), the_locking_inset(0),
	  need_break_row(0), refresh_y(0), refresh_row(0),
	  status_(LyXText::UNCHANGED), firstrow(0), lastrow(0)
{}


LyXText::LyXText(InsetText * inset)
	:  number_of_rows(0),  height(0), width(0), first_y(0),
	   bv_owner(0), inset_owner(inset), the_locking_inset(0),
	   need_break_row(0), refresh_y(0), refresh_row(0),
	   status_(LyXText::UNCHANGED), firstrow(0), lastrow(0)
{}


void LyXText::init(BufferView * bview, bool reinit)
{
	if (reinit) {
		// Delete all rows, this does not touch the paragraphs!
		Row * tmprow = firstrow;
		while (firstrow) {
			tmprow = firstrow->next();
			delete firstrow;
			firstrow = tmprow;
		}

		lastrow = 0;
		refresh_row = 0;
		need_break_row = 0;
		width = height = 0;
		copylayouttype.erase();
		number_of_rows = first_y = refresh_y = 0;
		status_ = LyXText::UNCHANGED;
	} else if (firstrow)
		return;

	Paragraph * par = ownerParagraph();
	current_font = getFont(bview->buffer(), par, 0);

	while (par) {
		insertParagraph(bview, par, lastrow);
		par = par->next();
	}
	setCursorIntern(bview, firstrow->par(), 0);
	selection.cursor = cursor;
}


LyXText::~LyXText()
{
	// Delete all rows, this does not touch the paragraphs!
	Row * tmprow = firstrow;
	while (firstrow) {
		tmprow = firstrow->next();
		delete firstrow;
		firstrow = tmprow;
	}
}


namespace {

LyXFont const realizeFont(LyXFont const & font,
			  Buffer const * buf,
			  Paragraph * par)
{
	LyXFont tmpfont(font);
	Paragraph::depth_type par_depth = par->getDepth();

	// Resolve against environment font information
	while (par && par_depth && !tmpfont.resolved()) {
		par = par->outerHook();
		if (par) {
#ifndef INHERIT_LANGUAGE
			tmpfont.realize(textclasslist[buf->params.textclass][
					      par->layout()].font);
#else
			tmpfont.realize(textclasslist.
					Style(buf->params.textclass,
					      par->layout()).font,
					buf->params.language);
#endif
			par_depth = par->getDepth();
		}
	}

#ifndef INHERIT_LANGUAGE
	tmpfont.realize(textclasslist[buf->params.textclass].defaultfont());
#else
	tmpfont.realize(textclasslist[buf->params.textclass].defaultfont(),
			buf->params.language);
#endif

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

	LyXLayout const & layout =
		textclasslist[buf->params.textclass][par->layout()];

	Paragraph::depth_type par_depth = par->getDepth();
	// We specialize the 95% common case:
	if (!par_depth) {
		if (layout.labeltype == LABEL_MANUAL
		    && pos < beginningOfMainBody(buf, par)) {
			// 1% goes here
			LyXFont f = par->getFontSettings(buf->params, pos);
			if (par->inInset())
				par->inInset()->getDrawFont(f);
#ifndef INHERIT_LANGUAGE
			return f.realize(layout.reslabelfont);
#else
			return f.realize(layout.reslabelfont, buf->params.language);
#endif
		} else {
			LyXFont f = par->getFontSettings(buf->params, pos);
			if (par->inInset())
				par->inInset()->getDrawFont(f);
#ifndef INHERIT_LANGUAGE
			return f.realize(layout.resfont);
#else
			return f.realize(layout.resfont, buf->params.language);
#endif
		}
	}

	// The uncommon case need not be optimized as much

	LyXFont layoutfont;

	if (pos < beginningOfMainBody(buf, par)) {
		// 1% goes here
		layoutfont = layout.labelfont;
	} else {
		// 99% goes here
		layoutfont = layout.font;
	}

	LyXFont tmpfont = par->getFontSettings(buf->params, pos);
#ifndef INHERIT_LANGUAGE
	tmpfont.realize(layoutfont);
#else
	tmpfont.realize(layoutfont, buf->params.language);
#endif
	if (par->inInset())
		par->inInset()->getDrawFont(tmpfont);

	return realizeFont(tmpfont, buf, par);
}


LyXFont const LyXText::getLayoutFont(Buffer const * buf, Paragraph * par) const
{
	LyXLayout const & layout =
		textclasslist[buf->params.textclass][par->layout()];

	Paragraph::depth_type par_depth = par->getDepth();

	if (!par_depth) {
		return layout.resfont;
	}

	return realizeFont(layout.font, buf, par);
}


LyXFont const LyXText::getLabelFont(Buffer const * buf, Paragraph * par) const
{
	LyXLayout const & layout =
		textclasslist[buf->params.textclass][par->layout()];

	Paragraph::depth_type par_depth = par->getDepth();

	if (!par_depth) {
		return layout.reslabelfont;
	}

	return realizeFont(layout.labelfont, buf, par);
}


void LyXText::setCharFont(BufferView * bv, Paragraph * par,
			  pos_type pos, LyXFont const & fnt,
			  bool toggleall)
{
	Buffer const * buf = bv->buffer();
	LyXFont font = getFont(buf, par, pos);
	font.update(fnt, buf->params.language, toggleall);
	// Let the insets convert their font
	if (par->isInset(pos)) {
		Inset * inset = par->getInset(pos);
		if (isEditableInset(inset)) {
			UpdatableInset * uinset =
				static_cast<UpdatableInset *>(inset);
			uinset->setFont(bv, fnt, toggleall, true);
		}
	}

	// Plug thru to version below:
	setCharFont(buf, par, pos, font);
}


void LyXText::setCharFont(Buffer const * buf, Paragraph * par,
			  pos_type pos, LyXFont const & fnt)
{
	LyXFont font(fnt);

	LyXTextClass const & tclass = textclasslist[buf->params.textclass];
	LyXLayout const & layout = tclass[par->layout()];

	// Get concrete layout font to reduce against
	LyXFont layoutfont;

	if (pos < beginningOfMainBody(buf, par))
		layoutfont = layout.labelfont;
	else
		layoutfont = layout.font;

	// Realize against environment font information
	if (par->getDepth()) {
		Paragraph * tp = par;
		while (!layoutfont.resolved() && tp && tp->getDepth()) {
			tp = tp->outerHook();
			if (tp)
#ifndef INHERIT_LANGUAGE
				layoutfont.realize(tclass[tp->layout()].font);
#else
				layoutfont.realize(textclasslist.
						   Style(buf->params.textclass,
							 tp->layout()).font,
						   buf->params.language);
#endif
		}
	}

#ifndef INHERIT_LANGUAGE
	layoutfont.realize(tclass.defaultfont());
#else
	layoutfont.realize(tclass.defaultfont(), buf->params.language);
#endif

	// Now, reduce font against full layout font
	font.reduce(layoutfont);

	par->setFont(pos, font);
}


// inserts a new row behind the specified row, increments
// the touched counters
void LyXText::insertRow(Row * row, Paragraph * par,
			pos_type pos) const
{
	Row * tmprow = new Row;
	if (!row) {
		tmprow->previous(0);
		tmprow->next(firstrow);
		firstrow = tmprow;
	} else {
		tmprow->previous(row);
		tmprow->next(row->next());
		row->next(tmprow);
	}

	if (tmprow->next())
		tmprow->next()->previous(tmprow);

	if (tmprow->previous())
		tmprow->previous()->next(tmprow);


	tmprow->par(par);
	tmprow->pos(pos);

	if (row == lastrow)
		lastrow = tmprow;
	++number_of_rows;
}


// removes the row and reset the touched counters
void LyXText::removeRow(Row * row) const
{
	Row * row_prev = row->previous();
	if (row->next())
		row->next()->previous(row_prev);
	if (!row_prev) {
		firstrow = row->next();
//		lyx::Assert(firstrow);
	} else  {
		row_prev->next(row->next());
	}
	if (row == lastrow) {
		lyx::Assert(!row->next());
		lastrow = row_prev;
	}
	if (refresh_row == row) {
		refresh_row = row_prev ? row_prev : row->next();
		// what about refresh_y, refresh_height
	}

	height -= row->height(); // the text becomes smaller

	delete row;
	--number_of_rows;	// one row less
}


// remove all following rows of the paragraph of the specified row.
void LyXText::removeParagraph(Row * row) const
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


// insert the specified paragraph behind the specified row
void LyXText::insertParagraph(BufferView * bview, Paragraph * par,
			      Row * row) const
{
	insertRow(row, par, 0);            /* insert a new row, starting
					    * at postition 0 */

	setCounter(bview->buffer(), par);  // set the counters

	// and now append the whole paragraph behind the new row
	if (!row) {
		firstrow->height(0);
		appendParagraph(bview, firstrow);
	} else {
		row->next()->height(0);
		appendParagraph(bview, row->next());
	}
}


Inset * LyXText::getInset() const
{
	Inset * inset = 0;
	if (cursor.pos() == 0 && cursor.par()->bibkey) {
		inset =	cursor.par()->bibkey;
	} else if (cursor.pos() < cursor.par()->size()
		   && cursor.par()->isInset(cursor.pos())) {
		inset = cursor.par()->getInset(cursor.pos());
	}
	return inset;
}


void LyXText::toggleInset(BufferView * bview)
{
	Inset * inset = getInset();
	// is there an editable inset at cursor position?
	if (!isEditableInset(inset)) {
		// No, try to see if we are inside a collapsable inset
		if (inset_owner && inset_owner->owner()
		    && inset_owner->owner()->isOpen()) {
			bview->unlockInset(static_cast<UpdatableInset *>(inset_owner->owner()));
			inset_owner->owner()->close(bview);
		}
		return;
	}
	//bview->owner()->message(inset->editMessage());

	// do we want to keep this?? (JMarc)
	if (!isHighlyEditableInset(inset))
		setCursorParUndo(bview);

	if (inset->isOpen()) {
		inset->close(bview);
	} else {
		inset->open(bview);
	}
#if 0
	inset->open(bview, !inset->isOpen());
#endif
}


/* used in setlayout */
// Asger is not sure we want to do this...
void LyXText::makeFontEntriesLayoutSpecific(Buffer const * buf,
					    Paragraph * par)
{
	LyXLayout const & layout =
		textclasslist[buf->params.textclass][par->layout()];

	LyXFont layoutfont;
	for (pos_type pos = 0; pos < par->size(); ++pos) {
		if (pos < beginningOfMainBody(buf, par))
			layoutfont = layout.labelfont;
		else
			layoutfont = layout.font;

		LyXFont tmpfont = par->getFontSettings(buf->params, pos);
		tmpfont.reduce(layoutfont);
		par->setFont(pos, tmpfont);
	}
}


Paragraph * LyXText::setLayout(BufferView * bview,
			       LyXCursor & cur, LyXCursor & sstart_cur,
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

	setUndo(bview, Undo::EDIT, sstart_cur.par(), undoendpar);

	// ok we have a selection. This is always between sstart_cur
	// and sel_end cursor
	cur = sstart_cur;

	LyXLayout const & lyxlayout =
		textclasslist[bview->buffer()->params.textclass][layout];

	do {
		cur.par()->applyLayout(layout);
		makeFontEntriesLayoutSpecific(bview->buffer(), cur.par());
		Paragraph * fppar = cur.par();
		fppar->params().spaceTop(lyxlayout.fill_top ?
					 VSpace(VSpace::VFILL)
					 : VSpace(VSpace::NONE));
		fppar->params().spaceBottom(lyxlayout.fill_bottom ?
					    VSpace(VSpace::VFILL)
					    : VSpace(VSpace::NONE));
		if (lyxlayout.margintype == MARGIN_MANUAL)
			cur.par()->setLabelWidthString(lyxlayout.labelstring());
		if (lyxlayout.labeltype != LABEL_BIBLIO
		    && fppar->bibkey) {
			delete fppar->bibkey;
			fppar->bibkey = 0;
		}
		if (cur.par() != send_cur.par())
			cur.par(cur.par()->next());
	} while (cur.par() != send_cur.par());

	return endpar;
}


// set layout over selection and make a total rebreak of those paragraphs
void LyXText::setLayout(BufferView * bview, string const & layout)
{
	LyXCursor tmpcursor = cursor;  /* store the current cursor  */

	// if there is no selection just set the layout
	// of the current paragraph  */
	if (!selection.set()) {
		selection.start = cursor;  // dummy selection
		selection.end = cursor;
	}
	Paragraph * endpar = setLayout(bview, cursor, selection.start,
				       selection.end, layout);
	redoParagraphs(bview, selection.start, endpar);

	// we have to reset the selection, because the
	// geometry could have changed
	setCursor(bview, selection.start.par(),
		  selection.start.pos(), false);
	selection.cursor = cursor;
	setCursor(bview, selection.end.par(), selection.end.pos(), false);
	updateCounters(bview, cursor.row());
	clearSelection();
	setSelection(bview);
	setCursor(bview, tmpcursor.par(), tmpcursor.pos(), true);
}


// increment depth over selection and
// make a total rebreak of those paragraphs
void  LyXText::incDepth(BufferView * bview)
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

	setUndo(bview, Undo::EDIT,
		selection.start.par(), undoendpar);

	LyXCursor tmpcursor = cursor; // store the current cursor

	// ok we have a selection. This is always between sel_start_cursor
	// and sel_end cursor
	cursor = selection.start;

	bool anything_changed = false;

	LyXTextClass const & tclass =
		textclasslist[bview->buffer()->params.textclass];

	while (true) {
		// NOTE: you can't change the depth of a bibliography entry
		if (tclass[cursor.par()->layout()].labeltype != LABEL_BIBLIO) {
			Paragraph * prev = cursor.par()->previous();

			if (prev) {
				if (cursor.par()->getDepth()
				    < prev->getMaxDepthAfter(bview->buffer())){
					cursor.par()->params().depth(cursor.par()->getDepth() + 1);
					anything_changed = true;
				}
			}
		}
		if (cursor.par() == selection.end.par())
			break;
		cursor.par(cursor.par()->next());
	}

	// if nothing changed set all depth to 0
	if (!anything_changed) {
		cursor = selection.start;
		while (cursor.par() != selection.end.par()) {
			cursor.par()->params().depth(0);
			cursor.par(cursor.par()->next());
		}
		cursor.par()->params().depth(0);
	}

	redoParagraphs(bview, selection.start, endpar);

	// we have to reset the selection, because the
	// geometry could have changed
	setCursor(bview, selection.start.par(), selection.start.pos());
	selection.cursor = cursor;
	setCursor(bview, selection.end.par(), selection.end.pos());
	updateCounters(bview, cursor.row());
	clearSelection();
	setSelection(bview);
	setCursor(bview, tmpcursor.par(), tmpcursor.pos());
}


// decrement depth over selection and
// make a total rebreak of those paragraphs
void  LyXText::decDepth(BufferView * bview)
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

	setUndo(bview, Undo::EDIT,
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

	redoParagraphs(bview, selection.start, endpar);

	// we have to reset the selection, because the
	// geometry could have changed
	setCursor(bview, selection.start.par(),
		  selection.start.pos());
	selection.cursor = cursor;
	setCursor(bview, selection.end.par(), selection.end.pos());
	updateCounters(bview, cursor.row());
	clearSelection();
	setSelection(bview);
	setCursor(bview, tmpcursor.par(), tmpcursor.pos());
}


// set font over selection and make a total rebreak of those paragraphs
void LyXText::setFont(BufferView * bview, LyXFont const & font, bool toggleall)
{
	// if there is no selection just set the current_font
	if (!selection.set()) {
		// Determine basis font
		LyXFont layoutfont;
		if (cursor.pos() < beginningOfMainBody(bview->buffer(),
						       cursor.par())) {
			layoutfont = getLabelFont(bview->buffer(),
						  cursor.par());
		} else {
			layoutfont = getLayoutFont(bview->buffer(),
						   cursor.par());
		}
		// Update current font
		real_current_font.update(font,
					 bview->buffer()->params.language,
					 toggleall);

		// Reduce to implicit settings
		current_font = real_current_font;
		current_font.reduce(layoutfont);
		// And resolve it completely
#ifndef INHERIT_LANGUAGE
		real_current_font.realize(layoutfont);
#else
		real_current_font.realize(layoutfont,
					  bview->buffer()->params.language);
#endif
		return;
	}

	LyXCursor tmpcursor = cursor; // store the current cursor

	// ok we have a selection. This is always between sel_start_cursor
	// and sel_end cursor

	setUndo(bview, Undo::EDIT,
		selection.start.par(), selection.end.par()->next());
	freezeUndo();
	cursor = selection.start;
	while (cursor.par() != selection.end.par() ||
	       (cursor.pos() < selection.end.pos()))
	{
		if (cursor.pos() < cursor.par()->size()) {
			// an open footnote should behave
			// like a closed one
			setCharFont(bview, cursor.par(), cursor.pos(),
				    font, toggleall);
			cursor.pos(cursor.pos() + 1);
		} else {
			cursor.pos(0);
			cursor.par(cursor.par()->next());
		}
	}
	unFreezeUndo();

	redoParagraphs(bview, selection.start, selection.end.par()->next());

	// we have to reset the selection, because the
	// geometry could have changed, but we keep
	// it for user convenience
	setCursor(bview, selection.start.par(), selection.start.pos());
	selection.cursor = cursor;
	setCursor(bview, selection.end.par(), selection.end.pos());
	setSelection(bview);
	setCursor(bview, tmpcursor.par(), tmpcursor.pos(), true,
		  tmpcursor.boundary());
}


void LyXText::redoHeightOfParagraph(BufferView * bview, LyXCursor const & cur)
{
	Row * tmprow = cur.row();
	int y = cur.y() - tmprow->baseline();

	setHeightOfRow(bview, tmprow);

	while (tmprow->previous()
	       && tmprow->previous()->par() == tmprow->par()) {
		tmprow = tmprow->previous();
		y -= tmprow->height();
		setHeightOfRow(bview, tmprow);
	}

	// we can set the refreshing parameters now
	status(bview, LyXText::NEED_MORE_REFRESH);
	refresh_y = y;
	refresh_row = tmprow;
	setCursor(bview, cur.par(), cur.pos(), false, cursor.boundary());
}


void LyXText::redoDrawingOfParagraph(BufferView * bview, LyXCursor const & cur)
{
	Row * tmprow = cur.row();

	int y = cur.y() - tmprow->baseline();
	setHeightOfRow(bview, tmprow);

	while (tmprow->previous()
	       && tmprow->previous()->par() == tmprow->par())  {
		tmprow = tmprow->previous();
		y -= tmprow->height();
	}

	// we can set the refreshing parameters now
	if (status_ == LyXText::UNCHANGED || y < refresh_y) {
		refresh_y = y;
		refresh_row = tmprow;
	}
	status(bview, LyXText::NEED_MORE_REFRESH);
	setCursor(bview, cur.par(), cur.pos());
}


// deletes and inserts again all paragaphs between the cursor
// and the specified par
// This function is needed after SetLayout and SetFont etc.
void LyXText::redoParagraphs(BufferView * bview, LyXCursor const & cur,
			     Paragraph const * endpar) const
{
	Row * tmprow2;
	Paragraph * tmppar = 0;
	Paragraph * first_phys_par = 0;

	Row * tmprow = cur.row();

	int y = cur.y() - tmprow->baseline();

	if (!tmprow->previous()) {
		// a trick/hack for UNDO
		// This is needed because in an UNDO/REDO we could have changed
		// the ownerParagrah() so the paragraph inside the row is NOT
		// my really first par anymore. Got it Lars ;) (Jug 20011206)
		first_phys_par = ownerParagraph();
	} else {
		first_phys_par = tmprow->par();
		while (tmprow->previous()
		       && tmprow->previous()->par() == first_phys_par)
		{
			tmprow = tmprow->previous();
			y -= tmprow->height();
		}
	}

	// we can set the refreshing parameters now
	status(bview, LyXText::NEED_MORE_REFRESH);
	refresh_y = y;
	refresh_row = tmprow->previous();	 /* the real refresh row will
						be deleted, so I store
						the previous here */
	// remove it
	if (tmprow->next())
		tmppar = tmprow->next()->par();
	else
		tmppar = 0;
	while (tmprow->next() && tmppar != endpar) {
		removeRow(tmprow->next());
		if (tmprow->next()) {
			tmppar = tmprow->next()->par();
		} else {
			tmppar = 0;
		}
	}

	// remove the first one
	tmprow2 = tmprow;     /* this is because tmprow->previous()
				 can be 0 */
	tmprow = tmprow->previous();
	removeRow(tmprow2);

	tmppar = first_phys_par;

	do {
		if (tmppar) {
			insertParagraph(bview, tmppar, tmprow);
			if (!tmprow) {
				tmprow = firstrow;
			}
			while (tmprow->next()
			       && tmprow->next()->par() == tmppar) {
				tmprow = tmprow->next();
			}
			tmppar = tmppar->next();
		}
	} while (tmppar && tmppar != endpar);

	// this is because of layout changes
	if (refresh_row) {
		refresh_y -= refresh_row->height();
		setHeightOfRow(bview, refresh_row);
	} else {
		refresh_row = firstrow;
		refresh_y = 0;
		setHeightOfRow(bview, refresh_row);
	}

	if (tmprow && tmprow->next())
		setHeightOfRow(bview, tmprow->next());
}


bool LyXText::fullRebreak(BufferView * bview)
{
	if (!firstrow) {
		init(bview);
		return true;
	}
	if (need_break_row) {
		breakAgain(bview, need_break_row);
		need_break_row = 0;
		return true;
	}
	return true;
}


// important for the screen


/* the cursor set functions have a special mechanism. When they
 * realize, that you left an empty paragraph, they will delete it.
 * They also delete the corresponding row */

// need the selection cursor:
void LyXText::setSelection(BufferView * bview)
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
		inset_owner->setUpdateStatus(bview, InsetText::SELECTION);
}


string const LyXText::selectionAsString(Buffer const * buffer,
					bool label) const
{
	if (!selection.set()) return string();
	string result;

	// Special handling if the whole selection is within one paragraph
	if (selection.start.par() == selection.end.par()) {
		result += selection.start.par()->asString(buffer,
							  selection.start.pos(),
							  selection.end.pos(),
							  label);
		return result;
	}

	// The selection spans more than one paragraph

	// First paragraph in selection
	result += selection.start.par()->asString(buffer,
						  selection.start.pos(),
						  selection.start.par()->size(),
						  label)
		+ "\n\n";

	// The paragraphs in between (if any)
	LyXCursor tmpcur(selection.start);
	tmpcur.par(tmpcur.par()->next());
	while (tmpcur.par() != selection.end.par()) {
		result += tmpcur.par()->asString(buffer, 0,
						 tmpcur.par()->size(),
						 label) + "\n\n";
		tmpcur.par(tmpcur.par()->next());
	}

	// Last paragraph in selection
	result += selection.end.par()->asString(buffer, 0,
						selection.end.pos(), label);

	return result;
}


void LyXText::clearSelection() const
{
	selection.set(false);
	selection.mark(false);
	last_sel_cursor = selection.end = selection.start = selection.cursor = cursor;
	// reset this in the bv_owner!
	if (bv_owner && bv_owner->text)
		bv_owner->text->xsel_cache.set(false);
}


void LyXText::cursorHome(BufferView * bview) const
{
	setCursor(bview, cursor.par(), cursor.row()->pos());
}


void LyXText::cursorEnd(BufferView * bview) const
{
	if (!cursor.row()->next()
	    || cursor.row()->next()->par() != cursor.row()->par()) {
		setCursor(bview, cursor.par(), rowLast(cursor.row()) + 1);
	} else {
		if (cursor.par()->size() &&
		    (cursor.par()->getChar(rowLast(cursor.row())) == ' '
		     || cursor.par()->isNewline(rowLast(cursor.row())))) {
			setCursor(bview, cursor.par(), rowLast(cursor.row()));
		} else {
			setCursor(bview,cursor.par(),
				  rowLast(cursor.row()) + 1);
		}
	}
}


void LyXText::cursorTop(BufferView * bview) const
{
	while (cursor.par()->previous())
		cursor.par(cursor.par()->previous());
	setCursor(bview, cursor.par(), 0);
}


void LyXText::cursorBottom(BufferView * bview) const
{
	while (cursor.par()->next())
		cursor.par(cursor.par()->next());
	setCursor(bview, cursor.par(), cursor.par()->size());
}


void LyXText::toggleFree(BufferView * bview,
			 LyXFont const & font, bool toggleall)
{
	// If the mask is completely neutral, tell user
	if (font == LyXFont(LyXFont::ALL_IGNORE)) {
		// Could only happen with user style
		bview->owner()->message(_("No font change defined. Use Character under the Layout menu to define font change."));
		return;
	}

	// Try implicit word selection
	// If there is a change in the language the implicit word selection
	// is disabled.
	LyXCursor resetCursor = cursor;
	bool implicitSelection = (font.language() == ignore_language
				  && font.number() == LyXFont::IGNORE)
		? selectWordWhenUnderCursor(bview, WHOLE_WORD_STRICT) : false;

	// Set font
	setFont(bview, font, toggleall);

	// Implicit selections are cleared afterwards
	//and cursor is set to the original position.
	if (implicitSelection) {
		clearSelection();
		cursor = resetCursor;
		setCursor(bview, cursor.par(), cursor.pos());
		selection.cursor = cursor;
	}
	if (inset_owner)
		inset_owner->setUpdateStatus(bview, InsetText::CURSOR_PAR);
}


string LyXText::getStringToIndex(BufferView * bview)
{
	string idxstring;

	// Try implicit word selection
	// If there is a change in the language the implicit word selection
	// is disabled.
	LyXCursor resetCursor = cursor;
	bool implicitSelection = selectWordWhenUnderCursor(bview, PREVIOUS_WORD);

	if (!selection.set()) {
		bview->owner()->message(_("Nothing to index!"));
		return string();
	}
	if (selection.start.par() != selection.end.par()) {
		bview->owner()->message(_("Cannot index more than one paragraph!"));
		return string();
	}

	idxstring = selectionAsString(bview->buffer(), false);

	// Implicit selections are cleared afterwards
	//and cursor is set to the original position.
	if (implicitSelection) {
		clearSelection();
		cursor = resetCursor;
		setCursor(bview, cursor.par(), cursor.pos());
		selection.cursor = cursor;
	}
	return idxstring;
}


pos_type LyXText::beginningOfMainBody(Buffer const * buf,
			     Paragraph const * par) const
{
	if (textclasslist[buf->params.textclass][
				par->layout()].labeltype != LABEL_MANUAL)
		return 0;
	else
		return par->beginningOfMainBody();
}


/* the DTP switches for paragraphs. LyX will store them in the
* first physicla paragraph. When a paragraph is broken, the top settings
* rest, the bottom settings are given to the new one. So I can make shure,
* they do not duplicate themself and you cannnot make dirty things with
* them!  */

void LyXText::setParagraph(BufferView * bview,
			   bool line_top, bool line_bottom,
			   bool pagebreak_top, bool pagebreak_bottom,
			   VSpace const & space_top,
			   VSpace const & space_bottom,
			   Spacing const & spacing,
			   LyXAlignment align,
			   string labelwidthstring,
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

	setUndo(bview, Undo::EDIT, selection.start.par(), undoendpar);


	Paragraph * tmppar = selection.end.par();
	LyXTextClass const & tclass =
		textclasslist[bview->buffer()->params.textclass];

	while (tmppar != selection.start.par()->previous()) {
		setCursor(bview, tmppar, 0);
		status(bview, LyXText::NEED_MORE_REFRESH);
		refresh_row = cursor.row();
		refresh_y = cursor.y() - cursor.row()->baseline();
		cursor.par()->params().lineTop(line_top);
		cursor.par()->params().lineBottom(line_bottom);
		cursor.par()->params().pagebreakTop(pagebreak_top);
		cursor.par()->params().pagebreakBottom(pagebreak_bottom);
		cursor.par()->params().spaceTop(space_top);
		cursor.par()->params().spaceBottom(space_bottom);
		cursor.par()->params().spacing(spacing);
		// does the layout allow the new alignment?
		LyXLayout const & layout = tclass[cursor.par()->layout()];

		if (align == LYX_ALIGN_LAYOUT)
			align = layout.align;
		if (align & layout.alignpossible) {
			if (align == layout.align)
				cursor.par()->params().align(LYX_ALIGN_LAYOUT);
			else
				cursor.par()->params().align(align);
		}
		cursor.par()->setLabelWidthString(labelwidthstring);
		cursor.par()->params().noindent(noindent);
		tmppar = cursor.par()->previous();
	}

	redoParagraphs(bview, selection.start, endpar);

	clearSelection();
	setCursor(bview, selection.start.par(), selection.start.pos());
	selection.cursor = cursor;
	setCursor(bview, selection.end.par(), selection.end.pos());
	setSelection(bview);
	setCursor(bview, tmpcursor.par(), tmpcursor.pos());
	if (inset_owner)
		bview->updateInset(inset_owner, true);
}


char loweralphaCounter(int n)
{
	if (n < 1 || n > 26)
		return '?';
	else
		return 'a' + n - 1;
}


namespace {

inline
char alphaCounter(int n)
{
	if (n < 1 || n > 26)
		return '?';
	else
		return 'A' + n - 1;
}


inline
char hebrewCounter(int n)
{
	static const char hebrew[22] = {
		'א', 'ב', 'ג', 'ד', 'ה', 'ו', 'ז', 'ח', 'ט',
		'י', 'כ', 'ל', 'מ', 'נ', 'ס', 'ע', 'פ', 'צ',
		'ק', 'ר', 'ש', 'ת'
	};
	if (n < 1 || n > 22)
		return '?';
	else
		return hebrew[n-1];
}


inline
string const romanCounter(int n)
{
	static char const * roman[20] = {
		"i",   "ii",  "iii", "iv", "v",
		"vi",  "vii", "viii", "ix", "x",
		"xi",  "xii", "xiii", "xiv", "xv",
		"xvi", "xvii", "xviii", "xix", "xx"
	};
	if (n < 1 || n > 20)
		return "??";
	else
		return roman[n-1];
}

} // namespace anon


// set the counter of a paragraph. This includes the labels
void LyXText::setCounter(Buffer const * buf, Paragraph * par) const
{
	LyXTextClass const & textclass = textclasslist[buf->params.textclass];
	LyXLayout const & layout = textclass[par->layout()];


	// copy the prev-counters to this one,
	// unless this is the first paragraph
	if (par->previous()) {
		for (int i = 0; i < 10; ++i) {
			par->setCounter(i, par->previous()->getFirstCounter(i));
		}
		par->params().appendix(par->previous()->params().appendix());
		if (!par->params().appendix() && par->params().startOfAppendix()) {
			par->params().appendix(true);
			for (int i = 0; i < 10; ++i) {
				par->setCounter(i, 0);
			}
		}
		par->enumdepth = par->previous()->enumdepth;
		par->itemdepth = par->previous()->itemdepth;
	} else {
		for (int i = 0; i < 10; ++i) {
			par->setCounter(i, 0);
		}
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
	    && textclass[par->previous()->layout()].labeltype == LABEL_COUNTER_ENUMI
	    && par->enumdepth < 3
	    && layout.labeltype != LABEL_BIBLIO) {
		par->enumdepth++;
	}

	// Maybe we have to decrement the enumeration depth, see note above
	if (par->previous()
	    && par->previous()->getDepth() > par->getDepth()
	    && layout.labeltype != LABEL_BIBLIO) {
		par->enumdepth = par->depthHook(par->getDepth())->enumdepth;
		par->setCounter(6 + par->enumdepth,
				par->depthHook(par->getDepth())->getCounter(6 + par->enumdepth));
		/* reset the counters.
		 * A depth change is like a breaking layout
		 */
		for (int i = 6 + par->enumdepth + 1; i < 10; ++i)
			par->setCounter(i, 0);
	}

	if (!par->params().labelString().empty()) {
		par->params().labelString(string());
	}

	if (layout.margintype == MARGIN_MANUAL) {
		if (par->params().labelWidthString().empty()) {
			par->setLabelWidthString(layout.labelstring());
		}
	} else {
		par->setLabelWidthString(string());
	}

	// is it a layout that has an automatic label?
	if (layout.labeltype >=  LABEL_COUNTER_CHAPTER) {

		int i = layout.labeltype - LABEL_COUNTER_CHAPTER;
		if (i >= 0 && i<= buf->params.secnumdepth) {
			par->incCounter(i);	// increment the counter

			// Is there a label? Useful for Chapter layout
			if (!par->params().appendix()) {
				if (!layout.labelstring().empty())
					par->params().labelString(layout.labelstring());
				else
					par->params().labelString(string());
			} else {
				if (!layout.labelstring_appendix().empty())
					par->params().labelString(layout.labelstring_appendix());
				else
					par->params().labelString(string());
			}

			ostringstream s;

			if (!par->params().appendix()) {
				switch (2 * LABEL_COUNTER_CHAPTER -
					textclass.maxcounter() + i) {
				case LABEL_COUNTER_CHAPTER:
					s << par->getCounter(i);
					break;
				case LABEL_COUNTER_SECTION:
					s << par->getCounter(i - 1) << '.'
					  << par->getCounter(i);
					break;
				case LABEL_COUNTER_SUBSECTION:
					s << par->getCounter(i - 2) << '.'
					  << par->getCounter(i - 1) << '.'
					  << par->getCounter(i);
					break;
				case LABEL_COUNTER_SUBSUBSECTION:
					s << par->getCounter(i - 3) << '.'
					  << par->getCounter(i - 2) << '.'
					  << par->getCounter(i - 1) << '.'
					  << par->getCounter(i);

					break;
				case LABEL_COUNTER_PARAGRAPH:
					s << par->getCounter(i - 4) << '.'
					  << par->getCounter(i - 3) << '.'
					  << par->getCounter(i - 2) << '.'
					  << par->getCounter(i - 1) << '.'
					  << par->getCounter(i);
					break;
				case LABEL_COUNTER_SUBPARAGRAPH:
					s << par->getCounter(i - 5) << '.'
					  << par->getCounter(i - 4) << '.'
					  << par->getCounter(i - 3) << '.'
					  << par->getCounter(i - 2) << '.'
					  << par->getCounter(i - 1) << '.'
					  << par->getCounter(i);

					break;
				default:
					// Can this ever be reached? And in the
					// case it is, how can this be correct?
					// (Lgb)
					s << par->getCounter(i) << '.';
					break;
				}
			} else { // appendix
				switch (2 * LABEL_COUNTER_CHAPTER - textclass.maxcounter() + i) {
				case LABEL_COUNTER_CHAPTER:
					if (par->isRightToLeftPar(buf->params))
						s << hebrewCounter(par->getCounter(i));
					else
						s << alphaCounter(par->getCounter(i));
					break;
				case LABEL_COUNTER_SECTION:
					if (par->isRightToLeftPar(buf->params))
						s << hebrewCounter(par->getCounter(i - 1));
					else
						s << alphaCounter(par->getCounter(i - 1));

					s << '.'
					  << par->getCounter(i);

					break;
				case LABEL_COUNTER_SUBSECTION:
					if (par->isRightToLeftPar(buf->params))
						s << hebrewCounter(par->getCounter(i - 2));
					else
						s << alphaCounter(par->getCounter(i - 2));

					s << '.'
					  << par->getCounter(i-1) << '.'
					  << par->getCounter(i);

					break;
				case LABEL_COUNTER_SUBSUBSECTION:
					if (par->isRightToLeftPar(buf->params))
						s << hebrewCounter(par->getCounter(i-3));
					else
						s << alphaCounter(par->getCounter(i-3));

					s << '.'
					  << par->getCounter(i-2) << '.'
					  << par->getCounter(i-1) << '.'
					  << par->getCounter(i);

					break;
				case LABEL_COUNTER_PARAGRAPH:
					if (par->isRightToLeftPar(buf->params))
						s << hebrewCounter(par->getCounter(i-4));
					else
						s << alphaCounter(par->getCounter(i-4));

					s << '.'
					  << par->getCounter(i-3) << '.'
					  << par->getCounter(i-2) << '.'
					  << par->getCounter(i-1) << '.'
					  << par->getCounter(i);

					break;
				case LABEL_COUNTER_SUBPARAGRAPH:
					if (par->isRightToLeftPar(buf->params))
						s << hebrewCounter(par->getCounter(i-5));
					else
						s << alphaCounter(par->getCounter(i-5));

					s << '.'
					  << par->getCounter(i-4) << '.'
					  << par->getCounter(i-3) << '.'
					  << par->getCounter(i-2) << '.'
					  << par->getCounter(i-1) << '.'
					  << par->getCounter(i);

					break;
				default:
					// Can this ever be reached? And in the
					// case it is, how can this be correct?
					// (Lgb)
					s << par->getCounter(i) << '.';

					break;
				}
			}

			par->params().labelString(par->params().labelString() +s.str().c_str());
			// We really want to remove the c_str as soon as
			// possible...

			for (i++; i < 10; ++i) {
				// reset the following counters
				par->setCounter(i, 0);
			}
		} else if (layout.labeltype < LABEL_COUNTER_ENUMI) {
			for (i++; i < 10; ++i) {
				// reset the following counters
				par->setCounter(i, 0);
			}
		} else if (layout.labeltype == LABEL_COUNTER_ENUMI) {
			par->incCounter(i + par->enumdepth);
			int number = par->getCounter(i + par->enumdepth);

			ostringstream s;

			switch (par->enumdepth) {
			case 1:
				if (par->isRightToLeftPar(buf->params))
					s << '('
					  << hebrewCounter(number)
					  << ')';
				else
					s << '('
					  << loweralphaCounter(number)
					  << ')';
				break;
			case 2:
				if (par->isRightToLeftPar(buf->params))
					s << '.' << romanCounter(number);
				else
					s << romanCounter(number) << '.';
				break;
			case 3:
				if (par->isRightToLeftPar(buf->params))
					s << '.'
					  << alphaCounter(number);
				else
					s << alphaCounter(number)
					  << '.';
				break;
			default:
				if (par->isRightToLeftPar(buf->params))
					s << '.' << number;
				else
					s << number << '.';
				break;
			}

			par->params().labelString(s.str().c_str());

			for (i += par->enumdepth + 1; i < 10; ++i) {
				// reset the following counters
				par->setCounter(i, 0);
			}

		}
	} else if (layout.labeltype == LABEL_BIBLIO) {// ale970302
		int i = LABEL_COUNTER_ENUMI - LABEL_COUNTER_CHAPTER + par->enumdepth;
		par->incCounter(i);
		int number = par->getCounter(i);
		if (!par->bibkey) {
			InsetCommandParams p("bibitem" );
			par->bibkey = new InsetBibKey(p);
		}
		par->bibkey->setCounter(number);
		par->params().labelString(layout.labelstring());

		// In biblio should't be following counters but...
	} else {
		string s = layout.labelstring();

		// the caption hack:
		if (layout.labeltype == LABEL_SENSITIVE) {
			bool isOK (par->inInset() && par->inInset()->owner() &&
				   (par->inInset()->owner()->lyxCode() == Inset::FLOAT_CODE));

			if (isOK) {
				InsetFloat * tmp = static_cast<InsetFloat*>(par->inInset()->owner());
				Floating const & fl
					= floatList.getType(tmp->type());
				// We should get the correct number here too.
				s = fl.name() + " #:";
			} else {
				/* par->SetLayout(0);
				   s = layout->labelstring;  */
				s = (par->getParLanguage(buf->params)->lang() == "hebrew")
					? " :תועמשמ רסח" : "Senseless: ";
			}
		}
		par->params().labelString(s);

		/* reset the enumeration counter. They are always resetted
		 * when there is any other layout between */
		for (int i = 6 + par->enumdepth; i < 10; ++i)
			par->setCounter(i, 0);
	}
}


// Updates all counters BEHIND the row. Changed paragraphs
// with a dynamic left margin will be rebroken.
void LyXText::updateCounters(BufferView * bview, Row * row) const
{
	Paragraph * par;

	if (!row) {
		row = firstrow;
		par = row->par();
	} else {
		par = row->par()->next();
	}

	while (par) {
		while (row->par() != par)
			row = row->next();

		setCounter(bview->buffer(), par);

		// now check for the headline layouts. remember that they
		// have a dynamic left margin
		LyXTextClass const & tclass =
			textclasslist[bview->buffer()->params.textclass];
		LyXLayout const & layout = tclass[par->layout()];

		if (layout.margintype == MARGIN_DYNAMIC
		    || layout.labeltype == LABEL_SENSITIVE) {
			// Rebreak the paragraph
			removeParagraph(row);
			appendParagraph(bview, row);
		}
		par = par->next();
	}
}


void LyXText::insertInset(BufferView * bview, Inset * inset)
{
	if (!cursor.par()->insetAllowed(inset->lyxCode()))
		return;
	// I don't know if this is necessary here (Jug 20020102)
	setUndo(bview, Undo::INSERT, cursor.par(), cursor.par()->next());
	cursor.par()->insertInset(cursor.pos(), inset);
	// Just to rebreak and refresh correctly.
	// The character will not be inserted a second time
	insertChar(bview, Paragraph::META_INSET);
#if 1
	// If we enter a highly editable inset the cursor should be to before
	// the inset. This couldn't happen before as Undo was not handled inside
	// inset now after the Undo LyX tries to call inset->Edit(...) again
	// and cannot do this as the cursor is behind the inset and GetInset
	// does not return the inset!
	if (isHighlyEditableInset(inset)) {
		cursorLeft(bview, true);
	}
#endif
}


void LyXText::copyEnvironmentType()
{
	copylayouttype = cursor.par()->layout();
}


void LyXText::pasteEnvironmentType(BufferView * bview)
{
	setLayout(bview, copylayouttype);
}


void LyXText::cutSelection(BufferView * bview, bool doclear, bool realcut)
{
	// Stuff what we got on the clipboard. Even if there is no selection.

	// There is a problem with having the stuffing here in that the
	// larger the selection the slower LyX will get. This can be
	// solved by running the line below only when the selection has
	// finished. The solution used currently just works, to make it
	// faster we need to be more clever and probably also have more
	// calls to stuffClipboard. (Lgb)
	bview->stuffClipboard(selectionAsString(bview->buffer(), true));

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

	setUndo(bview, Undo::DELETE,
		selection.start.par(), undoendpar);

	// there are two cases: cut only within one paragraph or
	// more than one paragraph
	if (selection.start.par() == selection.end.par()) {
		// only within one paragraph
		endpar = selection.end.par();
		int pos = selection.end.pos();
		CutAndPaste::cutSelection(selection.start.par(), &endpar,
					  selection.start.pos(), pos,
					  bview->buffer()->params.textclass,
					  doclear, realcut);
		selection.end.pos(pos);
	} else {
		endpar = selection.end.par();
		int pos = selection.end.pos();
		CutAndPaste::cutSelection(selection.start.par(), &endpar,
					  selection.start.pos(), pos,
					  bview->buffer()->params.textclass,
					  doclear, realcut);
		cursor.par(endpar);
		selection.end.par(endpar);
		selection.end.pos(pos);
		cursor.pos(selection.end.pos());
	}
	endpar = endpar->next();

	// sometimes necessary
	if (doclear)
		selection.start.par()->stripLeadingSpaces(bview->buffer()->params.textclass);

	redoParagraphs(bview, selection.start, endpar);

	// cutSelection can invalidate the cursor so we need to set
	// it anew. (Lgb)
	cursor = selection.start;

	// need a valid cursor. (Lgb)
	clearSelection();

	setCursor(bview, cursor.par(), cursor.pos());
	selection.cursor = cursor;
	updateCounters(bview, cursor.row());
}


void LyXText::copySelection(BufferView * bview)
{
	// stuff the selection onto the X clipboard, from an explicit copy request
	bview->stuffClipboard(selectionAsString(bview->buffer(), true));

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
				   bview->buffer()->params.textclass);
}


void LyXText::pasteSelection(BufferView * bview)
{
	// this does not make sense, if there is nothing to paste
	if (!CutAndPaste::checkPastePossible(cursor.par()))
		return;

	setUndo(bview, Undo::INSERT,
		cursor.par(), cursor.par()->next());

	Paragraph * endpar;
	Paragraph * actpar = cursor.par();
	int pos = cursor.pos();

	CutAndPaste::pasteSelection(&actpar, &endpar, pos,
				    bview->buffer()->params.textclass);

	redoParagraphs(bview, cursor, endpar);

	setCursor(bview, cursor.par(), cursor.pos());
	clearSelection();

	selection.cursor = cursor;
	setCursor(bview, actpar, pos);
	setSelection(bview);
	updateCounters(bview, cursor.row());
}


// sets the selection over the number of characters of string, no check!!
void LyXText::setSelectionOverString(BufferView * bview, string const & str)
{
	if (str.empty())
		return;

	selection.cursor = cursor;
	for (string::size_type i = 0; i < str.length(); ++i)
		cursorRight(bview);
	setSelection(bview);
}


// simple replacing. The font of the first selected character is used
void LyXText::replaceSelectionWithString(BufferView * bview,
					 string const & str)
{
	setCursorParUndo(bview);
	freezeUndo();

	if (!selection.set()) { // create a dummy selection
		selection.end = cursor;
		selection.start = cursor;
	}

	// Get font setting before we cut
	pos_type pos = selection.end.pos();
	LyXFont const font = selection.start.par()
		->getFontSettings(bview->buffer()->params,
				  selection.start.pos());

	// Insert the new string
	for (string::const_iterator cit = str.begin(); cit != str.end(); ++cit) {
		selection.end.par()->insertChar(pos, (*cit), font);
		++pos;
	}

	// Cut the selection
	cutSelection(bview, true, false);

	unFreezeUndo();
}


// needed to insert the selection
void LyXText::insertStringAsLines(BufferView * bview, string const & str)
{
	Paragraph * par = cursor.par();
	pos_type pos = cursor.pos();
	Paragraph * endpar = cursor.par()->next();

	setCursorParUndo(bview);

	// only to be sure, should not be neccessary
	clearSelection();

	bview->buffer()->insertStringAsLines(par, pos, current_font, str);

	redoParagraphs(bview, cursor, endpar);
	setCursor(bview, cursor.par(), cursor.pos());
	selection.cursor = cursor;
	setCursor(bview, par, pos);
	setSelection(bview);
}


// turns double-CR to single CR, others where converted into one
// blank. Then InsertStringAsLines is called
void LyXText::insertStringAsParagraphs(BufferView * bview, string const & str)
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
	insertStringAsLines(bview, linestr);
}


bool LyXText::gotoNextInset(BufferView * bview,
			    vector<Inset::Code> const & codes,
			    string const & contents) const
{
	LyXCursor res = cursor;
	Inset * inset;
	do {
		if (res.pos() < res.par()->size() - 1) {
			res.pos(res.pos() + 1);
		} else  {
			res.par(res.par()->next());
			res.pos(0);
		}

	} while (res.par() &&
		 !(res.par()->isInset(res.pos())
		   && (inset = res.par()->getInset(res.pos())) != 0
		   && find(codes.begin(), codes.end(), inset->lyxCode())
		   != codes.end()
		   && (contents.empty() ||
		       static_cast<InsetCommand *>(res.par()->getInset(res.pos()))->getContents()
		       == contents)));

	if (res.par()) {
		setCursor(bview, res.par(), res.pos(), false);
		return true;
	}
	return false;
}


void LyXText::checkParagraph(BufferView * bview, Paragraph * par,
			     pos_type pos)
{
	LyXCursor tmpcursor;

	int y = 0;
	pos_type z;
	Row * row = getRow(par, pos, y);

	// is there a break one row above
	if (row->previous() && row->previous()->par() == row->par()) {
		z = nextBreakPoint(bview, row->previous(), workWidth(bview));
		if (z >= row->pos()) {
			// set the dimensions of the row above
			y -= row->previous()->height();
			refresh_y = y;
			refresh_row = row->previous();
			status(bview, LyXText::NEED_MORE_REFRESH);

			breakAgain(bview, row->previous());

			// set the cursor again. Otherwise
			// dangling pointers are possible
			setCursor(bview, cursor.par(), cursor.pos(),
				  false, cursor.boundary());
			selection.cursor = cursor;
			return;
		}
	}

	int const tmpheight = row->height();
	pos_type const tmplast = rowLast(row);
	refresh_y = y;
	refresh_row = row;

	breakAgain(bview, row);
	if (row->height() == tmpheight && rowLast(row) == tmplast)
		status(bview, LyXText::NEED_VERY_LITTLE_REFRESH);
	else
		status(bview, LyXText::NEED_MORE_REFRESH);

	// check the special right address boxes
	if (textclasslist[bview->buffer()->params.textclass][
				par->layout()].margintype
	    == MARGIN_RIGHT_ADDRESS_BOX)
	{
		tmpcursor.par(par);
		tmpcursor.row(row);
		tmpcursor.y(y);
		tmpcursor.x(0);
		tmpcursor.x_fix(0);
		tmpcursor.pos(pos);
		redoDrawingOfParagraph(bview, tmpcursor);
	}

	// set the cursor again. Otherwise dangling pointers are possible
	// also set the selection

	if (selection.set()) {
		tmpcursor = cursor;
		setCursorIntern(bview, selection.cursor.par(), selection.cursor.pos(),
				false, selection.cursor.boundary());
		selection.cursor = cursor;
		setCursorIntern(bview, selection.start.par(),
				selection.start.pos(),
				false, selection.start.boundary());
		selection.start = cursor;
		setCursorIntern(bview, selection.end.par(),
				selection.end.pos(),
				false, selection.end.boundary());
		selection.end = cursor;
		setCursorIntern(bview, last_sel_cursor.par(),
				last_sel_cursor.pos(),
				false, last_sel_cursor.boundary());
		last_sel_cursor = cursor;
		cursor = tmpcursor;
	}
	setCursorIntern(bview, cursor.par(), cursor.pos(),
			false, cursor.boundary());
}


// returns false if inset wasn't found
bool LyXText::updateInset(BufferView * bview, Inset * inset)
{
	// first check the current paragraph
	int pos = cursor.par()->getPositionOfInset(inset);
	if (pos != -1) {
		checkParagraph(bview, cursor.par(), pos);
		return true;
	}

	// check every paragraph

	Paragraph * par = ownerParagraph();
	do {
		pos = par->getPositionOfInset(inset);
		if (pos != -1) {
			checkParagraph(bview, par, pos);
			return true;
		}
		par = par->next();
	} while (par);

	return false;
}


bool LyXText::setCursor(BufferView * bview, Paragraph * par,
			pos_type pos,
			bool setfont, bool boundary) const
{
	LyXCursor old_cursor = cursor;
	setCursorIntern(bview, par, pos, setfont, boundary);
	return deleteEmptyParagraphMechanism(bview, old_cursor);
}


void LyXText::setCursor(BufferView * bview, LyXCursor & cur, Paragraph * par,
			pos_type pos, bool boundary) const
{
	lyx::Assert(par);
	lyx::Assert(bview);

	cur.par(par);
	cur.pos(pos);
	cur.boundary(boundary);

	// get the cursor y position in text
	int y = 0;
	Row * row = getRow(par, pos, y);
	Row * old_row = row;
	// if we are before the first char of this row and are still in the
	// same paragraph and there is a previous row then put the cursor on
	// the end of the previous row
	cur.iy(y + row->baseline());
	Inset * ins;
	if (row->previous() && pos &&
		par->getChar(pos) == Paragraph::META_INSET &&
		(ins=par->getInset(pos)) && (ins->needFullRow() || ins->display()))
	{
		row = row->previous();
		y -= row->height();
	}

	cur.row(row);
	// y is now the beginning of the cursor row
	y += row->baseline();
	// y is now the cursor baseline
	cur.y(y);

	// now get the cursors x position
	float x;
	float fill_separator;
	float fill_hfill;
	float fill_label_hfill;
	prepareToPrint(bview, row, x, fill_separator, fill_hfill,
		       fill_label_hfill);
	pos_type cursor_vpos = 0;
	pos_type last = rowLastPrintable(old_row);

	if (pos > last + 1) {
		// This shouldn't happen.
		pos = last + 1;
		cur.pos(pos);
	} else if (pos < row->pos()) {
		pos = row->pos();
		cur.pos(pos);
	}

	if (last < row->pos())
		cursor_vpos = row->pos();
	else if (pos > last && !boundary)
		cursor_vpos = (row->par()->isRightToLeftPar(bview->buffer()->params))
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

	pos_type main_body =
		beginningOfMainBody(bview->buffer(), row->par());
	if ((main_body > 0) &&
	    ((main_body-1 > last) ||
	     !row->par()->isLineSeparator(main_body-1)))
		main_body = 0;

	for (pos_type vpos = row->pos(); vpos < cursor_vpos; ++vpos) {
		pos = vis2log(vpos);
		if (main_body > 0 && pos == main_body - 1) {
			x += fill_label_hfill +
				lyxfont::width(textclasslist[
						       bview->buffer()->params.textclass][
					row->par()->layout()]
					       .labelsep,
					       getLabelFont(bview->buffer(), row->par()));
			if (row->par()->isLineSeparator(main_body-1))
				x -= singleWidth(bview, row->par(),main_body-1);
		}
		if (hfillExpansion(bview->buffer(), row, pos)) {
			x += singleWidth(bview, row->par(), pos);
			if (pos >= main_body)
				x += fill_hfill;
			else
				x += fill_label_hfill;
		} else if (row->par()->isSeparator(pos)) {
			x += singleWidth(bview, row->par(), pos);
			if (pos >= main_body)
				x += fill_separator;
		} else
			x += singleWidth(bview, row->par(), pos);
	}

	cur.x(int(x));
	cur.x_fix(cur.x());
}


void LyXText::setCursorIntern(BufferView * bview, Paragraph * par,
			      pos_type pos, bool setfont, bool boundary) const
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
			// it->getLyXText(bview)->setCursorIntern(bview, par, pos, setfont, boundary);
			return;
		}
	}

	setCursor(bview, cursor, par, pos, boundary);
	if (setfont)
		setCurrentFont(bview);
}


void LyXText::setCurrentFont(BufferView * bview) const
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
		cursor.par()->getFontSettings(bview->buffer()->params, pos);
	real_current_font = getFont(bview->buffer(), cursor.par(), pos);

	if (cursor.pos() == cursor.par()->size() &&
	    isBoundary(bview->buffer(), cursor.par(), cursor.pos()) &&
	    !cursor.boundary()) {
		Language const * lang =
			cursor.par()->getParLanguage(bview->buffer()->params);
		current_font.setLanguage(lang);
		current_font.setNumber(LyXFont::OFF);
		real_current_font.setLanguage(lang);
		real_current_font.setNumber(LyXFont::OFF);
	}
}


void LyXText::setCursorFromCoordinates(BufferView * bview, int x, int y) const
{
	LyXCursor old_cursor = cursor;

	setCursorFromCoordinates(bview, cursor, x, y);
	setCurrentFont(bview);
	deleteEmptyParagraphMechanism(bview, old_cursor);
}


void LyXText::setCursorFromCoordinates(BufferView * bview, LyXCursor & cur,
				       int x, int y) const
{
	// Get the row first.

	Row * row = getRowNearY(y);
	bool bound = false;
	pos_type const column = getColumnNearX(bview, row, x, bound);
	cur.par(row->par());
	cur.pos(row->pos() + column);
	cur.x(x);
	cur.y(y + row->baseline());
	Inset * ins;
	if (row->next() && cur.pos() &&
		cur.par()->getChar(cur.pos()) == Paragraph::META_INSET &&
		(ins=cur.par()->getInset(cur.pos())) &&
		(ins->needFullRow() || ins->display()))
	{
		cur.iy(y + row->height() + row->next()->baseline());
	} else {
		cur.iy(cur.y());
	}
	cur.row(row);
	cur.boundary(bound);
}


void LyXText::cursorLeft(BufferView * bview, bool internal) const
{
	if (cursor.pos() > 0) {
		bool boundary = cursor.boundary();
		setCursor(bview, cursor.par(), cursor.pos() - 1, true, false);
		if (!internal && !boundary &&
		    isBoundary(bview->buffer(), cursor.par(), cursor.pos() + 1))
			setCursor(bview, cursor.par(), cursor.pos() + 1, true, true);
	} else if (cursor.par()->previous()) { // steps into the above paragraph.
		Paragraph * par = cursor.par()->previous();
		setCursor(bview, par, par->size());
	}
}


void LyXText::cursorRight(BufferView * bview, bool internal) const
{
	if (!internal && cursor.boundary() &&
	    !cursor.par()->isNewline(cursor.pos()))
		setCursor(bview, cursor.par(), cursor.pos(), true, false);
	else if (cursor.pos() < cursor.par()->size()) {
		setCursor(bview, cursor.par(), cursor.pos() + 1, true, false);
		if (!internal &&
		    isBoundary(bview->buffer(), cursor.par(), cursor.pos()))
			setCursor(bview, cursor.par(), cursor.pos(), true, true);
	} else if (cursor.par()->next())
		setCursor(bview, cursor.par()->next(), 0);
}


void LyXText::cursorUp(BufferView * bview) const
{
	setCursorFromCoordinates(bview, cursor.x_fix(),
				 cursor.y() - cursor.row()->baseline() - 1);
}


void LyXText::cursorDown(BufferView * bview) const
{
	setCursorFromCoordinates(bview, cursor.x_fix(),
				 cursor.y() - cursor.row()->baseline()
				 + cursor.row()->height() + 1);
}


void LyXText::cursorUpParagraph(BufferView * bview) const
{
	if (cursor.pos() > 0) {
		setCursor(bview, cursor.par(), 0);
	}
	else if (cursor.par()->previous()) {
		setCursor(bview, cursor.par()->previous(), 0);
	}
}


void LyXText::cursorDownParagraph(BufferView * bview) const
{
	if (cursor.par()->next()) {
		setCursor(bview, cursor.par()->next(), 0);
	} else {
		setCursor(bview, cursor.par(), cursor.par()->size());
	}
}

// fix the cursor `cur' after a characters has been deleted at `where'
// position. Called by deleteEmptyParagraphMechanism
void LyXText::fixCursorAfterDelete(BufferView * bview,
				   LyXCursor & cur,
				   LyXCursor const & where) const
{
	// if cursor is not in the paragraph where the delete occured,
	// do nothing
	if (cur.par() != where.par())
		return;

	// if cursor position is after the place where the delete occured,
	// update it
	if (cur.pos() > where.pos())
		cur.pos(cur.pos()-1);

	// recompute row et al. for this cursor
	setCursor(bview, cur, cur.par(), cur.pos(), cur.boundary());
}


bool LyXText::deleteEmptyParagraphMechanism(BufferView * bview,
					    LyXCursor const & old_cursor) const
{
	// Would be wrong to delete anything if we have a selection.
	if (selection.set())
		return false;

	// We allow all kinds of "mumbo-jumbo" when freespacing.
	if (textclasslist[bview->buffer()->params.textclass][
				old_cursor.par()->layout()].free_spacing
	    || old_cursor.par()->isFreeSpacing())
	{
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
			redoParagraphs(bview, old_cursor, old_cursor.par()->next());

#ifdef WITH_WARNINGS
#warning This will not work anymore when we have multiple views of the same buffer
// In this case, we will have to correct also the cursors held by
// other bufferviews. It will probably be easier to do that in a more
// automated way in LyXCursor code. (JMarc 26/09/2001)
#endif
			// correct all cursors held by the LyXText
			fixCursorAfterDelete(bview, cursor, old_cursor);
			fixCursorAfterDelete(bview, selection.cursor,
					     old_cursor);
			fixCursorAfterDelete(bview, selection.start,
					     old_cursor);
			fixCursorAfterDelete(bview, selection.end, old_cursor);
			fixCursorAfterDelete(bview, last_sel_cursor,
					     old_cursor);
			fixCursorAfterDelete(bview, toggle_cursor, old_cursor);
			fixCursorAfterDelete(bview, toggle_end_cursor,
					     old_cursor);
			return false;
		}
	}

	// don't delete anything if this is the ONLY paragraph!
	if (!old_cursor.par()->next() && !old_cursor.par()->previous())
		return false;

	// Do not delete empty paragraphs with keepempty set.
	if (textclasslist
	    [bview->buffer()->params.textclass]
	    [old_cursor.par()->layout()].keepempty)
		return false;

	// only do our magic if we changed paragraph
	if (old_cursor.par() == cursor.par())
		return false;

	// record if we have deleted a paragraph
	// we can't possibly have deleted a paragraph before this point
	bool deleted = false;

	if ((old_cursor.par()->size() == 0
	     || (old_cursor.par()->size() == 1
		 && old_cursor.par()->isLineSeparator(0)))) {
		// ok, we will delete anything
		LyXCursor tmpcursor;

		// make sure that you do not delete any environments
		status(bview, LyXText::NEED_MORE_REFRESH);
		deleted = true;

		if (old_cursor.row()->previous()) {
			refresh_row = old_cursor.row()->previous();
			refresh_y = old_cursor.y() - old_cursor.row()->baseline() - refresh_row->height();
			tmpcursor = cursor;
			cursor = old_cursor; // that undo can restore the right cursor position
			Paragraph * endpar = old_cursor.par()->next();
			if (endpar && endpar->getDepth()) {
				while (endpar && endpar->getDepth()) {
					endpar = endpar->next();
				}
			}
			setUndo(bview, Undo::DELETE, old_cursor.par(), endpar);
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
			if (refresh_row->next()) {
				breakAgain(bview, refresh_row->next());
				updateCounters(bview, refresh_row);
			}
			setHeightOfRow(bview, refresh_row);
		} else {
			refresh_row = old_cursor.row()->next();
			refresh_y = old_cursor.y() - old_cursor.row()->baseline();

			tmpcursor = cursor;
			cursor = old_cursor; // that undo can restore the right cursor position
			Paragraph * endpar = old_cursor.par()->next();
			if (endpar && endpar->getDepth()) {
				while (endpar && endpar->getDepth()) {
					endpar = endpar->next();
				}
			}
			setUndo(bview, Undo::DELETE, old_cursor.par(), endpar);
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
			if (refresh_row) {
				breakAgain(bview, refresh_row);
				updateCounters(bview, refresh_row->previous());
			}
		}

		// correct cursor y
		setCursorIntern(bview, cursor.par(), cursor.pos());

		if (selection.cursor.par()  == old_cursor.par()
		    && selection.cursor.pos() == old_cursor.pos()) {
			// correct selection
			selection.cursor = cursor;
		}
	}
	if (!deleted) {
		if (old_cursor.par()->stripLeadingSpaces(bview->buffer()->params.textclass)) {
			redoParagraphs(bview, old_cursor,
				       old_cursor.par()->next());
			// correct cursor y
			setCursorIntern(bview, cursor.par(), cursor.pos());
			selection.cursor = cursor;
		}
	}
	return deleted;
}


void LyXText::toggleAppendix(BufferView * bview)
{
	Paragraph * par = cursor.par();
	bool start = !par->params().startOfAppendix();

	// ensure that we have only one start_of_appendix in this document
	Paragraph * tmp = ownerParagraph();
	for (; tmp; tmp = tmp->next()) {
		tmp->params().startOfAppendix(false);
	}

	par->params().startOfAppendix(start);

	// we can set the refreshing parameters now
	status(bview, LyXText::NEED_MORE_REFRESH);
	refresh_y = 0;
	refresh_row = 0; // not needed for full update
	updateCounters(bview, 0);
	setCursor(bview, cursor.par(), cursor.pos());
}


Paragraph * LyXText::ownerParagraph() const
{
	if (inset_owner) {
		return inset_owner->paragraph();
	}
	return bv_owner->buffer()->paragraph;
}


void LyXText::ownerParagraph(Paragraph * p) const
{
	if (inset_owner) {
		inset_owner->paragraph(p);
	} else {
		bv_owner->buffer()->paragraph = p;
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


LyXText::text_status LyXText::status() const
{
	return status_;
}


void LyXText::status(BufferView * bview, LyXText::text_status st) const
{
	// well as much as I know && binds more then || so the above and the
	// below are identical (this for your known use of parentesis!)
	// Now some explanation:
	// We should only go up with refreshing code so this means that if
	// we have a MORE refresh we should never set it to LITTLE if we still
	// didn't handle it (and then it will be UNCHANGED. Now as long as
	// we stay inside one LyXText this may work but we need to tell the
	// outermost LyXText that it should REALLY draw us if there is some
	// change in a Inset::LyXText. So you see that when we are inside a
	// inset's LyXText we give the LITTLE to the outermost LyXText to
	// tell'em that it should redraw the actual row (where the inset
	// resides! Capito?!

	if ((status_ != NEED_MORE_REFRESH)
	    || (status_ == NEED_MORE_REFRESH
		&& st != NEED_VERY_LITTLE_REFRESH))
	{
		status_ = st;
		if (inset_owner && st != UNCHANGED) {
			bview->text->status(bview, NEED_VERY_LITTLE_REFRESH);
			if (!bview->text->refresh_row) {
				bview->text->refresh_row = bview->text->cursor.row();
				bview->text->refresh_y = bview->text->cursor.y() -
					bview->text->cursor.row()->baseline();
			}
		}
	}
}
