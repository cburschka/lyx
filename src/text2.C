/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#include FORMS_H_LOCATION


#ifdef __GNUG__
#pragma implementation "lyxtext.h"
#endif

#include "LString.h"
#include "lyxparagraph.h"
#include "insets/inseterror.h"
#include "insets/insetbib.h"
#include "insets/insetspecialchar.h"
#include "insets/insettext.h"
#include "insets/insetfloat.h"
#include "layout.h"
#include "LyXView.h"
#include "support/textutils.h"
#include "undo.h"
#include "minibuffer.h"
#include "buffer.h"
#include "bufferparams.h"
#include "lyx_gui_misc.h"
#include "lyxtext.h"
#include "gettext.h"
#include "BufferView.h"
#include "LyXView.h"
#include "lyxrow.h"
#include "CutAndPaste.h"
#include "Painter.h"
#include "font.h"
#include "debug.h"
#include "lyxrc.h"
#include "FloatList.h"

using std::copy;
using std::endl;
using std::pair;


LyXText::LyXText(BufferView * bv)
{
	bv_owner = bv;
	inset_owner = 0;
	init();
}


LyXText::LyXText(InsetText * inset)
{
	inset_owner = inset;
	bv_owner = 0;
	init();
}


void LyXText::init()
{
	the_locking_inset = 0;
	firstrow = 0;
	lastrow = 0;
	number_of_rows = 0;
	refresh_y = 0;
	height = 0;
	width = 0;
	first = 0;
	status = LyXText::UNCHANGED;
	// set cursor at the very top position
	selection = true;	    /* these setting is necessary 
				       because of the delete-empty-
				       paragraph mechanism in
				       SetCursor */
	if (bv_owner) {
		LyXParagraph * par = OwnerParagraph();
		current_font = GetFont(bv_owner->buffer(), par, 0);
		while (par) {
			InsertParagraph(bv_owner, par, lastrow);
			par = par->Next();
		}
		SetCursor(bv_owner, firstrow->par(), 0);
	} else
		current_font = LyXFont(LyXFont::ALL_SANE);

	sel_cursor = cursor;
	selection = false;
	mark_set = false;
   
	// no rebreak necessary
	need_break_row = 0;
   
	undo_finished = true;
	undo_frozen = false;

	// Default layouttype for copy environment type
	copylayouttype = 0;

#if 0
	// Dump all rowinformation:
	Row * tmprow = firstrow;
	lyxerr << "Baseline Paragraph Pos Height Ascent Fill\n";
	while (tmprow) {
		lyxerr << tmprow->baseline() << '\t'
		       << tmprow->par << '\t'
		       << tmprow->pos() << '\t'
		       << tmprow->height << '\t'
		       << tmprow->ascent_of_text << '\t'
		       << tmprow->fill << '\n';
		tmprow = tmprow->next();
	}
	lyxerr.flush();
#endif
}


void LyXText::init(BufferView * bview)
{
	if (firstrow)
		return;

	LyXParagraph * par = OwnerParagraph();
	current_font = GetFont(bview->buffer(), par, 0);
	while (par) {
		InsertParagraph(bview, par, lastrow);
		par = par->Next();
	}
	SetCursorIntern(bview, firstrow->par(), 0);
	sel_cursor = cursor;
#if 0
	// Dump all rowinformation:
	Row * tmprow = firstrow;
	lyxerr << "Width = " << width << endl;
	lyxerr << "Baseline Paragraph Pos Height Ascent Fill\n";
	while (tmprow) {
		lyxerr << tmprow->baseline() << '\t'
		       << tmprow->par() << '\t'
		       << tmprow->pos() << '\t'
		       << tmprow->height() << '\t'
		       << tmprow->ascent_of_text() << '\t'
		       << tmprow->fill() << '\n';
		tmprow = tmprow->next();
	}
	lyxerr.flush();
#endif
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


// Gets the fully instantiated font at a given position in a paragraph
// Basically the same routine as LyXParagraph::getFont() in paragraph.C.
// The difference is that this one is used for displaying, and thus we
// are allowed to make cosmetic improvements. For instance make footnotes
// smaller. (Asger)
// If position is -1, we get the layout font of the paragraph.
// If position is -2, we get the font of the manual label of the paragraph.
LyXFont const LyXText::GetFont(Buffer const * buf, LyXParagraph * par,
			 LyXParagraph::size_type pos) const
{
	LyXLayout const & layout = 
		textclasslist.Style(buf->params.textclass, par->GetLayout());

	char par_depth = par->GetDepth();
	// We specialize the 95% common case:
	if (
#ifndef NEW_INSETS
		par->footnoteflag == LyXParagraph::NO_FOOTNOTE &&
#endif
		!par_depth) {
		if (pos >= 0){
			// 95% goes here
			if (layout.labeltype == LABEL_MANUAL
			    && pos < BeginningOfMainBody(buf, par)) {
				// 1% goes here
				LyXFont f = par->GetFontSettings(buf->params,
								 pos);
				return f.realize(layout.reslabelfont);
			} else {
				LyXFont f = par->GetFontSettings(buf->params, pos);
				return f.realize(layout.resfont);
			}
			
		} else {
			// 5% goes here.
			// process layoutfont for pos == -1 and labelfont for pos < -1
			if (pos == -1)
				return layout.resfont;
			else
				return layout.reslabelfont;
		}
	}

	// The uncommon case need not be optimized as much

	LyXFont layoutfont, tmpfont;

	if (pos >= 0){
		// 95% goes here
		if (pos < BeginningOfMainBody(buf, par)) {
			// 1% goes here
			layoutfont = layout.labelfont;
		} else {
			// 99% goes here
			layoutfont = layout.font;
		}
		tmpfont = par->GetFontSettings(buf->params, pos);
		tmpfont.realize(layoutfont);
	} else {
		// 5% goes here.
		// process layoutfont for pos == -1 and labelfont for pos < -1
		if (pos == -1)
			tmpfont = layout.font;
		else
			tmpfont = layout.labelfont;
	}

	// Resolve against environment font information
	while (par && par_depth && !tmpfont.resolved()) {
		par = par->DepthHook(par_depth - 1);
		if (par) {
			tmpfont.realize(textclasslist.
					Style(buf->params.textclass,
					      par->GetLayout()).font);
			par_depth = par->GetDepth();
		}
	}

	tmpfont.realize(textclasslist.TextClass(buf->params.textclass).defaultfont());

#ifndef NEW_INSETS
	// Cosmetic improvement: If this is an open footnote, make the font 
	// smaller.
	if (par->footnoteflag == LyXParagraph::OPEN_FOOTNOTE
	    && par->footnotekind == LyXParagraph::FOOTNOTE) {
		tmpfont.decSize();
	}
#endif
	return tmpfont;
}


void LyXText::SetCharFont(Buffer const * buf, LyXParagraph * par,
			  LyXParagraph::size_type pos,
			  LyXFont const & fnt)
{
	LyXFont font(fnt);
	// Let the insets convert their font
	if (par->GetChar(pos) == LyXParagraph::META_INSET) {
		if (par->GetInset(pos))
			font = par->GetInset(pos)->ConvertFont(font);
	}

	LyXLayout const & layout =
		textclasslist.Style(buf->params.textclass,
				    par->GetLayout());

	// Get concrete layout font to reduce against
	LyXFont layoutfont;

	if (pos < BeginningOfMainBody(buf, par))
		layoutfont = layout.labelfont;
	else
		layoutfont = layout.font;

	// Realize against environment font information
	if (par->GetDepth()){
		LyXParagraph * tp = par;
		while (!layoutfont.resolved() && tp && tp->GetDepth()) {
			tp = tp->DepthHook(tp->GetDepth()-1);
			if (tp)
				layoutfont.realize(textclasslist.
						Style(buf->params.textclass,
						      tp->GetLayout()).font);
		}
	}

	layoutfont.realize(textclasslist.TextClass(buf->params.textclass).defaultfont());

#ifndef NEW_INSETS
	if (par->footnoteflag == LyXParagraph::OPEN_FOOTNOTE
	    && par->footnotekind == LyXParagraph::FOOTNOTE) {
		layoutfont.decSize();
	}
#endif
	// Now, reduce font against full layout font
	font.reduce(layoutfont);

	par->SetFont(pos, font);
}


/* inserts a new row behind the specified row, increments
 * the touched counters */
void LyXText::InsertRow(Row * row, LyXParagraph * par,
			LyXParagraph::size_type pos) const
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
	++number_of_rows; // one more row
}


// removes the row and reset the touched counters
void LyXText::RemoveRow(Row * row) const
{
	/* this must not happen before the currentrow for clear reasons.
	   so the trick is just to set the current row onto the previous
	   row of this row */
	int unused_y;
	GetRow(row->par(), row->pos(), unused_y);
   
	if (row->next())
		row->next()->previous(row->previous());
	if (!row->previous()) {
		firstrow = row->next();
	} else  {
		row->previous()->next(row->next());
	}
	if (row == lastrow)
		lastrow = row->previous();
   
	height -= row->height(); // the text becomes smaller
   
	delete row;
	--number_of_rows;	// one row less
}


// remove all following rows of the paragraph of the specified row.
void LyXText::RemoveParagraph(Row * row) const
{
	LyXParagraph * tmppar = row->par();
	row = row->next();
    
	Row * tmprow;
	while (row && row->par() == tmppar) {
		tmprow = row->next();
		RemoveRow(row);
		row = tmprow;
	}
}
   

// insert the specified paragraph behind the specified row
void LyXText::InsertParagraph(BufferView * bview, LyXParagraph * par,
			      Row * row) const
{
	InsertRow(row, par, 0);	       /* insert a new row, starting 
					* at postition 0 */

	SetCounter(bview->buffer(), par);  // set the counters
   
	// and now append the whole paragraph behind the new row
	if (!row) {
		firstrow->height(0);
		AppendParagraph(bview, firstrow);
	} else {
		row->next()->height(0);
		AppendParagraph(bview, row->next());
	}
}


#ifndef NEW_INSETS
void LyXText::ToggleFootnote(BufferView * bview)
{
	LyXParagraph * par = cursor.par()->ParFromPos(cursor.pos());
	if (par->next
	    && par->next->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE) {
		OpenFootnote(bview);
		bview->owner()->getMiniBuffer()->Set(_("Opened float"));
	} else {
		bview->owner()->getMiniBuffer()->Set(_("Closed float"));
		CloseFootnote(bview);
	}
}
#endif


#ifndef NEW_INSETS
void LyXText::OpenStuff(BufferView * bview)
{
     	if (cursor.pos() == 0 && cursor.par()->bibkey){
		cursor.par()->bibkey->Edit(bview, 0, 0, 0);
	} else if (cursor.pos() < cursor.par()->Last() 
		 && cursor.par()->GetChar(cursor.pos()) == LyXParagraph::META_INSET
		 && cursor.par()->GetInset(cursor.pos())->Editable()) {
		bview->owner()->getMiniBuffer()
			->Set(cursor.par()->GetInset(cursor.pos())->EditMessage());
		if (cursor.par()->GetInset(cursor.pos())->Editable() != Inset::HIGHLY_EDITABLE)
			SetCursorParUndo(bview->buffer());
		cursor.par()->GetInset(cursor.pos())->Edit(bview, 0, 0, 0);
	}
#ifndef NEW_INSETS
	else {
		ToggleFootnote(bview);
	}
#endif
}
#endif


#ifndef NEW_INSETS
void LyXText::CloseFootnote(BufferView * bview)
{
	LyXParagraph * tmppar;
	LyXParagraph * par = cursor.par()->ParFromPos(cursor.pos());
   
	// if the cursor is not in an open footnote, or 
	// there is no open footnote in this paragraph, just return.
	if (cursor.par()->footnoteflag != LyXParagraph::OPEN_FOOTNOTE) {
      
		if (!par->next ||
		    par->next->footnoteflag != LyXParagraph::OPEN_FOOTNOTE) {
			bview->owner()->getMiniBuffer()
				->Set(_("Nothing to do"));
			return;
		}
   
		// ok, move the cursor right before the footnote
 		// just a little faster than using CursorRight()
		for (cursor.pos(0);
		     cursor.par()->ParFromPos(cursor.pos()) != par;) {
				cursor.pos(cursor.pos() + 1);
			}
		
		// now the cursor is at the beginning of the physical par
		SetCursor(bview, cursor.par(),
			  cursor.pos() +
			  cursor.par()->ParFromPos(cursor.pos())->size());
	} else  {
		/* we are in a footnote, so let us move at the beginning */ 
		/* this is just faster than using just CursorLeft() */ 
       
		tmppar = cursor.par();
		while (tmppar->footnoteflag == LyXParagraph::OPEN_FOOTNOTE) {
			// just a little bit faster than movin the cursor
			tmppar = tmppar->Previous();
		}
		SetCursor(bview, tmppar, tmppar->Last());
	}
   
	// the cursor must be exactly before the footnote
	par = cursor.par()->ParFromPos(cursor.pos());
   
	status = LyXText::NEED_MORE_REFRESH;
	refresh_row = cursor.row();
	refresh_y = cursor.y() - cursor.row()->baseline();
   
	tmppar = cursor.par();
	LyXParagraph * endpar = par->NextAfterFootnote()->Next();
	Row * row = cursor.row();
   
	tmppar->CloseFootnote(cursor.pos());

	while (tmppar != endpar) {
		RemoveRow(row->next());
		if (row->next())
			tmppar = row->next()->par();
		else
			tmppar = 0;
	}
   
	AppendParagraph(bview, cursor.row());
   
	SetCursor(bview, cursor.par(), cursor.pos());
	sel_cursor = cursor;
   
	// just necessary
	if (cursor.row()->next())
		SetHeightOfRow(bview, cursor.row()->next());
}
#endif


/* used in setlayout */
// Asger is not sure we want to do this...
void LyXText::MakeFontEntriesLayoutSpecific(Buffer const * buf,
					    LyXParagraph * par)
{
   
	LyXLayout const & layout =
		textclasslist.Style(buf->params.textclass, par->GetLayout());

	LyXFont layoutfont, tmpfont;
	for (LyXParagraph::size_type pos = 0;
	     pos < par->Last(); ++pos) {
		if (pos < BeginningOfMainBody(buf, par))
			layoutfont = layout.labelfont;
		else
			layoutfont = layout.font;
      
		tmpfont = par->GetFontSettings(buf->params, pos);
		tmpfont.reduce(layoutfont);
		par->SetFont(pos, tmpfont);
	}
}


LyXParagraph * LyXText::SetLayout(BufferView * bview,
				  LyXCursor & cur, LyXCursor & sstart_cur,
				  LyXCursor & send_cur,
				  LyXTextClass::size_type layout)
{
#ifndef NEW_INSETS
	LyXParagraph * endpar = send_cur.par()->LastPhysicalPar()->Next();
#else
	LyXParagraph * endpar = send_cur.par()->Next();
#endif
	LyXParagraph * undoendpar = endpar;

	if (endpar && endpar->GetDepth()) {
		while (endpar && endpar->GetDepth()) {
#ifndef NEW_INSETS
			endpar = endpar->LastPhysicalPar()->Next();
#else
			endpar = endpar->Next();
#endif
			undoendpar = endpar;
		}
	} else if (endpar) {
		endpar = endpar->Next(); // because of parindents etc.
	}
   
	SetUndo(bview->buffer(), Undo::EDIT,
#ifndef NEW_INSETS
		sstart_cur.par()->ParFromPos(sstart_cur.pos())->previous,
#else
		sstart_cur.par()->previous,
#endif
		undoendpar);

	/* ok we have a selection. This is always between sstart_cur
	 * and sel_end cursor */ 
	cur = sstart_cur;
   
	LyXLayout const & lyxlayout =
		textclasslist.Style(bview->buffer()->params.textclass, layout);
   
	while (cur.par() != send_cur.par()) {
#ifndef NEW_INSETS
		if (cur.par()->footnoteflag == sstart_cur.par()->footnoteflag) {
#endif
			cur.par()->SetLayout(bview->buffer()->params, layout);
			MakeFontEntriesLayoutSpecific(bview->buffer(), cur.par());
#ifndef NEW_INSETS
			LyXParagraph * fppar = cur.par()->FirstPhysicalPar();
#else
			LyXParagraph * fppar = cur.par();
#endif
			fppar->added_space_top = lyxlayout.fill_top ?
				VSpace(VSpace::VFILL) : VSpace(VSpace::NONE);
			fppar->added_space_bottom = lyxlayout.fill_bottom ? 
				VSpace(VSpace::VFILL) : VSpace(VSpace::NONE); 
			if (lyxlayout.margintype == MARGIN_MANUAL)
				cur.par()->SetLabelWidthString(lyxlayout.labelstring());
			if (lyxlayout.labeltype != LABEL_BIBLIO
			    && fppar->bibkey) {
				delete fppar->bibkey;
				fppar->bibkey = 0;
			}
#ifndef NEW_INSETS
		}
#endif
		cur.par(cur.par()->Next());
	}
#ifndef NEW_INSETS
	if (cur.par()->footnoteflag == sstart_cur.par()->footnoteflag) {
#endif
		cur.par()->SetLayout(bview->buffer()->params, layout);
		MakeFontEntriesLayoutSpecific(bview->buffer(), cur.par());
#ifndef NEW_INSETS
		LyXParagraph * fppar = cur.par()->FirstPhysicalPar();
#else
		LyXParagraph * fppar = cur.par();
#endif
		fppar->added_space_top = lyxlayout.fill_top ?
			VSpace(VSpace::VFILL) : VSpace(VSpace::NONE);
		fppar->added_space_bottom = lyxlayout.fill_bottom ? 
			VSpace(VSpace::VFILL) : VSpace(VSpace::NONE); 
		if (lyxlayout.margintype == MARGIN_MANUAL)
			cur.par()->SetLabelWidthString(lyxlayout.labelstring());
		if (lyxlayout.labeltype != LABEL_BIBLIO
		    && fppar->bibkey) {
			delete fppar->bibkey;
			fppar->bibkey = 0;
		}
#ifndef NEW_INSETS
	}
#endif
	return endpar;
}

// set layout over selection and make a total rebreak of those paragraphs
void LyXText::SetLayout(BufferView * bview, LyXTextClass::size_type layout)
{
	LyXCursor tmpcursor = cursor;  /* store the current cursor  */

	// if there is no selection just set the layout
	// of the current paragraph  */
	if (!selection) {
		sel_start_cursor = cursor;  // dummy selection
		sel_end_cursor = cursor;
	}
	LyXParagraph *
		endpar = SetLayout(bview, cursor, sel_start_cursor,
				   sel_end_cursor, layout);
	RedoParagraphs(bview, sel_start_cursor, endpar);
   
	// we have to reset the selection, because the
	// geometry could have changed
	SetCursor(bview, sel_start_cursor.par(),
		  sel_start_cursor.pos(), false);
	sel_cursor = cursor;
	SetCursor(bview, sel_end_cursor.par(), sel_end_cursor.pos(),
		  false);
	UpdateCounters(bview, cursor.row());
	ClearSelection();
	SetSelection();
	SetCursor(bview, tmpcursor.par(), tmpcursor.pos(), true);
}


// increment depth over selection and
// make a total rebreak of those paragraphs
void  LyXText::IncDepth(BufferView * bview)
{
	// If there is no selection, just use the current paragraph
	if (!selection) {
		sel_start_cursor = cursor; // dummy selection
		sel_end_cursor = cursor;
	}

	// We end at the next paragraph with depth 0
	LyXParagraph * endpar =
#ifndef NEW_INSETS
		sel_end_cursor.par()->LastPhysicalPar()->Next();
#else
		sel_end_cursor.par()->Next();
#endif
	LyXParagraph * undoendpar = endpar;

	if (endpar && endpar->GetDepth()) {
		while (endpar && endpar->GetDepth()) {
#ifndef NEW_INSETS
			endpar = endpar->LastPhysicalPar()->Next();
#else
			endpar = endpar->Next();
#endif
			undoendpar = endpar;
		}
	}
	else if (endpar) {
		endpar = endpar->Next(); // because of parindents etc.
	}
	
	SetUndo(bview->buffer(), Undo::EDIT,
#ifndef NEW_INSETS
		sel_start_cursor
		.par()->ParFromPos(sel_start_cursor.pos())->previous,
#else
		sel_start_cursor.par()->previous,
#endif
		undoendpar);

	LyXCursor tmpcursor = cursor; // store the current cursor

	// ok we have a selection. This is always between sel_start_cursor
	// and sel_end cursor
	cursor = sel_start_cursor;
   
	bool anything_changed = false;
   
	while (true) {
		// NOTE: you can't change the depth of a bibliography entry
		if (
#ifndef NEW_INSETS
			cursor.par()->footnoteflag ==
		    sel_start_cursor.par()->footnoteflag &&
#endif
		    textclasslist.Style(bview->buffer()->params.textclass,
				      cursor.par()->GetLayout()
				     ).labeltype != LABEL_BIBLIO) {
			LyXParagraph * prev =
#ifndef NEW_INSETS
				cursor.par()->FirstPhysicalPar()->Previous();
#else
				cursor.par()->Previous();
#endif
			if (prev 
			    && (prev->GetDepth() - cursor.par()->GetDepth() > 0
				|| (prev->GetDepth() == cursor.par()->GetDepth()
				    && textclasslist.Style(bview->buffer()->params.textclass,
						      prev->GetLayout()).isEnvironment()))) {
#ifndef NEW_INSETS
				cursor.par()->FirstPhysicalPar()->depth++;
#else
				cursor.par()->depth++;
#endif
				anything_changed = true;
				}
		}
		if (cursor.par() == sel_end_cursor.par())
                       break;
		cursor.par(cursor.par()->Next());
	}
   
	// if nothing changed set all depth to 0
	if (!anything_changed) {
		cursor = sel_start_cursor;
		while (cursor.par() != sel_end_cursor.par()) {
#ifndef NEW_INSETS
			cursor.par()->FirstPhysicalPar()->depth = 0;
#else
			cursor.par()->depth = 0;
#endif
			cursor.par(cursor.par()->Next());
		}
#ifndef NEW_INSETS
		if (cursor.par()->footnoteflag == sel_start_cursor.par()->footnoteflag)
			cursor.par()->FirstPhysicalPar()->depth = 0;
#else
			cursor.par()->depth = 0;
#endif
	}
   
	RedoParagraphs(bview, sel_start_cursor, endpar);
   
	// we have to reset the selection, because the
	// geometry could have changed
	SetCursor(bview, sel_start_cursor.par(),
		  sel_start_cursor.pos());
	sel_cursor = cursor;
	SetCursor(bview, sel_end_cursor.par(), sel_end_cursor.pos());
	UpdateCounters(bview, cursor.row());
	ClearSelection();
	SetSelection();
	SetCursor(bview, tmpcursor.par(), tmpcursor.pos());
}


// decrement depth over selection and
// make a total rebreak of those paragraphs
void  LyXText::DecDepth(BufferView * bview)
{
	// if there is no selection just set the layout
	// of the current paragraph
	if (!selection) {
		sel_start_cursor = cursor; // dummy selection
		sel_end_cursor = cursor;
	}
#ifndef NEW_INSETS
	LyXParagraph * endpar = sel_end_cursor.par()->LastPhysicalPar()->Next();
#else
	LyXParagraph * endpar = sel_end_cursor.par()->Next();
#endif
	LyXParagraph * undoendpar = endpar;

	if (endpar && endpar->GetDepth()) {
		while (endpar && endpar->GetDepth()) {
#ifndef NEW_INSETS
			endpar = endpar->LastPhysicalPar()->Next();
#else
			endpar = endpar->Next();
#endif
			undoendpar = endpar;
		}
	}
	else if (endpar) {
		endpar = endpar->Next(); // because of parindents etc.
	}
   
	SetUndo(bview->buffer(), Undo::EDIT,
#ifndef NEW_INSETS
		sel_start_cursor
		.par()->ParFromPos(sel_start_cursor.pos())->previous,
#else
		sel_start_cursor.par()->previous,
#endif
		undoendpar);

	LyXCursor tmpcursor = cursor; // store the current cursor

	// ok we have a selection. This is always between sel_start_cursor
	// and sel_end cursor
	cursor = sel_start_cursor;

	while (true) {
#ifndef NEW_INSETS
		if (cursor.par()->footnoteflag ==
		    sel_start_cursor.par()->footnoteflag) {
			if (cursor.par()->FirstPhysicalPar()->depth)
				cursor.par()->FirstPhysicalPar()->depth--;
		}
#else
			if (cursor.par()->depth)
				cursor.par()->depth--;
#endif
		if (cursor.par() == sel_end_cursor.par())
			break;
		cursor.par(cursor.par()->Next());
	}

	RedoParagraphs(bview, sel_start_cursor, endpar);
   
	// we have to reset the selection, because the
	// geometry could have changed
	SetCursor(bview, sel_start_cursor.par(),
		  sel_start_cursor.pos());
	sel_cursor = cursor;
	SetCursor(bview, sel_end_cursor.par(), sel_end_cursor.pos());
	UpdateCounters(bview, cursor.row());
	ClearSelection();
	SetSelection();
	SetCursor(bview, tmpcursor.par(), tmpcursor.pos());
}


// set font over selection and make a total rebreak of those paragraphs
void LyXText::SetFont(BufferView * bview, LyXFont const & font, bool toggleall)
{
	// if there is no selection just set the current_font
	if (!selection) {
		// Determine basis font
		LyXFont layoutfont;
		if (cursor.pos() < BeginningOfMainBody(bview->buffer(),
						       cursor.par()))
			layoutfont = GetFont(bview->buffer(), cursor.par(),-2);
		else
			layoutfont = GetFont(bview->buffer(), cursor.par(),-1);
		// Update current font
		real_current_font.update(font,
					 bview->buffer()->params.language,
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
   
	SetUndo(bview->buffer(), Undo::EDIT,
#ifndef NEW_INSETS
		sel_start_cursor.par()->ParFromPos(sel_start_cursor.pos())->previous, 
		sel_end_cursor.par()->ParFromPos(sel_end_cursor.pos())->next
#else
		sel_start_cursor.par()->previous, 
		sel_end_cursor.par()->next
#endif
		); 
	cursor = sel_start_cursor;
	while (cursor.par() != sel_end_cursor.par() ||
	       (
#ifndef NEW_INSETS
		       cursor.par()->footnoteflag == sel_start_cursor.par()->footnoteflag &&
#endif
		cursor.pos() < sel_end_cursor.pos())) 
	{
		if (cursor.pos() < cursor.par()->Last()
#ifndef NEW_INSETS
		    && cursor.par()->footnoteflag
		    == sel_start_cursor.par()->footnoteflag
#endif
			) {
			// an open footnote should behave
			// like a closed one
			LyXFont newfont = GetFont(bview->buffer(), 
						  cursor.par(), cursor.pos());
			newfont.update(font,
				       bview->buffer()->params.language,
				       toggleall);
			SetCharFont(bview->buffer(),
				    cursor.par(), cursor.pos(), newfont);
			cursor.pos(cursor.pos() + 1);
		} else {
			cursor.pos(0);
			cursor.par(cursor.par()->Next());
		}
	}
   
	RedoParagraphs(bview, sel_start_cursor, sel_end_cursor.par()->Next());
   
	// we have to reset the selection, because the
	// geometry could have changed
	SetCursor(bview, sel_start_cursor.par(), sel_start_cursor.pos());
	sel_cursor = cursor;
	SetCursor(bview, sel_end_cursor.par(), sel_end_cursor.pos());
	ClearSelection();
	SetSelection();
	SetCursor(bview, tmpcursor.par(), tmpcursor.pos(), true,
		  tmpcursor.boundary());
}


void LyXText::RedoHeightOfParagraph(BufferView * bview, LyXCursor const & cur)
{
	Row * tmprow = cur.row();
	int y = cur.y() - tmprow->baseline();

	SetHeightOfRow(bview, tmprow);
#ifndef NEW_INSETS
	LyXParagraph * first_phys_par = tmprow->par()->FirstPhysicalPar();
#else
	LyXParagraph * first_phys_par = tmprow->par();
#endif
	// find the first row of the paragraph
	if (first_phys_par != tmprow->par())
		while (tmprow->previous()
		       && tmprow->previous()->par() != first_phys_par) {
			tmprow = tmprow->previous();
			y -= tmprow->height();
			SetHeightOfRow(bview, tmprow);
		}
	while (tmprow->previous() && tmprow->previous()->par() == first_phys_par) {
		tmprow = tmprow->previous();
		y -= tmprow->height();
		SetHeightOfRow(bview, tmprow);
	}
	
	// we can set the refreshing parameters now
	status = LyXText::NEED_MORE_REFRESH;
	refresh_y = y;
	refresh_row = tmprow;
	SetCursor(bview, cur.par(), cur.pos(), false, cursor.boundary());
}


void LyXText::RedoDrawingOfParagraph(BufferView * bview, LyXCursor const & cur)
{
	Row * tmprow = cur.row();
   
	int y = cur.y() - tmprow->baseline();
	SetHeightOfRow(bview, tmprow);
#ifndef NEW_INSETS
	LyXParagraph * first_phys_par = tmprow->par()->FirstPhysicalPar();
#else
	LyXParagraph * first_phys_par = tmprow->par();
#endif
	// find the first row of the paragraph
	if (first_phys_par != tmprow->par())
		while (tmprow->previous() && tmprow->previous()->par() != first_phys_par)  {
			tmprow = tmprow->previous();
			y -= tmprow->height();
		}
	while (tmprow->previous() && tmprow->previous()->par() == first_phys_par)  {
		tmprow = tmprow->previous();
		y -= tmprow->height();
	}
   
	// we can set the refreshing parameters now
	if (status == LyXText::UNCHANGED || y < refresh_y) {
		refresh_y = y;
		refresh_row = tmprow;
	}
	status = LyXText::NEED_MORE_REFRESH;
	SetCursor(bview, cur.par(), cur.pos());
}


/* deletes and inserts again all paragaphs between the cursor
* and the specified par 
* This function is needed after SetLayout and SetFont etc. */
void LyXText::RedoParagraphs(BufferView * bview, LyXCursor const & cur,
			     LyXParagraph const * endpar) const
{
	Row * tmprow2;
	LyXParagraph * tmppar = 0, * first_phys_par = 0;
   
	Row * tmprow = cur.row();
   
	int y = cur.y() - tmprow->baseline();
   
	if (!tmprow->previous()){
		first_phys_par = FirstParagraph();   // a trick/hack for UNDO
	} else {
#ifndef NEW_INSETS
		first_phys_par = tmprow->par()->FirstPhysicalPar();
#else
		first_phys_par = tmprow->par();
#endif
		// find the first row of the paragraph
		if (first_phys_par != tmprow->par())
			while (tmprow->previous() &&
			       (tmprow->previous()->par() != first_phys_par)) {
				tmprow = tmprow->previous();
				y -= tmprow->height();
			}
		while (tmprow->previous()
		       && tmprow->previous()->par() == first_phys_par) {
			tmprow = tmprow->previous();
			y -= tmprow->height();
		}
	}
   
	// we can set the refreshing parameters now
	status = LyXText::NEED_MORE_REFRESH;
	refresh_y = y;
	refresh_row = tmprow->previous();	 /* the real refresh row will
					    be deleted, so I store
					    the previous here */ 
	// remove it
	if (tmprow->next())
		tmppar = tmprow->next()->par();
	else
		tmppar = 0;
	while (tmppar != endpar) {
		RemoveRow(tmprow->next());
		if (tmprow->next())
			tmppar = tmprow->next()->par();
		else
			tmppar = 0;
	}  
   
	// remove the first one
	tmprow2 = tmprow;     /* this is because tmprow->previous()
				 can be 0 */
	tmprow = tmprow->previous();
	RemoveRow(tmprow2);
   
	tmppar = first_phys_par;

	do {
		if (tmppar) {
			InsertParagraph(bview, tmppar, tmprow);
			if (!tmprow)
				tmprow = firstrow;
			while (tmprow->next() && tmprow->next()->par() == tmppar)
				tmprow = tmprow->next();
			tmppar = tmppar->Next();
		}
	} while (tmppar != endpar);
   
	// this is because of layout changes
	if (refresh_row) {
		refresh_y -= refresh_row->height();
		SetHeightOfRow(bview, refresh_row);   
	} else {
		refresh_row = firstrow;
		refresh_y = 0;
		SetHeightOfRow(bview, refresh_row);   
	}
   
	if (tmprow && tmprow->next())
		SetHeightOfRow(bview, tmprow->next());
}


bool LyXText::FullRebreak(BufferView * bview)
{
	if (!firstrow) {
		init(bview);
		return true;
	}
	if (need_break_row) {
		BreakAgain(bview, need_break_row);
		need_break_row = 0;
		return true;
	}
	return false;
}


/* important for the screen */


/* the cursor set functions have a special mechanism. When they
 * realize, that you left an empty paragraph, they will delete it.
 * They also delete the corresponding row */
   
// need the selection cursor:
void LyXText::SetSelection()
{
	if (!selection) {
		last_sel_cursor = sel_cursor;
		sel_start_cursor = sel_cursor;
		sel_end_cursor = sel_cursor;
	}
   
	selection = true;
   
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

	if (sel_cursor.par() == cursor.par())
	   if (sel_cursor.pos() < cursor.pos()) {
		sel_end_cursor = cursor;
		sel_start_cursor = sel_cursor;
	} else {
   		sel_end_cursor = sel_cursor; 
		sel_start_cursor = cursor;
	}
	else if (sel_cursor.y() < cursor.y() ||
	    (sel_cursor.y() == cursor.y() && sel_cursor.x() < cursor.x())) {
		sel_end_cursor = cursor;
		sel_start_cursor = sel_cursor;
	}
	else {
		sel_end_cursor = sel_cursor; 
		sel_start_cursor = cursor;
	}
   
	// a selection with no contents is not a selection
	if (sel_start_cursor.par() == sel_end_cursor.par() && 
	    sel_start_cursor.pos() == sel_end_cursor.pos())
		selection = false;
}


string const LyXText::selectionAsString(Buffer const * buffer) const
{
	if (!selection) return string();
	string result;
	
	// Special handling if the whole selection is within one paragraph
	if (sel_start_cursor.par() == sel_end_cursor.par()) {
		result += sel_start_cursor.par()->String(buffer,
							 sel_start_cursor.pos(),
							 sel_end_cursor.pos());
		return result;
	}
	
	// The selection spans more than one paragraph

	// First paragraph in selection
	result += sel_start_cursor.par()->String(buffer,
						 sel_start_cursor.pos(),
						 sel_start_cursor.par()->Last())
		+ "\n\n";
	
	// The paragraphs in between (if any)
	LyXCursor tmpcur(sel_start_cursor);
	tmpcur.par(tmpcur.par()->Next());
	while (tmpcur.par() != sel_end_cursor.par()) {
		result += tmpcur.par()->String(buffer, 0, tmpcur.par()->Last()) + "\n\n";
		tmpcur.par(tmpcur.par()->Next()); // Or NextAfterFootnote??
	}

	// Last paragraph in selection
	result += sel_end_cursor.par()->String(buffer, 0, sel_end_cursor.pos());
	
	return result;
}


void LyXText::ClearSelection() const
{
	if (selection)
		status = LyXText::NEED_MORE_REFRESH;
	selection = false;
	mark_set = false;
}


void LyXText::CursorHome(BufferView * bview) const
{
	SetCursor(bview, cursor.par(), cursor.row()->pos());
}


void LyXText::CursorEnd(BufferView * bview) const
{
	if (!cursor.row()->next() || cursor.row()->next()->par() != cursor.row()->par())
		SetCursor(bview, cursor.par(), RowLast(cursor.row()) + 1);
	else {
		if (cursor.par()->Last() && 
		    (cursor.par()->GetChar(RowLast(cursor.row())) == ' '
		     || cursor.par()->IsNewline(RowLast(cursor.row()))))
			SetCursor(bview, cursor.par(), RowLast(cursor.row()));
		else
			SetCursor(bview,cursor.par(), RowLast(cursor.row()) + 1);
	}
}


void  LyXText::CursorTop(BufferView * bview) const
{
	while (cursor.par()->Previous())
		cursor.par(cursor.par()->Previous());
	SetCursor(bview, cursor.par(), 0);
}


void  LyXText::CursorBottom(BufferView * bview) const
{
	while (cursor.par()->Next())
		cursor.par(cursor.par()->Next());
	SetCursor(bview, cursor.par(), cursor.par()->Last());
}
   
   
/* returns a pointer to the row near the specified y-coordinate
* (relative to the whole text). y is set to the real beginning
* of this row */
Row * LyXText::GetRowNearY(int & y) const
{
	Row * tmprow = firstrow;
	int tmpy = 0;

	while (tmprow->next() && tmpy + tmprow->height() <= y) {
		tmpy += tmprow->height();
		tmprow = tmprow->next();
	}

	y = tmpy;   // return the real y
	return tmprow;
}


void LyXText::ToggleFree(BufferView * bview,
			 LyXFont const & font, bool toggleall)
{
	// If the mask is completely neutral, tell user
	if (font == LyXFont(LyXFont::ALL_IGNORE)) {
		// Could only happen with user style
		bview->owner()->getMiniBuffer()
			->Set(_("No font change defined. Use Character under"
				" the Layout menu to define font change."));
		return;
	}

	// Try implicit word selection
	// If there is a change in the language the implicit word selection 
	// is disabled.
	LyXCursor resetCursor = cursor;
	bool implicitSelection = (font.language() == ignore_language
				  && font.number() == LyXFont::IGNORE)
		? SelectWordWhenUnderCursor(bview) : false;

	// Set font
	SetFont(bview, font, toggleall);

	/* Implicit selections are cleared afterwards and cursor is set to the
	   original position. */
	if (implicitSelection) {
		ClearSelection();
		cursor = resetCursor;
		SetCursor(bview, cursor.par(), cursor.pos());
		sel_cursor = cursor;
	}
}


LyXParagraph::size_type
LyXText::BeginningOfMainBody(Buffer const * buf,
			     LyXParagraph const * par) const
{
	if (textclasslist.Style(buf->params.textclass,
				par->GetLayout()).labeltype != LABEL_MANUAL)
		return 0;
	else
		return par->BeginningOfMainBody();
}


#ifndef NEW_INSETS
/* if there is a selection, reset every environment you can find
* in the selection, otherwise just the environment you are in */ 
void LyXText::MeltFootnoteEnvironment(BufferView * bview)
{
	LyXParagraph * tmppar, * firsttmppar;
   
	ClearSelection();
   
	/* is is only allowed, if the cursor is IN an open footnote.
	 * Otherwise it is too dangerous */ 
	if (cursor.par()->footnoteflag != LyXParagraph::OPEN_FOOTNOTE)
		return;
   
	SetUndo(bview->buffer(), Undo::FINISH, 
		cursor.par()->PreviousBeforeFootnote()->previous,
		cursor.par()->NextAfterFootnote()->next);

	/* ok, move to the beginning of the footnote. */ 
	while (cursor.par()->footnoteflag == LyXParagraph::OPEN_FOOTNOTE)
		cursor.par(cursor.par()->Previous());
   
	SetCursor(bview, cursor.par(), cursor.par()->Last());
	/* this is just faster than using CursorLeft(); */ 
   
	firsttmppar = cursor.par()->ParFromPos(cursor.pos());
	tmppar = firsttmppar;
	/* tmppar is now the paragraph right before the footnote */

	bool first_footnote_par_is_not_empty = tmppar->next->size();
   
	while (tmppar->next
	       && tmppar->next->footnoteflag == LyXParagraph::OPEN_FOOTNOTE) {
		tmppar = tmppar->next;	 /* I use next instead of Next(),
					  * because there cannot be any
					  * footnotes in a footnote
					  * environment */
		tmppar->footnoteflag = LyXParagraph::NO_FOOTNOTE;
      
		/* remember the captions and empty paragraphs */
		if ((textclasslist.Style(bview->buffer()->params.textclass,
					 tmppar->GetLayout())
		     .labeltype == LABEL_SENSITIVE)
		    || !tmppar->Last())
			tmppar->SetLayout(bview->buffer()->params, 0);
	}
   
	// now we will paste the ex-footnote, if the layouts allow it
	// first restore the layout of the paragraph right behind
	// the footnote
	if (tmppar->next) 
		tmppar->next->MakeSameLayout(cursor.par());

	// first the end
	if (!tmppar->GetLayout()
	    || (tmppar->Next()
		&& (!tmppar->Next()->Last()
		    || tmppar->Next()->HasSameLayout(tmppar)))) {
		if (tmppar->Next()->Last()
		    && tmppar->Next()->IsLineSeparator(0))
			tmppar->Next()->Erase(0);
		tmppar->PasteParagraph(bview->buffer()->params);
	}

	tmppar = tmppar->Next();  /* make sure tmppar cannot be touched
				   * by the pasting of the beginning */

	/* then the beginning */ 
	/* if there is no space between the text and the footnote, so we insert
	 * a blank 
	 * (only if the previous par and the footnotepar are not empty!) */
	if (!firsttmppar->next->GetLayout()
	    || firsttmppar->HasSameLayout(firsttmppar->next)) {
		if (firsttmppar->size()
		    && !firsttmppar->IsSeparator(firsttmppar->size() - 1)
		    && first_footnote_par_is_not_empty) {
			firsttmppar->next->InsertChar(0, ' ');
		}
		firsttmppar->PasteParagraph(bview->buffer()->params);
	}
   
	/* now redo the paragaphs */
	RedoParagraphs(bview, cursor, tmppar);
   
	SetCursor(bview, cursor.par(), cursor.pos());
   
	/* sometimes it can happen, that there is a counter change */ 
	Row * row = cursor.row();
	while (row->next() && row->par() != tmppar && row->next()->par() != tmppar)
		row = row->next();
	UpdateCounters(bview, row);
   
   
	ClearSelection();
}
#endif


/* the DTP switches for paragraphs. LyX will store them in the 
* first physicla paragraph. When a paragraph is broken, the top settings 
* rest, the bottom settings are given to the new one. So I can make shure, 
* they do not duplicate themself and you cannnot make dirty things with 
* them!  */ 

void LyXText::SetParagraph(BufferView * bview,
			   bool line_top, bool line_bottom,
			   bool pagebreak_top, bool pagebreak_bottom,
			   VSpace const & space_top,
			   VSpace const & space_bottom,
			   LyXAlignment align, 
			   string labelwidthstring,
			   bool noindent) 
{
	LyXCursor tmpcursor = cursor;
	if (!selection) {
		sel_start_cursor = cursor;
		sel_end_cursor = cursor;
	}

	// make sure that the depth behind the selection are restored, too
#ifndef NEW_INSETS
	LyXParagraph * endpar = sel_end_cursor.par()->LastPhysicalPar()->Next();
#else
	LyXParagraph * endpar = sel_end_cursor.par()->Next();
#endif
	LyXParagraph * undoendpar = endpar;

	if (endpar && endpar->GetDepth()) {
		while (endpar && endpar->GetDepth()) {
#ifndef NEW_INSETS
			endpar = endpar->LastPhysicalPar()->Next();
#else
			endpar = endpar->Next();
#endif
			undoendpar = endpar;
		}
	}
	else if (endpar) {
		endpar = endpar->Next(); // because of parindents etc.
	}
   
	SetUndo(bview->buffer(), Undo::EDIT,
#ifndef NEW_INSETS
		sel_start_cursor
		.par()->ParFromPos(sel_start_cursor.pos())->previous,
#else
		sel_start_cursor.par()->previous,
#endif
		undoendpar);

	
	LyXParagraph * tmppar = sel_end_cursor.par();
#ifndef NEW_INSETS
	while (tmppar != sel_start_cursor.par()->FirstPhysicalPar()->Previous()) {
		SetCursor(bview, tmppar->FirstPhysicalPar(), 0);
#else
	while (tmppar != sel_start_cursor.par()->Previous()) {
		SetCursor(bview, tmppar, 0);
#endif
		status = LyXText::NEED_MORE_REFRESH;
		refresh_row = cursor.row();
		refresh_y = cursor.y() - cursor.row()->baseline();
#ifndef NEW_INSETS
		if (cursor.par()->footnoteflag ==
		    sel_start_cursor.par()->footnoteflag) {
#endif
			cursor.par()->line_top = line_top;
			cursor.par()->line_bottom = line_bottom;
			cursor.par()->pagebreak_top = pagebreak_top;
			cursor.par()->pagebreak_bottom = pagebreak_bottom;
			cursor.par()->added_space_top = space_top;
			cursor.par()->added_space_bottom = space_bottom;
			// does the layout allow the new alignment?
			if (align == LYX_ALIGN_LAYOUT)
				align = textclasslist
					.Style(bview->buffer()->params.textclass,
					       cursor.par()->GetLayout()).align;
			if (align & textclasslist
			    .Style(bview->buffer()->params.textclass,
				   cursor.par()->GetLayout()).alignpossible) {
				if (align == textclasslist
				    .Style(bview->buffer()->params.textclass,
					   cursor.par()->GetLayout()).align)
					cursor.par()->align = LYX_ALIGN_LAYOUT;
				else
					cursor.par()->align = align;
			}
			cursor.par()->SetLabelWidthString(labelwidthstring);
			cursor.par()->noindent = noindent;
#ifndef NEW_INSETS
		}
		
		tmppar = cursor.par()->FirstPhysicalPar()->Previous();
#else
		tmppar = cursor.par()->Previous();
#endif
	}
	
	RedoParagraphs(bview, sel_start_cursor, endpar);
	
	ClearSelection();
	SetCursor(bview, sel_start_cursor.par(), sel_start_cursor.pos());
	sel_cursor = cursor;
	SetCursor(bview, sel_end_cursor.par(), sel_end_cursor.pos());
	SetSelection();
	SetCursor(bview, tmpcursor.par(), tmpcursor.pos());
	if (inset_owner)
	    bview->updateInset(inset_owner, true);
}


void LyXText::SetParagraphExtraOpt(BufferView * bview, int type,
                                   string const & width,
                                   string const & widthp,
                                   int alignment, bool hfill,
                                   bool start_minipage)
{
	LyXCursor tmpcursor = cursor;
	LyXParagraph * tmppar;
	if (!selection) {
		sel_start_cursor = cursor;
		sel_end_cursor = cursor;
	}

	// make sure that the depth behind the selection are restored, too
#ifndef NEW_INSETS
	LyXParagraph * endpar = sel_end_cursor.par()->LastPhysicalPar()->Next();
#else
	LyXParagraph * endpar = sel_end_cursor.par()->Next();
#endif
	LyXParagraph * undoendpar = endpar;

	if (endpar && endpar->GetDepth()) {
		while (endpar && endpar->GetDepth()) {
#ifndef NEW_INSETS
			endpar = endpar->LastPhysicalPar()->Next();
#else
			endpar = endpar->Next();
#endif
			undoendpar = endpar;
		}
	}
	else if (endpar) {
		endpar = endpar->Next(); // because of parindents etc.
	}
   
	SetUndo(bview->buffer(), Undo::EDIT,
#ifndef NEW_INSETS
		sel_start_cursor
		.par()->ParFromPos(sel_start_cursor.pos())->previous,
#else
		sel_start_cursor.par()->previous,
#endif
		undoendpar);
	
	tmppar = sel_end_cursor.par();
#ifndef NEW_INSETS
	while(tmppar != sel_start_cursor.par()->FirstPhysicalPar()->Previous()) {
                SetCursor(bview, tmppar->FirstPhysicalPar(), 0);
#else
	while(tmppar != sel_start_cursor.par()->Previous()) {
                SetCursor(bview, tmppar, 0);
#endif
                status = LyXText::NEED_MORE_REFRESH;
                refresh_row = cursor.row();
                refresh_y = cursor.y() - cursor.row()->baseline();
#ifndef NEW_INSETS
                if (cursor.par()->footnoteflag ==
                    sel_start_cursor.par()->footnoteflag) {
#endif
                        if (type == LyXParagraph::PEXTRA_NONE) {
                                if (cursor.par()->pextra_type != LyXParagraph::PEXTRA_NONE) {
                                        cursor.par()->UnsetPExtraType(bview->buffer()->params);
                                        cursor.par()->pextra_type = LyXParagraph::PEXTRA_NONE;
                                }
                        } else {
                                cursor.par()->SetPExtraType(bview->buffer()->params,
							  type, width, widthp);
                                cursor.par()->pextra_hfill = hfill;
                                cursor.par()->pextra_start_minipage = start_minipage;
                                cursor.par()->pextra_alignment = alignment;
                        }
#ifndef NEW_INSETS
		}
                tmppar = cursor.par()->FirstPhysicalPar()->Previous();
#else
                tmppar = cursor.par()->Previous();
#endif
        }
	RedoParagraphs(bview, sel_start_cursor, endpar);
	ClearSelection();
	SetCursor(bview, sel_start_cursor.par(), sel_start_cursor.pos());
	sel_cursor = cursor;
	SetCursor(bview, sel_end_cursor.par(), sel_end_cursor.pos());
	SetSelection();
	SetCursor(bview, tmpcursor.par(), tmpcursor.pos());
}


char loweralphaCounter(int n)
{
	if (n < 1 || n > 26)
		return '?';
	else
		return 'a' + n - 1;
}


static inline
char alphaCounter(int n)
{
	if (n < 1 || n > 26)
		return '?';
	else
		return 'A' + n - 1;
}


static inline
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


static inline
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


// set the counter of a paragraph. This includes the labels
void LyXText::SetCounter(Buffer const * buf, LyXParagraph * par) const
{
#ifndef NEW_INSETS
	// this is only relevant for the beginning of paragraph
	par = par->FirstPhysicalPar();
#endif
	LyXLayout const & layout =
		textclasslist.Style(buf->params.textclass, 
				    par->GetLayout());

	LyXTextClass const & textclass =
		textclasslist.TextClass(buf->params.textclass);

	/* copy the prev-counters to this one, unless this is the start of a 
	   footnote or of a bibliography or the very first paragraph */
	if (par->Previous()
#ifndef NEW_INSETS
	    && !(par->Previous()->footnoteflag == LyXParagraph::NO_FOOTNOTE 
		    && par->footnoteflag == LyXParagraph::OPEN_FOOTNOTE
		    && par->footnotekind == LyXParagraph::FOOTNOTE)
#endif
	    && !(textclasslist.Style(buf->params.textclass,
				par->Previous()->GetLayout()
				).labeltype != LABEL_BIBLIO
		 && layout.labeltype == LABEL_BIBLIO)) {
		for (int i = 0; i < 10; ++i) {
			par->setCounter(i, par->Previous()->GetFirstCounter(i));
		}
#ifndef NEW_INSETS
		par->appendix = par->Previous()->FirstPhysicalPar()->appendix;
#else
		par->appendix = par->Previous()->appendix;
#endif
		if (!par->appendix && par->start_of_appendix){
		  par->appendix = true;
		  for (int i = 0; i < 10; ++i) {
		    par->setCounter(i, 0);
		  }  
		}
#ifndef NEW_INSETS
		par->enumdepth = par->Previous()->FirstPhysicalPar()->enumdepth;
		par->itemdepth = par->Previous()->FirstPhysicalPar()->itemdepth;
#else
		par->enumdepth = par->Previous()->enumdepth;
		par->itemdepth = par->Previous()->itemdepth;
#endif
	} else {
		for (int i = 0; i < 10; ++i) {
			par->setCounter(i, 0);
		}  
		par->appendix = par->start_of_appendix;
		par->enumdepth = 0;
		par->itemdepth = 0;
	}

#ifndef NEW_INSETS
        // if this is an open marginnote and this is the first
        // entry in the marginnote and the enclosing
        // environment is an enum/item then correct for the
        // LaTeX behaviour (ARRae)
        if (par->footnoteflag == LyXParagraph::OPEN_FOOTNOTE
	   && par->footnotekind == LyXParagraph::MARGIN
           && par->Previous()
           && par->Previous()->footnoteflag != LyXParagraph::OPEN_FOOTNOTE
           && (par->PreviousBeforeFootnote()
               && textclasslist.Style(buf->params.textclass,
                                 par->PreviousBeforeFootnote()->GetLayout()
				 ).labeltype >= LABEL_COUNTER_ENUMI)) {
                // Any itemize or enumerate environment in a marginnote
                // that is embedded in an itemize or enumerate
                // paragraph is seen by LaTeX as being at a deeper
                // level within that enclosing itemization/enumeration
                // even if there is a "standard" layout at the start of
                // the marginnote.
                par->enumdepth++;
                par->itemdepth++;
        }
#endif
	/* Maybe we have to increment the enumeration depth.
	 * BUT, enumeration in a footnote is considered in isolation from its
	 *	surrounding paragraph so don't increment if this is the
	 *	first line of the footnote
	 * AND, bibliographies can't have their depth changed ie. they
	 *	are always of depth 0
	 */
	if (par->Previous()
	    && par->Previous()->GetDepth() < par->GetDepth()
	    && textclasslist.Style(buf->params.textclass,
			      par->Previous()->GetLayout()
			     ).labeltype == LABEL_COUNTER_ENUMI
	    && par->enumdepth < 3
#ifndef NEW_INSETS
	    && !(par->Previous()->footnoteflag == LyXParagraph::NO_FOOTNOTE 
		    && par->footnoteflag == LyXParagraph::OPEN_FOOTNOTE
		    && par->footnotekind == LyXParagraph::FOOTNOTE)
#endif
	    && layout.labeltype != LABEL_BIBLIO) {
		par->enumdepth++;
	}

	/* Maybe we have to decrement the enumeration depth, see note above */
	if (par->Previous()
	    && par->Previous()->GetDepth() > par->GetDepth()
#ifndef NEW_INSETS
	    && !(par->Previous()->footnoteflag == LyXParagraph::NO_FOOTNOTE
		    && par->footnoteflag == LyXParagraph::OPEN_FOOTNOTE
		    && par->footnotekind == LyXParagraph::FOOTNOTE)
#endif
	    && layout.labeltype != LABEL_BIBLIO) {
		par->enumdepth = par->DepthHook(par->GetDepth())->enumdepth;
		par->setCounter(6 + par->enumdepth,
			par->DepthHook(par->GetDepth())->getCounter(6 + par->enumdepth));
		/* reset the counters.
		 * A depth change is like a breaking layout
		 */
		for (int i = 6 + par->enumdepth + 1; i < 10; ++i)
			par->setCounter(i, 0);
	}
   
	if (!par->labelstring.empty()) {
		par->labelstring.erase();
	}
   
	if (layout.margintype == MARGIN_MANUAL) {
		if (par->labelwidthstring.empty()) {
			par->SetLabelWidthString(layout.labelstring());
		}
	} else {
		par->SetLabelWidthString(string());
	}
   
	/* is it a layout that has an automatic label ? */ 
	if (layout.labeltype >=  LABEL_COUNTER_CHAPTER) {
      
		int i = layout.labeltype - LABEL_COUNTER_CHAPTER;
		if (i >= 0 && i<= buf->params.secnumdepth) {
			par->incCounter(i);	// increment the counter  
	 
			// Is there a label? Useful for Chapter layout
			if (!par->appendix){
				if (!layout.labelstring().empty())
					par->labelstring = layout.labelstring();
				else
					par->labelstring.erase();
                        } else {
				if (!layout.labelstring_appendix().empty())
					par->labelstring = layout.labelstring_appendix();
				else
					par->labelstring.erase();
			}

			std::ostringstream s;

			if (!par->appendix) {
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

			par->labelstring += s.str().c_str();
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

			std::ostringstream s;

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

			par->labelstring = s.str().c_str();
			// we really want to get rid of that c_str()

			for (i += par->enumdepth + 1; i < 10; ++i)
				par->setCounter(i, 0);  /* reset the following counters  */
	 
		} 
	} else if (layout.labeltype == LABEL_BIBLIO) {// ale970302
		int i = LABEL_COUNTER_ENUMI - LABEL_COUNTER_CHAPTER + par->enumdepth;
		par->incCounter(i);
		int number = par->getCounter(i);
		if (!par->bibkey) {
			InsetCommandParams p( "bibitem" );
			par->bibkey = new InsetBibKey(p);
		}
		par->bibkey->setCounter(number);
		par->labelstring = layout.labelstring();
		
		// In biblio should't be following counters but...
	} else {
		string s = layout.labelstring();
		
		// the caption hack:
		if (layout.labeltype == LABEL_SENSITIVE) {
			bool isOK (par->InInset() && par->InInset()->owner() &&
				   (par->InInset()->owner()->LyxCode() == Inset::FLOAT_CODE));
#ifndef NEW_INSETS
			if (par->footnoteflag != LyXParagraph::NO_FOOTNOTE
			    && (par->footnotekind == LyXParagraph::FIG
				|| par->footnotekind == LyXParagraph::WIDE_FIG)) {
				s = (par->getParLanguage(buf->params)->lang() == "hebrew")
					? ":רויא" : "Figure:";
			} else if (par->footnoteflag != LyXParagraph::NO_FOOTNOTE
				 && (par->footnotekind == LyXParagraph::TAB
				     || par->footnotekind == LyXParagraph::WIDE_TAB)) {
				s = (par->getParLanguage(buf->params)->lang() == "hebrew")
					? ":הלבט" : "Table:";
			} else if (par->footnoteflag != LyXParagraph::NO_FOOTNOTE
				   && par->footnotekind == LyXParagraph::ALGORITHM) {
				s = (par->getParLanguage(buf->params)->lang() == "hebrew")
					? ":םתירוגלא" : "Algorithm:";
			} else
#endif
			if (isOK) {
				InsetFloat * tmp = static_cast<InsetFloat*>(par->InInset()->owner());
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
		par->labelstring = s;
		
		/* reset the enumeration counter. They are always resetted
		 * when there is any other layout between */ 
		for (int i = 6 + par->enumdepth; i < 10; ++i)
			par->setCounter(i, 0);
	}
}


/* Updates all counters BEHIND the row. Changed paragraphs
* with a dynamic left margin will be rebroken. */ 
void LyXText::UpdateCounters(BufferView * bview, Row * row) const
{
	LyXParagraph * par;
	if (!row) {
		row = firstrow;
		par = row->par();
	} else {
		if (row->par()->next
#ifndef NEW_INSETS
		    && row->par()->next->footnoteflag != LyXParagraph::OPEN_FOOTNOTE
#endif
			) {
#ifndef NEW_INSETS
			par = row->par()->LastPhysicalPar()->Next();
#else
			par = row->par()->Next();
#endif
		} else {
			par = row->par()->next;
		}
	}

	while (par) {
		while (row->par() != par)
			row = row->next();
		
		SetCounter(bview->buffer(), par);
		
		/* now  check for the headline layouts. remember that they
		 * have a dynamic left margin */ 
		if (
#ifndef NEW_INSETS
			!par->IsDummy() &&
#endif
		    ( textclasslist.Style(bview->buffer()->params.textclass,
					     par->layout).margintype == MARGIN_DYNAMIC
			 || textclasslist.Style(bview->buffer()->params.textclass,
						par->layout).labeltype == LABEL_SENSITIVE)
			) {
	 
			/* Rebreak the paragraph */ 
			RemoveParagraph(row);
			AppendParagraph(bview, row);

#ifndef NEW_INSETS
			/* think about the damned open footnotes! */ 
			while (par->Next() &&
			       (par->Next()->footnoteflag == LyXParagraph::OPEN_FOOTNOTE
				|| par->Next()->IsDummy())){
				par = par->Next();
				if (par->IsDummy()) {
					while (row->par() != par)
						row = row->next();
					RemoveParagraph(row);
					AppendParagraph(bview, row);
				}
			}
#endif
		}
#ifndef NEW_INSETS
		par = par->LastPhysicalPar()->Next();
#else
		par = par->Next();
#endif
     
	}
}


/* insets an inset. */ 
void LyXText::InsertInset(BufferView * bview, Inset * inset)
{
	if (!cursor.par()->InsertInsetAllowed(inset))
		return;
	SetUndo(bview->buffer(), Undo::INSERT,
#ifndef NEW_INSETS
		cursor.par()->ParFromPos(cursor.pos())->previous, 
		cursor.par()->ParFromPos(cursor.pos())->next
#else
		cursor.par()->previous, 
		cursor.par()->next
#endif
		);
	cursor.par()->InsertInset(cursor.pos(), inset);
	InsertChar(bview, LyXParagraph::META_INSET);  /* just to rebreak and refresh correctly.
				      * The character will not be inserted a
				      * second time */
#if 1
	// if we enter a text-inset the cursor should be to the left side
	// of it! This couldn't happen before as Undo was not handled inside
	// inset now after the Undo LyX tries to call inset->Edit(...) again
	// and cannot do this as the cursor is behind the inset and GetInset
	// does not return the inset!
	if (inset->IsTextInset()) {
		if (cursor.par()->isRightToLeftPar(bview->buffer()->params))
			CursorRight(bview);
		else
			CursorLeft(bview);
	}
#endif
}


void LyXText::copyEnvironmentType()
{
	copylayouttype = cursor.par()->GetLayout();
}


void LyXText::pasteEnvironmentType(BufferView * bview)
{
	SetLayout(bview, copylayouttype);
}


void LyXText::CutSelection(BufferView * bview, bool doclear)
{
	// Stuff what we got on the clipboard. Even if there is no selection.

	// There is a problem with having the stuffing here in that the
	// larger the selection the slower LyX will get. This can be
	// solved by running the line below only when the selection has
	// finished. The solution used currently just works, to make it
	// faster we need to be more clever and probably also have more
	// calls to stuffClipboard. (Lgb)
	bview->stuffClipboard(selectionAsString(bview->buffer()));

	// This doesn't make sense, if there is no selection
	if (!selection)
		return;
   
	// OK, we have a selection. This is always between sel_start_cursor
	// and sel_end_cursor
#ifndef NEW_INSETS
	// Check whether there are half footnotes in the selection
	if (sel_start_cursor.par()->footnoteflag != LyXParagraph::NO_FOOTNOTE
	    || sel_end_cursor.par()->footnoteflag != LyXParagraph::NO_FOOTNOTE) {
	LyXParagraph * tmppar = sel_start_cursor.par();
		while (tmppar != sel_end_cursor.par()){
			if (tmppar->footnoteflag != sel_end_cursor.par()->footnoteflag) {
				WriteAlert(_("Impossible operation"),
					   _("Don't know what to do with half floats."),
					   _("sorry."));
				return;
			}
			tmppar = tmppar->Next();
		}
	}
#endif

	// make sure that the depth behind the selection are restored, too
#ifndef NEW_INSETS
	LyXParagraph * endpar = sel_end_cursor.par()->LastPhysicalPar()->Next();
#else
	LyXParagraph * endpar = sel_end_cursor.par()->Next();
#endif
	LyXParagraph * undoendpar = endpar;
    
	if (endpar && endpar->GetDepth()) {
		while (endpar && endpar->GetDepth()) {
#ifndef NEW_INSETS
			endpar = endpar->LastPhysicalPar()->Next();
#else
			endpar = endpar->Next();
#endif
			undoendpar = endpar;
		}
	} else if (endpar) {
		endpar = endpar->Next(); // because of parindents etc.
	}
    
	SetUndo(bview->buffer(), Undo::DELETE,
#ifndef NEW_INSETS
		sel_start_cursor
		.par()->ParFromPos(sel_start_cursor.pos())->previous,
#else
		sel_start_cursor.par()->previous,
#endif
		undoendpar);
    
	CutAndPaste cap;

	// there are two cases: cut only within one paragraph or
	// more than one paragraph
#ifndef NEW_INSETS
	if (sel_start_cursor.par()->ParFromPos(sel_start_cursor.pos()) 
	    == sel_end_cursor.par()->ParFromPos(sel_end_cursor.pos()))
#else
	if (sel_start_cursor.par() == sel_end_cursor.par())
#endif
		{
		// only within one paragraph
		endpar = sel_start_cursor.par();
		int pos = sel_end_cursor.pos();
		cap.cutSelection(sel_start_cursor.par(), &endpar,
				 sel_start_cursor.pos(), pos,
				 bview->buffer()->params.textclass, doclear);
		sel_end_cursor.pos(pos);
	} else {
		endpar = sel_end_cursor.par();

		int pos = sel_end_cursor.pos();
		cap.cutSelection(sel_start_cursor.par(), &endpar,
				 sel_start_cursor.pos(), pos,
				 bview->buffer()->params.textclass, doclear);
		cursor.par(endpar);
		sel_end_cursor.par(endpar);
		sel_end_cursor.pos(pos);
		cursor.pos(sel_end_cursor.pos());
	}
	endpar = endpar->Next();

	// sometimes necessary
	if (doclear)
		sel_start_cursor.par()->StripLeadingSpaces(bview->buffer()->params.textclass);

	RedoParagraphs(bview, sel_start_cursor, endpar);
   
	ClearSelection();
	cursor = sel_start_cursor;
	SetCursor(bview, cursor.par(), cursor.pos());
	sel_cursor = cursor;
	UpdateCounters(bview, cursor.row());
}


void LyXText::CopySelection(BufferView * bview)
{
	// Stuff what we got on the clipboard. Even if there is no selection.

	// There is a problem with having the stuffing here in that the
	// larger the selection the slower LyX will get. This can be
	// solved by running the line below only when the selection has
	// finished. The solution used currently just works, to make it
	// faster we need to be more clever and probably also have more
	// calls to stuffClipboard. (Lgb)
	bview->stuffClipboard(selectionAsString(bview->buffer()));

	// this doesnt make sense, if there is no selection
	if (!selection)
		return;

	// ok we have a selection. This is always between sel_start_cursor
	// and sel_end cursor

#ifndef NEW_INSETS
	/* check wether there are half footnotes in the selection */
	if (sel_start_cursor.par()->footnoteflag != LyXParagraph::NO_FOOTNOTE
	    || sel_end_cursor.par()->footnoteflag != LyXParagraph::NO_FOOTNOTE) {
		LyXParagraph * tmppar = sel_start_cursor.par();
		while (tmppar != sel_end_cursor.par()) {
			if (tmppar->footnoteflag !=
			    sel_end_cursor.par()->footnoteflag) {
				WriteAlert(_("Impossible operation"),
					   _("Don't know what to do"
					     " with half floats."),
					   _("sorry."));
				return;
			}
			tmppar = tmppar->Next();
		}
	}
#endif
   
	// copy behind a space if there is one
	while (sel_start_cursor.par()->Last() > sel_start_cursor.pos()
	       && sel_start_cursor.par()->IsLineSeparator(sel_start_cursor.pos())
	       && (sel_start_cursor.par() != sel_end_cursor.par()
		   || sel_start_cursor.pos() < sel_end_cursor.pos()))
		sel_start_cursor.pos(sel_start_cursor.pos() + 1); 

	CutAndPaste cap;

	cap.copySelection(sel_start_cursor.par(), sel_end_cursor.par(),
			  sel_start_cursor.pos(), sel_end_cursor.pos(),
			  bview->buffer()->params.textclass);
}


void LyXText::PasteSelection(BufferView * bview)
{
	CutAndPaste cap;

	// this does not make sense, if there is nothing to paste
	if (!cap.checkPastePossible(cursor.par(), cursor.pos()))
		return;

	SetUndo(bview->buffer(), Undo::INSERT,
#ifndef NEW_INSETS
		cursor.par()->ParFromPos(cursor.pos())->previous, 
		cursor.par()->ParFromPos(cursor.pos())->next
#else
		cursor.par()->previous, 
		cursor.par()->next
#endif
		); 

	LyXParagraph * endpar;
	LyXParagraph * actpar = cursor.par();

	int pos = cursor.pos();
	cap.pasteSelection(&actpar, &endpar, pos, bview->buffer()->params.textclass);
    
	RedoParagraphs(bview, cursor, endpar);
	
	SetCursor(bview, cursor.par(), cursor.pos());
	ClearSelection();
   
	sel_cursor = cursor;
	SetCursor(bview, actpar, pos);
	SetSelection();
	UpdateCounters(bview, cursor.row());
}


// returns a pointer to the very first LyXParagraph
LyXParagraph * LyXText::FirstParagraph() const
{
	return OwnerParagraph();
}


// returns true if the specified string is at the specified position
bool LyXText::IsStringInText(LyXParagraph * par,
			     LyXParagraph::size_type pos,
			     string const & str) const
{
	if (!par)
		return false;

	LyXParagraph::size_type i = 0;
	while (pos + i < par->Last()
	       && string::size_type(i) < str.length()
	       && str[i] == par->GetChar(pos + i)) {
		++i;
	}
	if (str.length() == string::size_type(i))
		return true;
	return false;
}


// sets the selection over the number of characters of string, no check!!
void LyXText::SetSelectionOverString(BufferView * bview, string const & str)
{
	sel_cursor = cursor;
	for (int i = 0; str[i]; ++i)
		CursorRight(bview);
	SetSelection();
}


// simple replacing. The font of the first selected character is used
void LyXText::ReplaceSelectionWithString(BufferView * bview,
					 string const & str)
{
	SetCursorParUndo(bview->buffer());
	FreezeUndo();

	if (!selection) { // create a dummy selection
		sel_end_cursor = cursor;
		sel_start_cursor = cursor;
	}

	// Get font setting before we cut
	LyXParagraph::size_type pos = sel_end_cursor.pos();
	LyXFont const font = sel_start_cursor.par()
		->GetFontSettings(bview->buffer()->params,
				  sel_start_cursor.pos());

	// Insert the new string
	for (string::const_iterator cit = str.begin(); cit != str.end(); ++cit) {
		sel_end_cursor.par()->InsertChar(pos, (*cit), font);
		++pos;
	}
	
	// Cut the selection
	CutSelection(bview);

	UnFreezeUndo();
}


// if the string can be found: return true and set the cursor to
// the new position
bool LyXText::SearchForward(BufferView * bview, string const & str) const
{
	LyXParagraph * par = cursor.par();
	LyXParagraph::size_type pos = cursor.pos();
	while (par && !IsStringInText(par, pos, str)) {
		if (pos < par->Last() - 1)
			++pos;
		else {
			pos = 0;
			par = par->Next();
		}
	}
	if (par) {
		SetCursor(bview, par, pos);
		return true;
	}
	else
		return false;
}


bool LyXText::SearchBackward(BufferView * bview, string const & str) const
{
	LyXParagraph * par = cursor.par();
	int pos = cursor.pos();

	do {
		if (pos > 0)
			--pos;
		else {
			// We skip empty paragraphs (Asger)
			do {
				par = par->Previous();
				if (par)
					pos = par->Last() - 1;
			} while (par && pos < 0);
		}
	} while (par && !IsStringInText(par, pos, str));
  
	if (par) {
		SetCursor(bview, par, pos);
		return true;
	} else
		return false;
}


// needed to insert the selection
void LyXText::InsertStringA(BufferView * bview, string const & str)
{
	LyXParagraph * par = cursor.par();
	LyXParagraph::size_type pos = cursor.pos();
	LyXParagraph::size_type a = 0;
	LyXParagraph * endpar = cursor.par()->Next();
	
	SetCursorParUndo(bview->buffer());
	
	bool flag =
		textclasslist.Style(bview->buffer()->params.textclass, 
				    cursor.par()->GetLayout()).isEnvironment();
	// only to be sure, should not be neccessary
	ClearSelection();
	
	// insert the string, don't insert doublespace
	string::size_type i = 0;
	while (i < str.length()) {
		if (str[i] != '\n') {
			if (str[i] == ' ' 
			    && i + 1 < str.length() && str[i + 1] != ' '
			    && pos && par->GetChar(pos - 1)!= ' ') {
				par->InsertChar(pos, ' ', current_font);
				++pos;
                        } else if (str[i] == ' ') {
				InsetSpecialChar * new_inset =
					new InsetSpecialChar(InsetSpecialChar::PROTECTED_SEPARATOR);
				if (par->InsertInsetAllowed(new_inset)) {
					par->InsertInset(pos, new_inset,
							 current_font);
				} else {
					delete new_inset;
				}
				++pos;
			} else if (str[i] == '\t') {
				for (a = pos; a < (pos / 8 + 1) * 8 ; ++a) {
				InsetSpecialChar * new_inset =
					new InsetSpecialChar(InsetSpecialChar::PROTECTED_SEPARATOR);
				if (par->InsertInsetAllowed(new_inset)) {
					par->InsertInset(pos, new_inset,
							 current_font);
				} else {
					delete new_inset;
				}
				}
				pos = a;
			} else if (str[i] != 13 && 
				   // Ignore unprintables
				   (str[i] & 127) >= ' ') {
				par->InsertChar(pos, str[i], current_font);
				++pos;
			}
		} else {
			if (!par->size()) { // par is empty
				InsetSpecialChar * new_inset =
					new InsetSpecialChar(InsetSpecialChar::PROTECTED_SEPARATOR);
				if (par->InsertInsetAllowed(new_inset)) {
					par->InsertInset(pos,
							 new_inset,
							 current_font);
				} else {
					delete new_inset;
				}
				++pos;
			}
			par->BreakParagraph(bview->buffer()->params, pos, flag);
			par = par->Next();
			pos = 0;
		}
		++i;
	}
	
	RedoParagraphs(bview, cursor, endpar);
	SetCursor(bview, cursor.par(), cursor.pos());
	sel_cursor = cursor;
	SetCursor(bview, par, pos);
	SetSelection();
}


/* turns double-CR to single CR, others where converted into one blank and 13s 
 * that are ignored .Double spaces are also converted into one. Spaces at
 * the beginning of a paragraph are forbidden. tabs are converted into one
 * space. then InsertStringA is called */ 
void LyXText::InsertStringB(BufferView * bview, string const & s)
{
	string str(s);
	string::size_type i = 1;
	while (i < str.length()) {
		if (str[i] == '\t')
			str[i] = ' ';
		if (str[i] == ' ' && i + 1 < str.length() && str[i + 1] == ' ')
			str[i] = 13;
		if (str[i] == '\n' && i + 1 < str.length()) {
			if (str[i + 1] != '\n') {
				if (str[i - 1] != ' ')
					str[i] = ' ';
				else
					str[i] = 13;
			}
			while (i + 1 < str.length() 
			       && (str[i + 1] == ' ' 
				   || str[i + 1] == '\t'
				   || str[i + 1] == '\n' 
				   || str[i + 1] == 13)) {
				str[i + 1] = 13;
				++i;
			}
		}
		++i;
	}
	InsertStringA(bview, str);
}


bool LyXText::GotoNextError(BufferView * bview) const
{
	LyXCursor res = cursor;
	do {
		if (res.pos() < res.par()->Last() - 1) {
			res.pos(res.pos() + 1);
		} else  {
			res.par(res.par()->Next());
			res.pos(0);
		}
      
	} while (res.par() && 
		 !(res.par()->GetChar(res.pos()) == LyXParagraph::META_INSET
		   && res.par()->GetInset(res.pos())->AutoDelete()));
   
	if (res.par()) {
		SetCursor(bview, res.par(), res.pos());
		return true;
	}
	return false;
}


bool LyXText::GotoNextNote(BufferView * bview) const
{
	LyXCursor res = cursor;
	do {
		if (res.pos() < res.par()->Last() - 1) {
			res.pos(res.pos() + 1);
		} else  {
			res.par(res.par()->Next());
			res.pos(0);
		}
      
	} while (res.par() && 
		 !(res.par()->GetChar(res.pos()) == LyXParagraph::META_INSET
		   && res.par()->GetInset(res.pos())->LyxCode() == Inset::IGNORE_CODE));
   
	if (res.par()) {
		SetCursor(bview, res.par(), res.pos());
		return true;
	}
	return false;
}


void LyXText::CheckParagraph(BufferView * bview, LyXParagraph * par,
			     LyXParagraph::size_type pos)
{
	LyXCursor tmpcursor;			

	int y = 0;
	LyXParagraph::size_type z;
	Row * row = GetRow(par, pos, y);
	
	// is there a break one row above
	if (row->previous() && row->previous()->par() == row->par()) {
		z = NextBreakPoint(bview, row->previous(), workWidth(bview));
		if (z >= row->pos()) {
			// set the dimensions of the row above
			y -= row->previous()->height();
			refresh_y = y;
			refresh_row = row->previous();
			status = LyXText::NEED_MORE_REFRESH;
			
			BreakAgain(bview, row->previous());
			
			// set the cursor again. Otherwise
			// dangling pointers are possible
			SetCursor(bview, cursor.par(), cursor.pos(),
				  false, cursor.boundary());
			sel_cursor = cursor;
			return;
		}
	}

	int const tmpheight = row->height();
	LyXParagraph::size_type const tmplast = RowLast(row);
	refresh_y = y;
	refresh_row = row;
	
	BreakAgain(bview, row);
	if (row->height() == tmpheight && RowLast(row) == tmplast)
		status = LyXText::NEED_VERY_LITTLE_REFRESH;
	else
		status = LyXText::NEED_MORE_REFRESH; 
	
	// check the special right address boxes
	if (textclasslist.Style(bview->buffer()->params.textclass,
				par->GetLayout()).margintype
	    == MARGIN_RIGHT_ADDRESS_BOX) {
		tmpcursor.par(par);
		tmpcursor.row(row);
		tmpcursor.y(y);
		tmpcursor.x(0);
		tmpcursor.x_fix(0);
		tmpcursor.pos(pos);
		RedoDrawingOfParagraph(bview, tmpcursor); 
	}

	// set the cursor again. Otherwise dangling pointers are possible
	// also set the selection
   
	if (selection) {
		tmpcursor = cursor;
		SetCursorIntern(bview, sel_cursor.par(), sel_cursor.pos(),
				false, sel_cursor.boundary());
		sel_cursor = cursor; 
		SetCursorIntern(bview, sel_start_cursor.par(),
				sel_start_cursor.pos(),
				false, sel_start_cursor.boundary());
		sel_start_cursor = cursor; 
		SetCursorIntern(bview, sel_end_cursor.par(),
				sel_end_cursor.pos(),
				false, sel_end_cursor.boundary());
		sel_end_cursor = cursor; 
		SetCursorIntern(bview, last_sel_cursor.par(),
				last_sel_cursor.pos(),
				false, last_sel_cursor.boundary());
		last_sel_cursor = cursor; 
		cursor = tmpcursor;
	}
	SetCursorIntern(bview, cursor.par(), cursor.pos(),
			false, cursor.boundary());
}


// returns false if inset wasn't found
bool LyXText::UpdateInset(BufferView * bview, Inset * inset)
{
	// first check the current paragraph
	int pos = cursor.par()->GetPositionOfInset(inset);
	if (pos != -1){
		CheckParagraph(bview, cursor.par(), pos);
		return true;
	}
  
	// check every paragraph
  
	LyXParagraph * par = FirstParagraph();
	do {
#ifndef NEW_INSETS
		// make sure the paragraph is open
		if (par->footnoteflag != LyXParagraph::CLOSED_FOOTNOTE){
#endif
			pos = par->GetPositionOfInset(inset);
			if (pos != -1){
				CheckParagraph(bview, par, pos);
				return true;
			}
#ifndef NEW_INSETS
		}
#endif
		par = par->Next();
	} while (par);
  
	return false;
}


void LyXText::SetCursor(BufferView * bview, LyXParagraph * par,
			LyXParagraph::size_type pos, 
			bool setfont, bool boundary) const
{
	LyXCursor old_cursor = cursor;
	SetCursorIntern(bview, par, pos, setfont, boundary);
	DeleteEmptyParagraphMechanism(bview, old_cursor);
}


void LyXText::SetCursor(BufferView *bview, LyXCursor & cur, LyXParagraph * par,
			LyXParagraph::size_type pos, bool boundary) const
{
#ifndef NEW_INSETS
	// correct the cursor position if impossible
	if (pos > par->Last()){
		LyXParagraph * tmppar = par->ParFromPos(pos);
		pos = par->PositionInParFromPos(pos);
		par = tmppar;
	}
	if (par->IsDummy() && par->previous &&
	    par->previous->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE) {
		while (par->previous &&
		       ((par->previous->IsDummy() &&
			 (par->previous->previous->footnoteflag ==
			  LyXParagraph::CLOSED_FOOTNOTE)) ||
			(par->previous->footnoteflag ==
			 LyXParagraph::CLOSED_FOOTNOTE))) {
			par = par->previous ;
			if (par->IsDummy() &&
			    (par->previous->footnoteflag ==
			     LyXParagraph::CLOSED_FOOTNOTE))
				pos += par->size() + 1;
		}
		if (par->previous) {
			par = par->previous;
		}
		pos += par->size() + 1;
	}
#endif
	cur.par(par);
	cur.pos(pos);
	cur.boundary(boundary);

	/* get the cursor y position in text  */
	int y = 0;
	Row * row = GetRow(par, pos, y);
	/* y is now the beginning of the cursor row */ 
	y += row->baseline();
	/* y is now the cursor baseline */ 
	cur.y(y);
   
	/* now get the cursors x position */
	float x;
	float fill_separator, fill_hfill, fill_label_hfill;
	PrepareToPrint(bview, row, x, fill_separator, fill_hfill,
		       fill_label_hfill);
	LyXParagraph::size_type cursor_vpos = 0;
	LyXParagraph::size_type last = RowLastPrintable(row);

	if (pos > last + 1)   // This shouldn't happen.
		pos = last + 1;
	else if (pos < row->pos())
		pos = row->pos();

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
	
	LyXParagraph::size_type main_body =
		BeginningOfMainBody(bview->buffer(), row->par());
	if ((main_body > 0) &&
	    ((main_body-1 > last) || 
	     !row->par()->IsLineSeparator(main_body-1)))
		main_body = 0;
	
	for (LyXParagraph::size_type vpos = row->pos();
	     vpos < cursor_vpos; ++vpos) {
		pos = vis2log(vpos);
		if (main_body > 0 && pos == main_body - 1) {
			x += fill_label_hfill +
				lyxfont::width(textclasslist.Style(
					bview->buffer()->params.textclass,
					row->par()->GetLayout())
					       .labelsep,
					       GetFont(bview->buffer(), row->par(), -2));
			if (row->par()->IsLineSeparator(main_body-1))
				x -= SingleWidth(bview, row->par(),main_body-1);
		}
		if (HfillExpansion(bview->buffer(), row, pos)) {
			x += SingleWidth(bview, row->par(), pos);
			if (pos >= main_body)
				x += fill_hfill;
			else 
				x += fill_label_hfill;
		} else if (row->par()->IsSeparator(pos)) {
			x += SingleWidth(bview, row->par(), pos);
			if (pos >= main_body)
				x += fill_separator;
		} else
			x += SingleWidth(bview, row->par(), pos);
	}
	
	cur.x(int(x));
   	cur.x_fix(cur.x());
	cur.row(row);
}


void LyXText::SetCursorIntern(BufferView * bview, LyXParagraph * par,
			      LyXParagraph::size_type pos,
			      bool setfont, bool boundary) const
{
	SetCursor(bview, cursor, par, pos, boundary);
	if (setfont)
		SetCurrentFont(bview);
}


void LyXText::SetCurrentFont(BufferView * bview) const
{
	LyXParagraph::size_type pos = cursor.pos();
	if (cursor.boundary() && pos > 0)
		--pos;

	if (pos > 0) {
		if (pos == cursor.par()->Last())
			--pos;
		else if (cursor.par()->IsSeparator(pos)) {
			if (pos > cursor.row()->pos() &&
			    bidi_level(pos) % 2 == 
			    bidi_level(pos - 1) % 2)
				--pos;
			else if (pos + 1 < cursor.par()->Last())
				++pos;
		}
	}

	current_font =
		cursor.par()->GetFontSettings(bview->buffer()->params, pos);
	real_current_font = GetFont(bview->buffer(), cursor.par(), pos);

	if (cursor.pos() == cursor.par()->Last() &&
	    IsBoundary(bview->buffer(), cursor.par(), cursor.pos()) &&
	    !cursor.boundary()) {
		Language const * lang =
			cursor.par()->getParLanguage(bview->buffer()->params);
		current_font.setLanguage(lang);
		real_current_font.setLanguage(lang);
	}
}


void LyXText::SetCursorFromCoordinates(BufferView * bview, int x, int y) const
{
	LyXCursor old_cursor = cursor;
   
	/* get the row first */ 
   
	Row * row = GetRowNearY(y);
	cursor.par(row->par());

	bool bound = false;
	int column = GetColumnNearX(bview, row, x, bound);
	cursor.pos(row->pos() + column);
	cursor.x(x);
	cursor.y(y + row->baseline());
	cursor.row(row);
	cursor.boundary(bound);
	SetCurrentFont(bview);
	DeleteEmptyParagraphMechanism(bview, old_cursor);
}


void LyXText::SetCursorFromCoordinates(BufferView * bview, LyXCursor & cur,
				       int x, int y) const
{
	/* get the row first */ 
   
	Row * row = GetRowNearY(y);
	bool bound = false;
	int column = GetColumnNearX(bview, row, x, bound);
   
	cur.par(row->par());
	cur.pos(row->pos() + column);
	cur.x(x);
	cur.y(y + row->baseline());
   	cur.row(row);
	cur.boundary(bound);
}


void LyXText::CursorLeft(BufferView * bview, bool internal) const
{
	if (cursor.pos() > 0) {
		bool boundary = cursor.boundary();
		SetCursor(bview, cursor.par(), cursor.pos() - 1, true, false);
		if (!internal && !boundary &&
		    IsBoundary(bview->buffer(), cursor.par(), cursor.pos() + 1))
			SetCursor(bview, cursor.par(), cursor.pos() + 1, true, true);
	} else if (cursor.par()->Previous()) { // steps into the above paragraph.
		LyXParagraph * par = cursor.par()->Previous();
		SetCursor(bview, par, par->Last());
	}
}


void LyXText::CursorRight(BufferView * bview, bool internal) const
{
	if (!internal && cursor.boundary() &&
	    !cursor.par()->IsNewline(cursor.pos()))
		SetCursor(bview, cursor.par(), cursor.pos(), true, false);
	else if (cursor.pos() < cursor.par()->Last()) {
		SetCursor(bview, cursor.par(), cursor.pos() + 1, true, false);
		if (!internal &&
		    IsBoundary(bview->buffer(), cursor.par(), cursor.pos()))
			SetCursor(bview, cursor.par(), cursor.pos(), true, true);
	} else if (cursor.par()->Next())
		SetCursor(bview, cursor.par()->Next(), 0);
}


void LyXText::CursorUp(BufferView * bview) const
{
	SetCursorFromCoordinates(bview, cursor.x_fix(), 
				 cursor.y() - cursor.row()->baseline() - 1);
}


void LyXText::CursorDown(BufferView * bview) const
{
	SetCursorFromCoordinates(bview, cursor.x_fix(), 
				 cursor.y() - cursor.row()->baseline()
				 + cursor.row()->height() + 1);
}


void LyXText::CursorUpParagraph(BufferView * bview) const
{
	if (cursor.pos() > 0) {
		SetCursor(bview, cursor.par(), 0);
	}
	else if (cursor.par()->Previous()) {
		SetCursor(bview, cursor.par()->Previous(), 0);
	}
}


void LyXText::CursorDownParagraph(BufferView * bview) const
{
	if (cursor.par()->Next()) {
		SetCursor(bview, cursor.par()->Next(), 0);
	} else {
		SetCursor(bview, cursor.par(), cursor.par()->Last());
	}
}


void LyXText::DeleteEmptyParagraphMechanism(BufferView * bview,
					    LyXCursor const & old_cursor) const
{
	// Would be wrong to delete anything if we have a selection.
	if (selection) return;

	// We allow all kinds of "mumbo-jumbo" when freespacing.
	if (textclasslist.Style(bview->buffer()->params.textclass,
				old_cursor.par()->GetLayout()).free_spacing)
		return;

	bool deleted = false;
	
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
	if (old_cursor.par() != cursor.par() || old_cursor.pos() != cursor.pos()) { 
		// Only if the cursor has really moved
		
		if (old_cursor.pos() > 0
		    && old_cursor.pos() < old_cursor.par()->Last()
		    && old_cursor.par()->IsLineSeparator(old_cursor.pos())
		    && old_cursor.par()->IsLineSeparator(old_cursor.pos() - 1)) {
			old_cursor.par()->Erase(old_cursor.pos() - 1);
			RedoParagraphs(bview, old_cursor, old_cursor.par()->Next());
			// correct cursor
			if (old_cursor.par() == cursor.par() &&
			    cursor.pos() > old_cursor.pos()) {
				SetCursorIntern(bview, cursor.par(),
						cursor.pos() - 1);
			} else
				SetCursorIntern(bview, cursor.par(),
						cursor.pos());
			return;
		}
	}

	// Do not delete empty paragraphs with keepempty set.
	if ((textclasslist.Style(bview->buffer()->params.textclass,
				 old_cursor.par()->GetLayout())).keepempty)
		return;

	LyXCursor tmpcursor;

	if (old_cursor.par() != cursor.par()) {
		if ((old_cursor.par()->Last() == 0
		      || (old_cursor.par()->Last() == 1
			  && old_cursor.par()->IsLineSeparator(0)))
#ifndef NEW_INSETS
		     && old_cursor.par()->FirstPhysicalPar()
		     == old_cursor.par()->LastPhysicalPar()
#endif
			) {
			// ok, we will delete anything
			
			// make sure that you do not delete any environments
#ifndef NEW_INSETS
			if ((
				old_cursor.par()->footnoteflag != LyXParagraph::OPEN_FOOTNOTE &&
			     !(old_cursor.row()->previous() 
			       && old_cursor.row()->previous()->par()->footnoteflag == LyXParagraph::OPEN_FOOTNOTE)
			     && !(old_cursor.row()->next() 
				  && old_cursor.row()->next()->par()->footnoteflag == LyXParagraph::OPEN_FOOTNOTE))
			    || (old_cursor.par()->footnoteflag == LyXParagraph::OPEN_FOOTNOTE
				&& ((old_cursor.row()->previous() 
				     && old_cursor.row()->previous()->par()->footnoteflag == LyXParagraph::OPEN_FOOTNOTE)
				    || (old_cursor.row()->next()
					&& old_cursor.row()->next()->par()->footnoteflag == LyXParagraph::OPEN_FOOTNOTE))
				    )) {
#endif
				status = LyXText::NEED_MORE_REFRESH;
				deleted = true;
				
				if (old_cursor.row()->previous()) {
					refresh_row = old_cursor.row()->previous();
					refresh_y = old_cursor.y() - old_cursor.row()->baseline() - refresh_row->height();
					tmpcursor = cursor;
					cursor = old_cursor; // that undo can restore the right cursor position
					LyXParagraph * endpar = old_cursor.par()->next;
					if (endpar && endpar->GetDepth()) {
						while (endpar && endpar->GetDepth()) {
#ifndef NEW_INSETS
							endpar = endpar->LastPhysicalPar()->Next();
#else
							endpar = endpar->Next();
#endif
						}
					}
					SetUndo(bview->buffer(), Undo::DELETE,
						old_cursor.par()->previous,
						endpar);
					cursor = tmpcursor;

					// delete old row
					RemoveRow(old_cursor.row());
					if (OwnerParagraph() == old_cursor.par()) {
						OwnerParagraph(OwnerParagraph()->next);
					}
					// delete old par
					delete old_cursor.par();
					
					/* Breakagain the next par. Needed
					 * because of the parindent that
					 * can occur or dissappear. The
					 * next row can change its height,
					 * if there is another layout before */
					if (refresh_row->next()) {
						BreakAgain(bview, refresh_row->next());
						UpdateCounters(bview, refresh_row);
					}
					SetHeightOfRow(bview, refresh_row);
				} else {
					refresh_row = old_cursor.row()->next();
					refresh_y = old_cursor.y() - old_cursor.row()->baseline();
					
					tmpcursor = cursor;
					cursor = old_cursor; // that undo can restore the right cursor position
					LyXParagraph * endpar = old_cursor.par()->next;
					if (endpar && endpar->GetDepth()) {
						while (endpar && endpar->GetDepth()) {
#ifndef NEW_INSETS
							endpar = endpar->LastPhysicalPar()->Next();
#else
							endpar = endpar->Next();
#endif
						}
					}
					SetUndo(bview->buffer(), Undo::DELETE,
						old_cursor.par()->previous,
						endpar);
					cursor = tmpcursor;

					// delete old row
					RemoveRow(old_cursor.row());
					// delete old par
					if (OwnerParagraph() == old_cursor.par()) {
						OwnerParagraph(OwnerParagraph()->next);
					}
					delete old_cursor.par();
					
					/* Breakagain the next par. Needed
					   because of the parindent that can
					   occur or dissappear.
					   The next row can change its height,
					   if there is another layout before
					*/ 
					if (refresh_row) {
						BreakAgain(bview, refresh_row);
						UpdateCounters(bview, refresh_row->previous());
					}
				}
				
				// correct cursor y

				SetCursorIntern(bview, cursor.par(), cursor.pos());

				if (sel_cursor.par()  == old_cursor.par()
				    && sel_cursor.pos() == sel_cursor.pos()) {
					// correct selection
					sel_cursor = cursor;
				}
#ifndef NEW_INSETS
			}
#endif
		}
		if (!deleted) {
			if (old_cursor.par()->StripLeadingSpaces(bview->buffer()->params.textclass)) {
				RedoParagraphs(bview, old_cursor, old_cursor.par()->Next());
				// correct cursor y
				SetCursorIntern(bview, cursor.par(), cursor.pos());
				sel_cursor = cursor;
			}
		}
	}
}


LyXParagraph * LyXText::GetParFromID(int id)
{
	LyXParagraph * result = FirstParagraph();
	while (result && result->id() != id)
		result = result->next;
	return result;
}


// undo functions
bool LyXText::TextUndo(BufferView * bview)
{
	if (inset_owner)
		return false;
	// returns false if no undo possible
	Undo * undo = bview->buffer()->undostack.pop();
	if (undo) {
		FinishUndo();
		if (!undo_frozen)
			bview->buffer()->redostack
				.push(CreateUndo(bview->buffer(), undo->kind, 
						 GetParFromID(undo->number_of_before_par),
						 GetParFromID(undo->number_of_behind_par)));
	}
	return TextHandleUndo(bview, undo);
}


bool LyXText::TextRedo(BufferView * bview)
{
	if (inset_owner)
		return false;
	// returns false if no redo possible
	Undo * undo = bview->buffer()->redostack.pop();
	if (undo) {
		FinishUndo();
		if (!undo_frozen)
			bview->buffer()->undostack
				.push(CreateUndo(bview->buffer(), undo->kind, 
						 GetParFromID(undo->number_of_before_par),
						 GetParFromID(undo->number_of_behind_par)));
	}
	return TextHandleUndo(bview, undo);
}


bool LyXText::TextHandleUndo(BufferView * bview, Undo * undo)
{
	if (inset_owner)
		return false;
	// returns false if no undo possible
	bool result = false;
	if (undo) {
		LyXParagraph * before =
			GetParFromID(undo->number_of_before_par); 
		LyXParagraph * behind =
			GetParFromID(undo->number_of_behind_par); 
		LyXParagraph * tmppar;
		LyXParagraph * tmppar2;
		LyXParagraph * endpar;
		LyXParagraph * tmppar5;
    
		// if there's no before take the beginning
		// of the document for redoing
		if (!before)
			SetCursorIntern(bview, FirstParagraph(), 0);

		// replace the paragraphs with the undo informations

		LyXParagraph * tmppar3 = undo->par;
		undo->par = 0; // otherwise the undo destructor would delete the paragraph
		LyXParagraph * tmppar4 = tmppar3;
		if (tmppar4){
			while (tmppar4->next)
				tmppar4 = tmppar4->next;
		} // get last undo par
    
		// now remove the old text if there is any
		if (before != behind || (!behind && !before)){
			if (before)
				tmppar5 = before->next;
			else
				tmppar5 = OwnerParagraph();
			tmppar2 = tmppar3;
			while (tmppar5 && tmppar5 != behind){
				tmppar = tmppar5;
				tmppar5 = tmppar5->next;
				// a memory optimization for edit: Only layout information
				// is stored in the undo. So restore the text informations.
				if (undo->kind == Undo::EDIT) {
					tmppar2->setContentsFromPar(tmppar);
					tmppar->clearContents();
					tmppar2 = tmppar2->next;
				}
			}
		}
    
		// put the new stuff in the list if there is one
		if (tmppar3){
			if (before)
				before->next = tmppar3;
			else
				OwnerParagraph(tmppar3);
			tmppar3->previous = before;
		} else {
			if (!before)
				OwnerParagraph(behind);
		}
		if (tmppar4) {
			tmppar4->next = behind;
			if (behind)
				behind->previous = tmppar4;
		}
    
    
		// Set the cursor for redoing
		if (before) {
#ifndef NEW_INSETS
			SetCursorIntern(bview, before->FirstSelfrowPar(), 0);
#else
			SetCursorIntern(bview, before, 0);
#endif
#ifndef NEW_INSETS
			// check wether before points to a closed float and open it if necessary
			if (before && before->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE
			    && before->next && before->next->footnoteflag != LyXParagraph::NO_FOOTNOTE){
				tmppar4 = before;
				while (tmppar4->previous && 
				       tmppar4->previous->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE)
					tmppar4 = tmppar4->previous;
				while (tmppar4 && tmppar4->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE){
					tmppar4->footnoteflag = LyXParagraph::OPEN_FOOTNOTE;
					tmppar4 = tmppar4->next;
				}
			}
#endif
		}

#ifndef NEW_INSETS
		// open a cosed footnote at the end if necessary
		if (behind && behind->previous && 
		    behind->previous->footnoteflag != LyXParagraph::NO_FOOTNOTE &&
		    behind->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE){
			while (behind && behind->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE){
				behind->footnoteflag = LyXParagraph::OPEN_FOOTNOTE;
				behind = behind->next;
			}
		}
#endif
    
		// calculate the endpar for redoing the paragraphs.
		if (behind) {
#ifndef NEW_INSETS
			if (behind->footnoteflag != LyXParagraph::CLOSED_FOOTNOTE)
				endpar = behind->LastPhysicalPar()->Next();
			else
				endpar = behind->NextAfterFootnote()->LastPhysicalPar()->Next();
#else
				endpar = behind->Next();
#endif
		} else
			endpar = behind;
    
		tmppar = GetParFromID(undo->number_of_cursor_par);
		RedoParagraphs(bview, cursor, endpar); 
		if (tmppar){
			SetCursorIntern(bview, tmppar, undo->cursor_pos);
			UpdateCounters(bview, cursor.row());
		}
		result = true;
		delete undo;
	}
	FinishUndo();
	return result;
}


void LyXText::FinishUndo()
{
	if (inset_owner)
		return;
	// makes sure the next operation will be stored
	undo_finished = true;
}


void LyXText::FreezeUndo()
{
	if (inset_owner)
		return;
	// this is dangerous and for internal use only
	undo_frozen = true;
}


void LyXText::UnFreezeUndo()
{
	if (inset_owner)
		return;
	// this is dangerous and for internal use only
	undo_frozen = false;
}


void LyXText::SetUndo(Buffer * buf, Undo::undo_kind kind,
		      LyXParagraph const * before,
		      LyXParagraph const * behind) const
{
	if (inset_owner)
		return;
	if (!undo_frozen)
		buf->undostack.push(CreateUndo(buf, kind, before, behind));
	buf->redostack.clear();
}


void LyXText::SetRedo(Buffer * buf, Undo::undo_kind kind,
		      LyXParagraph const * before, LyXParagraph const * behind)
{
	if (inset_owner)
		return;
	buf->redostack.push(CreateUndo(buf, kind, before, behind));
}


Undo * LyXText::CreateUndo(Buffer * buf, Undo::undo_kind kind,
			   LyXParagraph const * before,
			   LyXParagraph const * behind) const
{
	if (inset_owner)
		return 0;

	int before_number = -1;
	int behind_number = -1;
	if (before)
		before_number = before->id();
	if (behind)
		behind_number = behind->id();
	// Undo::EDIT  and Undo::FINISH are
	// always finished. (no overlapping there)
	// overlapping only with insert and delete inside one paragraph: 
	// Nobody wants all removed  character
	// appear one by one when undoing. 
	// EDIT is special since only layout information, not the
	// contents of a paragaph are stored.
	if (!undo_finished && (kind != Undo::EDIT) && (kind != Undo::FINISH)){
		// check wether storing is needed
		if (!buf->undostack.empty() && 
		    buf->undostack.top()->kind == kind &&
		    buf->undostack.top()->number_of_before_par ==  before_number &&
		    buf->undostack.top()->number_of_behind_par ==  behind_number ){
			// no undo needed
			return 0;
		}
	}
	// create a new Undo
	LyXParagraph * undopar;
	LyXParagraph * tmppar;
	LyXParagraph * tmppar2;

	LyXParagraph * start = 0;
	LyXParagraph * end = 0;
  
	if (before)
		start = before->next;
	else
		start = FirstParagraph();
	if (behind)
		end = behind->previous;
	else {
		end = FirstParagraph();
		while (end->next)
			end = end->next;
	}

	if (start && end && (start != end->next) &&
	    ((before != behind) || (!before && !behind)))
	{
		tmppar = start;
		tmppar2 = tmppar->Clone();
		tmppar2->id(tmppar->id());

		// a memory optimization: Just store the layout information
		// when only edit
		if (kind == Undo::EDIT){
			//tmppar2->text.clear();
			tmppar2->clearContents();
		}

		undopar = tmppar2;
  
		while (tmppar != end && tmppar->next) {
			tmppar = tmppar->next;
			tmppar2->next = tmppar->Clone();
			tmppar2->next->id(tmppar->id());
			// a memory optimization: Just store the layout
			// information when only edit
			if (kind == Undo::EDIT){
				//tmppar2->next->text.clear();
				tmppar2->clearContents();
			}
			tmppar2->next->previous = tmppar2;
			tmppar2 = tmppar2->next;
		}
		tmppar2->next = 0;
	} else
		undopar = 0; // nothing to replace (undo of delete maybe)

#ifndef NEW_INSETS
	int cursor_par = cursor.par()->ParFromPos(cursor.pos())->id();
	int cursor_pos =  cursor.par()->PositionInParFromPos(cursor.pos());
#else
	int cursor_par = cursor.par()->id();
	int cursor_pos =  cursor.pos();
#endif

	Undo * undo = new Undo(kind, 
			       before_number, behind_number,  
			       cursor_par, cursor_pos, 
			       undopar);
  
	undo_finished = false;
	return undo;
}


void LyXText::SetCursorParUndo(Buffer * buf)
{
	if (inset_owner)
		return;
	SetUndo(buf, Undo::FINISH,
#ifndef NEW_INSETS
		cursor.par()->ParFromPos(cursor.pos())->previous, 
		cursor.par()->ParFromPos(cursor.pos())->next
#else
		cursor.par()->previous, 
		cursor.par()->next
#endif
		); 
}


void LyXText::toggleAppendix(BufferView * bview)
{
#ifndef NEW_INSETS
	LyXParagraph * par = cursor.par()->FirstPhysicalPar();
#else
	LyXParagraph * par = cursor.par();
#endif
	bool start = !par->start_of_appendix;

	// ensure that we have only one start_of_appendix in this document
	LyXParagraph * tmp = FirstParagraph();
	for (; tmp; tmp = tmp->next)
		tmp->start_of_appendix = 0;
	par->start_of_appendix = start;

	// we can set the refreshing parameters now
	status = LyXText::NEED_MORE_REFRESH;
	refresh_y = 0;
	refresh_row = 0; // not needed for full update
	UpdateCounters(bview, 0);
	SetCursor(bview, cursor.par(), cursor.pos());
}


LyXParagraph * LyXText::OwnerParagraph() const
{
	if (inset_owner)
		return inset_owner->par;

	return bv_owner->buffer()->paragraph;
}


LyXParagraph * LyXText::OwnerParagraph(LyXParagraph * p) const
{
	if (inset_owner)
		inset_owner->par = p;
	else
		bv_owner->buffer()->paragraph = p;
	return 0;
}
