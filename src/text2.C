/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-1999 The LyX Team.
 *
 * ======================================================*/

#include <config.h>

#include <cctype>
#include FORMS_H_LOCATION

#ifdef __GNUG__
#pragma implementation "lyxtext.h"
#pragma implementation "undo.h"
#endif

#include "LString.h"
#include "lyxparagraph.h"
#include "insets/inseterror.h"
#include "layout.h"
#include "LyXView.h"
#include "support/textutils.h"
#include "lyx_cb.h"
#include "undo.h"
#include "minibuffer.h"
#include "buffer.h"
#include "bufferparams.h"
#include "lyx_gui_misc.h"
#include "lyxtext.h"
#include "gettext.h"

extern MiniBuffer * minibuffer;

// Constructor
LyXText::LyXText(int pw, Buffer * p)
{
	firstrow = 0;
	lastrow = 0;
	currentrow = 0;
	currentrow_y = 0;
	paperwidth = pw;
	parameters = &p->params;
	params = p;
	number_of_rows = 0;
	refresh_y= 0;
	status = LyXText::UNCHANGED;
	LyXParagraph * par = p->paragraph;
	current_font = GetFont(par, 0);
   
	height = 0;
   
	while (par) {
		InsertParagraph(par, lastrow);
		par = par->Next();
	}
	/* set cursor at the very top position */
	selection = true;		       /* these setting is necessary 
						* because of the delete-empty-
						* paragraph mechanism in
						* SetCursor */
	SetCursor(firstrow->par, 0);
	sel_cursor = cursor;
	selection = false;
	mark_set = false;
   
	/* no rebreak necessary */ 
	need_break_row = 0;
   
	undo_finished = true;
	undo_frozen = false;

	// Default layouttype for copy environment type
	copylayouttype = 0;
}


// Destructor
LyXText::~LyXText()
{
	// Delete all rows, this does not touch the paragraphs!
	Row * tmprow = firstrow;
	while (firstrow) {
		tmprow = firstrow->next;
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
#ifdef NEW_TEXT
LyXFont LyXText::GetFont(LyXParagraph * par,
			 LyXParagraph::size_type pos)
#else
LyXFont LyXText::GetFont(LyXParagraph * par, int pos)
#endif
{
	LyXLayout const & layout = 
		textclasslist.Style(parameters->textclass, par->GetLayout());

	char par_depth = par->GetDepth();
	// We specialize the 95% common case:
	if (par->footnoteflag == LyXParagraph::NO_FOOTNOTE && !par_depth) {
		if (pos >= 0){
			// 95% goes here
			if (layout.labeltype == LABEL_MANUAL
			    && pos < BeginningOfMainBody(par)) {
				// 1% goes here
				return par->GetFontSettings(pos).
						realize(layout.reslabelfont);
			} else
				return par->GetFontSettings(pos).
						realize(layout.resfont);
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
		if (pos < BeginningOfMainBody(par)) {
			// 1% goes here
			layoutfont = layout.labelfont;
		} else {
			// 99% goes here
			layoutfont = layout.font;
		}
		tmpfont = par->GetFontSettings(pos);
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
					Style(parameters->textclass,
					      par->GetLayout()).font);
			par_depth = par->GetDepth();
		}
	}

	tmpfont.realize(textclasslist.TextClass(parameters->textclass).defaultfont());

	// Cosmetic improvement: If this is an open footnote, make the font 
	// smaller.
	if (par->footnoteflag == LyXParagraph::OPEN_FOOTNOTE
	    && par->footnotekind == LyXParagraph::FOOTNOTE) {
		tmpfont.decSize();
	}

	return tmpfont;
}


#ifdef NEW_TEXT
void LyXText::SetCharFont(LyXParagraph * par,
			  LyXParagraph::size_type pos,
			  LyXFont font)
#else
void LyXText::SetCharFont(LyXParagraph * par, int pos, LyXFont font)
#endif
{
	/* let the insets convert their font */ 
	if (par->GetChar(pos) == LYX_META_INSET) {
		if (par->GetInset(pos))
			font = par->GetInset(pos)->ConvertFont(font);
	}

	LyXLayout const & layout = textclasslist.Style(parameters->textclass,
					   par->GetLayout());

	// Get concrete layout font to reduce against
	LyXFont layoutfont;

	if (pos < BeginningOfMainBody(par))
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
						Style(parameters->textclass,
						      tp->GetLayout()).font);
		}
	}

	layoutfont.realize(textclasslist.TextClass(parameters->textclass).defaultfont());

	if (par->footnoteflag == LyXParagraph::OPEN_FOOTNOTE
	    && par->footnotekind == LyXParagraph::FOOTNOTE) {
		layoutfont.decSize();
	}

	// Now, reduce font against full layout font
	font.reduce(layoutfont);

	par->SetFont(pos, font);
}


/* inserts a new row behind the specified row, increments
 * the touched counters */
#ifdef NEW_TEXT
void LyXText::InsertRow(Row * row, LyXParagraph * par,
			LyXParagraph::size_type pos)
#else
void LyXText::InsertRow(Row * row, LyXParagraph * par, int pos)
#endif
{
	Row * tmprow = new Row;
	if (!row) {
		tmprow->previous = 0;
		tmprow->next = firstrow;
		firstrow = tmprow;
	}
	else {
		tmprow->previous = row;
		tmprow->next = row->next;
		row->next = tmprow;
	}
   
	if (tmprow->next)
		tmprow->next->previous = tmprow;
   
	if (tmprow->previous)
		tmprow->previous->next = tmprow;
   
   
	tmprow->par = par;
	tmprow->pos = pos;
   
	if (row == lastrow)
		lastrow = tmprow;
	number_of_rows++;			       /* one more row  */
}


/* removes the row and reset the touched counters */
void LyXText::RemoveRow(Row * row)
{
	/* this must not happen before the currentrow for clear reasons.
	   so the trick is just to set the current row onto the previous
	   row of this row */
	long unused_y;
	GetRow(row->par, row->pos, unused_y);
	currentrow = currentrow->previous;
	if (currentrow)
		currentrow_y -= currentrow->height;
	else
		currentrow_y = 0;
   
	if (row->next)
		row->next->previous = row->previous;
	if (!row->previous) {
		firstrow = row->next;
	}
	else  {
		row->previous->next = row->next;
	}
	if (row == lastrow)
		lastrow = row->previous;
   
	height -= row->height;	       /* the text becomes smaller  */
   
	delete row;
	--number_of_rows;			       /* one row less  */
}


/* remove all following rows of the paragraph of the specified row. */
void LyXText::RemoveParagraph(Row * row)
{
	Row * tmprow;

	LyXParagraph * tmppar = row->par;
	row = row->next;
    
	while (row && row->par == tmppar) {
		tmprow = row->next;
		RemoveRow(row);
		row = tmprow;
	}
}
   
  
/* insert the specified paragraph behind the specified row */
void LyXText::InsertParagraph(LyXParagraph * par, Row * row)
{
	InsertRow(row, par, 0);	       /* insert a new row, starting 
					* at postition 0 */

	SetCounter(par);		       /* set the counters  */
   
	/* and now append the whole paragraph behind the new row */
	if (!row) {
		firstrow->height = 0;
		AppendParagraph(firstrow);
	}
	else {
		row->next->height = 0;
		AppendParagraph(row->next);
	}
}
    

void LyXText::ToggleFootnote()
{
	LyXParagraph * par = cursor.par->ParFromPos(cursor.pos);
	if (par->next && par->next->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE){
		OpenFootnote();
		minibuffer->Set(_("Opened float"));
	}
	else {
		minibuffer->Set(_("Closed float"));
		CloseFootnote();
	}
}


void LyXText::OpenStuff()
{
     	if (cursor.pos == 0 && cursor.par->bibkey){
		cursor.par->bibkey->Edit(0,0);
	}
	else if (cursor.pos < cursor.par->Last() 
		 && cursor.par->GetChar(cursor.pos) == LYX_META_INSET
		 && cursor.par->GetInset(cursor.pos)->Editable()) {
		minibuffer->Set(cursor.par->GetInset(cursor.pos)->EditMessage());
		if (cursor.par->GetInset(cursor.pos)->Editable() != 2)
			SetCursorParUndo();
		cursor.par->GetInset(cursor.pos)->Edit(0,0);
	}
	else {
		ToggleFootnote();
	}
}


void LyXText::CloseFootnote()
{
	LyXParagraph * endpar, * tmppar;
	Row * row;
   
	LyXParagraph * par = cursor.par->ParFromPos(cursor.pos);
   
	/* if the cursor is not in an open footnote, or 
	 * there is no open footnote in this paragraph, just return. */ 
	if (cursor.par->footnoteflag != LyXParagraph::OPEN_FOOTNOTE) {
      
		if (!par->next
		    || par->next->footnoteflag != LyXParagraph::OPEN_FOOTNOTE) {
			minibuffer->Set(_("Nothing to do"));
			return;
		}
   
		/* ok, move the cursor right before the footnote */ 
 
		/* just a little faster than using CursorRight() */
		for (cursor.pos=0; cursor.par->ParFromPos(cursor.pos)!=par; cursor.pos++);
		/* now the cursor is at the beginning of the physical par */
#ifdef NEW_TEXT
		SetCursor(cursor.par,
			  cursor.pos +
			  cursor.par->ParFromPos(cursor.pos)->text.size());
#else
		SetCursor(cursor.par, cursor.pos + cursor.par->ParFromPos(cursor.pos)->last);
#endif
	}
	else  {
		/* we are in a footnote, so let us move at the beginning */ 
		/* this is just faster than using just CursorLeft() */ 
       
		tmppar = cursor.par;
		while (tmppar->footnoteflag == LyXParagraph::OPEN_FOOTNOTE) {
			/* just a little bit faster than movin the cursor */
			tmppar = tmppar->Previous();
		}
		SetCursor(tmppar, tmppar->Last());
	}
   
	/* the cursor must be exactly before the footnote */ 
	par = cursor.par->ParFromPos(cursor.pos);
   
	status = LyXText::NEED_MORE_REFRESH;
	refresh_row = cursor.row;
	refresh_y = cursor.y - cursor.row->baseline;
   
	tmppar = cursor.par;
	endpar = par->NextAfterFootnote()->Next();
	row = cursor.row;
   
	tmppar->CloseFootnote(cursor.pos);

	while (tmppar != endpar) {
		RemoveRow(row->next);
		if (row->next)
			tmppar = row->next->par;
		else
			tmppar = 0;
	}
   
	AppendParagraph(cursor.row);
   
	SetCursor(cursor.par, cursor.pos);
	sel_cursor = cursor;
   
	/* just necessary */
	if (cursor.row->next)
		SetHeightOfRow(cursor.row->next);
}


/* used in setlayout */
// Asger is not sure we want to do this...
void LyXText::MakeFontEntriesLayoutSpecific(LyXParagraph * par)
{
	LyXFont layoutfont, tmpfont;
   
	LyXLayout const & layout = textclasslist.Style(parameters->textclass, par->GetLayout());

#ifdef NEW_TEXT
	for (LyXParagraph::size_type pos = 0;
	     pos < par->Last(); ++pos) {
#else
	for (int pos = 0; pos < par->Last(); pos++) {
#endif
		if (pos < BeginningOfMainBody(par))
			layoutfont = layout.labelfont;
		else
			layoutfont = layout.font;
      
		tmpfont = par->GetFontSettings(pos);
		tmpfont.reduce(layoutfont);
		par->SetFont(pos, tmpfont);
	}
}


/* set layout over selection and make a total rebreak of those paragraphs */
void  LyXText::SetLayout(char layout)
{
	LyXCursor tmpcursor;

	/* if there is no selection just set the layout of the current paragraph  */
	if (!selection) {
		sel_start_cursor = cursor;       /* dummy selection  */
		sel_end_cursor = cursor;
	}

	LyXParagraph * endpar = sel_end_cursor.par->LastPhysicalPar()->Next();
	LyXParagraph * undoendpar = endpar;

	if (endpar && endpar->GetDepth()) {
		while (endpar && endpar->GetDepth()) {
			endpar = endpar->LastPhysicalPar()->Next();
			undoendpar = endpar;
		}
	}
	else if (endpar) {
		endpar = endpar->Next();	       /* because of parindents etc.  */
	}
   
	SetUndo(Undo::EDIT, 
		sel_start_cursor.par->ParFromPos(sel_start_cursor.pos)->previous, 
		undoendpar);

	tmpcursor = cursor;		       /* store the current cursor  */

	/* ok we have a selection. This is always between sel_start_cursor
	 * and sel_end cursor */ 
	cursor = sel_start_cursor;
   
	LyXLayout const & lyxlayout = textclasslist.Style(parameters->textclass, layout);
   
	while (cursor.par != sel_end_cursor.par) {
		if (cursor.par->footnoteflag ==
		    sel_start_cursor.par->footnoteflag) {
			cursor.par->SetLayout(layout);
			MakeFontEntriesLayoutSpecific(cursor.par);
			LyXParagraph* fppar = cursor.par->FirstPhysicalPar();
			fppar->added_space_top = lyxlayout.fill_top ?
				VSpace(VSpace::VFILL) : VSpace(VSpace::NONE);
			fppar->added_space_bottom = lyxlayout.fill_bottom ? 
				VSpace(VSpace::VFILL) : VSpace(VSpace::NONE); 
			if (lyxlayout.margintype == MARGIN_MANUAL)
				cursor.par->SetLabelWidthString(lyxlayout.labelstring());
			if (lyxlayout.labeltype != LABEL_BIBLIO
			    && fppar->bibkey) {
				delete fppar->bibkey;
				fppar->bibkey = 0;
			}
		}
		cursor.par = cursor.par->Next();
	}
	if (cursor.par->footnoteflag ==
	    sel_start_cursor.par->footnoteflag) {
		cursor.par->SetLayout(layout);
		MakeFontEntriesLayoutSpecific(cursor.par);
		LyXParagraph* fppar = cursor.par->FirstPhysicalPar();
		fppar->added_space_top = lyxlayout.fill_top ?
			VSpace(VSpace::VFILL) : VSpace(VSpace::NONE);
		fppar->added_space_bottom = lyxlayout.fill_bottom ? 
			VSpace(VSpace::VFILL) : VSpace(VSpace::NONE); 
		if (lyxlayout.margintype == MARGIN_MANUAL)
			cursor.par->SetLabelWidthString(lyxlayout.labelstring());
		if (lyxlayout.labeltype != LABEL_BIBLIO
		    && fppar->bibkey) {
			delete fppar->bibkey;
			fppar->bibkey = 0;
		}
	}
   
	RedoParagraphs(sel_start_cursor, endpar);
   
	/* we have to reset the selection, because the
	 * geometry could have changed */ 
	SetCursor(sel_start_cursor.par, sel_start_cursor.pos);
	sel_cursor = cursor;
	SetCursor(sel_end_cursor.par, sel_end_cursor.pos);
	UpdateCounters(cursor.row);
	ClearSelection();
	SetSelection();
	SetCursor(tmpcursor.par, tmpcursor.pos);
}


/* increment depth over selection and
 * make a total rebreak of those paragraphs */
void  LyXText::IncDepth()
{
	// If there is no selection, just use the current paragraph
	if (!selection) {
		sel_start_cursor = cursor;       /* dummy selection */
		sel_end_cursor = cursor;
	}

	// We end at the next paragraph with depth 0
	LyXParagraph * endpar = sel_end_cursor.par->LastPhysicalPar()->Next();
	LyXParagraph * undoendpar = endpar;

	if (endpar && endpar->GetDepth()) {
		while (endpar && endpar->GetDepth()) {
			endpar = endpar->LastPhysicalPar()->Next();
			undoendpar = endpar;
		}
	}
	else if (endpar) {
		endpar = endpar->Next();	       /* because of parindents etc.  */
	}
	
	SetUndo(Undo::EDIT, 
		sel_start_cursor.par->ParFromPos(sel_start_cursor.pos)->previous, 
		undoendpar);

	LyXCursor tmpcursor = cursor;	     /* store the current cursor  */

	/* ok we have a selection. This is always between sel_start_cursor
	 * and sel_end cursor */ 
	cursor = sel_start_cursor;
   
	bool anything_changed = false;
   
	while (true) {
		// NOTE: you can't change the depth of a bibliography entry
		if (cursor.par->footnoteflag ==
		    sel_start_cursor.par->footnoteflag
		    && textclasslist.Style(parameters->textclass,
				      cursor.par->GetLayout()
				     ).labeltype != LABEL_BIBLIO) {
			LyXParagraph * prev = cursor.par->FirstPhysicalPar()->Previous();
			if (prev 
			    && (prev->GetDepth() - cursor.par->GetDepth() > 0
				|| (prev->GetDepth() == cursor.par->GetDepth()
				    && textclasslist.Style(parameters->textclass,
						      prev->GetLayout()).isEnvironment()))) {
				cursor.par->FirstPhysicalPar()->depth++;
				anything_changed = true;
				}
		}
		if (cursor.par == sel_end_cursor.par)
                       break;
		cursor.par = cursor.par->Next();
	}
   
	/* if nothing changed set all depth to 0 */ 
	if (!anything_changed) {
		cursor = sel_start_cursor;
		while (cursor.par != sel_end_cursor.par) {
			cursor.par->FirstPhysicalPar()->depth = 0;
			cursor.par = cursor.par->Next();
		}
		if (cursor.par->footnoteflag == sel_start_cursor.par->footnoteflag)
			cursor.par->FirstPhysicalPar()->depth = 0;
	}
   
	RedoParagraphs(sel_start_cursor, endpar);
   
	/* we have to reset the selection, because the
	 * geometry could have changed */ 
	SetCursor(sel_start_cursor.par, sel_start_cursor.pos);
	sel_cursor = cursor;
	SetCursor(sel_end_cursor.par, sel_end_cursor.pos);
	UpdateCounters(cursor.row);
	ClearSelection();
	SetSelection();
	SetCursor(tmpcursor.par, tmpcursor.pos);
}


/* decrement depth over selection and
 * make a total rebreak of those paragraphs */
void  LyXText::DecDepth()
{
	/* if there is no selection just set the layout of the current paragraph  */
	if (!selection) {
		sel_start_cursor = cursor;       /* dummy selection  */
		sel_end_cursor = cursor;
	}
   
	LyXParagraph * endpar = sel_end_cursor.par->LastPhysicalPar()->Next();
	LyXParagraph * undoendpar = endpar;

	if (endpar && endpar->GetDepth()) {
		while (endpar && endpar->GetDepth()) {
			endpar = endpar->LastPhysicalPar()->Next();
			undoendpar = endpar;
		}
	}
	else if (endpar) {
		endpar = endpar->Next();	       /* because of parindents etc.  */
	}
   
	SetUndo(Undo::EDIT, 
		sel_start_cursor.par->ParFromPos(sel_start_cursor.pos)->previous, 
		undoendpar);

	LyXCursor tmpcursor = cursor;		       /* store the current cursor  */

	/* ok we have a selection. This is always between sel_start_cursor
	 * and sel_end cursor */ 
	cursor = sel_start_cursor;

	while (true) {
		if (cursor.par->footnoteflag ==
		    sel_start_cursor.par->footnoteflag) {
			if (cursor.par->FirstPhysicalPar()->depth)
				cursor.par->FirstPhysicalPar()->depth--;
		}
		if (cursor.par == sel_end_cursor.par)
			break;
		cursor.par = cursor.par->Next();
	}

	RedoParagraphs(sel_start_cursor, endpar);
   
	/* we have to reset the selection, because the
	 * geometry could have changed */ 
	SetCursor(sel_start_cursor.par, sel_start_cursor.pos);
	sel_cursor = cursor;
	SetCursor(sel_end_cursor.par, sel_end_cursor.pos);
	UpdateCounters(cursor.row);
	ClearSelection();
	SetSelection();
	SetCursor(tmpcursor.par, tmpcursor.pos);
}


/* set font over selection and make a total rebreak of those paragraphs */
void  LyXText::SetFont(LyXFont font, bool toggleall)
{
	/* if there is no selection just set the current_font */
	if (!selection) {
		// Determine basis font
		LyXFont layoutfont;
		if (cursor.pos < BeginningOfMainBody(cursor.par))
			layoutfont = GetFont(cursor.par, -2);
		else
			layoutfont = GetFont(cursor.par, -1);
		// Update current font
		real_current_font.update(font,toggleall);

		// Reduce to implicit settings
		current_font = real_current_font;
		current_font.reduce(layoutfont);
		// And resolve it completely
		real_current_font.realize(layoutfont);
		return;
	}

	LyXCursor tmpcursor = cursor;		       /* store the current cursor  */
   
	/* ok we have a selection. This is always between sel_start_cursor
	 * and sel_end cursor */ 
   
	SetUndo(Undo::EDIT, 
		sel_start_cursor.par->ParFromPos(sel_start_cursor.pos)->previous, 
		sel_end_cursor.par->ParFromPos(sel_end_cursor.pos)->next); 
	cursor = sel_start_cursor;
	while (cursor.par != sel_end_cursor.par ||
	       (cursor.par->footnoteflag == sel_start_cursor.par->footnoteflag
		&& cursor.pos < sel_end_cursor.pos)) 
	{
		if (cursor.pos < cursor.par->Last()
		    && cursor.par->footnoteflag
		    == sel_start_cursor.par->footnoteflag) {   /* an open footnote
								* should behave
								* like a closed */
			LyXFont newfont = GetFont(cursor.par,cursor.pos);
			newfont.update(font,toggleall);
			SetCharFont(cursor.par, cursor.pos, newfont);
			cursor.pos++;
		} else {
			cursor.pos = 0;
			cursor.par = cursor.par->Next();
		}
	}
   
	RedoParagraphs(sel_start_cursor, sel_end_cursor.par->Next());
   
	/* we have to reset the selection, because the
	 * geometry could have changed */ 
	SetCursor(sel_start_cursor.par, sel_start_cursor.pos);
	sel_cursor = cursor;
	SetCursor(sel_end_cursor.par, sel_end_cursor.pos);
	ClearSelection();
	SetSelection();
	SetCursor(tmpcursor.par, tmpcursor.pos);
}


void LyXText::RedoHeightOfParagraph(LyXCursor cursor)
{
	Row * tmprow = cursor.row;
	long y = cursor.y - tmprow->baseline;

	SetHeightOfRow(tmprow);
	LyXParagraph * first_phys_par = tmprow->par->FirstPhysicalPar();
	/* find the first row of the paragraph */
	if (first_phys_par != tmprow->par)
		while (tmprow->previous && tmprow->previous->par != first_phys_par)  {
			tmprow = tmprow->previous;
			y -= tmprow->height;
			SetHeightOfRow(tmprow);
		}
	while (tmprow->previous && tmprow->previous->par == first_phys_par)  {
		tmprow = tmprow->previous;
		y -= tmprow->height;
		SetHeightOfRow(tmprow);
	}
   
	/* we can set the refreshing parameters now */
	status = LyXText::NEED_MORE_REFRESH;
	refresh_y = y;
	refresh_row = tmprow;
	SetCursor(cursor.par, cursor.pos);
}


void LyXText::RedoDrawingOfParagraph(LyXCursor cursor)
{
	Row * tmprow = cursor.row;
   
	long y = cursor.y - tmprow->baseline;
	SetHeightOfRow(tmprow);
	LyXParagraph * first_phys_par = tmprow->par->FirstPhysicalPar();
	/* find the first row of the paragraph */
	if (first_phys_par != tmprow->par)
		while (tmprow->previous && tmprow->previous->par != first_phys_par)  {
			tmprow = tmprow->previous;
			y -= tmprow->height;
		}
	while (tmprow->previous && tmprow->previous->par == first_phys_par)  {
		tmprow = tmprow->previous;
		y -= tmprow->height;
      
	}
   
	/* we can set the refreshing parameters now */
	if (status == LyXText::UNCHANGED || y < refresh_y) {
		refresh_y = y;
		refresh_row = tmprow;
	}
	status = LyXText::NEED_MORE_REFRESH;
	SetCursor(cursor.par, cursor.pos);
}


/* deletes and inserts again all paragaphs between the cursor
* and the specified par 
* This function is needed after SetLayout and SetFont etc. */
void LyXText::RedoParagraphs(LyXCursor cursor, LyXParagraph * endpar)
{
	Row * tmprow2;
	LyXParagraph * tmppar, * first_phys_par;
   
	Row * tmprow = cursor.row;
   
	long y = cursor.y - tmprow->baseline;
   
	if (!tmprow->previous){
		first_phys_par = FirstParagraph();   // a trick/hack for UNDO
	}
	else {
		first_phys_par = tmprow->par->FirstPhysicalPar();
		/* find the first row of the paragraph */
		if (first_phys_par != tmprow->par)
			while (tmprow->previous && tmprow->previous->par != first_phys_par)  {
				tmprow = tmprow->previous;
				y -= tmprow->height;
			}
		while (tmprow->previous && tmprow->previous->par == first_phys_par)  {
			tmprow = tmprow->previous;
			y -= tmprow->height;
		}
	}
   
	/* we can set the refreshing parameters now */
	status = LyXText::NEED_MORE_REFRESH;
	refresh_y = y;
	refresh_row = tmprow->previous;	       /* the real refresh row will
						* be deleted, so I store
						* the previous here */ 
	/* remove it */
	if (tmprow->next)
		tmppar = tmprow->next->par;
	else
		tmppar = 0;
	while (tmppar != endpar) {
		RemoveRow(tmprow->next);
		if (tmprow->next)
			tmppar = tmprow->next->par;
		else
			tmppar = 0;
	}  
   
	/* remove the first one */
	tmprow2 = tmprow;		       /* this is because tmprow->previous
						* can be 0 */
	tmprow = tmprow->previous;
	RemoveRow(tmprow2);
   
	tmppar = first_phys_par;

	do {
		if (tmppar) {
			InsertParagraph(tmppar, tmprow);
			if (!tmprow)
				tmprow = firstrow;
			while (tmprow->next && tmprow->next->par == tmppar)
				tmprow = tmprow->next;
			tmppar = tmppar->Next();
	 
		}
	}
	while (tmppar != endpar);
   
	/* this is because of layout changes */ 
	if (refresh_row) {
		refresh_y -= refresh_row->height;
		SetHeightOfRow(refresh_row);   
	}
	else {
		refresh_row = firstrow;
		refresh_y = 0;
		SetHeightOfRow(refresh_row);   
	}
   
	if (tmprow && tmprow->next)
		SetHeightOfRow(tmprow->next);
}


int LyXText::FullRebreak()
{
	if (need_break_row) {
		BreakAgain(need_break_row);
		need_break_row = 0;
		return 1;
	}
	return 0;
}


/* important for the screen */


/* the cursor set functions have a special mechanism. When they
* realize, that you left an empty paragraph, they will delete it.
* They also delet the corresponding row */
   
/* need the selection cursor: */ 
void LyXText::SetSelection()
{
	if (!selection) {
		last_sel_cursor = sel_cursor;
		sel_start_cursor = sel_cursor;
		sel_end_cursor = sel_cursor;
	}
   
	selection = True;
   
	/* first the toggling area */ 
	if (cursor.y < last_sel_cursor.y ||
	    (cursor.y == last_sel_cursor.y && cursor.x < last_sel_cursor.x)) {
		toggle_end_cursor = last_sel_cursor;
		toggle_cursor = cursor;
	}
	else {
		toggle_end_cursor = cursor;
		toggle_cursor = last_sel_cursor;
	}
   
	last_sel_cursor = cursor;
   
	/* and now the whole selection */ 
   
	if (sel_cursor.y < cursor.y ||
	    (sel_cursor.y == cursor.y && sel_cursor.x < cursor.x)) {
		sel_end_cursor = cursor;
		sel_start_cursor = sel_cursor;
	}
	else {
		sel_end_cursor = sel_cursor; 
		sel_start_cursor = cursor;
	}
   
	/* a selection with no contents is not a selection */ 
	if (sel_start_cursor.x == sel_end_cursor.x && 
	    sel_start_cursor.y == sel_end_cursor.y)
		selection = false;
}


void LyXText::ClearSelection()
{
	selection = false;
	mark_set = false;
}


void  LyXText::CursorHome()
{
	SetCursor(cursor.par, cursor.row->pos);
}


void  LyXText::CursorEnd()
{
	if (!cursor.row->next || cursor.row->next->par != cursor.row->par)
		SetCursor(cursor.par, RowLast(cursor.row) + 1);
	else {
		if (cursor.par->Last() && 
		    (cursor.par->GetChar(RowLast(cursor.row)) == ' '
		     || cursor.par->IsNewline(RowLast(cursor.row))))
			SetCursor(cursor.par, RowLast(cursor.row));
		else
			SetCursor(cursor.par, RowLast(cursor.row) + 1);
	}
        if (cursor.par->table) {
                int cell = NumberOfCell(cursor.par, cursor.pos);
                if (cursor.par->table->RowHasContRow(cell) &&
                    cursor.par->table->CellHasContRow(cell)<0) {
                        if (!cursor.row->next || cursor.row->next->par != cursor.row->par)
                                SetCursor(cursor.par, RowLast(cursor.row) + 1);
                        else {
                                if (cursor.par->Last() && 
                                    (cursor.par->GetChar(RowLast(cursor.row)) == ' '
                                     || cursor.par->IsNewline(RowLast(cursor.row))))
                                        SetCursor(cursor.par, RowLast(cursor.row));
                                else
                                        SetCursor(cursor.par, RowLast(cursor.row) + 1);
                        }
                }
        }
}


void  LyXText::CursorTop()
{
	while (cursor.par->Previous())
		cursor.par = cursor.par->Previous();
	SetCursor(cursor.par, 0);
}


void  LyXText::CursorBottom()
{
	while (cursor.par->Next())
		cursor.par = cursor.par->Next();
	SetCursor(cursor.par, cursor.par->Last());
}
   
   
/* returns a pointer to the row near the specified y-coordinate
* (relative to the whole text). y is set to the real beginning
* of this row */ 
Row * LyXText::GetRowNearY(long & y)
{
	Row * tmprow;
	long tmpy;
   
	if (currentrow){
		tmprow = currentrow;
		tmpy = currentrow_y;
	}
	else {
		tmprow = firstrow;
		tmpy = 0;
	}

	if (tmpy<=y)
		while (tmprow->next && tmpy + tmprow->height <= y) {
			tmpy += tmprow->height;
			tmprow = tmprow->next;
		}
	else
		while (tmprow->previous && tmpy > y) {
			tmprow = tmprow->previous;
			tmpy -= tmprow->height;
		}

	currentrow = tmprow;
	currentrow_y = tmpy;

	y = tmpy;			       /* return the real y  */
	return tmprow;
}
   

void LyXText::ToggleFree(LyXFont font, bool toggleall)
{
	// If the mask is completely neutral, tell user
	if (font == LyXFont(LyXFont::ALL_IGNORE)){
		// Could only happen with user style
		minibuffer->Set(_("No font change defined. Use Character under"
				  " the Layout menu to define font change."));
		return;
	}

	// Try implicit word selection
	LyXCursor resetCursor = cursor;
	int implicitSelection = SelectWordWhenUnderCursor();

	// Set font
	SetFont(font,toggleall);
	//minibuffer->Set(_("Font style changed"));

	/* Implicit selections are cleared afterwards and cursor is set to the
	   original position. */
	if (implicitSelection) {
		ClearSelection();
		cursor = resetCursor;
		SetCursor( cursor.par, cursor.pos );
		sel_cursor = cursor;
	}
}


#ifdef NEW_TEXT
LyXParagraph::size_type
LyXText::BeginningOfMainBody(LyXParagraph * par)
#else
int LyXText::BeginningOfMainBody(LyXParagraph * par)
#endif
{
	if (textclasslist.Style(parameters->textclass, par->GetLayout()).labeltype != LABEL_MANUAL)
		return 0;
	else
		return par->BeginningOfMainBody();
}


/* if there is a selection, reset every environment you can find
* in the selection, otherwise just the environment you are in */ 
void LyXText::MeltFootnoteEnvironment()
{
	LyXParagraph * tmppar, * firsttmppar;
   
	ClearSelection();
   
	/* is is only allowed, if the cursor is IN an open footnote.
	 * Otherwise it is too dangerous */ 
	if (cursor.par->footnoteflag != LyXParagraph::OPEN_FOOTNOTE)
		return;
   
	SetUndo(Undo::FINISH, 
		cursor.par->PreviousBeforeFootnote()->previous,
		cursor.par->NextAfterFootnote()->next);

	/* ok, move to the beginning of the footnote. */ 
	while (cursor.par->footnoteflag == LyXParagraph::OPEN_FOOTNOTE)
		cursor.par = cursor.par->Previous();
   
	SetCursor(cursor.par, cursor.par->Last());
	/* this is just faster than using CursorLeft(); */ 
   
	firsttmppar = cursor.par->ParFromPos(cursor.pos);
	tmppar = firsttmppar;
	/* tmppar is now the paragraph right before the footnote */

#ifdef NEW_TEXT
	char first_footnote_par_is_not_empty = tmppar->next->text.size();
#else
	char first_footnote_par_is_not_empty = tmppar->next->last;
#endif
   
	while (tmppar->next && tmppar->next->footnoteflag == LyXParagraph::OPEN_FOOTNOTE) {
		tmppar = tmppar->next;	       /* I use next instead of Next(),
						* because there cannot be any
						* footnotes in a footnote
						* environment */
		tmppar->footnoteflag = LyXParagraph::NO_FOOTNOTE;
      
		/* remember the captions and empty paragraphs */
		if ((textclasslist.Style(parameters->textclass,
				    tmppar->GetLayout()).labeltype == LABEL_SENSITIVE)
		    || !tmppar->Last())
			tmppar->SetLayout(0);
	}
   
	/* now we will paste the ex-footnote, if the layouts allow it */
	/* first restore the layout of the paragraph right behind the footnote*/ 
	if (tmppar->next) 
		tmppar->next->MakeSameLayout(cursor.par);

	/* first the end */ 
	if ((!tmppar->GetLayout() && !tmppar->table)
	    || (tmppar->Next() && (!tmppar->Next()->Last()
				   || tmppar->Next()->HasSameLayout(tmppar)))) {
		if (tmppar->Next()->Last() && tmppar->Next()->IsLineSeparator(0))
			tmppar->Next()->Erase(0);
		tmppar->PasteParagraph();
	}

	tmppar = tmppar->Next();	       /* make shure tmppar cannot be touched
						* by the pasting of the beginning */

	/* then the beginning */ 
	/* if there is no space between the text and the footnote, so we insert
	 * a blank 
	 * (only if the previous par and the footnotepar are not empty!) */
	if ((!firsttmppar->next->GetLayout() && !firsttmppar->next->table)
	    || firsttmppar->HasSameLayout(firsttmppar->next)) {
#ifdef NEW_TEXT
		if (firsttmppar->text.size()
		    && !firsttmppar->IsSeparator(firsttmppar->text.size() - 1)
#else
		if (firsttmppar->last
		    && !firsttmppar->IsSeparator(firsttmppar->last - 1)
#endif
		    && first_footnote_par_is_not_empty) {
			firsttmppar->next->InsertChar(0, ' ');
		}
		firsttmppar->PasteParagraph();
	}
   
	/* now redo the paragaphs */
	RedoParagraphs(cursor, tmppar);
   
	SetCursor(cursor.par, cursor.pos);
   
	/* sometimes it can happen, that there is a counter change */ 
	Row * row = cursor.row;
	while (row->next && row->par != tmppar && row->next->par != tmppar)
		row = row->next;
	UpdateCounters(row);
   
   
	ClearSelection();
}


/* the DTP switches for paragraphs. LyX will store them in the 
* first physicla paragraph. When a paragraph is broken, the top settings 
* rest, the bottom settings are given to the new one. So I can make shure, 
* they do not duplicate themself and you cannnot make dirty things with 
* them!  */ 

void LyXText::SetParagraph(bool line_top, bool line_bottom,
			   bool pagebreak_top, bool pagebreak_bottom,
			   VSpace space_top, VSpace space_bottom,
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
	LyXParagraph * endpar = sel_end_cursor.par->LastPhysicalPar()->Next();
	LyXParagraph * undoendpar = endpar;

	if (endpar && endpar->GetDepth()) {
		while (endpar && endpar->GetDepth()) {
			endpar = endpar->LastPhysicalPar()->Next();
			undoendpar = endpar;
		}
	}
	else if (endpar) {
		endpar = endpar->Next(); /* because of parindents etc.  */
	}
   
	SetUndo(Undo::EDIT, 
		sel_start_cursor.par->ParFromPos(sel_start_cursor.pos)->previous, 
		undoendpar);

	
	LyXParagraph * tmppar = sel_end_cursor.par;
	while (tmppar != sel_start_cursor.par->FirstPhysicalPar()->Previous()){
		SetCursor(tmppar->FirstPhysicalPar(), 0);
		status = LyXText::NEED_MORE_REFRESH;
		refresh_row = cursor.row;
		refresh_y = cursor.y - cursor.row->baseline;
		if (cursor.par->footnoteflag ==
		    sel_start_cursor.par->footnoteflag) {
			cursor.par->line_top = line_top;
			cursor.par->line_bottom = line_bottom;
			cursor.par->pagebreak_top = pagebreak_top;
			cursor.par->pagebreak_bottom = pagebreak_bottom;
			cursor.par->added_space_top = space_top;
			cursor.par->added_space_bottom = space_bottom;
				/* does the layout allow the new alignment? */
			if (align == LYX_ALIGN_LAYOUT)
				align = textclasslist
					.Style(parameters->textclass,
					       cursor.par->GetLayout()).align;
			if (align & textclasslist
			    .Style(parameters->textclass,
				   cursor.par->GetLayout()).alignpossible) {
				if (align == textclasslist
				    .Style(parameters->textclass,
					   cursor.par->GetLayout()).align)
					cursor.par->align = LYX_ALIGN_LAYOUT;
				else
					cursor.par->align = align;
			}
			cursor.par->SetLabelWidthString(labelwidthstring);
			cursor.par->noindent = noindent;
		}
		
		tmppar = cursor.par->FirstPhysicalPar()->Previous();
	}
	
	RedoParagraphs(sel_start_cursor, endpar);
	
	ClearSelection();
	SetCursor(sel_start_cursor.par, sel_start_cursor.pos);
	sel_cursor = cursor;
	SetCursor(sel_end_cursor.par, sel_end_cursor.pos);
	SetSelection();
	SetCursor(tmpcursor.par, tmpcursor.pos);
}


void LyXText::SetParagraphExtraOpt(int type,
                                   char const * width,
                                   char const * widthp,
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
	LyXParagraph * endpar = sel_end_cursor.par->LastPhysicalPar()->Next();
	LyXParagraph * undoendpar = endpar;

	if (endpar && endpar->GetDepth()) {
		while (endpar && endpar->GetDepth()) {
			endpar = endpar->LastPhysicalPar()->Next();
			undoendpar = endpar;
		}
	}
	else if (endpar) {
		endpar = endpar->Next(); /* because of parindents etc.  */
	}
   
	SetUndo(Undo::EDIT, 
		sel_start_cursor.par->ParFromPos(sel_start_cursor.pos)->previous, 
		undoendpar);
	
	tmppar = sel_end_cursor.par;
	while(tmppar != sel_start_cursor.par->FirstPhysicalPar()->Previous()) {
                SetCursor(tmppar->FirstPhysicalPar(), 0);
                status = LyXText::NEED_MORE_REFRESH;
                refresh_row = cursor.row;
                refresh_y = cursor.y - cursor.row->baseline;
                if (cursor.par->footnoteflag ==
                    sel_start_cursor.par->footnoteflag) {
                        if (type == PEXTRA_NONE) {
                                if (cursor.par->pextra_type != PEXTRA_NONE) {
                                        cursor.par->UnsetPExtraType();
                                        cursor.par->pextra_type=PEXTRA_NONE;
                                }
                        } else {
                                cursor.par->SetPExtraType(type,width,widthp);
                                cursor.par->pextra_hfill = hfill;
                                cursor.par->pextra_start_minipage = start_minipage;
                                cursor.par->pextra_alignment = alignment;
                        }
                }
                tmppar = cursor.par->FirstPhysicalPar()->Previous();
        }
	RedoParagraphs(sel_start_cursor, endpar);
	ClearSelection();
	SetCursor(sel_start_cursor.par, sel_start_cursor.pos);
	sel_cursor = cursor;
	SetCursor(sel_end_cursor.par, sel_end_cursor.pos);
	SetSelection();
	SetCursor(tmpcursor.par, tmpcursor.pos);
}


static char const * alphaCounter(int n){
  static char result[2];
  result[1] = 0;
  if (n == 0)
    return "";
  else {
    result[0] = 64 + n;
    if (n > 26)
      return "??";
  }
  return result;
}


/* set the counter of a paragraph. This includes the labels */ 
void LyXText::SetCounter(LyXParagraph * par)
{
	int i;
   
	/* this is only relevant for the beginning of paragraph */ 
	par = par->FirstPhysicalPar();

	LyXLayout const & layout = textclasslist.Style(parameters->textclass, 
					   par->GetLayout());

	LyXTextClass const & textclass = textclasslist.TextClass(parameters->textclass);

	/* copy the prev-counters to this one, unless this is the start of a 
	   footnote or of a bibliography or the very first paragraph */
	if (par->Previous()
	    && !(par->Previous()->footnoteflag == LyXParagraph::NO_FOOTNOTE 
		    && par->footnoteflag == LyXParagraph::OPEN_FOOTNOTE
		    && par->footnotekind == LyXParagraph::FOOTNOTE)
	    && !(textclasslist.Style(parameters->textclass,
				par->Previous()->GetLayout()
				).labeltype != LABEL_BIBLIO
		 && layout.labeltype == LABEL_BIBLIO)) {
		for (i=0; i<10; i++) {
			par->setCounter(i, par->Previous()->GetFirstCounter(i));
		}
		par->appendix = par->Previous()->FirstPhysicalPar()->appendix;
		if (!par->appendix && par->start_of_appendix){
		  par->appendix = true;
		  for (i=0; i<10; i++) {
		    par->setCounter(i, 0);
		  }  
		}
		par->enumdepth = par->Previous()->FirstPhysicalPar()->enumdepth;
		par->itemdepth = par->Previous()->FirstPhysicalPar()->itemdepth;
	}
	else {
		for (i=0; i<10; i++) {
			par->setCounter(i, 0);
		}  
		par->appendix = par->start_of_appendix;
		par->enumdepth = 0;
		par->itemdepth = 0;
	}

        // if this is an open marginnote and this is the first
        // entry in the marginnote and the enclosing
        // environment is an enum/item then correct for the
        // LaTeX behaviour (ARRae)
        if(par->footnoteflag == LyXParagraph::OPEN_FOOTNOTE
	   && par->footnotekind == LyXParagraph::MARGIN
           && par->Previous()
           && par->Previous()->footnoteflag != LyXParagraph::OPEN_FOOTNOTE
           && (par->PreviousBeforeFootnote()
               && textclasslist.Style(parameters->textclass,
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

	/* Maybe we have to increment the enumeration depth.
	 * BUT, enumeration in a footnote is considered in isolation from its
	 *	surrounding paragraph so don't increment if this is the
	 *	first line of the footnote
	 * AND, bibliographies can't have their depth changed ie. they
	 *	are always of depth 0
	 */
	if (par->Previous()
	    && par->Previous()->GetDepth() < par->GetDepth()
	    && textclasslist.Style(parameters->textclass,
			      par->Previous()->GetLayout()
			     ).labeltype == LABEL_COUNTER_ENUMI
	    && par->enumdepth < 3
	    && !(par->Previous()->footnoteflag == LyXParagraph::NO_FOOTNOTE 
		    && par->footnoteflag == LyXParagraph::OPEN_FOOTNOTE
		    && par->footnotekind == LyXParagraph::FOOTNOTE)
	    && layout.labeltype != LABEL_BIBLIO) {
		par->enumdepth++;
	}

	/* Maybe we have to decrement the enumeration depth, see note above */
	if (par->Previous()
	    && par->Previous()->GetDepth() > par->GetDepth()
	    && !(par->Previous()->footnoteflag == LyXParagraph::NO_FOOTNOTE
		    && par->footnoteflag == LyXParagraph::OPEN_FOOTNOTE
		    && par->footnotekind == LyXParagraph::FOOTNOTE)
	    && layout.labeltype != LABEL_BIBLIO) {
		par->enumdepth = par->DepthHook(par->GetDepth())->enumdepth;
		par->setCounter(6 + par->enumdepth,
			par->DepthHook(par->GetDepth())->getCounter(6 + par->enumdepth));
		/* reset the counters.
		 * A depth change is like a breaking layout
		 */
		for (i=6 + par->enumdepth + 1; i<10;i++)
			par->setCounter(i, 0);
	}
   
	if (!par->labelstring.empty()) {
		par->labelstring.clear();
	}
   
	if (layout.margintype == MARGIN_MANUAL) {
		if (par->labelwidthstring.empty()) {
			par->SetLabelWidthString(layout.labelstring());
		}
	}
	else {
		par->SetLabelWidthString(string());
	}
   
	/* is it a layout that has an automatic label ? */ 
	if (layout.labeltype >=  LABEL_FIRST_COUNTER) {
      
		i = layout.labeltype - LABEL_FIRST_COUNTER;
		if (i >= 0 && i<=parameters->secnumdepth) {
			par->incCounter(i);	// increment the counter  
	 
			char * s = new char[50];

			// Is there a label? Useful for Chapter layout
			if (!par->appendix){
				if (!layout.labelstring().empty())
					par->labelstring = layout.labelstring();
				else
					par->labelstring.clear();
                        }
			else {
				if (!layout.labelstring_appendix().empty())
					par->labelstring = layout.labelstring_appendix();
				else
					par->labelstring.clear();
			}
 
			if (!par->appendix){
				switch (2 * LABEL_FIRST_COUNTER -
					textclass.maxcounter() + i) {
				case LABEL_COUNTER_CHAPTER:
					sprintf(s, "%d",
						par->getCounter(i));
					break;
				case LABEL_COUNTER_SECTION:
					sprintf(s, "%d.%d",
						par->getCounter(i - 1),
						par->getCounter(i));
					break;
				case LABEL_COUNTER_SUBSECTION:
					sprintf(s, "%d.%d.%d",
						par->getCounter(i-2),
						par->getCounter(i-1),
						par->getCounter(i));
					break;
				case LABEL_COUNTER_SUBSUBSECTION:
					sprintf(s, "%d.%d.%d.%d",
						par->getCounter(i-3),
						par->getCounter(i-2),
						par->getCounter(i-1),
						par->getCounter(i));
					break;
				case LABEL_COUNTER_PARAGRAPH:
					sprintf(s, "%d.%d.%d.%d.%d",
						par->getCounter(i-4),
						par->getCounter(i-3),
						par->getCounter(i-2),
						par->getCounter(i-1),
						par->getCounter(i));
					break;
				case LABEL_COUNTER_SUBPARAGRAPH:
					sprintf(s, "%d.%d.%d.%d.%d.%d",
						par->getCounter(i-5),
						par->getCounter(i-4),
						par->getCounter(i-3),
						par->getCounter(i-2),
						par->getCounter(i-1),
						par->getCounter(i));
					break;
				default:
					sprintf(s, "%d.", par->getCounter(i));
                                        break;
				}
			}
			else {
				switch (2 * LABEL_FIRST_COUNTER - textclass.maxcounter() + i) {
				case LABEL_COUNTER_CHAPTER:
					sprintf(s, "%s",
						alphaCounter(par->getCounter(i)));
					break;
				case LABEL_COUNTER_SECTION:
					sprintf(s, "%s.%d",
						alphaCounter(par->getCounter(i - 1)),
						par->getCounter(i));
					break;
				case LABEL_COUNTER_SUBSECTION:
					sprintf(s, "%s.%d.%d",
						alphaCounter(par->getCounter(i-2)),
						par->getCounter(i-1),
						par->getCounter(i));
					break;
				case LABEL_COUNTER_SUBSUBSECTION:
					sprintf(s, "%s.%d.%d.%d",
						alphaCounter(par->getCounter(i-3)),
						par->getCounter(i-2),
						par->getCounter(i-1),
						par->getCounter(i));
					break;
				case LABEL_COUNTER_PARAGRAPH:
					sprintf(s, "%s.%d.%d.%d.%d",
						alphaCounter(par->getCounter(i-4)),
						par->getCounter(i-3),
						par->getCounter(i-2),
						par->getCounter(i-1),
						par->getCounter(i));
					break;
				case LABEL_COUNTER_SUBPARAGRAPH:
					sprintf(s, "%s.%d.%d.%d.%d.%d",
						alphaCounter(par->getCounter(i-5)),
						par->getCounter(i-4),
						par->getCounter(i-3),
						par->getCounter(i-2),
						par->getCounter(i-1),
						par->getCounter(i));
					break;
				default:
					sprintf(s, "%c.", par->getCounter(i));
					break;
				}
			}
	 
			par->labelstring += s;
			delete[] s;
	 
			for (i++; i<10; i++) {
				/* reset the following counters  */
				par->setCounter(i, 0);
			}
		} else if (layout.labeltype < LABEL_COUNTER_ENUMI) {
			for (i++; i<10; i++) {
				/* reset the following counters  */
				par->setCounter(i, 0);
			}
		} else if (layout.labeltype == LABEL_COUNTER_ENUMI) {
			par->incCounter(i + par->enumdepth);
			char * s = new char[25];
			int number = par->getCounter(i + par->enumdepth);
			switch (par->enumdepth) {
			case 1:
				sprintf(s, "(%c)", (number % 27) + 'a' - 1);
				break;
			case 2:
				switch (number) {
				case 1: sprintf(s, "i."); break;
				case 2: sprintf(s, "ii."); break;
				case 3: sprintf(s, "iii."); break;
				case 4: sprintf(s, "iv."); break;
				case 5: sprintf(s, "v."); break;
				case 6: sprintf(s, "vi."); break;
				case 7: sprintf(s, "vii."); break;
				case 8: sprintf(s, "viii."); break;
				case 9: sprintf(s, "ix."); break;
				case 10: sprintf(s, "x."); break;
				case 11: sprintf(s, "xi."); break;
				case 12: sprintf(s, "xii."); break;
				case 13: sprintf(s, "xiii."); break;
				default:
					sprintf(s, "\\roman{%d}.", number);
					break;
				}
				break;
			case 3:
				sprintf(s, "%c.", (number % 27) + 'A' - 1);
				break;
			default:
				sprintf(s, "%d.", number);
				break;
			}
			par->labelstring = s;
			delete[] s;

			for (i += par->enumdepth + 1;i<10;i++)
				par->setCounter(i, 0);  /* reset the following counters  */
	 
		} 
	} else if (layout.labeltype == LABEL_BIBLIO) {// ale970302
	    i = LABEL_COUNTER_ENUMI - LABEL_FIRST_COUNTER + par->enumdepth;
	    par->incCounter(i);
	    int number = par->getCounter(i);
	    if (!par->bibkey)
	      par->bibkey = new InsetBibKey();
	    par->bibkey->setCounter(number);
	    par->labelstring = layout.labelstring();
	    
	    // In biblio should't be following counters but...
	}						
	else  {
		string s = layout.labelstring();
      
		/* the caption hack: */
      
		if (layout.labeltype == LABEL_SENSITIVE) {
			if (par->footnoteflag != LyXParagraph::NO_FOOTNOTE
			    && (par->footnotekind == LyXParagraph::FIG
			    || par->footnotekind == LyXParagraph::WIDE_FIG))
				s = "Figure:";
			else if (par->footnoteflag != LyXParagraph::NO_FOOTNOTE
				 && (par->footnotekind == LyXParagraph::TAB
				 || par->footnotekind == LyXParagraph::WIDE_TAB))
				s = "Table:";
			else if (par->footnoteflag != LyXParagraph::NO_FOOTNOTE
				 && par->footnotekind == LyXParagraph::ALGORITHM)
				s = "Algorithm:";
			else {
				/* par->SetLayout(0); 
				   s = layout->labelstring;  */
				s = "Senseless: "; 
	   
			}
		}
		par->labelstring = s;
      
		/* reset the enumeration counter. They are always resetted
		 * when there is any other layout between */ 
		for (i=6 + par->enumdepth; i<10;i++)
			par->setCounter(i, 0);
	}
}


/* Updates all counters BEHIND the row. Changed paragraphs
* with a dynamic left margin will be rebroken. */ 
void LyXText::UpdateCounters(Row * row)
{
	LyXParagraph * par;
	if (!row) {
		row = firstrow;
		par = row->par;
	}
	else {
		if (row->par->next && row->par->next->footnoteflag != LyXParagraph::OPEN_FOOTNOTE) {
			par = row->par->LastPhysicalPar()->Next();
		} else {
			par = row->par->next;
		}
	}

	while (par) {
		while (row->par != par)
			row = row->next;
      
		SetCounter(par);
      
		/* now  check for the headline layouts. remember that they
		 * have a dynamic left margin */ 
		if (!par->IsDummy()
		    && ( textclasslist.Style(parameters->textclass, par->layout).margintype == MARGIN_DYNAMIC
			 || textclasslist.Style(parameters->textclass, par->layout).labeltype == LABEL_SENSITIVE)
			){
	 
			/* Rebreak the paragraph */ 
			RemoveParagraph(row);
			AppendParagraph(row);
       
			/* think about the damned open footnotes! */ 
			while (par->Next() &&
			       (par->Next()->footnoteflag == LyXParagraph::OPEN_FOOTNOTE
				|| par->Next()->IsDummy())){
				par = par->Next();
				if (par->IsDummy()) {
					while (row->par != par)
						row = row->next;
					RemoveParagraph(row);
					AppendParagraph(row);
				}
			}
		}
     
		par = par->LastPhysicalPar()->Next();
     
	}
}


/* insets an inset. */ 
void LyXText::InsertInset(Inset *inset)
{
	SetUndo(Undo::INSERT, 
		cursor.par->ParFromPos(cursor.pos)->previous, 
		cursor.par->ParFromPos(cursor.pos)->next);
	cursor.par->InsertChar(cursor.pos, LYX_META_INSET);
	cursor.par->InsertInset(cursor.pos, inset);
	InsertChar(LYX_META_INSET);  /* just to rebreak and refresh correctly.
				      * The character will not be inserted a
				      * second time */
}


/* this is for the simple cut and paste mechanism */ 
static LyXParagraph * simple_cut_buffer = 0;
static char simple_cut_buffer_textclass = 0;

void DeleteSimpleCutBuffer()
{
	if (!simple_cut_buffer)
		return;
	LyXParagraph *tmppar;

	while (simple_cut_buffer) {
		tmppar =  simple_cut_buffer;
		simple_cut_buffer = simple_cut_buffer->next;
		delete tmppar;
	}
	simple_cut_buffer = 0;
}


void LyXText::copyEnvironmentType()
{
	copylayouttype = cursor.par->GetLayout();
}


void LyXText::pasteEnvironmentType()
{
	SetLayout(copylayouttype);
}


void LyXText::CutSelection(bool doclear)
{
	/* This doesn't make sense, if there is no selection */ 
	if (!selection) {
		return;
	}
   
	/* OK, we have a selection. This is always between sel_start_cursor
	 * and sel_end cursor */
	LyXParagraph * tmppar;
   
	/* Check whether there are half footnotes in the selection */
	if (sel_start_cursor.par->footnoteflag != LyXParagraph::NO_FOOTNOTE
	    || sel_end_cursor.par->footnoteflag != LyXParagraph::NO_FOOTNOTE){
		tmppar = sel_start_cursor.par;
		while (tmppar != sel_end_cursor.par){
			if (tmppar->footnoteflag != sel_end_cursor.par->footnoteflag){
				WriteAlert(_("Impossible operation"), _("Don't know what to do with half floats."), _("sorry."));
				return;
			}
			tmppar = tmppar->Next();
		}
	}

	/* table stuff -- begin*/
	if (sel_start_cursor.par->table || sel_end_cursor.par->table){
		if ( sel_start_cursor.par != sel_end_cursor.par){
			WriteAlert(_("Impossible operation"), _("Don't know what to do with half tables."), _("sorry."));
			return;
		}
		sel_start_cursor.par->table->Reinit();
	}
	/* table stuff -- end*/

	// make sure that the depth behind the selection are restored, too
	LyXParagraph * endpar = sel_end_cursor.par->LastPhysicalPar()->Next();
	LyXParagraph * undoendpar = endpar;

	if (endpar && endpar->GetDepth()) {
		while (endpar && endpar->GetDepth()) {
			endpar = endpar->LastPhysicalPar()->Next();
			undoendpar = endpar;
		}
	}
	else if (endpar) {
		endpar = endpar->Next();	       /* because of parindents etc.  */
	}
   
	SetUndo(Undo::DELETE, 
		sel_start_cursor.par->ParFromPos(sel_start_cursor.pos)->previous, 
		undoendpar);
   
	/* delete the simple_cut_buffer */ 
	DeleteSimpleCutBuffer();
   
	/* set the textclass */
	simple_cut_buffer_textclass = parameters->textclass;

#ifdef WITH_WARNINGS
#warning Asger: Make cut more intelligent here.
#endif
/* 
White paper for "intelligent" cutting:

Example: "This is our text."
Using " our " as selection, cutting will give "This istext.".
Using "our" as selection, cutting will give "This is text.".
Using " our" as selection, cutting will give "This is text.".
Using "our " as selection, cutting will give "This is text.".

All those four selections will (however) paste identically:
Pasting with the cursor right after the "is" will give the
original text with all four selections.

The rationale is to be intelligent such that words are copied,
cut and pasted in a functional manner.

This is not implemented yet.
*/

	bool space_wrapped =
		sel_end_cursor.par->IsLineSeparator(sel_end_cursor.pos);
	if (sel_end_cursor.pos > 0
	    && sel_end_cursor.par->IsLineSeparator(sel_end_cursor.pos - 1)) {
		sel_end_cursor.pos--;  /* please break before a space at
					* the end */
		space_wrapped = true;
	}

	// cut behind a space if there is one
	while (sel_start_cursor.par->Last() > sel_start_cursor.pos
	       && sel_start_cursor.par->IsLineSeparator(sel_start_cursor.pos)
	       && (sel_start_cursor.par != sel_end_cursor.par
		   || sel_start_cursor.pos < sel_end_cursor.pos))
		sel_start_cursor.pos++; 
   
	/* there are two cases: cut only within one paragraph or
	 * more than one paragraph */
   
	if (sel_start_cursor.par->ParFromPos(sel_start_cursor.pos) 
	    == sel_end_cursor.par->ParFromPos(sel_end_cursor.pos)) {
		/* only within one paragraph */
		simple_cut_buffer = new LyXParagraph;
#ifdef NEW_TEXT
		simple_cut_buffer->text.reserve(500);
		LyXParagraph::size_type i =
			sel_start_cursor.pos;
#else
		int i = sel_start_cursor.pos;
#endif
		for (; i< sel_end_cursor.pos; i++){
			/* table stuff -- begin*/
			if (sel_start_cursor.par->table
			    && sel_start_cursor.par->IsNewline(sel_start_cursor.pos)){
				sel_start_cursor.par->CopyIntoMinibuffer(sel_start_cursor.pos);
				sel_start_cursor.pos++;
			} else {
				/* table stuff -- end*/
				sel_start_cursor.par->CopyIntoMinibuffer(sel_start_cursor.pos);
				sel_start_cursor.par->Erase(sel_start_cursor.pos);
			}
			simple_cut_buffer->InsertFromMinibuffer(simple_cut_buffer->Last());
		}
		/* check for double spaces */
		if (sel_start_cursor.pos &&
		    sel_start_cursor.par->Last()>sel_start_cursor.pos &&
		    sel_start_cursor.par->IsLineSeparator(sel_start_cursor.pos - 1) &&
		    sel_start_cursor.par->IsLineSeparator(sel_start_cursor.pos)){
			sel_start_cursor.par->Erase(sel_start_cursor.pos);
		}
		if (space_wrapped)
			simple_cut_buffer->InsertChar(i - sel_start_cursor.pos, ' ');
		endpar = sel_end_cursor.par->Next();
	}
	else {
		/* cut more than one paragraph */ 
   
		sel_end_cursor.par->BreakParagraphConservative(sel_end_cursor.pos);
		/* insert a space at the end if there was one */
		if (space_wrapped)
			sel_end_cursor.par->InsertChar(sel_end_cursor.par->Last(), ' ');
   
		sel_end_cursor.par = sel_end_cursor.par->Next();
		sel_end_cursor.pos = 0;
   
		cursor = sel_end_cursor;
   
		/* please break behind a space, if there is one. The space should
		 * be copied too */ 
		if (sel_start_cursor.par->IsLineSeparator(sel_start_cursor.pos))
			sel_start_cursor.pos++;
   
		sel_start_cursor.par->BreakParagraphConservative(sel_start_cursor.pos);
		if (!sel_start_cursor.pos
		    || sel_start_cursor.par->IsLineSeparator(sel_start_cursor.pos - 1)
		    || sel_start_cursor.par->IsNewline(sel_start_cursor.pos - 1)) {
			sel_start_cursor.par->Next()->InsertChar(0, ' ');
		}
   
		/* store the endparagraph for redoing later */
		endpar = sel_end_cursor.par->Next();   /* needed because
							* the sel_end_
							* cursor.par
							* will be pasted!*/
   
		/*store the selection */ 
		simple_cut_buffer = sel_start_cursor.par->ParFromPos(sel_start_cursor.pos)->next;
		simple_cut_buffer->previous = 0;
		sel_end_cursor.par->previous->next = 0;

		/* cut the selection */ 
		sel_start_cursor.par->ParFromPos(sel_start_cursor.pos)->next 
			= sel_end_cursor.par;
   
		sel_end_cursor.par->previous 
			= sel_start_cursor.par->ParFromPos(sel_start_cursor.pos);

		/* care about footnotes */
		if (simple_cut_buffer->footnoteflag) {
			LyXParagraph *tmppar = simple_cut_buffer;
			while (tmppar){
				tmppar->footnoteflag = LyXParagraph::NO_FOOTNOTE;
				tmppar = tmppar->next;
			}
		}

		/* the cut selection should begin with standard layout */
		simple_cut_buffer->Clear(); 
   
		/* paste the paragraphs again, if possible  */
		if (doclear)
			sel_start_cursor.par->Next()->ClearParagraph();
		if (sel_start_cursor.par->FirstPhysicalPar()->HasSameLayout(sel_start_cursor.par->Next())
		    || 
		    !sel_start_cursor.par->Next()->Last())
			sel_start_cursor.par->ParFromPos(sel_start_cursor.pos)->PasteParagraph();

   
		/* maybe a forgotten blank */
		if (sel_start_cursor.pos 
		    && sel_start_cursor.par->IsLineSeparator(sel_start_cursor.pos)
		    && sel_start_cursor.par->IsLineSeparator(sel_start_cursor.pos - 1)) {
			sel_start_cursor.par->Erase(sel_start_cursor.pos);
		}
	}   


	/* sometimes necessary */
	if (doclear)
		sel_start_cursor.par->ClearParagraph();

	RedoParagraphs(sel_start_cursor, endpar);
   
	ClearSelection();
	cursor = sel_start_cursor;
	SetCursor(cursor.par, cursor.pos);
	sel_cursor = cursor;
	UpdateCounters(cursor.row);
}

    
void LyXText::CopySelection()
{
#ifdef NEW_TEXT
	LyXParagraph::size_type i = 0;
#else
	int i = 0;
#endif
	/* this doesnt make sense, if there is no selection */ 
	if (!selection) {
		return;
	}

	/* ok we have a selection. This is always between sel_start_cursor
	 * and sel_end cursor */
	LyXParagraph * tmppar;
   
	/* check wether there are half footnotes in the selection */
	if (sel_start_cursor.par->footnoteflag != LyXParagraph::NO_FOOTNOTE
	    || sel_end_cursor.par->footnoteflag != LyXParagraph::NO_FOOTNOTE){
		tmppar = sel_start_cursor.par;
		while (tmppar != sel_end_cursor.par){
			if (tmppar->footnoteflag != sel_end_cursor.par->footnoteflag){
				WriteAlert(_("Impossible operation"), _("Don't know what to do with half floats."), _("sorry."));
				return;
			}
			tmppar = tmppar->Next();
		}
	}

	/* table stuff -- begin*/
	if (sel_start_cursor.par->table || sel_end_cursor.par->table){
		if ( sel_start_cursor.par != sel_end_cursor.par){
			WriteAlert(_("Impossible operation"), _("Don't know what to do with half tables."), _("sorry."));
			return;
		}
	}
	/* table stuff -- end*/
   
	/* delete the simple_cut_buffer */ 
	DeleteSimpleCutBuffer();

	/* set the textclass */
	simple_cut_buffer_textclass = parameters->textclass;

	// copy behind a space if there is one
	while (sel_start_cursor.par->Last() > sel_start_cursor.pos
	       && sel_start_cursor.par->IsLineSeparator(sel_start_cursor.pos)
	       && (sel_start_cursor.par != sel_end_cursor.par
		   || sel_start_cursor.pos < sel_end_cursor.pos))
		sel_start_cursor.pos++; 

	/* there are two cases: copy only within one paragraph or more than one paragraph */
	if (sel_start_cursor.par->ParFromPos(sel_start_cursor.pos) 
	    == sel_end_cursor.par->ParFromPos(sel_end_cursor.pos)) {
		/* only within one paragraph */
		simple_cut_buffer = new LyXParagraph;
#if NEW_TEXT
		simple_cut_buffer->text.reserve(500);
#endif
		for (i = sel_start_cursor.pos; i < sel_end_cursor.pos; ++i){
			sel_start_cursor.par->CopyIntoMinibuffer(i);
			simple_cut_buffer->InsertFromMinibuffer(i - sel_start_cursor.pos);
		}
	}
	else {
		/* copy more than one paragraph */ 
		/* clone the paragraphs within the selection*/
		tmppar = sel_start_cursor.par->ParFromPos(sel_start_cursor.pos);
		simple_cut_buffer = tmppar->Clone();
		LyXParagraph *tmppar2 = simple_cut_buffer;
     
		while (tmppar != sel_end_cursor.par->ParFromPos(sel_end_cursor.pos)
		       && tmppar->next) {
			tmppar = tmppar->next;
			tmppar2->next = tmppar->Clone();
			tmppar2->next->previous = tmppar2;
			tmppar2=tmppar2->next;
		}
		tmppar2->next = 0;

		/* care about footnotes */
		if (simple_cut_buffer->footnoteflag) {
			tmppar = simple_cut_buffer;
			while (tmppar){
				tmppar->footnoteflag = LyXParagraph::NO_FOOTNOTE;
				tmppar = tmppar->next;
			}
		}
     
		/* the simple_cut_buffer paragraph is too big */
#ifdef NEW_TEXT
		LyXParagraph::size_type tmpi2 =
			sel_start_cursor.par->PositionInParFromPos(sel_start_cursor.pos);
#else
		int tmpi2 =
			sel_start_cursor.par->PositionInParFromPos(sel_start_cursor.pos);
#endif
		for (;tmpi2;tmpi2--)
			simple_cut_buffer->Erase(0);

		/* now tmppar 2 is too big, delete all after sel_end_cursor.pos */
     
		tmpi2 = sel_end_cursor.par->PositionInParFromPos(sel_end_cursor.pos);
#ifdef NEW_TEXT
		while (tmppar2->size() > tmpi2) {
			tmppar2->Erase(tmppar2->text.size() - 1);
		}
#else
		while (tmppar2->last > tmpi2) {
			tmppar2->Erase(tmppar2->last-1);
		}
#endif
	}
}
          

void LyXText::PasteSelection()
{
	/* this does not make sense, if there is nothing to paste */ 
	if (!simple_cut_buffer)
		return;

	LyXParagraph * tmppar;
	LyXParagraph * endpar;

	LyXCursor tmpcursor;

	/* be carefull with footnotes in footnotes */ 
	if (cursor.par->footnoteflag != LyXParagraph::NO_FOOTNOTE) {
      
		/* check whether the cut_buffer includes a footnote */
		tmppar = simple_cut_buffer;
		while (tmppar && tmppar->footnoteflag == LyXParagraph::NO_FOOTNOTE)
			tmppar = tmppar->next;
      
		if (tmppar) {
			WriteAlert(_("Impossible operation"),
				   _("Can't paste float into float!"), _("Sorry."));
			return;
		}
	}

	/* table stuff -- begin*/
	if (cursor.par->table){
		if (simple_cut_buffer->next){
			WriteAlert(_("Impossible operation"),
				   _("Table cell cannot include more than one paragraph!"),
				   _("Sorry."));
			return;
		}
	}
	/* table stuff -- end*/
   
	SetUndo(Undo::INSERT, 
		cursor.par->ParFromPos(cursor.pos)->previous, 
		cursor.par->ParFromPos(cursor.pos)->next); 

	tmpcursor = cursor;

	/* There are two cases: cutbuffer only one paragraph or many */
	if (!simple_cut_buffer->next) {
		/* only within a paragraph */
     
		/* please break behind a space, if there is one */
		while (tmpcursor.par->Last() > tmpcursor.pos
		       && tmpcursor.par->IsLineSeparator(tmpcursor.pos))
			tmpcursor.pos++; 

		tmppar = simple_cut_buffer->Clone();
		/* table stuff -- begin*/
		bool table_too_small = false;
		if (tmpcursor.par->table) {
#ifdef NEW_TEXT
			while (simple_cut_buffer->text.size()
			       && !table_too_small) {
#else
			while (simple_cut_buffer->last && !table_too_small){
#endif
				if (simple_cut_buffer->IsNewline(0)){
					while(tmpcursor.pos < tmpcursor.par->Last() && !tmpcursor.par->IsNewline(tmpcursor.pos))
						tmpcursor.pos++;
					simple_cut_buffer->Erase(0);
					if (tmpcursor.pos < tmpcursor.par->Last())
						tmpcursor.pos++;
					else
						table_too_small = true;
				} else {
					simple_cut_buffer->CutIntoMinibuffer(0);
					simple_cut_buffer->Erase(0);
					tmpcursor.par->InsertFromMinibuffer(tmpcursor.pos);
					tmpcursor.pos++;
				}
			}
		} else {
			/* table stuff -- end*/
#ifdef NEW_TEXT
			while (simple_cut_buffer->text.size()){
#else
			while (simple_cut_buffer->last){
#endif
				simple_cut_buffer->CutIntoMinibuffer(0);
				simple_cut_buffer->Erase(0);
				tmpcursor.par->InsertFromMinibuffer(tmpcursor.pos);
				tmpcursor.pos++;
			}
		}

		delete simple_cut_buffer;
		simple_cut_buffer = tmppar;
		endpar = tmpcursor.par->Next();
	} else {
		/* many paragraphs */

		/* make a copy of the simple cut_buffer */
		tmppar = simple_cut_buffer;
		LyXParagraph * simple_cut_clone = tmppar->Clone();
		LyXParagraph * tmppar2 = simple_cut_clone;
		if (cursor.par->footnoteflag){
			tmppar->footnoteflag = cursor.par->footnoteflag;
			tmppar->footnotekind = cursor.par->footnotekind;
		}
		while (tmppar->next) {
			tmppar = tmppar->next;
			tmppar2->next = tmppar->Clone();
			tmppar2->next->previous = tmppar2;
			tmppar2=tmppar2->next;
			if (cursor.par->footnoteflag){
				tmppar->footnoteflag = cursor.par->footnoteflag;
				tmppar->footnotekind = cursor.par->footnotekind;
			}
		}
     
		/* make sure there is no class difference */ 
		SwitchLayoutsBetweenClasses(simple_cut_buffer_textclass,
					    parameters->textclass,
					    simple_cut_buffer);
     
		/* make the simple_cut_buffer exactly the same layout than
		   the cursor paragraph */
		simple_cut_buffer->MakeSameLayout(cursor.par);
     
		/* find the end of the buffer */ 
		LyXParagraph *lastbuffer = simple_cut_buffer;
		while (lastbuffer->Next())
			lastbuffer=lastbuffer->Next();
     
		/* find the physical end of the buffer */ 
		lastbuffer = simple_cut_buffer;
		while (lastbuffer->Next())
			lastbuffer=lastbuffer->Next();
     
		/* please break behind a space, if there is one. The space 
		 * should be copied too */
		if (cursor.par->Last() > cursor.pos && cursor.par->IsLineSeparator(cursor.pos))
			cursor.pos++; 
     
		bool paste_the_end = false;

		/* open the paragraph for inserting the simple_cut_buffer
		   if necessary */
		if (cursor.par->Last() > cursor.pos || !cursor.par->Next()){
			cursor.par->BreakParagraphConservative(cursor.pos);
			paste_the_end = true;
		}
     
		/* be careful with double spaces */ 
		if ((!cursor.par->Last()
		     || cursor.par->IsLineSeparator(cursor.pos - 1)
		     || cursor.par->IsNewline(cursor.pos - 1))
#ifdef NEW_TEXT
		    && simple_cut_buffer->text.size()
#else
		    && simple_cut_buffer->last
#endif
		    && simple_cut_buffer->IsLineSeparator(0))
			simple_cut_buffer->Erase(0);
     
		/* set the end for redoing later */ 
		endpar = cursor.par->ParFromPos(cursor.pos)->next->Next();
     
		/* paste it! */ 
		lastbuffer->ParFromPos(lastbuffer->Last())->next = cursor.par->ParFromPos(cursor.pos)->next;
		cursor.par->ParFromPos(cursor.pos)->next->previous = lastbuffer->ParFromPos(lastbuffer->Last());
     
		cursor.par->ParFromPos(cursor.pos)->next = simple_cut_buffer;
		simple_cut_buffer->previous = cursor.par->ParFromPos(cursor.pos);
   
		if (cursor.par->ParFromPos(cursor.pos)->Next() == lastbuffer)
			lastbuffer = cursor.par;
     
		cursor.par->ParFromPos(cursor.pos)->PasteParagraph();
     
		/* store the new cursor position  */
		tmpcursor.par = lastbuffer;
		tmpcursor.pos = lastbuffer->Last();
     
		/* maybe some pasting */ 
		if (lastbuffer->Next() && paste_the_end) {
			if (lastbuffer->Next()->HasSameLayout(lastbuffer)) {
	 
				/* be careful witth double spaces */ 
				if ((!lastbuffer->Last()
				     || lastbuffer->IsLineSeparator(lastbuffer->Last() - 1)
				     || lastbuffer->IsNewline(lastbuffer->Last() - 1))
				    && lastbuffer->Next()->Last()
				    && lastbuffer->Next()->IsLineSeparator(0))
					lastbuffer->Next()->Erase(0);
	 
				lastbuffer->ParFromPos(lastbuffer->Last())->PasteParagraph();
	 
			}
			else if (!lastbuffer->Next()->Last()) {
				lastbuffer->Next()->MakeSameLayout(lastbuffer);
	 
				/* be careful witth double spaces */ 
				if ((!lastbuffer->Last()
				     || lastbuffer->IsLineSeparator(lastbuffer->Last() - 1)
				     || lastbuffer->IsNewline(lastbuffer->Last() - 1))
				    && lastbuffer->Next()->Last()
				    && lastbuffer->Next()->IsLineSeparator(0))
					lastbuffer->Next()->Erase(0);
	 
				lastbuffer->ParFromPos(lastbuffer->Last())->PasteParagraph();
	 
			}
			else if (!lastbuffer->Last()) {
				lastbuffer->MakeSameLayout(lastbuffer->next);
	 
				/* be careful witth double spaces */ 
				if ((!lastbuffer->Last()
				     || lastbuffer->IsLineSeparator(lastbuffer->Last() - 1)
				     || lastbuffer->IsNewline(lastbuffer->Last() - 1))
				    && lastbuffer->Next()->Last()
				    && lastbuffer->Next()->IsLineSeparator(0))
					lastbuffer->Next()->Erase(0);
	 
				lastbuffer->ParFromPos(lastbuffer->Last())->PasteParagraph();
	 
			}
			else lastbuffer->Next()->ClearParagraph();
		}

		/* restore the simple cut buffer */
		simple_cut_buffer = simple_cut_clone;
	}

	RedoParagraphs(cursor, endpar);
    
	SetCursor(cursor.par, cursor.pos);
	ClearSelection();
   
	sel_cursor = cursor;
	SetCursor(tmpcursor.par, tmpcursor.pos);
	SetSelection();
	UpdateCounters(cursor.row);
}
   

/* returns a pointer to the very first LyXParagraph */ 
LyXParagraph * LyXText::FirstParagraph()
{
	return params->paragraph;
}


/* returns true if the specified string is at the specified position */
#ifdef NEW_TEXT
bool LyXText::IsStringInText(LyXParagraph * par,
			     LyXParagraph::size_type pos,
			     char const * str)
#else 
bool LyXText::IsStringInText(LyXParagraph * par, int pos, char const * str)
#endif
{
	if (par) {
		int i = 0;
		while (pos + i < par->Last() && str[i] && 
		       str[i] == par->GetChar(pos + i)) {
			++i;
		}
		if (!str[i])
			return true;
	}
	return false;
}


/* sets the selection over the number of characters of string, no check!! */
void LyXText::SetSelectionOverString(char const * string)
{
	sel_cursor = cursor;
	for (int i = 0; string[i]; ++i)
		CursorRight();
	SetSelection();
}


/* simple replacing. The font of the first selected character is used */
void LyXText::ReplaceSelectionWithString(char const * str)
{
	SetCursorParUndo();
	FreezeUndo();

	if (!selection) { /* create a dummy selection */
		sel_end_cursor = cursor;
		sel_start_cursor = cursor;
	}

	// Get font setting before we cut
#ifdef NEW_TEXT
	LyXParagraph::size_type pos = sel_end_cursor.pos;
#else
	int pos = sel_end_cursor.pos;
#endif
	LyXFont font = sel_start_cursor.par->GetFontSettings(sel_start_cursor.pos);

	// Insert the new string
	for (int i = 0; str[i]; ++i) {
		sel_end_cursor.par->InsertChar(pos, str[i]);
		sel_end_cursor.par->SetFont(pos, font);
		++pos;
	}

	// Cut the selection
	CutSelection();

	UnFreezeUndo();
}


/* if the string can be found: return true and set the cursor to
 * the new position */
bool LyXText::SearchForward(char const * str)
{
	LyXParagraph * par = cursor.par;
#ifdef NEW_TEXT
	LyXParagraph::size_type pos = cursor.pos;
#else
	int pos = cursor.pos;
#endif
	while (par && !IsStringInText(par, pos, str)) {
		if (pos < par->Last() - 1)
			++pos;
		else {
			pos = 0;
			par = par->Next();
		}
	}
	if (par) {
		SetCursor(par,pos);
		return true;
	}
	else
		return false;
}


bool LyXText::SearchBackward(char const * string)
{
	LyXParagraph * par = cursor.par;
	int pos = cursor.pos;

	do {
		if (pos>0)
			pos--;
		else {
			// We skip empty paragraphs (Asger)
			do {
				par = par->Previous();
				if (par)
					pos = par->Last()-1;
			} while (par && pos<0);
		}
	} while (par && !IsStringInText(par,pos,string));
  
	if (par) {
		SetCursor(par,pos);
		return true;
	}
	else
		return false;
}


#ifdef NEW_TEXT
void LyXText::InsertStringA(LyXParagraph::TextContainer const & text)
{
	char * str = new char[text.size() + 1];
	copy(text.begin(), text.end(), str);
	str[text.size()] = '\0';
	InsertStringA(str);
	delete [] str;
}
#endif

/* needed to insert the selection */
void LyXText::InsertStringA(char const * str)
{
	LyXParagraph * par = cursor.par;
#ifdef NEW_TEXT
	LyXParagraph::size_type pos = cursor.pos;
	LyXParagraph::size_type a = 0;
#else
	int pos = cursor.pos;
	int a = 0;
#endif
        int cell = 0;
	LyXParagraph * endpar = cursor.par->Next();

	SetCursorParUndo();

	char flag = textclasslist.Style(parameters->textclass, 
				   cursor.par->GetLayout()).isEnvironment();
	/* only to be sure, should not be neccessary */ 
	ClearSelection();
   
	/* insert the string, don't insert doublespace */ 
	int i = 0;
#ifndef NEW_TEXT
	int i2 = 0;
	for (i2 = i; str[i2] && str[i2] != '\n'; ++i2);
	par->Enlarge(pos, i2 - i);
#endif
	while (str[i]) {
		if (str[i]!='\n') {
			if (str[i]==' ' && (str[i+1]!=' ')
			    && pos && par->GetChar(pos-1)!=' ') {
				par->InsertChar(pos,' ');
				pos++;
			}
                        else if (par->table) {
                            if (str[i] == '\t') {
#ifdef NEW_TEXT
                                while((pos < par->size()) &&
                                      (par->GetChar(pos) != LYX_META_NEWLINE))
                                        ++pos;
                                if (pos < par->size())
                                        ++pos;
#else
                                while((pos < par->last) &&
                                      (par->GetChar(pos) != LYX_META_NEWLINE))
                                        ++pos;
                                if (pos < par->last)
                                        ++pos;
#endif
                                else // no more fields to fill skip the rest
                                        break;
                            } else if ((str[i] != 13) &&
                                ((str[i] & 127) >= ' ')) {
                                par->InsertChar(pos,str[i]);
                                pos++;
                            }
                        }
			else if (str[i]==' ') {
				par->InsertChar(pos,LYX_META_PROTECTED_SEPARATOR);
				pos++;
			}
			else if (str[i]=='\t') {
				for (a = pos; a < (pos/8 + 1) * 8 ; ++a) {
					par->InsertChar(a, LYX_META_PROTECTED_SEPARATOR);
				}
				pos = a;
			}
			else if (str[i]!=13 && 
				 // Ignore unprintables
				 (str[i] & 127) >= ' ') {
				par->InsertChar(pos,str[i]);
				pos++;
			}
		} else {
                        if (par->table) {
                                if (!str[i+1]) {
                                        pos++;
                                        break;
                                }
#ifdef NEW_TEXT
                                while((pos < par->size()) &&
#else
                                while((pos < par->last) &&
#endif
                                      (par->GetChar(pos) != LYX_META_NEWLINE))
                                        pos++;
                                pos++;
                                cell=NumberOfCell(par,pos);
#ifdef NEW_TEXT
                                while((pos < par->size()) &&
                                      !(par->table->IsFirstCell(cell))) {
                                        while((pos < par->size()) &&
                                              (par->GetChar(pos) != LYX_META_NEWLINE))
                                                ++pos;
                                        ++pos;
                                        cell=NumberOfCell(par,pos);
                                }
                                if (pos >= par->size())
                                        // no more fields to fill skip the rest
                                        break;
#else
                                while((pos < par->last) &&
                                      !(par->table->IsFirstCell(cell))) {
                                        while((pos < par->last) &&
                                              (par->GetChar(pos) != LYX_META_NEWLINE))
                                                pos++;
                                        pos++;
                                        cell=NumberOfCell(par,pos);
                                }
                                if (pos >= par->last)
                                        // no more fields to fill skip the rest
                                        break;
#endif
                        } else {
#ifdef NEW_TEXT
                                if (!par->text.size()) {
#else
                                if (!par->last) {
#endif
                                        par->InsertChar(pos,LYX_META_PROTECTED_SEPARATOR);
                                        pos++;
                                }
                                par->BreakParagraph(pos, flag);
                                par = par->Next();
                                pos = 0;
                        }
#ifndef NEW_TEXT
			for (i2 = i; str[i2] && str[i2] != '\n'; i2++);
			par->Enlarge(pos, i2 - i);
#endif
		}
      
		i++;
	}
   
	RedoParagraphs(cursor,endpar);
	SetCursor(cursor.par, cursor.pos);
	sel_cursor = cursor;
	SetCursor(par, pos);
	SetSelection();
}


#ifdef NEW_TEXT
void LyXText::InsertStringB(LyXParagraph::TextContainer const & text)
{
	char * str = new char[text.size() + 1];
	copy(text.begin(), text.end(), str);
	str[text.size()] = '\0';
	InsertStringB(str);
	delete [] str;
}
#endif
 
/* turns double-CR to single CR, others where converted into one blank and 13s 
 * that are ignored .Double spaces are also converted into one. Spaces at
 * the beginning of a paragraph are forbidden. tabs are converted into one
 * space. then InsertStringA is called */ 
void LyXText::InsertStringB(char const * s)
{
	string str(s);
	LyXParagraph * par = cursor.par;
	int i = 1;
	while (str[i]) {
		if (str[i] == '\t' && !par->table)
			str[i] = ' ';
		if (str[i] == ' ' && str[i + 1] == ' ')
			str[i] = 13;
		if (str[i] == '\n' && str[i + 1] && !par->table){
			if (str[i + 1] != '\n') {
				if (str[i - 1] != ' ')
					str[i] = ' ';
				else
					str[i] = 13;
			}
			while (str[i + 1] && (str[i + 1] == ' '
					       || str[i + 1] == '\t'
					       || str[i + 1] == '\n'
					       || str[i + 1] == 13)) {
				str[i + 1] = 13;
				++i;
			}
		}
		++i;
	}
	InsertStringA(str.c_str());
}


bool LyXText::GotoNextError()
{
	LyXCursor res=cursor;
	do {
		if (res.pos < res.par->Last() - 1) {
			res.pos++;
		}
		else  {
			res.par=res.par->Next();
			res.pos = 0;
		}
      
	} while (res.par && 
		 !(res.par->GetChar(res.pos)==LYX_META_INSET
		   && res.par->GetInset(res.pos)->AutoDelete()));
   
	if (res.par) {
		SetCursor(res.par, res.pos);
		return true;
	}
   
	return false;
}


bool LyXText::GotoNextNote()
{
	LyXCursor res=cursor;
	do {
		if (res.pos < res.par->Last()-1) {
			res.pos++;
		}
		else  {
			res.par=res.par->Next();
			res.pos = 0;
		}
      
	} while (res.par && 
		 !(res.par->GetChar(res.pos)==LYX_META_INSET
		   && res.par->GetInset(res.pos)->LyxCode()==Inset::IGNORE_CODE));
   
	if (res.par) {
		SetCursor(res.par, res.pos);
		return true;
	}
   
	return false;
}


int LyXText::SwitchLayoutsBetweenClasses(char class1, char class2,
					 LyXParagraph * par)
{
	InsetError * new_inset = 0;
	int ret = 0;
	if (!par || class1 == class2)
		return ret;
	par = par->FirstPhysicalPar();
	while (par) {
		string name = textclasslist.NameOfLayout(class1, par->layout);
		int lay = 0;
		pair<bool, LyXTextClass::LayoutList::size_type> pp =
			textclasslist.NumberOfLayout(class2, name);
		if (pp.first) {
			lay = pp.second;
		} else { // layout not found
			// use default layout "Standard" (0)
			lay = 0;
		}
		par->layout = lay;
      
		if (name != textclasslist.NameOfLayout(class2, par->layout)) {
			ret++;
			string s = "Layout had to be changed from\n"
				+ name + " to " + textclasslist.NameOfLayout(class2, par->layout)
				+ "\nbecause of class conversion from\n"
				+ textclasslist.NameOfClass(class1) + " to "
				+ textclasslist.NameOfClass(class2);
			new_inset = new InsetError(s);
			par->InsertChar(0, LYX_META_INSET);
			par->InsertInset(0, new_inset);
		}
      
		par = par->next;
	}
	return ret;
}


#ifdef NEW_TEXT
void LyXText::CheckParagraph(LyXParagraph * par,
			     LyXParagraph::size_type pos)
#else
void LyXText::CheckParagraph(LyXParagraph * par, int pos)
#endif
{
  
	LyXCursor tmpcursor;

	/* table stuff -- begin*/
   
	if (par->table) {
		CheckParagraphInTable(par, pos);
	}
	else {
		/* table stuff -- end*/
     
		long y = 0;
#ifdef NEW_TEXT
		LyXParagraph::size_type z;
#else
		int z;
#endif
		Row * row = GetRow(par, pos, y);
     
		/* is there a break one row above */ 
		if (row->previous && row->previous->par == row->par) {
			z = NextBreakPoint(row->previous, paperwidth);
			if ( z >= row->pos) {
				/* set the dimensions of the row above  */ 
				y -= row->previous->height;
				refresh_y = y;
				refresh_row = row->previous;
				status = LyXText::NEED_MORE_REFRESH;
       
				BreakAgain(row->previous);

				/* set the cursor again. Otherwise dungling pointers are possible */
				SetCursor(cursor.par, cursor.pos);
				sel_cursor = cursor;
				return;
			}
		}

		int tmpheight = row->height;
#ifdef NEW_TEXT
		LyXParagraph::size_type tmplast = RowLast(row);
#else
		int tmplast = RowLast(row);
#endif
		refresh_y = y;
		refresh_row = row;

		BreakAgain(row);
		if (row->height == tmpheight && RowLast(row) == tmplast)
			status = LyXText::NEED_VERY_LITTLE_REFRESH;
		else
			status = LyXText::NEED_MORE_REFRESH; 
   
		/* check the special right address boxes */
		if (textclasslist.Style(parameters->textclass, par->GetLayout()).margintype == MARGIN_RIGHT_ADDRESS_BOX) {
			tmpcursor.par = par;
			tmpcursor.row = row;
			tmpcursor.y = y;
			tmpcursor.x = 0;
			tmpcursor.x_fix = 0;
			tmpcursor.pos = pos;
			RedoDrawingOfParagraph(tmpcursor); 
		}
   
	}

	/* set the cursor again. Otherwise dangling pointers are possible */
	// also set the selection
   
	if (selection){
		tmpcursor = cursor;
		SetCursorIntern(sel_cursor.par, sel_cursor.pos);
		sel_cursor = cursor; 
		SetCursorIntern(sel_start_cursor.par, sel_start_cursor.pos);
		sel_start_cursor = cursor; 
		SetCursorIntern(sel_end_cursor.par, sel_end_cursor.pos);
		sel_end_cursor = cursor; 
		SetCursorIntern(last_sel_cursor.par, last_sel_cursor.pos);
		last_sel_cursor = cursor; 
		cursor = tmpcursor;
	}
	SetCursorIntern(cursor.par, cursor.pos);
}


/* returns 0 if inset wasn't found */
int LyXText::UpdateInset(Inset * inset)
{
	/* first check the current paragraph */
	int pos = cursor.par->GetPositionOfInset(inset);
	if (pos != -1){
		CheckParagraph(cursor.par, pos);
		return 1;
	}
  
	/* check every paragraph */
  
	LyXParagraph * par = FirstParagraph();
	do {
		/* make sure the paragraph is open */
		if (par->footnoteflag != LyXParagraph::CLOSED_FOOTNOTE){
			pos = par->GetPositionOfInset(inset);
			if (pos != -1){
				CheckParagraph(par, pos);
				return 1;
			}
		}
		par = par->Next();
	} while (par);
  
	return 0;
}


#ifdef NEW_TEXT
void LyXText::SetCursor(LyXParagraph * par,
			LyXParagraph::size_type pos)
#else
void LyXText::SetCursor(LyXParagraph * par, int pos)
#endif
{
	LyXCursor old_cursor = cursor;
	SetCursorIntern(par, pos);
	DeleteEmptyParagraphMechanism(old_cursor);
}


#ifdef NEW_TEXT
void LyXText::SetCursorIntern(LyXParagraph * par, LyXParagraph::size_type pos)
#else
void LyXText::SetCursorIntern(LyXParagraph * par, int pos)
#endif
{
	long y;
	Row * row;
	int left_margin;
	LyXParagraph * tmppar;
   
	/* correct the cursor position if impossible */
	if (pos > par->Last()){
		tmppar = par->ParFromPos(pos);
		pos = par->PositionInParFromPos(pos);
		par = tmppar;
	}
	if (par->IsDummy() && par->previous &&
	    par->previous->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE) {
		while (par->previous &&
		       ((par->previous->IsDummy() && par->previous->previous->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE) ||
			(par->previous->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE))) {
			par = par->previous ;
			if (par->IsDummy() &&
			    par->previous->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE)
#ifdef NEW_TEXT
				pos += par->text.size() + 1;
#else
				pos += par->last + 1;
#endif
		}
		if (par->previous) {
			par = par->previous;
		}
#ifdef NEW_TEXT
		pos += par->text.size() + 1;
#else
		pos += par->last + 1;
#endif
	}

	cursor.par = par;
	cursor.pos = pos;

	/* get the cursor y position in text  */
	row = GetRow(par, pos, y);
	/* y is now the beginning of the cursor row */ 
	y += row->baseline;
	/* y is now the cursor baseline */ 
	cursor.y = y;
   
	/* now get the cursors x position */
   
	float x;
	float fill_separator, fill_hfill, fill_label_hfill;
	left_margin = LabelEnd(row);
	PrepareToPrint(row, x, fill_separator, fill_hfill, fill_label_hfill);
#ifdef NEW_TEXT
	LyXParagraph::size_type main_body =
		BeginningOfMainBody(row->par);
#else
	int main_body = BeginningOfMainBody(row->par);
#endif 
	/* table stuff -- begin*/
	if (row->par->table) {
		int cell = NumberOfCell(row->par, row->pos);
		float x_old = x;
		x += row->par->table->GetBeginningOfTextInCell(cell);
		for (pos = row->pos; pos < cursor.pos; pos++)  {
			if (row->par->IsNewline(pos)) {
				x = x_old + row->par->table->WidthOfColumn(cell);
				x_old = x;
				cell++;
				x += row->par->table->GetBeginningOfTextInCell(cell);
			} else {
				x += SingleWidth(row->par, pos);
			}
		}
	} else
		/* table stuff -- end*/

		for (pos = row->pos; pos < cursor.pos; pos++)  {
			if (pos && pos == main_body
			    && !row->par->IsLineSeparator(pos - 1)) {
				x += GetFont(row->par, -2).stringWidth(
						    textclasslist.Style(parameters->textclass, row->par->GetLayout()).labelsep);
				if (x < left_margin)
					x = left_margin;
			}
      
			x += SingleWidth(row->par, pos);
			if (HfillExpansion(row, pos)) {
				if (pos >= main_body)
					x += fill_hfill;
				else 
					x += fill_label_hfill;
			}
			else if (pos >= main_body && row->par->IsSeparator(pos)) {
				x+= fill_separator;
			}
      
			if (pos + 1 == main_body
			    && row->par->IsLineSeparator(pos)) {
				x += GetFont(row->par, -2).stringWidth(
						    textclasslist.Style(parameters->textclass, row->par->GetLayout()).labelsep);
				if (row->par->IsLineSeparator(pos))
					x-= SingleWidth(row->par, pos);
				if (x < left_margin)
					x = left_margin;
			}
		}
   
	cursor.x = int(x);
   
	cursor.x_fix = cursor.x;
	cursor.row = row;
   
	if (cursor.pos && 
	    (cursor.pos == cursor.par->Last() || cursor.par->IsSeparator(cursor.pos)
	     || (cursor.pos && cursor.pos == BeginningOfMainBody(cursor.par)
		 && !cursor.par->IsSeparator(cursor.pos))
		    )) {
		current_font = cursor.par->GetFontSettings(cursor.pos - 1);
		real_current_font = GetFont(cursor.par, cursor.pos - 1);
	} else {
		current_font = cursor.par->GetFontSettings(cursor.pos);
		real_current_font = GetFont(cursor.par, cursor.pos);
	}
}


void LyXText::SetCursorFromCoordinates(int x, long y)
{
	LyXCursor old_cursor = cursor;
   
	/* get the row first */ 
   
	Row * row = GetRowNearY(y);
   
	cursor.par = row->par;
   
	int column = GetColumnNearX(row, x);
	cursor.pos = row->pos + column;
	cursor.x = x;
	cursor.y = y + row->baseline;
   
	cursor.row = row;
    
	if (cursor.pos && 
	    (cursor.pos == cursor.par->Last()
	     || cursor.par->IsSeparator(cursor.pos)
	     || (cursor.pos && cursor.pos == BeginningOfMainBody(cursor.par)
		 && !cursor.par->IsSeparator(cursor.pos))
		    )) {
		current_font = cursor.par->GetFontSettings(cursor.pos - 1);
		real_current_font = GetFont(cursor.par, cursor.pos - 1);
	} else {
		current_font = cursor.par->GetFontSettings(cursor.pos);
		real_current_font = GetFont(cursor.par, cursor.pos);
	}
	DeleteEmptyParagraphMechanism(old_cursor);
}


void LyXText::CursorLeft()
{
	CursorLeftIntern();
        if (cursor.par->table) {
                int cell = NumberOfCell(cursor.par, cursor.pos);
                if (cursor.par->table->IsContRow(cell) &&
                    cursor.par->table->CellHasContRow(cursor.par->table->GetCellAbove(cell))<0) {
                        CursorUp();
                }
        }
}


void LyXText::CursorLeftIntern()
{
	if (cursor.pos > 0) {
		SetCursor(cursor.par, cursor.pos - 1);
	}
	else if (cursor.par->Previous()) {
		SetCursor(cursor.par->Previous(), cursor.par->Previous()->Last());
	}
}


void LyXText::CursorRight()
{
	CursorRightIntern();
        if (cursor.par->table) {
                int cell = NumberOfCell(cursor.par, cursor.pos);
                if (cursor.par->table->IsContRow(cell) &&
                    cursor.par->table->CellHasContRow(cursor.par->table->GetCellAbove(cell))<0) {
                        CursorUp();
                }
        }
}


void LyXText::CursorRightIntern()
{
	if (cursor.pos < cursor.par->Last()) {
		SetCursor(cursor.par, cursor.pos + 1);
	}
	else if (cursor.par->Next()) {
		SetCursor(cursor.par->Next(), 0);
	}
}


void LyXText::CursorUp()
{
	SetCursorFromCoordinates(cursor.x_fix, 
				 cursor.y - cursor.row->baseline - 1);
        if (cursor.par->table) {
                int cell = NumberOfCell(cursor.par, cursor.pos);
                if (cursor.par->table->IsContRow(cell) &&
                    cursor.par->table->CellHasContRow(cursor.par->table->GetCellAbove(cell))<0) {
                        CursorUp();
                }
        }
}


void LyXText::CursorDown()
{
        if (cursor.par->table &&
            cursor.par->table->ShouldBeVeryLastRow(NumberOfCell(cursor.par, cursor.pos)) &&
            !cursor.par->next)
                return;
	SetCursorFromCoordinates(cursor.x_fix, 
				 cursor.y - cursor.row->baseline
				 + cursor.row->height + 1);
        if (cursor.par->table) {
                int cell = NumberOfCell(cursor.par, cursor.pos);
                int cell_above = cursor.par->table->GetCellAbove(cell);
                while(cursor.par->table &&
                      cursor.par->table->IsContRow(cell) &&
                      (cursor.par->table->CellHasContRow(cell_above)<0)) {
                    SetCursorFromCoordinates(cursor.x_fix, 
                                             cursor.y - cursor.row->baseline
                                             + cursor.row->height + 1);
                    if (cursor.par->table) {
                        cell = NumberOfCell(cursor.par, cursor.pos);
                        cell_above = cursor.par->table->GetCellAbove(cell);
                    }
                }
        }
}


void LyXText::CursorUpParagraph()
{
	if (cursor.pos > 0) {
		SetCursor(cursor.par, 0);
	}
	else if (cursor.par->Previous()) {
		SetCursor(cursor.par->Previous(), 0);
	}
}


void LyXText::CursorDownParagraph()
{
	if (cursor.par->Next()) {
		SetCursor(cursor.par->Next(), 0);
	} else {
		SetCursor(cursor.par,cursor.par->Last());
	}
}



void LyXText::DeleteEmptyParagraphMechanism(LyXCursor old_cursor)
{
    bool deleted = false;
	
    /* this is the delete-empty-paragraph-mechanism. */ 
    if (selection)
        return;

    // Paragraph should not be deleted if empty
    if ((textclasslist.Style(parameters->textclass,
                        old_cursor.par->GetLayout())).keepempty)
        return;

    LyXCursor tmpcursor;

    if (old_cursor.par != cursor.par) {
        if ( (old_cursor.par->Last() == 0
              || (old_cursor.par->Last() == 1
                  && (old_cursor.par->IsLineSeparator(0))))
             && old_cursor.par->FirstPhysicalPar()
             == old_cursor.par->LastPhysicalPar()) {
			
            /* ok, we will delete anything */ 
			
            // make sure that you do not delete any environments
            if ((old_cursor.par->footnoteflag != LyXParagraph::OPEN_FOOTNOTE &&
                 !(old_cursor.row->previous 
                   && old_cursor.row->previous->par->footnoteflag == LyXParagraph::OPEN_FOOTNOTE)
                 && !(old_cursor.row->next 
                      && old_cursor.row->next->par->footnoteflag == LyXParagraph::OPEN_FOOTNOTE))
                || 
                (old_cursor.par->footnoteflag == LyXParagraph::OPEN_FOOTNOTE &&
                 ((old_cursor.row->previous 
                   && old_cursor.row->previous->par->footnoteflag == LyXParagraph::OPEN_FOOTNOTE)
                  || 
                  (old_cursor.row->next
                   && old_cursor.row->next->par->footnoteflag == LyXParagraph::OPEN_FOOTNOTE))
                 )){
                status = LyXText::NEED_MORE_REFRESH;
                deleted = true;
				
                if (old_cursor.row->previous) {
                    refresh_row = old_cursor.row->previous;
                    refresh_y = old_cursor.y - old_cursor.row->baseline - refresh_row->height;
                    tmpcursor = cursor;
                    cursor = old_cursor; // that undo can restore the right cursor position
                    LyXParagraph *endpar = old_cursor.par->next;
                    if (endpar && endpar->GetDepth()) {
                        while (endpar && endpar->GetDepth()) {
                            endpar = endpar->LastPhysicalPar()->Next();
                        }
                    }
                    SetUndo(Undo::DELETE,
                            old_cursor.par->previous,
                            endpar);
                    cursor = tmpcursor;

                    /* delete old row */ 
                    RemoveRow(old_cursor.row);
                    if (params->paragraph == old_cursor.par) {
                        params->paragraph = params->paragraph->next;
                    }
                    /* delete old par */ 
                    delete old_cursor.par;
					
                    /* Breakagain the next par. Needed
                     * because of the parindent that
                     * can occur or dissappear. The
                     * next row can change its height,
                     * if there is another layout before */
                    if (refresh_row->next) {
                        BreakAgain(refresh_row->next);
                        UpdateCounters(refresh_row);
                    }
                    SetHeightOfRow(refresh_row);
                }
                else {
                    refresh_row = old_cursor.row->next;
                    refresh_y = old_cursor.y - old_cursor.row->baseline;
					
                    tmpcursor = cursor;
                    cursor = old_cursor; // that undo can restore the right cursor position
                    LyXParagraph *endpar = old_cursor.par->next;
                    if (endpar && endpar->GetDepth()) {
                        while (endpar && endpar->GetDepth()) {
                            endpar = endpar->LastPhysicalPar()->Next();
                        }
                    }
                    SetUndo(Undo::DELETE,
                            old_cursor.par->previous,
                            endpar);
                    cursor = tmpcursor;

                    /* delete old row */ 
                    RemoveRow(old_cursor.row);
                    /* delete old par */ 
                    if (params->paragraph == old_cursor.par) {
                        params->paragraph = params->paragraph->next;
                    }
                    delete old_cursor.par;
					
                    /* Breakagain the next par. Needed because of
                     * the parindent that can occur or dissappear.
                     * The next row can change its height, if there
                     * is another layout before */ 
                    if (refresh_row) {
                        BreakAgain(refresh_row);
                        UpdateCounters(refresh_row->previous);
                    }
                }
				
				/* correct cursor y */
                SetCursor(cursor.par, cursor.pos);
		     
				/* if (cursor.y > old_cursor.y)
				   cursor.y -= old_cursor.row->height; */ 
	 
                if (sel_cursor.par  == old_cursor.par
                    && sel_cursor.pos == sel_cursor.pos) {
                    /* correct selection*/ 
                    sel_cursor = cursor;
                }
            }
       
        }
        if (!deleted){
            if (old_cursor.par->ClearParagraph()){
                RedoParagraphs(old_cursor, old_cursor.par->Next());
				/* correct cursor y */
                SetCursor(cursor.par, cursor.pos);
                sel_cursor = cursor;
            }
        }
    } else if (cursor.par->table && (cursor.row != old_cursor.row)) {
        int cell = NumberOfCell(old_cursor.par, old_cursor.pos);
        if (old_cursor.par->table->IsContRow(cell) &&
            IsEmptyTableRow(&old_cursor)) {
            RemoveTableRow(&old_cursor);
            RedoParagraph();
        }
    }
}


LyXParagraph * LyXText::GetParFromID(int id)
{
	LyXParagraph * result = FirstParagraph();
	while (result && result->GetID() != id)
		result = result->next;
	return result;
}


// undo functions
bool  LyXText::TextUndo()
{ // returns false if no undo possible
	Undo * undo = params->undostack.Pop();
	if (undo){
		FinishUndo();
		if (!undo_frozen)
			params->redostack.Push(CreateUndo(undo->kind, 
							  GetParFromID(undo->number_of_before_par),
							  GetParFromID(undo->number_of_behind_par)));
	}
	return TextHandleUndo(undo);
}


bool LyXText::TextRedo()
{ // returns false if no redo possible
	Undo * undo = params->redostack.Pop();
	if (undo){
		FinishUndo();
		if (!undo_frozen)
			params->undostack.Push(CreateUndo(undo->kind, 
							  GetParFromID(undo->number_of_before_par),
							  GetParFromID(undo->number_of_behind_par)));
	}
	return TextHandleUndo(undo);
}


bool LyXText::TextHandleUndo(Undo * undo){ // returns false if no undo possible
	bool result = false;
	if (undo){
		LyXParagraph * before = GetParFromID(undo->number_of_before_par); 
		LyXParagraph * behind = GetParFromID(undo->number_of_behind_par); 
		LyXParagraph * tmppar;
		LyXParagraph * tmppar2;
		LyXParagraph * tmppar3;
		LyXParagraph * tmppar4;
		LyXParagraph * endpar;
		LyXParagraph * tmppar5;
    
		// if there's no before take the beginning of the document for redoing
		if (!before)
			SetCursorIntern(FirstParagraph(), 0);

		// replace the paragraphs with the undo informations

		tmppar3 = undo->par;
		undo->par = 0; // otherwise the undo destructor would delete the paragraph
		tmppar4 = tmppar3;
		if (tmppar4){
			while (tmppar4->next)
				tmppar4 = tmppar4->next;
		} // get last undo par
    
		// now remove the old text if there is any
		if (before != behind || (!behind && !before)){
			if (before)
				tmppar5 = before->next;
			else
				tmppar5 = params->paragraph;
			tmppar2 = tmppar3;
			while (tmppar5 && tmppar5 != behind){
				tmppar = tmppar5;
				tmppar5 = tmppar5->next;
				// a memory optimization for edit: Only layout information
				// is stored in the undo. So restore the text informations.
				if (undo->kind == Undo::EDIT){
					tmppar2->text = tmppar->text;
#ifdef NEW_TEXT
					//tmppar->text.clear();
					tmppar->text.erase(tmppar->text.begin(),
							   tmppar->text.end());
#else
					tmppar->text = 0;
#endif
					tmppar2 = tmppar2->next;
				}
				if ( currentrow && currentrow->par == tmppar )
					currentrow = currentrow -> previous;
				delete tmppar;
			}
		}
    
		// put the new stuff in the list if there is one
		if (tmppar3){
			if (before)
				before->next = tmppar3;
			else
				params->paragraph = tmppar3;
			tmppar3->previous = before;
		}
		else {
			if (!before)
				params->paragraph = behind;
		}
		if (tmppar4) {
			tmppar4->next = behind;
			if (behind)
				behind->previous = tmppar4;
		}
    
    
		// Set the cursor for redoing
		if (before){
			SetCursorIntern(before->FirstSelfrowPar(), 0);
			// check wether before points to a closed float and open it if necessary
			if (before && before->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE
			    && before->next && before->next->footnoteflag != LyXParagraph::NO_FOOTNOTE){
				tmppar4 =before;
				while (tmppar4->previous && 
				       tmppar4->previous->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE)
					tmppar4 = tmppar4->previous;
				while (tmppar4 && tmppar4->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE){
					tmppar4->footnoteflag = LyXParagraph::OPEN_FOOTNOTE;
					tmppar4 = tmppar4->next;
				}
			}
		}
    
		// open a cosed footnote at the end if necessary
		if (behind && behind->previous && 
		    behind->previous->footnoteflag != LyXParagraph::NO_FOOTNOTE &&
		    behind->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE){
			while (behind && behind->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE){
				behind->footnoteflag = LyXParagraph::OPEN_FOOTNOTE;
				behind = behind->next;
			}
		}
    
		// calculate the endpar for redoing the paragraphs.
		if (behind){
			if (behind->footnoteflag != LyXParagraph::CLOSED_FOOTNOTE)
				endpar = behind->LastPhysicalPar()->Next();
			else
				endpar = behind->NextAfterFootnote()->LastPhysicalPar()->Next();
		}
		else
			endpar = behind;
    
		tmppar = GetParFromID(undo->number_of_cursor_par);
		RedoParagraphs(cursor, endpar); 
		if (tmppar){
			SetCursorIntern(tmppar, undo->cursor_pos);
			UpdateCounters(cursor.row);
		}
		result = true;
		delete undo;
	}
	FinishUndo();
	return result;
}


void LyXText::FinishUndo()
{ // makes sure the next operation will be stored
	undo_finished = True;
}


void LyXText::FreezeUndo()
{ // this is dangerous and for internal use only
	undo_frozen = True;
}


void LyXText::UnFreezeUndo()
{ // this is dangerous and for internal use only
	undo_frozen = false;
}


void LyXText::SetUndo(Undo::undo_kind kind, LyXParagraph * before,
		      LyXParagraph * behind)
{
	if (!undo_frozen)
		params->undostack.Push(CreateUndo(kind, before, behind));
	params->redostack.Clear();
}


void LyXText::SetRedo(Undo::undo_kind kind, LyXParagraph * before,
		      LyXParagraph * behind)
{
	params->redostack.Push(CreateUndo(kind, before, behind));
}


Undo * LyXText::CreateUndo(Undo::undo_kind kind, LyXParagraph * before,
			  LyXParagraph * behind)
{
	int before_number = -1;
	int behind_number = -1;
	if (before)
		before_number = before->GetID();
	if (behind)
		behind_number = behind->GetID();
	// Undo::EDIT  and Undo::FINISH are
	// always finished. (no overlapping there)
	// overlapping only with insert and delete inside one paragraph: 
	// Nobody wants all removed  character
	// appear one by one when undoing. 
	// EDIT is special since only layout information, not the
	// contents of a paragaph are stored.
	if (!undo_finished && kind != Undo::EDIT && 
	    kind != Undo::FINISH){
		// check wether storing is needed
		if (params->undostack.Top() && 
		    params->undostack.Top()->kind == kind &&
		    params->undostack.Top()->number_of_before_par ==  before_number &&
		    params->undostack.Top()->number_of_behind_par ==  behind_number ){
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

	if (start && end && start != end->next && (before != behind || (!before && !behind))) {
		tmppar = start;
		tmppar2 = tmppar->Clone();
		tmppar2->SetID(tmppar->GetID());

		// a memory optimization: Just store the layout information when only edit
		if (kind == Undo::EDIT){
#ifdef NEW_TEXT
			//tmppar2->text.clear();
			tmppar2->text.erase(tmppar2->text.begin(),
					    tmppar2->text.end());
#else
			if (tmppar2->text)
				delete[] tmppar2->text;
			tmppar2->text = 0;
#endif
		}

		undopar = tmppar2;
  
		while (tmppar != end && tmppar->next) {
			tmppar = tmppar->next;
			tmppar2->next = tmppar->Clone();
			tmppar2->next->SetID(tmppar->GetID());
			// a memory optimization: Just store the layout information when only edit
			if (kind == Undo::EDIT){
#ifdef NEW_TEXT
				//tmppar2->next->text.clear();
				tmppar2->next->text.erase(tmppar2->next->text.begin(), tmppar2->next->text.end());
#else
				if (tmppar2->next->text)
					delete[] tmppar2->next->text;
				tmppar2->next->text = 0;
#endif
			}
			tmppar2->next->previous = tmppar2;
			tmppar2=tmppar2->next;
		}
		tmppar2->next = 0;
	}
	else
		undopar = 0; // nothing to replace (undo of delete maybe)
  
	int cursor_par = cursor.par->ParFromPos(cursor.pos)->GetID();
	int cursor_pos =  cursor.par->PositionInParFromPos(cursor.pos);

	Undo * undo = new Undo(kind, 
			      before_number, behind_number,  
			      cursor_par, cursor_pos, 
			      undopar);
  
	undo_finished = false;
	return undo;
}


void LyXText::SetCursorParUndo()
{
	SetUndo(Undo::FINISH, 
		cursor.par->ParFromPos(cursor.pos)->previous, 
		cursor.par->ParFromPos(cursor.pos)->next); 
}

void LyXText::RemoveTableRow(LyXCursor * cursor)
{
    int
        cell_act,
        cell = -1,
        cell_org = 0,
        ocell = 0;
    
    /* move to the previous row */
    cell_act = NumberOfCell(cursor->par, cursor->pos);
    if (cell < 0)
        cell = cell_act;
    while (cursor->pos && !cursor->par->IsNewline(cursor->pos-1))
        cursor->pos--;
    while (cursor->pos && 
           !cursor->par->table->IsFirstCell(cell_act)){
        cursor->pos--;
        while (cursor->pos && !cursor->par->IsNewline(cursor->pos-1))
            cursor->pos--;
            cell--;
            cell_act--;
    }
    /* now we have to pay attention if the actual table is the
       main row of TableContRows and if yes to delete all of them */
    if (!cell_org)
        cell_org = cell;
    do {
        ocell = cell;
        /* delete up to the next row */
        while (cursor->pos < cursor->par->Last() && 
               (cell_act == ocell
                || !cursor->par->table->IsFirstCell(cell_act))){
            while (cursor->pos < cursor->par->Last() &&
                   !cursor->par->IsNewline(cursor->pos))
                cursor->par->Erase(cursor->pos);
            cell++;
            cell_act++;
            if (cursor->pos < cursor->par->Last())
                cursor->par-> Erase(cursor->pos);
        }
        if (cursor->pos && cursor->pos == cursor->par->Last()){
            cursor->pos--;
            cursor->par->Erase(cursor->pos); // no newline at the very end!
        }
    } while (((cell+1) < cursor->par->table->GetNumberOfCells()) &&
             !cursor->par->table->IsContRow(cell_org) &&
             cursor->par->table->IsContRow(cell));
    cursor->par->table->DeleteRow(cell_org);
    return;
}


bool LyXText::IsEmptyTableRow(LyXCursor * old_cursor)
{
	if (!old_cursor->par->table)
		return false;
#ifdef I_DONT_KNOW_IF_I_SHOULD_DO_THIS
	int
		pos = old_cursor->pos,
		cell = NumberOfCell(old_cursor->par, pos);

	// search first charater of this table row
	while (pos && !old_cursor->par->table->IsFirstCell(cell)) {
		pos--;
		while (pos && !old_cursor->par->IsNewline(pos-1))
			pos--;
		cell--;
	}
	if (!old_cursor->par->IsNewline(pos))
		return false;
	cell++;
	pos++;
	while ((pos < old_cursor->par->Last()) &&
	       !old_cursor->par->table->IsFirstCell(cell)) {
		if (!old_cursor->par->IsNewline(pos))
			return false;
		pos++;
		cell++;
	}
	return true;
#endif
	return false;
}


bool LyXText::IsEmptyTableCell()
{
#ifdef NEW_TEXT
	LyXParagraph::size_type pos = cursor.pos - 1;
#else
	int pos = cursor.pos - 1;
#endif
	while (pos >= 0 && pos < cursor.par->Last()
	       && !cursor.par->IsNewline(pos))
		--pos;
	return cursor.par->IsNewline(pos + 1);
}

void LyXText::toggleAppendix(){
  LyXParagraph * par = cursor.par->FirstPhysicalPar();
  bool start = !par->start_of_appendix;

  /* ensure that we have only one start_of_appendix in this document */
  LyXParagraph * tmp = FirstParagraph();
  for (;tmp;tmp=tmp->next)
    tmp->start_of_appendix = 0;
  par->start_of_appendix = start;

  /* we can set the refreshing parameters now */
  status = LyXText::NEED_MORE_REFRESH;
  refresh_y = 0;
  refresh_row = 0; // not needed for full update
  UpdateCounters(0);
  SetCursor(cursor.par, cursor.pos);
}

