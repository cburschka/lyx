/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *           Copyright 1995-1999 The LyX Team. 
 *
 * ======================================================*/

#include <config.h>

#ifdef __GNUG__
#pragma implementation "lyxparagraph.h"
#endif

#include "lyxparagraph.h"
#include "lyxrc.h"
#include "layout.h"
#include "tex-strings.h"
#include "bufferparams.h"
#include "support/FileInfo.h"
#include "debug.h"
#include "LaTeXFeatures.h"
#include "insets/insetinclude.h"
#include "support/filetools.h"
#include "lyx_gui_misc.h"
#include "texrow.h"

#define INITIAL_SIZE_PAR 10 /*Number of bytes in one paragraph*/
#define STEP_SIZE_PAR 10 /*Number of bytes added when reallocated*/

extern void addNewlineAndDepth(string & file, int const depth); // Jug 990923
extern unsigned char GetCurrentTextClass(); // this should be fixed/removed
int tex_code_break_column = 72;  // needs non-zero initialization. set later.
// this is a bad idea, but how can LyXParagraph find its buffer to get
// parameters? (JMarc)
extern BufferView *current_view;
extern LyXRC *lyxrc;


// ale970405
extern string bibitemWidthest();

/* this is a minibuffer */
static char minibuffer_char;
static LyXFont minibuffer_font;
static Inset *minibuffer_inset;


// Initialization of the counter for the paragraph id's,
// declared in lyxparagraph.h
unsigned int LyXParagraph::paragraph_id = 0;


LyXParagraph::LyXParagraph()
{
	size = INITIAL_SIZE_PAR;
	last = 0;
	text = new char[size];
	for (int i=0; i<10; i++) setCounter(i , 0);
	appendix = false;
	enumdepth = 0;
	itemdepth = 0;
	next = 0;
	previous = 0;
	fonttable = 0;
	insettable = 0;
	footnoteflag = LyXParagraph::NO_FOOTNOTE;

	align = LYX_ALIGN_BLOCK;

	/* table stuff -- begin*/ 
	table = 0;
	/* table stuff -- end*/ 
	id = paragraph_id++;
        bibkey = 0; // ale970302
	Clear();
}


/* this konstruktor inserts the new paragraph in a list */ 
LyXParagraph::LyXParagraph(LyXParagraph *par)
{
	size = INITIAL_SIZE_PAR;
	last = 0;
	text = new char[size];
	for (int i=0; i<10; i++) setCounter(i, 0);
	appendix = false;
	enumdepth = 0;
	itemdepth = 0;
	next = par->next;
	if (next)
		next->previous = this;
	previous = par;
	previous->next = this;
	fonttable = 0;
	insettable = 0;
	footnoteflag = LyXParagraph::NO_FOOTNOTE;
	footnotekind = LyXParagraph::FOOTNOTE;
	
	/* table stuff -- begin*/ 
	table = 0;
	/* table stuff -- end*/ 
	id = paragraph_id++;

        bibkey = 0; // ale970302        
        // ale970302
//        lyxerr << "new bib " << endl;
//        if (par->bibkey) {
//	    bibkey = new InsetBibKey(par->bibkey);
//        }
    
	Clear();
}

/// Used by the spellchecker
bool LyXParagraph::IsLetter(int pos){
	unsigned char c = GetChar(pos);
	if (IsLetterChar(c))
		return true;
	// '\0' is not a letter, allthough every string contains "" (below)
	if( c == '\0')
		return false;
	// We want to pass the ' and escape chars to ispell
	string extra = lyxrc->isp_esc_chars + '\'';
	char ch[2];
	ch[0] = c;
	ch[1] = 0;
	return contains(extra, ch);
}


void LyXParagraph::writeFile(FILE *file, BufferParams &params,
			     char footflag, char dth)
{
	LyXFont font1, font2;
	Inset *inset;
	int column = 0;
	int h = 0;
	char c = 0;

	if (footnoteflag != LyXParagraph::NO_FOOTNOTE
	    || !previous
	    || previous->footnoteflag == LyXParagraph::NO_FOOTNOTE){
		
		/* The beginning or the end of a footnote environment? */ 
		if (footflag != footnoteflag) {
			footflag = footnoteflag;
			if (footflag) {
				fprintf(file, "\n\\begin_float %s ", 
					string_footnotekinds[footnotekind]);
			}
			else {
				fprintf(file, "\n\\end_float ");
			}
		}

		/* The beginning or end of a deeper (i.e. nested) area? */
		if (dth != depth) {
			if (depth > dth) {
				while (depth > dth) {
					fprintf(file, "\n\\begin_deeper ");
					dth++;
				}
			}
			else {
				while (depth < dth) {
					fprintf(file, "\n\\end_deeper ");
					dth--;
				}
			}
		}

		/* First write the layout */ 
		fprintf(file, "\n\\layout %s\n",
			lyxstyle.NameOfLayout(params.textclass,layout).c_str());

		/* maybe some vertical spaces */ 
		if (added_space_top.kind() != VSpace::NONE)
			fprintf(file, "\\added_space_top %s ", 
				added_space_top.asLyXCommand().c_str());
		if (added_space_bottom.kind() != VSpace::NONE)
			fprintf(file, "\\added_space_bottom %s ",
				added_space_bottom.asLyXCommand().c_str());
			
		/* The labelwidth string used in lists */
		if (!labelwidthstring.empty())
			fprintf(file, "\\labelwidthstring %s\n",
				labelwidthstring.c_str());

		/* Lines above or below? */
		if (line_top)
			fprintf(file, "\\line_top ");
		if (line_bottom)
			fprintf(file, "\\line_bottom ");

		/* Pagebreaks above or below? */
		if (pagebreak_top)
			fprintf(file, "\\pagebreak_top ");
		if (pagebreak_bottom)
			fprintf(file, "\\pagebreak_bottom ");
			
		/* Start of appendix? */
		if (start_of_appendix)
			fprintf(file, "\\start_of_appendix ");

		/* Noindent? */
		if (noindent)
			fprintf(file, "\\noindent ");
			
		/* Alignment? */
		if (align != LYX_ALIGN_LAYOUT) {
			switch (align) {
			case LYX_ALIGN_LEFT: h = 1; break;
			case LYX_ALIGN_RIGHT: h = 2; break;
			case LYX_ALIGN_CENTER: h = 3; break;
			default: h = 0; break;
			}
			fprintf(file, "\\align %s ", string_align[h]);
		}
                if (pextra_type != PEXTRA_NONE) {
                        fprintf(file, "\\pextra_type %d", pextra_type);
                        if (pextra_type == PEXTRA_MINIPAGE) {
				fprintf(file, " \\pextra_alignment %d",
					pextra_alignment);
				if (pextra_hfill)
					fprintf(file, " \\pextra_hfill %d",
						pextra_hfill);
				if (pextra_start_minipage)
					fprintf(file, " \\pextra_start_minipage %d",
						pextra_start_minipage);
                        }
                        if (!pextra_width.empty()) {
				fprintf(file, " \\pextra_width %s",
                                        VSpace(pextra_width).asLyXCommand().c_str());
                        } else if (!pextra_widthp.empty()) {
				fprintf(file, " \\pextra_widthp %s",pextra_widthp.c_str());
                        }
                        fprintf(file,"\n");
                }
	}
	else {
   		/* Dummy layout. This means that a footnote ended */
		fprintf(file, "\n\\end_float ");
		footflag = LyXParagraph::NO_FOOTNOTE;
	}
		
	/* It might be a table */ 
	if (table){
		fprintf(file, "\\LyXTable\n");
		table->Write(file);
	}

	// bibitem  ale970302
	if (bibkey)
		bibkey->Write(file);

	font1 = LyXFont(LyXFont::ALL_INHERIT);

	column = 0;
	for (int i = 0; i < last; i++) {
		if (!i){
			fprintf(file, "\n");
			column = 0;
		}

		// Write font changes
		font2 = GetFontSettings(i);
		if (font2 != font1) {
			font2.lyxWriteChanges(font1, file);
			column = 0;
			font1 = font2;
		}

		c = GetChar(i);
		switch (c) {
		case LYX_META_INSET:
			inset = GetInset(i);
			if (inset)
				if (inset->DirectWrite()) {
					// international char, let it write
					// code directly so it's shorter in
					// the file
					inset->Write(file);
				} else {
					fprintf(file, "\n\\begin_inset ");
					inset->Write(file);
					fprintf(file, "\n\\end_inset \n");
					fprintf(file, "\n");
					column = 0;
				}
			break;
		case LYX_META_NEWLINE: 
			fprintf(file, "\n\\newline \n");
			column = 0;
			break;
		case LYX_META_HFILL: 
			fprintf(file, "\n\\hfill \n");
			column = 0;
			break;
		case LYX_META_PROTECTED_SEPARATOR: 
			fprintf(file, "\n\\protected_separator \n");
			column = 0;
			break;
		case '\\': 
			fprintf(file, "\n\\backslash \n");
			column = 0;
			break;
		case '.':
			if (i + 1 < last && GetChar(i + 1) == ' ') {
				fprintf(file, ".\n");
				column = 0;
			} else
				fprintf(file, ".");
			break;
		default:
			if ((column > 70 && c==' ')
			    || column > 79){
				fprintf(file, "\n");
				column = 0;
			}
			// this check is to amend a bug. LyX sometimes
			// inserts '\0' this could cause problems.
			if (c != '\0')
				fprintf(file, "%c", c);
			else
				lyxerr << "ERROR (LyXParagraph::writeFile):"
					" NULL char in structure." << endl;
			column++;
			break;
		}
	}

	// now write the next paragraph
	if (next)
		next->writeFile(file, params, footflag, dth);
}


void LyXParagraph::validate(LaTeXFeatures &features)
{
	// this will be useful later
	LyXLayout *layout = lyxstyle.Style(GetCurrentTextClass(), 
					   GetLayout());
	
	// check the params.
	if (line_top || line_bottom)
		features.lyxline = true;
	
	// then the layouts
	features.layout[GetLayout()] = true;

	// then the fonts
	FontTable *tmpfonttable = fonttable;
	while (tmpfonttable) {
		if (tmpfonttable->font.noun() == LyXFont::ON) {
			lyxerr[Debug::LATEX] << "font.noun: " 
					     << tmpfonttable->font.noun()
					     << endl;
			features.noun = true;
			lyxerr[Debug::LATEX] << "Noun enabled. Font: "
					     << tmpfonttable->font.stateText()
					     << endl;
		}
		switch (tmpfonttable->font.color()) {
		case LyXFont::NONE: 
		case LyXFont::INHERIT_COLOR:
		case LyXFont::IGNORE_COLOR:
			break;
		default:
			features.color = true;
			lyxerr[Debug::LATEX] << "Color enabled. Font: "
					     << tmpfonttable->font.stateText()
					     << endl;
		}
		tmpfonttable = tmpfonttable->next;
	}
	
	// then the insets
	InsetTable *tmpinsettable = insettable;
	while (tmpinsettable) {
		if (tmpinsettable->inset) {
			tmpinsettable->inset->Validate(features);
		}
		tmpinsettable = tmpinsettable->next;
	}
        if (table && table->IsLongTable())
		features.longtable = true;
        if (pextra_type == PEXTRA_INDENT)
                features.LyXParagraphIndent = true;
        if (pextra_type == PEXTRA_FLOATFLT)
                features.floatflt = true;
        if (layout->needprotect 
	    && next && next->footnoteflag != LyXParagraph::NO_FOOTNOTE)
		features.NeedLyXFootnoteCode = true;
        if ((current_view->currentBuffer()->params.paragraph_separation == LYX_PARSEP_INDENT) &&
            (pextra_type == PEXTRA_MINIPAGE))
		features.NeedLyXMinipageIndent = true;
        if (table && table->NeedRotating())
		features.rotating = true;
	if (footnoteflag != NO_FOOTNOTE && footnotekind == ALGORITHM)
		features.algorithm = true;
}


/* first few functions needed for cut and paste and paragraph breaking */
void LyXParagraph::CopyIntoMinibuffer(int pos)
{
	minibuffer_char = GetChar(pos);
	minibuffer_font = GetFontSettings(pos);
	minibuffer_inset = 0;
	if (minibuffer_char == LYX_META_INSET) {
		if (GetInset(pos)) {
			minibuffer_inset = GetInset(pos)->Clone();
		} else {
			minibuffer_inset = 0;
			minibuffer_char = ' ';
			// This reflects what GetInset() does (ARRae)
		}
	}
}


void LyXParagraph::CutIntoMinibuffer(int pos)
{
	minibuffer_char = GetChar(pos);
	minibuffer_font = GetFontSettings(pos);
	minibuffer_inset = 0;
	if (minibuffer_char == LYX_META_INSET) {
		if (GetInset(pos)) {
			minibuffer_inset = GetInset(pos);
			/* This is a little hack since I want exactly the inset,
			   not just a clone. Otherwise the inset would be deleted
			   when calling Erase(pos) */
			
			/* find the entry */ 
			InsetTable *tmpi = insettable;
			while (tmpi && tmpi->pos != pos) {
				tmpi=tmpi->next;
			}
			if (tmpi) {  /* This should always be true */
				tmpi->inset = 0;
			}
		} else {
			minibuffer_inset = 0;
			minibuffer_char = ' ';
			// This reflects what GetInset() does (ARRae)
		}

	}

	/* Erase(pos); now the caller is responsible for that*/
}


void LyXParagraph::InsertFromMinibuffer(int pos)
{
	InsertChar(pos, minibuffer_char);
	SetFont(pos, minibuffer_font);
	if (minibuffer_char == LYX_META_INSET)
		InsertInset(pos, minibuffer_inset);
}

/* end of minibuffer */ 



void LyXParagraph::Clear()
{
	line_top = false;
	line_bottom = false;
   
	pagebreak_top = false;
	pagebreak_bottom = false;

	added_space_top = VSpace(VSpace::NONE);
	added_space_bottom = VSpace(VSpace::NONE);

	align = LYX_ALIGN_LAYOUT;
	depth = 0;
	noindent = false;

        pextra_type = PEXTRA_NONE;
        pextra_width.clear();
        pextra_widthp.clear();
        pextra_alignment = MINIPAGE_ALIGN_TOP;
        pextra_hfill = false;
        pextra_start_minipage = false;

        labelstring.clear();
	labelwidthstring.clear();
	layout = 0;
	bibkey = 0;
	
	start_of_appendix = false;
}


/* the destructor removes the new paragraph from the list */ 
LyXParagraph::~LyXParagraph()
{
	if (previous)
		previous->next = next;
	if (next)
		next->previous = previous;

	if (text)
		delete[] text;
   
	InsetTable *tmpinset;
	while (insettable) {
		tmpinset = insettable;
		insettable = insettable->next;
		if (tmpinset->inset)
			delete tmpinset->inset;
		delete tmpinset;
		if (insettable && insettable->next == insettable) {
			// somehow this recursion appears occasionally
			// but I can't find where.  This bandaid
			// helps but isn't the best fix. (ARRae)
			if (insettable->inset) {
				delete insettable->inset;
			}
			delete insettable;
			break;
		}
	}

	FontTable *tmpfont;
	while (fonttable) {
		tmpfont = fonttable;
		fonttable = fonttable->next;
		delete tmpfont;
	}

	/* table stuff -- begin*/ 
	if (table)
		delete table;
	/* table stuff -- end*/ 

        // ale970302
	if (bibkey)
	        delete bibkey;
}


void LyXParagraph::Erase(int pos)
{
	int i;

	/* > because last is the next unused position, and you can 
	 * use it if you want  */
	if (pos > last) {
		if (next && next->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE) 
			NextAfterFootnote()->Erase(pos - last - 1);
		else 
			lyxerr.debug() << "ERROR (LyXParagraph::Erase): "
				"position does not exist." << endl;
		return;
	}
	if (pos < last) { // last is free for insertation, but should be empty
      
		/* if it is an inset, delete the inset entry */ 
		if (text[pos] == LYX_META_INSET) {
			/* find the entry */ 
			InsetTable *tmpi = insettable;
			InsetTable *tmpi2 = tmpi;
			while (tmpi && tmpi->pos != pos) {
				tmpi2=tmpi;
				tmpi=tmpi->next;
			}
			if (tmpi) {	// this should always be true
				if (tmpi->inset) // delete the inset if it exists
					delete tmpi->inset;
				if (tmpi == insettable)
					insettable = tmpi->next;
				else 
					tmpi2->next = tmpi->next;
				delete tmpi;
			}
		}

		// Shift rest of text      
		for (i = pos; i < last - 1; i++) {
			text[i]=text[i+1];
		}
		last--;

		/* erase entries in the tables */ 
		int found = 0;
		FontTable *tmp = fonttable;
		FontTable *prev = 0;
		while (tmp && !found) {
			if (pos >= tmp->pos && pos <= tmp->pos_end)
				found = 1;
			else {
				prev = tmp;
				tmp = tmp->next;
			}
		}
      
		if (found && tmp->pos == tmp->pos_end) {  
			/* if it is a multi-character font entry, we just make 
			 * it smaller (see update below), otherwise we should 
			 * delete it */
			if (prev)
				prev->next = tmp->next;
			else
				fonttable = tmp->next;
	 
			delete tmp;
		}

		/* update all other entries */

		tmp = fonttable;
		while (tmp) {
			if (tmp->pos > pos)
				tmp->pos--;
			if (tmp->pos_end >= pos)
				tmp->pos_end--;
			tmp = tmp->next;
		}
      
		/* update the inset table */ 
		InsetTable *tmpi = insettable;
		while (tmpi) {
			if (tmpi->pos > pos)
				tmpi->pos--;
			tmpi=tmpi->next;
		}
      
	} else {
		lyxerr << "ERROR (LyXParagraph::Erase): "
			"can't erase non-existant char." << endl;
	}
}


/* pos is needed to specify the paragraph correctly. Remember the
* closed footnotes */ 
void LyXParagraph::Enlarge(int pos, int number)
{
	int i;

	/* > because last is the next unused position, and you can 
	 * use it if you want  */
	if (pos > last) {
		if (next && next->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE) 
			NextAfterFootnote()->Enlarge(pos - last - 1, number);
		else 
			lyxerr << "ERROR (LyXParagraph::Enlarge): "
				"position does not exist." << endl;
		return;
	}

	if (size - last < number) {
		size += number - size + last + STEP_SIZE_PAR;
		char *tmp = new char[size];
		for (i = 0; i < last; i++)
			tmp[i] = text[i];
		delete[] text;
		text = tmp;
	}
}


/* make the allocated memory fit to the needed size */
/* used to make a paragraph smaller */
void LyXParagraph::FitSize()
{
	int i;
  
	if (size - last > STEP_SIZE_PAR) {
		size = last + STEP_SIZE_PAR;
		char *tmp = new char[size];
		for (i = 0; i < last; i++)
			tmp[i] = text[i];
		delete[] text;
		text = tmp;
	}
}


void LyXParagraph::InsertChar(int pos, char c)
{
	register int i;
   
	/* > because last is the next unused position, and you can 
	 * use it if you want  */
	if (pos > last) {
		if (next && next->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE) 
			NextAfterFootnote()->InsertChar(pos - last - 1, c);
		else 
			lyxerr.debug() << "ERROR (LyXParagraph::InsertChar): "
				"position does not exist." << endl;
		return;
	}

	// Are we full? If so, enlarge.
	if (last == size) {
		size += STEP_SIZE_PAR;
		char *tmp = new char[size];
		for (i = 0; i < last; i++)
			tmp[i] = text[i];
		delete[] text;
		text = tmp;
	}

	// Shift rest of character
	for (i = last; i>pos; i--) {
		text[i]=text[i-1];
	}

	text[pos]=c;
	last++;
   
	/* update the font table */ 
	FontTable *tmp = fonttable;
	while (tmp) {
		if (tmp->pos >= pos)
			tmp->pos++;
		if (tmp->pos_end >= pos)
			tmp->pos_end++;
		tmp = tmp->next;
	}
   
	/* update the inset table */ 
	InsetTable *tmpi = insettable;
	while (tmpi) {
		if (tmpi->pos >= pos)
			tmpi->pos++;
		tmpi=tmpi->next;
	}
}


void LyXParagraph::InsertInset(int pos, Inset *inset)
{
	/* > because last is the next unused position, and you can 
	 * use it if you want  */
	if (pos > last) {
		if (next && next->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE) 
			NextAfterFootnote()->InsertInset(pos - last - 1, inset);
		else 
			lyxerr << "ERROR (LyXParagraph::InsertInset): " 
				"position does not exist: " << pos << endl;
		return;
	}

	if (text[pos]!=LYX_META_INSET) {
		lyxerr << "ERROR (LyXParagraph::InsertInset): "
			"there is no LYX_META_INSET" << endl;
		return;
	}

	if (inset) {
		/* add a new entry in the inset table */ 
		InsetTable *tmpi = new InsetTable;
		tmpi->pos = pos;
		tmpi->inset = inset;
		tmpi->next = insettable;
		insettable = tmpi;
	}
}


Inset* LyXParagraph::GetInset(int pos)
{
	if (pos >= last) {
		if (next && next->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE) 
			return NextAfterFootnote()->GetInset(pos - last - 1);
		else { 
		        lyxerr << "ERROR (LyXParagraph::GetInset): position does not exist: "
			       << pos << endl;
		}
		return 0;
	}

	/* find the inset */ 
	InsetTable *tmpi = insettable;

	while (tmpi && tmpi->pos != pos)
		tmpi = tmpi->next;

	if (tmpi)
		return tmpi->inset;
	else {
		lyxerr << "ERROR (LyXParagraph::GetInset): "
			"Inset does not exist: " << pos << endl;
		text[pos] = ' '; /// WHY!!! does this set the pos to ' '????
		// Did this commenting out introduce a bug? So far I have not
		// seen any, please enlighten me. (Lgb)
		// My guess is that since the inset does not exist, we might
		// as well replace it with a space to prevent crashes. (Asger)
		return 0;
	}
}


// Gets uninstantiated font setting at position.
// Optimized after profiling. (Asger)
LyXFont LyXParagraph::GetFontSettings(int pos)
{
	if (pos < last) {
		FontTable *tmp = fonttable;
		while (tmp) {
			if (pos >= tmp->pos && pos <= tmp->pos_end) 
				return tmp->font;
			tmp = tmp->next;
		}
	}

	/* > because last is the next unused position, and you can 
	 * use it if you want  */
	else if (pos > last) {
		if (next && next->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE) 
			return NextAfterFootnote()->GetFontSettings(pos - last - 1);
		else {
			// Why is it an error to ask for the font of a
			// position that does not exist? Would it be
			// enough for this to be anable on debug?
			// We want strict error checking, but it's ok to only
			// have it when debugging. (Asger)
			lyxerr << "ERROR (LyXParagraph::GetFontSettings): "
				"position does not exist. " << pos << endl;
		}
	} else if (pos) {
		return GetFontSettings(pos - 1);
	}
	return LyXFont(LyXFont::ALL_INHERIT);
}


// Gets the fully instantiated font at a given position in a paragraph
// This is basically the same function as LyXText::GetFont() in text2.C.
// The difference is that this one is used for generating the LaTeX file,
// and thus cosmetic "improvements" are disallowed: This has to deliver
// the true picture of the buffer. (Asger)
// If position is -1, we get the layout font of the paragraph.
// If position is -2, we get the font of the manual label of the paragraph.
LyXFont LyXParagraph::getFont(int pos)
{
	LyXFont tmpfont;
	LyXLayout *layout = lyxstyle.Style(GetCurrentTextClass(), 
					   GetLayout());
   
	int main_body=0;
	if (layout->labeltype == LABEL_MANUAL)
		main_body = BeginningOfMainBody();

	if (pos >= 0){
		LyXFont layoutfont;
		if (pos < main_body)
			layoutfont = layout->labelfont;
		else
			layoutfont = layout->font;
		tmpfont = GetFontSettings(pos);
		tmpfont.realize(layoutfont);
	} else {
		// process layoutfont for pos == -1 and labelfont for pos < -1
		if (pos == -1)
			tmpfont = layout->font;
		else
			tmpfont = layout->labelfont;
	}

	// check for environment font information
	char par_depth = GetDepth();
	LyXParagraph * par = this;
	while (par && par_depth && !tmpfont.resolved()) {
		par = par->DepthHook(par_depth - 1);
		if (par) {
			tmpfont.realize(lyxstyle.
					Style(GetCurrentTextClass(),
					      par->GetLayout())->font);
			par_depth = par->GetDepth();
		}
	}

	tmpfont.realize(lyxstyle.TextClass(GetCurrentTextClass())->defaultfont);
	return tmpfont;
}


/// Returns the height of the highest font in range
LyXFont::FONT_SIZE LyXParagraph::HighestFontInRange(int startpos, int endpos) const
{
	LyXFont::FONT_SIZE maxsize = LyXFont::SIZE_TINY;

	FontTable *tmp = fonttable;
	while (tmp) {
		if (startpos <= tmp->pos_end && endpos >= tmp->pos) {
			LyXFont::FONT_SIZE size = tmp->font.size();
			if (size > maxsize && size<=LyXFont::SIZE_HUGER)
				maxsize = size;
		}
		tmp = tmp->next;
	}
	return maxsize;
}


char LyXParagraph::GetChar(int pos)
{
#ifdef DEVEL_VERSION
	/* a workaround to 'fix' some bugs in text-class */
	if (pos < 0) {
		// This function is important. It should not work around bugs.
		// Let's find the bugs instead and fix them. (Asger)
		lyxerr << "FATAL ERROR (LyXParagraph::GetChar):"
			" bad position "  << pos << endl;
		abort();
	}
#endif

	if (pos < last) {
		return text[pos];
	}

	/* > because last is the next unused position, and you can 
	 * use it if you want  */
	else if (pos > last) {
		if (next && next->footnoteflag != LyXParagraph::NO_FOOTNOTE) 
			return NextAfterFootnote()->GetChar(pos - last - 1);
		else 
			lyxerr << "ERROR (LyXParagraph::GetChar): "
				"position does not exist." << endl;
		return '\0';
	} else { // pos==last
		/* we should have a footnote environment */ 
		if (!next || next->footnoteflag == LyXParagraph::NO_FOOTNOTE) {
// Notice that LyX does request the last char from time to time. (Asger)
//			lyxerr << "ERROR (LyXParagraph::GetChar): "
//				      "expected footnote." << endl;
			return '\0';
		}
		switch (next->footnotekind) {
		case LyXParagraph::FOOTNOTE:
			return LYX_META_FOOTNOTE;
		case LyXParagraph::MARGIN:
			return LYX_META_MARGIN;
		case LyXParagraph::FIG:
		case LyXParagraph::WIDE_FIG:
			return LYX_META_FIG;
		case LyXParagraph::TAB:
		case LyXParagraph::WIDE_TAB:
			return LYX_META_TAB;
		case LyXParagraph::ALGORITHM:
			return LYX_META_ALGORITHM;
		}
// 		if (next->footnotekind == LyXParagraph::FOOTNOTE)
// 			return LYX_META_FOOTNOTE;
// 		if (next->footnotekind == LyXParagraph::MARGIN)
// 			return LYX_META_MARGIN;
// 		if (next->footnotekind == LyXParagraph::FIG)
// 			return LYX_META_FIG;
// 		if (next->footnotekind == LyXParagraph::TAB)
// 			return LYX_META_TAB;
// 		if (next->footnotekind == LyXParagraph::ALGORITHM)
// 			return LYX_META_ALGORITHM;
// 		lyxerr << "ERROR (LyXParagraph::GetChar): "
// 			      "unknown footnote kind." << endl;
// 		return 'F';		/* this should not happen!  */
		// This _can_ not happen, due to the type of next->footnotekind
		// being LyXParagraph::footnot_kind
		return '\0'; // to shut up gcc
	}
}


string LyXParagraph::GetWord(int & lastpos)


  //Added 98/9/21 by REH
  // return an string of the current word, and the end of the word
  // in lastpos.

  // the current word is defined as starting at the first character from
  // the immediate left of lastpospos which meets the definition of IsLetter(),
  // continuing to the last character to the right of this meeting
  // IsLetter.


  // i just left this in from GetChar()
{
#ifdef DEVEL_VERSION
	/* a workaround to 'fix' some bugs in text-class */
	if (lastpos < 0) {
		// This function is important. It should not work around bugs.
		// Let's find the bugs instead and fix them. (Asger)
		lyxerr << "FATAL ERROR (LyXParagraph::GetWord):"
			" bad position " << lastpos << endl;
		abort();
	}
#endif

    	string theword;

	// grab a word
	
	
	//i think the devcode aborts before this, but why not be
	// versatile?
	if (lastpos < 0) lastpos=0; 

	
	// move back until we have a letter

	//there's no real reason to have firstpos & lastpos as
	//separate variables as this is written, but maybe someon
	// will want to return firstpos in the future.

	//since someone might have typed a punctuation first
	int firstpos = lastpos;
 	
	while ((firstpos >=0) && !IsLetter(firstpos))
		firstpos--;

	// now find the beginning by looking for a nonletter
	
	while ((firstpos>=0) && IsLetter(firstpos))
		firstpos--;

	// the above is now pointing to the preceeding non-letter
	firstpos++;
	lastpos=firstpos;

	// so copy characters into theword  until we get a nonletter
	// note that this can easily exceed lastpos, wich means
	// that if used in the middle of a word, the whole word
	// is included

	while (IsLetter(lastpos)) theword += GetChar(lastpos++);
	
	return  theword;

}


int LyXParagraph::Last()
{
	if (next && next->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE)
		return last + NextAfterFootnote()->Last() + 1;   /* the 1 is the symbol
								  * for the footnote */
	else
		return last;
}


LyXParagraph *LyXParagraph::ParFromPos(int pos)
{
	/* > because last is the next unused position, and you can 
	 * use it if you want  */
	if (pos > last) {
		if (next && next->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE) 
			return NextAfterFootnote()->ParFromPos(pos - last - 1);
		else 
			lyxerr << "ERROR (LyXParagraph::ParFromPos): "
				"position does not exist." << endl;
		return this;
	}
	else
		return this;
}


int LyXParagraph::PositionInParFromPos(int pos)
{
	/* > because last is the next unused position, and you can 
	 * use it if you want  */
	if (pos > last) {
		if (next && next->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE) 
			return NextAfterFootnote()->PositionInParFromPos(pos - last - 1);
		else 
			lyxerr <<
				"ERROR (LyXParagraph::PositionInParFromPos): "
				"position does not exist." << endl;
		return pos;
	}
	else
		return pos;
}


void LyXParagraph::SetFont(int pos, LyXFont const & font)
{
	/* > because last is the next unused position, and you can 
	 * use it if you want  */
	if (pos > last) {
		if (next && next->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE) {
			NextAfterFootnote()->SetFont(pos - last - 1, font);
		} else {
			lyxerr << "ERROR (LyXParagraph::SetFont): "
				"position does not exist." << endl;
		}
		return;
	}

	LyXFont patternfont(LyXFont::ALL_INHERIT);

// First, reduce font against layout/label font
// Update: The SetCharFont() routine in text2.C already reduces font, so
// we don't need to do that here. (Asger)
// No need to simplify this because it will disappear in a new kernel. (Asger)

	// Next search font table
	FontTable *tmp2;

	bool found = false;
	FontTable *tmp = fonttable;
	while (tmp && !found) {
		if (pos >= tmp->pos && pos <= tmp->pos_end)
			found = true;
		else
			tmp = tmp->next;
	}

	if (!found) {
		/* if we did not find a font entry, but if the font at hand
		 * is the same as default, we just forget it */
		if (font == patternfont)
			return;

		/* ok, we did not find a font entry. But maybe there is exactly
		 * the needed font entry one position left */ 
		found = false;
		tmp2 = fonttable;
		while (tmp2 && !found) {
			if (pos - 1 >= tmp2->pos && pos - 1 <= tmp2->pos_end)
				found = true;
			else
				tmp2 = tmp2->next;
		}
		if (found) {
			/* ok there is one. maybe it is exactly the needed font */
			if (tmp2->font == font) {
				/* put the position under the font */ 
				tmp2->pos_end++;
				return;
			}
		}
		/* Add a new entry in the 
		 * fonttable for the position */
		tmp = new FontTable;
		tmp->pos = pos;
		tmp->pos_end = pos;
		tmp->font = patternfont;
		tmp->next = fonttable;
		fonttable = tmp;
	}
	else {
		/* we found a font entry. maybe we have to split it and create
		 * a new one */ 

		if (tmp->pos != tmp->pos_end) {  /* more than one character  */

			if (pos == tmp->pos) {
				/* maybe we could enlarge the left fonttable */ 

				found = false;
				tmp2 = fonttable;
				while (tmp2 && !found) {
					if (pos - 1 >= tmp2->pos && pos - 1 <= tmp2->pos_end)
						found = true;
					else
						tmp2 = tmp2->next;
				}

				/* Is there is one, and is it exactly the needed font? */
				if (found && tmp2->font == font) {
					/* put the position under the font */ 
					tmp2->pos_end++;
					tmp->pos++;
					return;
				}

				/* Add a new entry in the 
				 * fonttable for the position */
				tmp2 = new FontTable;
				tmp2->pos = pos + 1;
				tmp2->pos_end = tmp->pos_end;
				tmp2->font = tmp->font;
				tmp->pos_end = pos;
				tmp2->next = fonttable;
				fonttable = tmp2;
			}
			else if (pos == tmp->pos_end) {
				/* Add a new entry in the 
				 * fonttable for the position */
				tmp2 = new FontTable;
				tmp2->pos = tmp->pos;
				tmp2->pos_end = tmp->pos_end - 1;
				tmp2->font = tmp->font;
				tmp->pos = tmp->pos_end;
				tmp2->next = fonttable;
				fonttable = tmp2;
			}
			else {
				/* Add a new entry in the 
				 * fonttable for the position */
				tmp2 = new FontTable;
				tmp2->pos = tmp->pos;
				tmp2->pos_end = pos - 1;
				tmp2->font = tmp->font;
				tmp2->next = fonttable;
				fonttable = tmp2;
	    
				tmp2 = new FontTable;
				tmp2->pos = pos + 1;
				tmp2->pos_end = tmp->pos_end;
				tmp2->font = tmp->font;
				tmp2->next = fonttable;
				fonttable = tmp2;
	    
				tmp->pos = pos;
				tmp->pos_end = pos;
			}
		}
	}

	tmp->font = font;
}

   
/* this function is able to hide closed footnotes */
LyXParagraph *LyXParagraph::Next()
{
	LyXParagraph *tmp;
	if (next && next->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE) {
		tmp = next;
		while (tmp && tmp->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE)
			tmp = tmp->next;
		if (tmp && tmp->footnoteflag != LyXParagraph::CLOSED_FOOTNOTE) 
			return tmp->Next(); /* there can be more than one footnote
					     * in a logical paragraph */
		else
			return next;	/* this should never happen! */
	}
	else
		return next;
}


LyXParagraph *LyXParagraph::NextAfterFootnote()
{
	LyXParagraph *tmp;
	if (next && next->footnoteflag != LyXParagraph::NO_FOOTNOTE) {
		tmp = next;
		while (tmp && tmp->footnoteflag != LyXParagraph::NO_FOOTNOTE)
			tmp = tmp->next;
		if (tmp && tmp->footnoteflag != LyXParagraph::CLOSED_FOOTNOTE) 
			return tmp;	       /* there can be more than one footnote
						* in a logical paragraph */
		else
			return next; 		       /* this should never happen! */
	}
	else
		return next;
}


LyXParagraph *LyXParagraph::PreviousBeforeFootnote()
{
	LyXParagraph *tmp;
	if (previous && previous->footnoteflag != LyXParagraph::NO_FOOTNOTE) {
		tmp = next;
		while (tmp && tmp->footnoteflag != LyXParagraph::NO_FOOTNOTE)
			tmp = tmp->previous;
		if (tmp && tmp->footnoteflag != LyXParagraph::CLOSED_FOOTNOTE) 
			return tmp;	       /* there can be more than one footnote
						* in a logical paragraph */
		else
			return previous; 		       /* this should never happen! */
	}
	else
		return previous;
}


LyXParagraph *LyXParagraph::LastPhysicalPar()
{
	LyXParagraph *tmp;
	if (footnoteflag != LyXParagraph::NO_FOOTNOTE)
		return this;
   
	tmp = this;
	while (tmp->next && tmp->next->footnoteflag != LyXParagraph::NO_FOOTNOTE)
		tmp = tmp->NextAfterFootnote();
   
	return tmp;
   
}


LyXParagraph *LyXParagraph::FirstPhysicalPar()
{
	if (!IsDummy())
		return this;
	LyXParagraph *tmppar = this;

	while (tmppar && (tmppar->IsDummy()
			  || tmppar->footnoteflag != LyXParagraph::NO_FOOTNOTE))
		tmppar = tmppar->previous;
   
	if (!tmppar)
		return this;	       /* this should never happen!  */
	else
		return tmppar;
}


/* this function is able to hide closed footnotes */
LyXParagraph *LyXParagraph::Previous()
{
	LyXParagraph *tmp = previous;
	if (!tmp)
		return tmp;
   
	if (tmp->previous && tmp->previous->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE) {
		tmp = tmp->previous;
		while (tmp && tmp->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE)
			tmp = tmp->previous;
		if (tmp && tmp->footnoteflag != LyXParagraph::CLOSED_FOOTNOTE) 
			return tmp->next->Previous();	

		else
			return previous; 
	}
	else
		return previous;
}

   
void LyXParagraph::BreakParagraph(int pos, int flag)
{
	int i, pos_end, pos_first;

	/* create a new paragraph */
	LyXParagraph *par = ParFromPos(pos);
	LyXParagraph *firstpar = FirstPhysicalPar();
   
	LyXParagraph *tmp = new LyXParagraph(par);
   
	tmp->footnoteflag = footnoteflag;
	tmp->footnotekind = footnotekind;
   
	/* this is an idea for a more userfriendly layout handling, I will
	 * see what the users say */
   
	/* layout stays the same with latex-environments */ 
	if (flag) {
		tmp->SetOnlyLayout(firstpar->layout);
		tmp->SetLabelWidthString(firstpar->labelwidthstring);
	}

	if (Last() > pos || !Last() || flag == 2) {
		tmp->SetOnlyLayout(firstpar->layout);
		tmp->align = firstpar->align;
		tmp->SetLabelWidthString(firstpar->labelwidthstring);
      
		tmp->line_bottom = firstpar->line_bottom;
		firstpar->line_bottom = false;
		tmp->pagebreak_bottom = firstpar->pagebreak_bottom;
		firstpar->pagebreak_bottom = false;
		tmp->added_space_bottom = firstpar->added_space_bottom;
		firstpar->added_space_bottom = VSpace(VSpace::NONE);
      
		tmp->depth = firstpar->depth;
		tmp->noindent = firstpar->noindent;
   
		/* copy everything behind the break-position to the new paragraph */
		pos_first = 0;
		while (ParFromPos(pos_first) != par)
			pos_first++;
   
		pos_end = pos_first + par->last - 1;
   
		/* make sure there is enough memory for the now larger
		   paragraph. This is not neccessary, because
		   InsertFromMinibuffer will enlarge the memory (it uses
		   InsertChar of course). But doing it by hand
		   is MUCH faster! (only one time, not thousend times!!) */

		tmp->Enlarge(0, pos_end - pos);

		for (i = pos; i <= pos_end; i++) {
			par->CutIntoMinibuffer(i - pos_first);
			tmp->InsertFromMinibuffer(i - pos);
		}

		for (i = pos_end; i >= pos; i--)
			par->Erase(i - pos_first);
		/* free memory of the now shorter paragraph*/
		par->FitSize();
	}

	/* just an idea of me */ 
	if (!pos) {
		tmp->line_top = firstpar->line_top;
		tmp->pagebreak_top = firstpar->pagebreak_top;
		tmp->added_space_top = firstpar->added_space_top;
		tmp->bibkey = firstpar->bibkey;
		firstpar->Clear();
		/* layout stays the same with latex-environments */ 
		if (flag) {
			firstpar->SetOnlyLayout(tmp->layout);
			firstpar->SetLabelWidthString(tmp->labelwidthstring);
			firstpar->depth = tmp->depth;
		}
	}
}


void LyXParagraph::MakeSameLayout(LyXParagraph *par)
{
	par = par->FirstPhysicalPar();
	footnoteflag = par->footnoteflag;
	footnotekind = par->footnotekind;

	layout = par->layout;
	align = par-> align;
	SetLabelWidthString(par->labelwidthstring);

	line_bottom = par->line_bottom;
	pagebreak_bottom = par->pagebreak_bottom;
	added_space_bottom = par->added_space_bottom;

	line_top = par->line_top;
	pagebreak_top = par->pagebreak_top;
	added_space_top = par->added_space_top;

        pextra_type = par->pextra_type;
        pextra_width = par->pextra_width;
        pextra_widthp = par->pextra_widthp;
        pextra_alignment = par->pextra_alignment;
        pextra_hfill = par->pextra_hfill;
        pextra_start_minipage = par->pextra_start_minipage;

	noindent = par->noindent;
	depth = par->depth;
}


LyXParagraph *LyXParagraph::FirstSelfrowPar()
{
	LyXParagraph *tmppar;
   
	tmppar = this;
	while (tmppar && (
		(tmppar->IsDummy() && tmppar->previous->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE)
		|| tmppar->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE))
		tmppar = tmppar->previous;
   
	if (!tmppar)
		return this;	       /* this should never happen!  */
	else
		return tmppar;
}


LyXParagraph *LyXParagraph::Clone()
{
	int i;
   
	/* create a new paragraph */
	LyXParagraph *result = new LyXParagraph();
   
	result->MakeSameLayout(this);

	/* this is because of the dummy layout of the paragraphs that
	   follow footnotes */
	result->layout = layout;
   
	/* table stuff -- begin*/ 
	if (table)
		result->table = table->Clone();
	else
		result->table = 0;
	/* table stuff -- end*/ 
   
        // ale970302
        result->bibkey = (bibkey) ? new InsetBibKey(bibkey): 0;
               
    
	/* copy everything behind the break-position to the new paragraph */
   
	/* make shure there is enough memory for the now larger paragraph.
	 * This is not neccessary, because InsertFromMinibuffer will enlarge
	 * the memory (it uses InsertChar of course). But doing it by hand
	 * is MUCH faster! (only one time, not thousend times!!) */
   
	result->Enlarge(0, last+2);
   
	for (i = 0; i < last; i++) {
		CopyIntoMinibuffer(i);
		result->InsertFromMinibuffer(i);
	}
	return result;
}


bool LyXParagraph::HasSameLayout(LyXParagraph* par)
{
	par = par->FirstPhysicalPar();

	return (
		par->footnoteflag == footnoteflag &&
		par->footnotekind == footnotekind &&

		par->layout == layout &&

		par->align == align &&

		par->line_bottom == line_bottom &&
		par->pagebreak_bottom == pagebreak_bottom &&
		par->added_space_bottom == added_space_bottom &&

		par->line_top == line_top &&
		par->pagebreak_top == pagebreak_top &&
		par->added_space_top == added_space_top &&

                par->pextra_type == pextra_type &&
                par->pextra_width == pextra_width && 
                par->pextra_widthp == pextra_widthp && 
                par->pextra_alignment == pextra_alignment && 
                par->pextra_hfill == pextra_hfill && 
                par->pextra_start_minipage == pextra_start_minipage && 

		par->table == table && // what means: NO TABLE AT ALL 

		par->noindent == noindent &&
		par->depth == depth);
}


void LyXParagraph::BreakParagraphConservative(int pos)
{
	int i, pos_end, pos_first;
   
	/* create a new paragraph */
	LyXParagraph *par = ParFromPos(pos);

	LyXParagraph *tmp = new LyXParagraph(par);
   
	tmp->MakeSameLayout(par);
   
	if (Last() > pos) {   
		/* copy everything behind the break-position to the new
		   paragraph */
		pos_first = 0;
		while (ParFromPos(pos_first) != par)
			pos_first++;
   
		pos_end = pos_first + par->last - 1;
   
		/* make shure there is enough memory for the now larger
		   paragraph. This is not neccessary, because
		   InsertFromMinibuffer will enlarge the memory (it uses
		   InsertChar of course). But doing it by hand
		   is MUCH faster! (only one time, not thousend times!!) */
   
		tmp->Enlarge(0, pos_end - pos);
   
		for (i = pos; i <= pos_end; i++) {
      
			par->CutIntoMinibuffer(i - pos_first);
			tmp->InsertFromMinibuffer(i - pos);
		}
		for (i = pos_end; i >= pos; i--)
			par->Erase(i - pos_first);

		/* free memory of the now shorter paragraph*/
		par->FitSize();
	}
}
   

/* be carefull, this does not make any check at all */ 
void LyXParagraph::PasteParagraph()
{
	int i, pos_end, pos_insert;
	LyXParagraph *the_next;
   
	/* copy the next paragraph to this one */
	the_next = Next();
   
	LyXParagraph *firstpar = FirstPhysicalPar();
   
	/* first the DTP-stuff */ 
	firstpar->line_bottom = the_next->line_bottom;
	firstpar->added_space_bottom = the_next->added_space_bottom;
	firstpar->pagebreak_bottom = the_next->pagebreak_bottom;
   
	pos_end = the_next->last - 1;
	pos_insert = Last();
   
	/* enlarge the paragraph. This is faster than enlarge it
	 * every 10th insertion. */ 
	if (pos_end >= 0)
		Enlarge(pos_insert, pos_end);
      
	/* ok, now copy the paragraph */ 
	for (i = 0; i <= pos_end; i++) {
		the_next->CutIntoMinibuffer(i);
		InsertFromMinibuffer(pos_insert + i);
	}
   
	/* delete the next paragraph */
	delete the_next;
}


void LyXParagraph::OpenFootnote(int pos)
{
	LyXParagraph *par = ParFromPos(pos);
	par = par->next;
	while (par && par->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE) {
		par->footnoteflag = LyXParagraph::OPEN_FOOTNOTE;
		par = par->next;
	}
}


void LyXParagraph::CloseFootnote(int pos)
{
	LyXParagraph *par = ParFromPos(pos);
	par = par->next;
	while (par && par->footnoteflag == LyXParagraph::OPEN_FOOTNOTE) {
		par->footnoteflag = LyXParagraph::CLOSED_FOOTNOTE;
		par = par->next;
	}
}


int LyXParagraph::GetLayout()
{
	return FirstPhysicalPar()->layout;
}


char LyXParagraph::GetDepth()
{
	return FirstPhysicalPar()->depth;
}


char LyXParagraph::GetAlign()
{
	return FirstPhysicalPar()->align;
}


string LyXParagraph::GetLabestring()
{
	return FirstPhysicalPar()->labelstring;
}


int LyXParagraph::GetFirstCounter(int i)
{
	return FirstPhysicalPar()->counter[i];
}


/* the next two functions are for the manual labels */ 
string LyXParagraph::GetLabelWidthString()
{
	if (!FirstPhysicalPar()->labelwidthstring.empty())
		return FirstPhysicalPar()->labelwidthstring;
	else
		return _("Senseless with this layout!");
}


void LyXParagraph::SetLabelWidthString(string const & s)
{
	LyXParagraph *par = FirstPhysicalPar();

	par->labelwidthstring = s;
}


void LyXParagraph::SetOnlyLayout(char new_layout)
{
	LyXParagraph
		*par = FirstPhysicalPar(),
		*ppar = 0,
                *npar = 0;

	par->layout = new_layout;
	/* table stuff -- begin*/ 
	if (table) 
		par->layout = 0;
	/* table stuff -- end*/ 
        if (par->pextra_type == PEXTRA_NONE) {
                if (par->Previous()) {
                        ppar = par->Previous()->FirstPhysicalPar();
                        while(ppar
			      && ppar->Previous()
			      && (ppar->depth > par->depth))
                                ppar = ppar->Previous()->FirstPhysicalPar();
                }
                if (par->Next()) {
                        npar = par->Next()->NextAfterFootnote();
                        while(npar
			      && npar->Next()
			      && (npar->depth > par->depth))
                                npar = npar->Next()->NextAfterFootnote();
                }
                if (ppar && (ppar->pextra_type != PEXTRA_NONE)) {
                        string
                                p1 = ppar->pextra_width,
                                p2 = ppar->pextra_widthp;
                        ppar->SetPExtraType(ppar->pextra_type,
                                            p1.c_str(),p2.c_str());
                }
                if ((par->pextra_type == PEXTRA_NONE) &&
                    npar && (npar->pextra_type != PEXTRA_NONE)) {
                        string
                                p1 = npar->pextra_width,
                                p2 = npar->pextra_widthp;
                        npar->SetPExtraType(npar->pextra_type,
                                            p1.c_str(),p2.c_str());
                }
        }
}


void LyXParagraph::SetLayout(char new_layout)
{
	LyXParagraph
		*par = FirstPhysicalPar(),
		*ppar = 0,
		*npar = 0;

        par->layout = new_layout;
	par->labelwidthstring.clear();
	par->align = LYX_ALIGN_LAYOUT;
	//par->depth = 0;
	par->added_space_top = VSpace(VSpace::NONE);
	par->added_space_bottom = VSpace(VSpace::NONE);
	/* table stuff -- begin*/ 
	if (table) 
		par->layout = 0;
	/* table stuff -- end*/
        if (par->pextra_type == PEXTRA_NONE) {
                if (par->Previous()) {
                        ppar = par->Previous()->FirstPhysicalPar();
                        while(ppar
			      && ppar->Previous()
			      && (ppar->depth > par->depth))
                                ppar = ppar->Previous()->FirstPhysicalPar();
                }
                if (par->Next()) {
                        npar = par->Next()->NextAfterFootnote();
                        while(npar
			      && npar->Next()
			      && (npar->depth > par->depth))
                                npar = npar->Next()->NextAfterFootnote();
                }
                if (ppar && (ppar->pextra_type != PEXTRA_NONE)) {
                        string
                                p1 = ppar->pextra_width,
                                p2 = ppar->pextra_widthp;
                        ppar->SetPExtraType(ppar->pextra_type,
                                            p1.c_str(),p2.c_str());
                }
                if ((par->pextra_type == PEXTRA_NONE) &&
                    npar && (npar->pextra_type != PEXTRA_NONE)) {
                        string
                                p1 = npar->pextra_width,
                                p2 = npar->pextra_widthp;
                        npar->SetPExtraType(npar->pextra_type,
                                            p1.c_str(),p2.c_str());
                }
        }
}


/* if the layout of a paragraph contains a manual label, the beginning of the 
* main body is the beginning of the second word. This is what the par-
* function returns. If the layout does not contain a label, the main
* body always starts with position 0. This differentiation is necessary,
* because there cannot be a newline or a blank <= the beginning of the 
* main body in TeX. */ 

int LyXParagraph::BeginningOfMainBody()
{
	if (FirstPhysicalPar() != this)
		return -1;
   
	int i = 0;
   
//	while (i < last   &&  !(i > 1 && GetChar(i-1)==' ')
//	       && GetChar(i)!=LYX_META_NEWLINE)
//		i++;
// Unroll the first two cycles of this loop
// and remember the previous character to remove unnecessary GetChar() calls

	if (i < last
	    && GetChar(i) != LYX_META_NEWLINE) {
		++i;
		char previous_char, temp;
		if (i < last
		    && (previous_char = GetChar(i)) != LYX_META_NEWLINE) {
			// Yes, this  ^ is supposed to be "=" not "=="
			++i;
			while (i < last
			       && previous_char != ' '
			       && (temp = GetChar(i)) != LYX_META_NEWLINE) {
				++i;
				previous_char = temp;
			}
		}
	}

	if (i==0 && i == last &&
	    !(footnoteflag==LyXParagraph::NO_FOOTNOTE
	      && next && next->footnoteflag != LyXParagraph::NO_FOOTNOTE)
		)
		i++;			       /* the cursor should not jump  
						* to the main body if there
						* is nothing in! */
	return i;
}


LyXParagraph* LyXParagraph::DepthHook(int deth)
{
	LyXParagraph *newpar = this;
	if (deth < 0)
		return 0;
   
	do {
		newpar = newpar->FirstPhysicalPar()->Previous();
	} while (newpar && newpar->GetDepth() > deth
		 && newpar->footnoteflag == footnoteflag);
   
	if (!newpar) {
		if (Previous() || GetDepth())
			lyxerr << "ERROR (LyXParagraph::DepthHook): "
				"no hook." << endl;
		newpar = this;
	}
	return newpar->FirstPhysicalPar();
}


int LyXParagraph::AutoDeleteInsets()
{
	InsetTable *tmpi = insettable;
	InsetTable *tmpi2 = tmpi;
	int i=0;
	while (tmpi) {
		tmpi2 = tmpi;
		tmpi = tmpi->next;
		if (tmpi2->inset)
			if (tmpi2->inset->AutoDelete()) {
				i++;
				Erase(tmpi2->pos);
			} else {}
		else
			lyxerr << "ERROR (LyXParagraph::AutoDeleteInsets): "
				"cannot auto-delete insets" << endl;
	}
	return i;
}


Inset* LyXParagraph::ReturnNextInsetPointer(int &pos)
{
	InsetTable *tmpi = insettable;
	InsetTable *tmpi2 = 0;
	while (tmpi){
		if (tmpi->pos >= pos) {
			if (!tmpi2 || tmpi->pos < tmpi2->pos)
				tmpi2 = tmpi;
		}
		tmpi=tmpi->next;
	}
	if (tmpi2){
		pos = tmpi2->pos;
		return tmpi2->inset;
	}
	else
		return 0;
}


/* returns -1 if inset not found */
int LyXParagraph::GetPositionOfInset(Inset* inset)
{
	/* find the entry */ 
	InsetTable *tmpi = insettable;
	while (tmpi && tmpi->inset != inset) {
		tmpi=tmpi->next;
	}
	if (tmpi && tmpi->inset)
		return tmpi->pos;
	else{
		/* think about footnotes */
		if (footnoteflag == LyXParagraph::NO_FOOTNOTE 
		    && next && next->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE) {
			int further =
				NextAfterFootnote()->GetPositionOfInset(inset);
			if (further != -1)
				return last + 1 + further;
		}
		return -1;
	}
}


void LyXParagraph::readSimpleWholeFile(FILE *myfile)
{
	char c;

	FileInfo fileInfo(fileno(myfile));
	long file_size = fileInfo.getSize();
	/* it is horrible, I know, but faster.
	 * I should not use InsertString for that :-( */
   
	/* I will write a better insertion in the future */ 
	Enlarge(0, file_size + 10);
   
	rewind(myfile);
   
	if (!feof(myfile)) {
		do {
			c = fgetc(myfile);
			InsertChar(last,c);
		} while (!feof(myfile));
      
	}
}


LyXParagraph* LyXParagraph::TeXOnePar(string &file, TexRow &texrow,
				      string &foot, TexRow &foot_texrow,
				      int &foot_count)
{
	lyxerr[Debug::LATEX] << "TeXOnePar...     " << this << endl;
	LyXParagraph *par = next;
	LyXLayout * style = lyxstyle.Style(GetCurrentTextClass(), layout);

	bool further_blank_line = false;
	if (IsDummy())
		lyxerr << "ERROR (LyXParagraph::TeXOnePar) is dummy." << endl;

	if (start_of_appendix) {
		file += "\\appendix\n";
		texrow.newline();
	}

	if (tex_code_break_column && style->isCommand()){
		file += '\n';
		texrow.newline();
	}

	if (pagebreak_top) {
		file += "\\newpage";
		further_blank_line = true;
	}
	if (added_space_top.kind() != VSpace::NONE) {
		file += added_space_top.asLatexCommand();
		further_blank_line = true;
	}
      
	if (line_top) {
		file += "\\lyxline{\\" + getFont(0).latexSize() + '}';
		file += "\\vspace{-1\\parskip}";
		further_blank_line = true;
	}

	if (further_blank_line){
		file += '\n';
		texrow.newline();
	}

	switch (style->latextype) {
	case LATEX_COMMAND:
		file += '\\';
		file += style->latexname;
		file += style->latexparam;
		break;
	case LATEX_ITEM_ENVIRONMENT:
	        if (bibkey) 
			bibkey->Latex(file, false);
		else
			file += "\\item ";
		break;
	case LATEX_LIST_ENVIRONMENT:
		file += "\\item ";
		break;
	default:
		break;
	}

	bool need_par = SimpleTeXOnePar(file, texrow);
 
	// Spit out footnotes
	while (par && par->footnoteflag != LyXParagraph::NO_FOOTNOTE
	       && par->footnoteflag != footnoteflag) {
		par = par->TeXFootnote(file, texrow,
				       foot, foot_texrow, foot_count);
	    	par->SimpleTeXOnePar(file, texrow);
	    	par = par->next;
	}

	// Make sure that \\par is done with the font of the last
	// character if this has another size as the default.
	// This is necessary because LaTeX (and LyX on the screen)
	// calculates the space between the baselines according
	// to this font. (Matthias)
	LyXFont font = getFont(Last()-1);
	if (need_par) {
		if (style->resfont.size() != font.size()) {
			file += '\\';
			file += font.latexSize();
			file += ' ';
		}
		file += "\\par}";
	} else if (lyxstyle.Style(GetCurrentTextClass(),
				  GetLayout())->isCommand()){
		if (style->resfont.size() != font.size()) {
			file += '\\';
			file += font.latexSize();
			file += ' ';
		}
		file += '}';
	} else if (style->resfont.size() != font.size()){
		file += "{\\" + font.latexSize() + " \\par}";
	}
	
	switch (style->latextype) {
	case LATEX_ITEM_ENVIRONMENT:
	case LATEX_LIST_ENVIRONMENT:
                if (par && (depth < par->depth)) {
                        file += '\n';
                        texrow.newline();
                }
		break;
	case LATEX_ENVIRONMENT:
		// if its the last paragraph of the current environment
		// skip it otherwise fall through
		if (par
		    && (par->layout != layout
			|| par->depth != depth
			|| par->pextra_type != pextra_type))
			break;
	default:
		if (!(footnoteflag != LyXParagraph::NO_FOOTNOTE
		      && footnotekind != LyXParagraph::FOOTNOTE
		      && footnotekind != LyXParagraph::MARGIN
		      && (table
			  || (par
			      && par->table)))) {
			// don't insert this if we would be adding it
			// before or after a table in a float.  This 
			// little trick is needed in order to allow
			// use of tables in \subfigures or \subtables.
			file += '\n';
			texrow.newline();
		}
	}
	
	further_blank_line = false;
	if (line_bottom) {
		file += "\\lyxline{\\" + getFont(Last()-1).latexSize() + '}';
		further_blank_line = true;
	}

	if (added_space_bottom.kind() != VSpace::NONE) {
		file += added_space_bottom.asLatexCommand();
		further_blank_line = true;
	}
      
	if (pagebreak_bottom) {
		file += "\\newpage";
		further_blank_line = true;
	}

	if (further_blank_line){
		file += '\n';
		texrow.newline();
	}

	if (!(footnoteflag != LyXParagraph::NO_FOOTNOTE && par &&
              par->footnoteflag == LyXParagraph::NO_FOOTNOTE) // &&
//            (pextra_type != PEXTRA_MINIPAGE ||
/*             (par && !par->pextra_type == PEXTRA_MINIPAGE)) */ ) {
		file += '\n';
		texrow.newline();
	}

	lyxerr[Debug::LATEX] << "TeXOnePar...done " << par << endl;
	return par;
}


// This one spits out the text of the paragraph
bool LyXParagraph::SimpleTeXOnePar(string &file, TexRow &texrow)
{
	lyxerr[Debug::LATEX] << "SimpleTeXOnePar...     " << this << endl;

	if (table)
		return SimpleTeXOneTablePar(file, texrow);

	char c;
	int main_body;
	int column;
	
	bool return_value = false;
	//bool underbar = false;

	LyXLayout * style = lyxstyle.Style(GetCurrentTextClass(), GetLayout());
	LyXFont basefont;

	/* maybe we have to create a optional argument */ 
	if (style->labeltype != LABEL_MANUAL)
		main_body = 0;
	else
		main_body = BeginningOfMainBody();

	if (main_body > 0) {
		file += '[';
		basefont = getFont(-2); // Get label font
	} else {
		basefont = getFont(-1); // Get layout font
	}

	column = 0;

	if (main_body >= 0
	    && !last
	    && !IsDummy()) {
		if (style->isCommand()) {
			file += '{';
			column++;
		} else if (align != LYX_ALIGN_LAYOUT) {
			file += '{';
			column++;
			return_value = true;
		}
	}
 
	// Which font is currently active?
	LyXFont running_font = basefont;
	// Do we have an open font change?
	bool open_font = false;

	texrow.start(this, 0);
 
	for (int i = 0; i < last; i++) {
		column++;
		// First char in paragraph or after label?
		if (i == main_body && !IsDummy()) {
			if (main_body > 0) {
				if (open_font) {
					column += running_font.latexWriteEndChanges(file, basefont);
					open_font = false;
				}
				basefont = getFont(-1); // Now use the layout font
				running_font = basefont;
				file += ']';
				column++;
			}
			if (style->isCommand()) {
				file += '{';
				column++;
			} else if (align != LYX_ALIGN_LAYOUT) {
				file += "{\\par";
				column += 4;
				return_value = true;
			}

			if (noindent) {
				file += "\\noindent ";
				column += 10;
			}
			switch (align) {
			case LYX_ALIGN_LEFT:
				file += "\\raggedright ";
				column+=13;
				break;
			case LYX_ALIGN_RIGHT:
				file += "\\raggedleft ";
				column+=12;
				break;
			case LYX_ALIGN_CENTER:
				file += "\\centering ";
				column+=11;
				break;
			}	 
		}

		c = GetChar(i);

		// Fully instantiated font
		LyXFont font = getFont(i);

		// Spaces at end of font change are simulated to be
		// outside font change, i.e. we write "\textXX{text} "
		// rather than "\textXX{text }". (Asger) 
		if (open_font && c == ' ' && i <= last-2 
		    && !getFont(i+1).equalExceptLatex(running_font) 
		    && !getFont(i+1).equalExceptLatex(font)) {
			font = getFont(i+1);
		}

		// We end font definition before blanks
		if (!font.equalExceptLatex(running_font) && open_font) {
			column += running_font.latexWriteEndChanges(file, basefont);
			running_font = basefont;
			open_font = false;
		}

		// Blanks are printed before start of fontswitch
		if (c == ' '){
			// Do not print the separation of the optional argument
			if (i != main_body - 1) {
				SimpleTeXBlanks(file, texrow, i, column, font, style);
			}
		}

		// Do we need to change font?
		if (!font.equalExceptLatex(running_font)
		    && i != main_body-1) {
			column += font.latexWriteStartChanges(file, basefont);
			running_font = font;
			open_font = true;
		}

		if (c == LYX_META_NEWLINE) {
			// newlines are handled differently here than
			// the default in SimpleTeXSpecialChars().
			if (!style->newline_allowed
			    || font.latex() == LyXFont::ON) {
				file += '\n';
			} else {
				if (open_font) {
					column += running_font.latexWriteEndChanges(file, basefont);
					open_font = false;
				}
				basefont = getFont(-1);
				running_font = basefont;
				if (font.family() == 
				    LyXFont::TYPEWRITER_FAMILY) {
					file += "~";
				}
				file += "\\\\\n";
			}
			texrow.newline();
			texrow.start(this, i+1);
			column = 0;
		} else {
			SimpleTeXSpecialChars(file, texrow,
					      font, running_font, basefont,
					      open_font, style, i, column, c);
		}
	}

	// If we have an open font definition, we have to close it
	if (open_font) {
		running_font.latexWriteEndChanges(file, basefont);
	}

	/* needed if there is an optional argument but no contents */ 
	if (main_body > 0 && main_body == last) {
		file += "]~";
		return_value = false;
	}

	lyxerr[Debug::LATEX] << "SimpleTeXOnePar...done " << this << endl;
	return return_value;
}


// This one spits out the text of a table paragraph
bool LyXParagraph::SimpleTeXOneTablePar(string &file, TexRow &texrow)
{
	lyxerr[Debug::LATEX] << "SimpleTeXOneTablePar...     " << this << endl;
	char c;
	int column, tmp;
   
	bool return_value = false;
	int current_cell_number = -1;

	LyXLayout * style = lyxstyle.Style(GetCurrentTextClass(), GetLayout());
	LyXFont basefont;

	basefont = getFont(-1); // Get layout font
	// Which font is currently active?
	LyXFont running_font = basefont;
	// Do we have an open font change?
	bool open_font = false;
 
	column = 0;
	if (!IsDummy()) { // it is dummy if it is in a float!!!
		if (style->isCommand()) {
			file += '{';
			column++;
		} else if (align != LYX_ALIGN_LAYOUT) {
			file += '{';
			column++;
			return_value = true;
		}
		if (noindent) {
			file += "\\noindent ";
			column += 10;
		}
		switch (align) {
		case LYX_ALIGN_LEFT:
			file += "\\raggedright ";
			column+=13;
			break;
		case LYX_ALIGN_RIGHT:
			file += "\\raggedleft ";
			column+=12;
			break;
		case LYX_ALIGN_CENTER:
			file += "\\centering ";
			column+=11;
			break;
		}
	}
	current_cell_number = -1;
	tmp = table->TexEndOfCell(file,current_cell_number);
	for (;tmp>0;tmp--)
		texrow.newline();
	
	texrow.start(this, 0);

	for (int i = 0; i < last; i++) {
		c = GetChar(i);
		if (table->IsContRow(current_cell_number+1)) {
			if (c == LYX_META_NEWLINE)
				current_cell_number++;
			continue;
		}
		column++;
		
		// Fully instantiated font
		LyXFont font = getFont(i);

		// Spaces at end of font change are simulated to be outside font change.
		// i.e. we write "\textXX{text} " rather than "\textXX{text }". (Asger)
		if (open_font && c == ' ' && i <= last-2 
		    && getFont(i+1) != running_font && getFont(i+1) != font) {
			font = getFont(i+1);
		}

		// We end font definition before blanks
		if (font != running_font && open_font) {
			column += running_font.latexWriteEndChanges(file, basefont);
			running_font = basefont;
			open_font = false;
		}
		// Blanks are printed before start of fontswitch
		if (c == ' '){
			SimpleTeXBlanks(file, texrow, i, column, font, style);
		}
		// Do we need to change font?
		if (font != running_font) {
			column += font.latexWriteStartChanges(file, basefont);
			running_font = font;
			open_font = true;
		}
		// Do we need to turn on LaTeX mode?
		if (font.latex() != running_font.latex()) {
			if (font.latex() == LyXFont::ON
			    && style->needprotect) {
				file += "\\protect ";
				column += 9;
			}
		}
		if (c == LYX_META_NEWLINE) {
			// special case for inside a table
			// different from default case in SimpleTeXSpecialChars()
			if (open_font) {
				column += running_font.latexWriteEndChanges(file, basefont);
				open_font = false;
			}
			basefont = getFont(-1);
			running_font = basefont;
			current_cell_number++;
			if (table->CellHasContRow(current_cell_number) >= 0) {
				TeXContTableRows(file, i+1, current_cell_number, column, texrow);
			}
			// if this cell follow only ContRows till end don't
			// put the EndOfCell because it is put after the
			// for(...)
			if (table->ShouldBeVeryLastCell(current_cell_number)) {
                            current_cell_number--;
                            break;
			}
			int tmp = table->TexEndOfCell(file, current_cell_number);
			if (tmp>0) {
				column = 0;
			} else if (tmp < 0) {
				tmp = -tmp;
			}
			for (;tmp--;) {
				texrow.newline();
			}
			texrow.start(this, i+1);
		} else {
			SimpleTeXSpecialChars(file, texrow,
					      font, running_font, basefont,
					      open_font, style, i, column, c);
		}
	}

	// If we have an open font definition, we have to close it
	if (open_font) {
		running_font.latexWriteEndChanges(file, basefont);
	}
	current_cell_number++;
	tmp = table->TexEndOfCell(file, current_cell_number);
	for (;tmp>0;tmp--)
		texrow.newline();
	lyxerr[Debug::LATEX] << "SimpleTeXOneTablePar...done " << this << endl;
	return return_value;
}


// This one spits out the text off ContRows in tables
bool LyXParagraph::TeXContTableRows(string &file, int i,
				    int current_cell_number,
				    int &column, TexRow &texrow)
{
	lyxerr[Debug::LATEX] << "TeXContTableRows...     " << this << endl;
	if (!table)
		return false;
    
	char c;
	int cell;
	int lastpos;
   
	bool return_value = false;
	LyXLayout * style = lyxstyle.Style(GetCurrentTextClass(), GetLayout());
	LyXFont basefont;

	basefont = getFont(-1); // Get layout font
	// Which font is currently active?
	LyXFont running_font = basefont;
	// Do we have an open font change?
	bool open_font = false;

	lastpos = i;
	cell = table->CellHasContRow(current_cell_number);
	current_cell_number++;
	while(cell >= 0) {
		// first find the right position
		i = lastpos;
		for (; (i<last) && (current_cell_number<cell); i++) {
			c = GetChar(i);
			if (c == LYX_META_NEWLINE)
				current_cell_number++;
		}
		lastpos = i;
		c=GetChar(i);
		if (table->Linebreaks(table->FirstVirtualCell(cell))) {
			file += " \\\\\n";
			texrow.newline();
			column = 0;
		} else if ((c != ' ') && (c != LYX_META_NEWLINE)) {
			file += ' ';
		}
		for (; (i < last) && ((c=GetChar(i)) != LYX_META_NEWLINE); i++) {
			column++;

			// Fully instantiated font
			LyXFont font = getFont(i);

			// Spaces at end of font change are simulated to be outside font change.
			// i.e. we write "\textXX{text} " rather than "\textXX{text }". (Asger)
			if (open_font && c == ' ' && i <= last-2 
			    && getFont(i+1) != running_font && getFont(i+1) != font) {
				font = getFont(i+1);
			}

			// We end font definition before blanks
			if (font != running_font && open_font) {
				column += running_font.latexWriteEndChanges(file, basefont);
				running_font = basefont;
				open_font = false;
			}
			// Blanks are printed before start of fontswitch
			if (c == ' '){
				SimpleTeXBlanks(file, texrow, i, column, font, style);
			}
			// Do we need to change font?
			if (font != running_font) {
				column += font.latexWriteStartChanges(file, basefont);
				running_font = font;
				open_font = true;
			}
			// Do we need to turn on LaTeX mode?
			if (font.latex() != running_font.latex()) {
				if (font.latex() == LyXFont::ON
				    && style->needprotect)
					{
						file += "\\protect ";
						column += 9;
					}
			}
			SimpleTeXSpecialChars(file, texrow, font, running_font, basefont,
					      open_font, style, i, column, c);
		}
		// If we have an open font definition, we have to close it
		if (open_font) {
			running_font.latexWriteEndChanges(file, basefont);
			open_font = false;
		}
		basefont = getFont(-1);
		running_font = basefont;
		cell = table->CellHasContRow(current_cell_number);
	}
	lyxerr[Debug::LATEX] << "TeXContTableRows...done " << this << endl;
	return return_value;
}


bool LyXParagraph::linuxDocConvertChar(char c, string &sgml_string)
{
	bool retval = false;
	switch (c) {
	case LYX_META_HFILL:
		sgml_string.clear();
		break;
	case LYX_META_PROTECTED_SEPARATOR: 
		sgml_string = ' ';
		break;
	case LYX_META_NEWLINE:
		sgml_string = '\n';
		break;
	case '&': 
		sgml_string = "&amp;";
		break;
	case '<': 
		sgml_string = "&lt;"; 
		break;
	case '>':
		sgml_string = "&gt;"; 
		break;
	case '$': 
		sgml_string = "&dollar;"; 
		break;
	case '#': 
		sgml_string = "&num;";
		break;
	case '%': 
		sgml_string = "&percnt;";
		break;
	case '[': 
		sgml_string = "&lsqb;";
		break;
	case ']': 
		sgml_string = "&rsqb;";
		break;
	case '{': 
		sgml_string = "&lcub;";
		break;
	case '}': 
		sgml_string = "&rcub;";
		break;
	case '~': 
		sgml_string = "&tilde;";
		break;
	case '"': 
		sgml_string = "&quot;";
		break;
	case '\\': 
		sgml_string = "&bsol;";
		break;
	case ' ':
		retval = true;
		sgml_string = ' ';
		break;
	case '\0': /* Ignore :-) */
		sgml_string.clear();
		break;
	default:
		sgml_string = c;
		break;
	}
	return retval;
}

void LyXParagraph::SimpleDocBookOneTablePar(string &file, string &extra,
					    int & desc_on, int depth) 
{
	if (!table)
		return;
	lyxerr[Debug::LATEX] << "SimpleDocbookOneTablePar... " << this << endl;
	int column, tmp;
	//bool return_value = false; // unused
	int current_cell_number = -1;
	LyXFont font1,font2;
	char c;
	Inset *inset;
	int  main_body; //, j; // unused
	string emph="emphasis";
	bool emph_flag=false;
	int char_line_count=0;
	
	LyXLayout * style = lyxstyle.Style(GetCurrentTextClass(), GetLayout());
	
	if (style->labeltype != LABEL_MANUAL)
		main_body = 0;
	else
		main_body = BeginningOfMainBody();
	
	/* gets paragraph main font */
	if (main_body > 0)
		font1 = style->labelfont;
	else
		font1 = style->font;
	
	char_line_count = depth;
	addNewlineAndDepth(file, depth);
	if (footnoteflag == LyXParagraph::NO_FOOTNOTE) {
		file += "<INFORMALTABLE>";
		addNewlineAndDepth(file, ++depth);
	}
	current_cell_number = -1;
	tmp = table->DocBookEndOfCell(file,current_cell_number, depth);
	
	/* parsing main loop */
	for (int i = 0; i < last; i++) {
		c = GetChar(i);
		if (table->IsContRow(current_cell_number+1)) {
			if (c == LYX_META_NEWLINE)
				current_cell_number++;
			continue;
		}
		column++;
		
		// Fully instantiated font
		font2 = getFont(i);
		
		/* handle <emphasis> tag */
		if (font1.emph() != font2.emph() && i) {
			if (font2.emph() == LyXFont::ON) {
				file += "<emphasis>";
				emph_flag=true;
			} else if (emph_flag) {
				file += "</emphasis>";
				emph_flag=false;
			}
		}
		if (c == LYX_META_NEWLINE) {
			// we have only to control for emphasis open here!
			if (emph_flag) {
				file += "</emphasis>";
				emph_flag=false;
			}
			font1 = font2 = getFont(-1);
			current_cell_number++;
			if (table->CellHasContRow(current_cell_number) >= 0) {
				DocBookContTableRows(file, extra, desc_on, i+1,
						     current_cell_number,
						     column);
			}
			// if this cell follow only ContRows till end don't
			// put the EndOfCell because it is put after the
			// for(...)
			if (table->ShouldBeVeryLastCell(current_cell_number)) {
				current_cell_number--;
				break;
			}
			tmp=table->DocBookEndOfCell(file, current_cell_number,
						    depth);
			
			if (tmp > 0)
				column = 0;
		} else if (c == LYX_META_INSET) {
			inset = GetInset(i);
			string tmp_out;
			inset->DocBook(tmp_out);
			//
			// This code needs some explanation:
			// Two insets are treated specially
			//   label if it is the first element in a command paragraph
			//         desc_on==3
			//   graphics inside tables or figure floats can't go on
				//   title (the equivalente in latex for this case is caption
			//   and title should come first
			//         desc_on==4
			//
			if(desc_on!=3 || i!=0) {
				if(tmp_out[0]=='@') {
					if(desc_on==4)
						extra += frontStrip(tmp_out, '@');
					else
						file += frontStrip(tmp_out, '@');
				} else
					file += tmp_out;
			}
		} else if (font2.latex() == LyXFont::ON) {
			// "TeX"-Mode on ==> SGML-Mode on.
			if (c!='\0')
				file += c;
			char_line_count++;
		} else {
			string sgml_string;
			if (linuxDocConvertChar(c, sgml_string) 
			    && !style->free_spacing) {
				// in freespacing mode, spaces are
				// non-breaking characters
				// char is ' '
				if (desc_on == 1) {
					char_line_count++;
					file += '\n';
					file += "</term><listitem><para>";
					desc_on = 2;
				} else  {
					file += c;
				}
			} else {
				file += sgml_string;
			}
		}
		font1 = font2;
	}
	
	/* needed if there is an optional argument but no contents */ 
	if (main_body > 0 && main_body == last) {
		font1 = style->font;
	}
	
	if (emph_flag) {
		file += "</emphasis>";
	}
	
	current_cell_number++;
	tmp = table->DocBookEndOfCell(file, current_cell_number, depth);
	/* resets description flag correctly */
	switch(desc_on){
	case 1:
		/* <term> not closed... */
		file += "</term>";
		break;
	}
	if (footnoteflag == LyXParagraph::NO_FOOTNOTE)
		file += "</INFORMALTABLE>";
	file += '\n';
	lyxerr[Debug::LATEX] << "SimpleDocbookOneTablePar...done "
			     << this << endl;
}

void LyXParagraph::DocBookContTableRows(string &file, string &extra,
                                        int & desc_on, int i,
                                        int current_cell_number, int &column) 
{
	if (!table)
		return;
	
	lyxerr[Debug::LATEX] << "DocBookContTableRows... " << this << endl;

	int cell, lastpos; //tmp; //unused
	LyXFont font1,font2;
	char c;
	Inset *inset;
	int main_body; // , j; // unused
	string emph="emphasis";
	bool emph_flag=false;
	int char_line_count=0;
	
	LyXLayout * style = lyxstyle.Style(GetCurrentTextClass(), GetLayout());
	
	if (style->labeltype != LABEL_MANUAL)
		main_body = 0;
	else
		main_body = BeginningOfMainBody();
	
	/* gets paragraph main font */
	if (main_body > 0)
		font1 = style->labelfont;
	else
		font1 = style->font;
	
	lastpos = i;
	cell = table->CellHasContRow(current_cell_number);
	current_cell_number++;
	while(cell >= 0) {
		// first find the right position
		i = lastpos;
		for (; (i<last) && (current_cell_number<cell); i++) {
			c = GetChar(i);
			if (c == LYX_META_NEWLINE)
				current_cell_number++;
		}
		lastpos = i;
		c=GetChar(i);
		// I don't know how to handle this so I comment it
                // for the moment (Jug)
//             if (table->Linebreaks(table->FirstVirtualCell(cell))) {
//                     file += " \\\\\n";
//                     column = 0;
//             } else
		if ((c != ' ') && (c != LYX_META_NEWLINE)) {
			file += ' ';
		}
		for (; (i < last) && ((c=GetChar(i)) != LYX_META_NEWLINE); i++) {
			column++;
			
			// Fully instantiated font
			font2 = getFont(i);
			
			/* handle <emphasis> tag */
			if (font1.emph() != font2.emph() && i) {
				if (font2.emph() == LyXFont::ON) {
					file += "<emphasis>";
					emph_flag=true;
				} else if (emph_flag) {
					file += "</emphasis>";
					emph_flag=false;
				}
			}
			if (c == LYX_META_INSET) {
				inset = GetInset(i);
				string tmp_out;
				inset->DocBook(tmp_out);
				//
				// This code needs some explanation:
				// Two insets are treated specially
				//   label if it is the first element in a command paragraph
				//       desc_on==3
				//   graphics inside tables or figure floats can't go on
				//   title (the equivalente in latex for this case is caption
				//   and title should come first
				//       desc_on==4
				//
				if(desc_on!=3 || i!=0) {
					if(tmp_out[0]=='@') {
						if(desc_on==4)
							extra += frontStrip(tmp_out, '@');
						else
							file += frontStrip(tmp_out, '@');
					} else
						file += tmp_out;
				}
			} else if (font2.latex() == LyXFont::ON) {
				// "TeX"-Mode on ==> SGML-Mode on.
				if (c!='\0')
					file += c;
				char_line_count++;
			} else {
				string sgml_string;
				if (linuxDocConvertChar(c, sgml_string) 
				    && !style->free_spacing) {
				// in freespacing mode, spaces are
				// non-breaking characters
				// char is ' '
					if (desc_on == 1) {
						char_line_count++;
						file += '\n';
						file += "</term><listitem><para>";
						desc_on = 2;
					} else  {
						file += c;
					}
				} else {
					file += sgml_string;
				}
			}
		}
		// we have only to control for emphasis open here!
		if (emph_flag) {
			file += "</emphasis>";
			emph_flag=false;
		}
		font1 = font2 = getFont(-1);
		cell = table->CellHasContRow(current_cell_number);
	}
	lyxerr[Debug::LATEX] << "DocBookContTableRows...done " << this << endl;
}


//
void LyXParagraph::SimpleTeXBlanks(string &file, TexRow &texrow,
				   int const i, int &column, LyXFont const &font,
				   LyXLayout const * const style)
{
	if (column > tex_code_break_column
	    && i 
	    && GetChar(i-1) != ' '
	    && (i < last-1)
	    // In LaTeX mode, we don't want to
	    // break lines since some commands
	    // do not like this
	    && ! (font.latex() == LyXFont::ON)
	    // same in FreeSpacing mode
	    && !style->free_spacing
	    // In typewriter mode, we want to avoid 
	    // ! . ? : at the end of a line
	    && !(font.family() == LyXFont::TYPEWRITER_FAMILY
		 && (GetChar(i-1) == '.'
		     || GetChar(i-1) == '?' 
		     || GetChar(i-1) == ':'
		     || GetChar(i-1) == '!'))) {
		if (tex_code_break_column == 0) {
			// in batchmode we need LaTeX to still
			// see it as a space not as an extra '\n'
			file += " %\n";
		} else {
			file += '\n';
		}
		texrow.newline();
		texrow.start(this, i+1);
		column = 0;
	} else if (font.latex() == LyXFont::OFF) {
		if (style->free_spacing) {
			file += '~';
		} else {
			file += ' ';
		}
	}
}


//
void LyXParagraph::SimpleTeXSpecialChars(string &file, TexRow &texrow,
					 LyXFont &font, LyXFont &running_font, LyXFont &basefont,
					 bool &open_font, LyXLayout const * const style,
					 int &i, int &column, char const c)
{
	Inset* inset;

	// Two major modes:  LaTeX or plain
	// Handle here those cases common to both modes
	// and then split to handle the two modes separately.
	switch (c) {
	case LYX_META_INSET:
		inset = GetInset(i);
		if (inset) {
			int len = file.length();
			int tmp = inset->Latex(file, style->isCommand());
			
			if (tmp) {
				column = 0;
			} else {
				column += file.length() - len;
			}
			for (;tmp--;) {
				texrow.newline();
			}
		}
		break;

	case LYX_META_NEWLINE:
		if (open_font) {
			column += running_font.latexWriteEndChanges(file, basefont);
			open_font = false;
		}
		basefont = getFont(-1);
		running_font = basefont;
		break;

	case LYX_META_HFILL: 
		file += "\\hfill{}";
		column += 7;
		break;

	default:
		// And now for the special cases within each mode
		// Are we in LaTeX mode?
		if (font.latex() == LyXFont::ON) {
			// at present we only have one option
			// but I'll leave it as a switch statement
			// so its simpler to extend. (ARRae)
			switch (c) {
			case LYX_META_PROTECTED_SEPARATOR: 
			        file += ' ';
			        break;

			default:
				// make sure that we will not print
				// error generating chars to the tex
				// file. This test would not be needed
				// if it were done in the buffer
				// itself.
				if (c != '\0') {
					file += c;
				}
				break;
			}
		} else {
			// Plain mode (i.e. not LaTeX)
			switch (c) {
			case LYX_META_PROTECTED_SEPARATOR: 
			        file += '~';
			        break;

			case '\\': 
				file += "\\textbackslash{}";
				column += 15;
				break;
		
			case '°': case '±': case '²': case '³':  
			case '×': case '÷': case '¹': case 'ª':
			case 'º': case '¬': case 'µ':
				if (current_view->currentBuffer()->params.inputenc == "latin1") {
					file += "\\ensuremath{";
					file += c;
					file += '}';
					column += 13;
				} else {
					file += c;
				}
				break;

			case '|': case '<': case '>':
				// In T1 encoding, these characters exist
				if (lyxrc->fontenc == "T1") {
					file += c;
					//... but we should avoid ligatures
					if ((c == '>' || c == '<')
					    && i <= last-2
					    && GetChar(i+1) == c){
						file += "\\textcompwordmark{}";
						column += 19;
					}
					break;
				}
				// Typewriter font also has them
				if (font.family() == LyXFont::TYPEWRITER_FAMILY) {
					file += c;
					break;
				} 
				// Otherwise, we use what LaTeX
				// provides us.
				switch(c) {
				case '<':
					file += "\\textless{}";
					column += 10;
					break;
				case '>':
					file += "\\textgreater{}";
					column += 13;
					break;
				case '|':
					file += "\\textbar{}";
					column += 9;
					break;
				}
				break;

			case '-': // "--" in Typewriter mode -> "-{}-"
				if (i <= last-2
				    && GetChar(i+1) == '-'
				    && font.family() == LyXFont::TYPEWRITER_FAMILY) {
					file += "-{}";
					column += 2;
				} else {
					file += '-';
				}
				break;

			case '\"': 
				file += "\\char`\\\"{}";
				column += 9;
				break;

			case '£':
				if (current_view->currentBuffer()->params.inputenc == "default") {
					file += "\\pounds{}";
					column += 8;
				} else {
					file += c;
				}
				break;

			case '$': case '&':
			case '%': case '#': case '{':
			case '}': case '_':
				file += '\\';
				file += c;
				column += 1;
				break;

			case '^': case '~':
				file += '\\';
				file += c;
				file += "{}";
				column += 3;
				break;

			case '*': case '[': case ']':
				// avoid being mistaken for optional arguments
				file += '{';
				file += c;
				file += '}';
				column += 2;
				break;

			case ' ':
				/* blanks are printed before font switching */
				// Sure? I am not! (try nice-latex)
				// I am sure it's correct. LyX might be smarter
				// in the future, but for now, nothing wrong is
				// written. (Asger)
				break;

			default:
				/* idea for labels --- begin*/
				/* check for LyX */
				if (c ==  'L'
				    && i <= last-3
				    && font.family() != LyXFont::TYPEWRITER_FAMILY
				    && GetChar(i+1) == 'y'
				    && GetChar(i+2) == 'X') {
					file += "\\LyX{}";
					i += 2;
					column += 5;
				}
				/* check for TeX */ 
				else if (c == 'T'
					 && i <= last-3
					 && font.family() != LyXFont::TYPEWRITER_FAMILY
					 && GetChar(i+1) == 'e'
					 && GetChar(i+2) == 'X') {
					file += "\\TeX{}";
					i += 2;
					column += 5;
				}
				/* check for LaTeX2e */ 
				else if (c == 'L'
					 && i <= last-7
					 && font.family() != LyXFont::TYPEWRITER_FAMILY
					 && GetChar(i+1) == 'a'
					 && GetChar(i+2) == 'T'
					 && GetChar(i+3) == 'e'
					 && GetChar(i+4) == 'X'
					 && GetChar(i+5) == '2'
					 && GetChar(i+6) == 'e') {
					file += "\\LaTeXe{}";
					i += 6;
					column += 8;
				}
				/* check for LaTeX */ 
				else if (c == 'L'
					 && i <= last-5
					 && font.family() != LyXFont::TYPEWRITER_FAMILY
					 && GetChar(i+1) == 'a'
					 && GetChar(i+2) == 'T'
					 && GetChar(i+3) == 'e'
					 && GetChar(i+4) == 'X') {
					file += "\\LaTeX{}";
					i += 4;
					column += 7;
					/* idea for labels --- end*/ 
				} else if (c != '\0') {
					file += c;
				}
				break;
			}
		}
	}
}


bool LyXParagraph::RoffContTableRows(FILE *fp, int i, int actcell)
{
	if (!table)
		return false;

	LyXFont
		font1 = LyXFont(LyXFont::ALL_INHERIT),
		font2;
	Inset
		*inset;
	int
		lastpos, cell;
	char
		c;
	string
		fname2;
	FILE
		*fp2;

	fname2 = TmpFileName(string(),"RAT2");
	lastpos = i;
	cell = table->CellHasContRow(actcell);
	actcell++;
	while(cell >= 0) {
		// first find the right position
		i = lastpos;
		for (; (i<last) && (actcell<cell); i++) {
			c = GetChar(i);
			if (c == LYX_META_NEWLINE)
				actcell++;
		}
		lastpos = i;
		c=GetChar(i);
		if ((c != ' ') && (c != LYX_META_NEWLINE))
			fprintf(fp," ");
		for (; (i < last) && ((c=GetChar(i)) != LYX_META_NEWLINE); i++) {
			font2 = GetFontSettings(i);
			if (font1.latex() != font2.latex()) {
				if (font2.latex() != LyXFont::OFF)
					continue;
			}
			c = GetChar(i);
			switch (c) {
			case LYX_META_INSET:
				if ((inset = GetInset(i))) {
					if (!(fp2=fopen(fname2.c_str(),"w+"))) {
						WriteAlert(_("LYX_ERROR:"), _("Cannot open temporary file:"), fname2);
						return false;
					}
					inset->Latex(fp2,-1);
					rewind(fp2);
					c = fgetc(fp2);
					while(!feof(fp2)) {
						if (c == '\\')
							fprintf(fp,"\\\\");
						else
							fputc(c,fp);
						c = fgetc(fp2);
					}
					fclose(fp2);
				}
				break;
			case LYX_META_NEWLINE:
				break;
			case LYX_META_HFILL: 
				break;
			case LYX_META_PROTECTED_SEPARATOR:
				break;
			case '\\': 
				fprintf(fp, "\\\\");
				break;
			default:
				if (c != '\0')
					fprintf(fp, "%c", c);
				else
					lyxerr.debug() << "RoffAsciiTable: NULL char in structure." << endl;
				break;
			}
		}
		cell = table->CellHasContRow(actcell);
	}
	return true;
}

LyXParagraph * LyXParagraph::TeXDeeper(string &file, TexRow &texrow,
				       string &foot, TexRow &foot_texrow,
				       int &foot_count)
{
	lyxerr[Debug::LATEX] << "TeXDeeper...     " << this << endl;
	LyXParagraph *par = this;

	while (par && par->depth == depth) {
		if (par->IsDummy())
			lyxerr << "ERROR (LyXParagraph::TeXDeeper)" << endl;
		if (lyxstyle.Style(GetCurrentTextClass(), 
				   par->layout)->isEnvironment()
		    || par->pextra_type != PEXTRA_NONE) 
			{
				par = par->TeXEnvironment(file, texrow,
							  foot, foot_texrow,
							  foot_count);
			} else {
				par = par->TeXOnePar(file, texrow,
						     foot, foot_texrow,
						     foot_count);
			}
	}
	lyxerr[Debug::LATEX] << "TeXDeeper...done " << par << endl;

	return par;
}


LyXParagraph* LyXParagraph::TeXEnvironment(string &file, TexRow &texrow,
					   string &foot, TexRow &foot_texrow,
					   int &foot_count)
{
	bool
		eindent_open = false;
	bool
		foot_this_level = false;
		// flags when footnotetext should be appended to file.
        static bool
		minipage_open = false;
        static int
                minipage_open_depth = 0;
	char
		par_sep = current_view->currentBuffer()->params.paragraph_separation;
    
	lyxerr[Debug::LATEX] << "TeXEnvironment...     " << this << endl;
	if (IsDummy())
		lyxerr << "ERROR (LyXParagraph::TeXEnvironment)" << endl;

	LyXLayout * style = lyxstyle.Style(GetCurrentTextClass(), layout);
       
	if (pextra_type == PEXTRA_INDENT) {
		if (!pextra_width.empty()) {
			file += "\\begin{LyXParagraphIndent}{"
				+ pextra_width + "}\n";
		} else {
			//float ib = atof(pextra_widthp.c_str())/100;
			// string can't handle floats at present (971109)
			// so I'll do a conversion by hand knowing that
			// the limits are 0.0 to 1.0. ARRae.
			file += "\\begin{LyXParagraphIndent}{";
			switch (pextra_widthp.length()) {
			case 3:
				file += "1.00";
				break;
			case 2:
				file += "0.";
				file += pextra_widthp;
				break;
			case 1:
				file += "0.0";
				file += pextra_widthp;
			}
			file += "\\columnwidth}\n";
		}
		texrow.newline();
		eindent_open = true;
	}
	if ((pextra_type == PEXTRA_MINIPAGE) && !minipage_open) {
		if (pextra_hfill && Previous() &&
		    (Previous()->pextra_type == PEXTRA_MINIPAGE)) {
			file += "\\hfill{}\n";
			texrow.newline();
		}
		if (par_sep == LYX_PARSEP_INDENT) {
			file += "{\\setlength\\parindent{0pt}\n";
			texrow.newline();
		}
		file += "\\begin{minipage}";
		switch(pextra_alignment) {
		case MINIPAGE_ALIGN_TOP:
			file += "[t]";
			break;
		case MINIPAGE_ALIGN_MIDDLE:
			file += "[m]";
			break;
		case MINIPAGE_ALIGN_BOTTOM:
			file += "[b]";
			break;
		}
		if (!pextra_width.empty()) {
			file += '{';
			file += pextra_width + "}\n";
		} else {
			//float ib = atof(par->pextra_width.c_str())/100;
			// string can't handle floats at present
			// so I'll do a conversion by hand knowing that
			// the limits are 0.0 to 1.0. ARRae.
			file += '{';
			switch (pextra_widthp.length()) {
			case 3:
				file += "1.00";
				break;
			case 2:
				file += "0.";
				file += pextra_widthp;
				break;
			case 1:
				file += "0.0";
				file += pextra_widthp;
			}
			file += "\\columnwidth}\n";
		}
		texrow.newline();
		if (par_sep == LYX_PARSEP_INDENT) {
			file += "\\setlength\\parindent{\\LyXMinipageIndent}\n";
			texrow.newline();
		}
		minipage_open = true;
                minipage_open_depth = depth;
	}

#ifdef WITH_WARNINGS
#warning Define FANCY_FOOTNOTE_CODE to re-enable Allan footnote code
	//I disabled it because it breaks when lists span on several
	//pages (JMarc)
#endif
	if (style->isEnvironment()){
		if (style->latextype == LATEX_LIST_ENVIRONMENT) {
#ifdef FANCY_FOOTNOTE_CODE
			if (foot_count < 0) {
				// flag that footnote[mark][text] should be
				// used for any footnotes from now on
				foot_count = 0;
				foot_this_level = true;
			}
#endif
			file += "\\begin{" + style->latexname + "}{"
				+ labelwidthstring + "}\n";
		} else if (style->labeltype == LABEL_BIBLIO) {
			// ale970405
			file += "\\begin{" + style->latexname + "}{"
				+ bibitemWidthest() + "}\n";
		} else if (style->latextype == LATEX_ITEM_ENVIRONMENT) {
#ifdef FANCY_FOOTNOTE_CODE
			if (foot_count < 0) {
				// flag that footnote[mark][text] should be
				// used for any footnotes from now on
				foot_count = 0;
				foot_this_level = true;
			}
#endif
			file += "\\begin{" + style->latexname + '}'
				+ style->latexparam + '\n';
		} else 
			file += "\\begin{" + style->latexname + '}'
				+ style->latexparam + '\n';
		texrow.newline();
	}
	LyXParagraph *par = this;
	do {
		par = par->TeXOnePar(file, texrow,
				     foot, foot_texrow, foot_count);
#if 0
		if (eindent_open && par && par->pextra_type != PEXTRA_INDENT) {
			file += "\\end{LyXParagraphIndent}\n";
			texrow.newline();
			eindent_open = false;
			if (!style->isEnvironment())
				break;
		}
#endif
                if (minipage_open && par && !style->isEnvironment() &&
                    (par->pextra_type == PEXTRA_MINIPAGE) &&
                    par->pextra_start_minipage) {
                    file += "\\end{minipage}\n";
                    texrow.newline();
                    if (par_sep == LYX_PARSEP_INDENT) {
                        file += "}\n";
			texrow.newline();
                    }
                    minipage_open = false;
                }
		if (par && par->depth > depth) {
			if (lyxstyle.Style(GetCurrentTextClass(),
					   par->layout)->isParagraph()
			    && !par->table
			    && !suffixIs(file, "\n\n")) {
			    // There should be at least one '\n' already
			    // but we need there to be two for Standard 
			    // paragraphs that are depth-increment'ed to be
			    // output correctly.  However, tables can also be
			    // paragraphs so don't adjust them.  ARRae
				file += '\n';
				texrow.newline();
			}
			par = par->TeXDeeper(file, texrow,
					     foot, foot_texrow, foot_count);
		}
		if (par && par->layout == layout && par->depth == depth &&
		    (par->pextra_type == PEXTRA_MINIPAGE) && !minipage_open) {
			if (par->pextra_hfill && par->Previous() &&
			    (par->Previous()->pextra_type == PEXTRA_MINIPAGE)){
				file += "\\hfill{}\n";
                                texrow.newline();
                        }
//			else
//				file += '\n';
			if (par_sep == LYX_PARSEP_INDENT) {
				file += "{\\setlength\\parindent{0pt}\n";
				texrow.newline();
			}
			file += "\\begin{minipage}";
			switch(par->pextra_alignment) {
			case MINIPAGE_ALIGN_TOP:
				file += "[t]";
				break;
			case MINIPAGE_ALIGN_MIDDLE:
				file += "[m]";
				break;
			case MINIPAGE_ALIGN_BOTTOM:
				file += "[b]";
				break;
			}
			if (!par->pextra_width.empty()) {
				file += '{';
				file += par->pextra_width;
				file += "}\n";
			} else {
				//float ib = atof(par->pextra_widthp.c_str())/100;
				// string can't handle floats at present
				// so I'll do a conversion by hand knowing that
				// the limits are 0.0 to 1.0. ARRae.
				file += '{';
				switch (par->pextra_widthp.length()) {
				case 3:
					file += "1.00";
					break;
				case 2:
					file += "0.";
					file += par->pextra_widthp;
					break;
				case 1:
					file += "0.0";
					file += par->pextra_widthp;
				}
				file += "\\columnwidth}\n";
			}
			texrow.newline();
			if (par_sep == LYX_PARSEP_INDENT) {
				file += "\\setlength\\parindent{\\LyXMinipageIndent}\n";
				texrow.newline();
			}
			minipage_open = true;
                        minipage_open_depth = par->depth;
		}
	} while (par
		 && par->layout == layout
		 && par->depth == depth
		 && par->pextra_type == pextra_type);
 
	if (style->isEnvironment()) {
		file += "\\end{" + style->latexname + '}';
		// maybe this should go after the minipage closes?
		if (foot_this_level) {
			if (foot_count >= 1) {
				if (foot_count > 1) {
					file += "\\addtocounter{footnote}{-";
					file += tostr(foot_count - 1);
					file += '}';
				}
				file += foot;
				texrow += foot_texrow;
				foot.clear();
				foot_texrow.reset();
				foot_count = 0;
			}
		}
	}
        if (minipage_open && (minipage_open_depth == depth) &&
            (!par || par->pextra_start_minipage ||
             par->pextra_type != PEXTRA_MINIPAGE)) {
                file += "\\end{minipage}\n";
		texrow.newline();
                if (par_sep == LYX_PARSEP_INDENT) {
                        file += "}\n";
			texrow.newline();
                }
                if (par && par->pextra_type != PEXTRA_MINIPAGE) {
                        file += "\\medskip\n\n";
			texrow.newline();
			texrow.newline();
                }
                minipage_open = false;
        }
	if (eindent_open) {
		file += "\\end{LyXParagraphIndent}\n";
		texrow.newline();
	}
//	if (tex_code_break_column){
        if (!(par && (par->pextra_type==PEXTRA_MINIPAGE) 
	      && par->pextra_hfill)) {
                file += '\n';
		texrow.newline();
	}
	lyxerr[Debug::LATEX] << "TeXEnvironment...done " << par << endl;
	return par;  // ale970302
}


LyXParagraph * LyXParagraph::TeXFootnote(string &file, TexRow &texrow,
					 string &foot, TexRow &foot_texrow,
					 int &foot_count)
{
	lyxerr[Debug::LATEX] << "TeXFootnote...  " << this << endl;
	if (footnoteflag == LyXParagraph::NO_FOOTNOTE)
		lyxerr << "ERROR (LyXParagraph::TeXFootnote): "
			"No footnote!" << endl;

	LyXParagraph *par = this;
	LyXLayout * style = lyxstyle.Style(GetCurrentTextClass(), 
					   previous->GetLayout());
	
	if (style->needprotect && footnotekind != LyXParagraph::FOOTNOTE){
		lyxerr << "ERROR (LyXParagraph::TeXFootnote): "
			"Float other than footnote in command"
			" with moving argument is illegal" << endl;
	}

	if (footnotekind != LyXParagraph::FOOTNOTE
	    && footnotekind != LyXParagraph::MARGIN
	    && file.length()
	    && !suffixIs(file, '\n')) {
		// we need to ensure that real floats like tables and figures
		// have their \begin{} on a new line otherwise we can get
		// incorrect results when using the endfloat.sty package
		// especially if two floats follow one another.  ARRae 981022
		// NOTE: if the file is length 0 it must have just been
		//       written out so we assume it ended with a '\n'
		file += '\n';
		texrow.newline();
	}
	
	BufferParams *params = &current_view->currentBuffer()->params;
	bool footer_in_body = true;
	switch (footnotekind) {
	case LyXParagraph::FOOTNOTE:
		if (style->intitle) {
			file += "\\thanks{\n";
			footer_in_body = false;
		} else {
			if (foot_count == -1) {
				// we're at depth 0 so we can use:
				file += "\\footnote{%\n";
				footer_in_body = false;
			} else {
				file += "\\footnotemark{}%\n";
				if (foot_count) {
					// we only need this when there are
					// multiple footnotes
					foot += "\\stepcounter{footnote}";
				}
				foot += "\\footnotetext{%\n";
				foot_texrow.start(this,0);
				foot_texrow.newline();
				++foot_count;
			}
		}
		break;
	case LyXParagraph::MARGIN:
		file += "\\marginpar{\n";
		break;
	case LyXParagraph::FIG:
		if (pextra_type == PEXTRA_FLOATFLT
		    && (!pextra_width.empty()
			|| !pextra_widthp.empty())) {
			char bufr[80];
			if (!pextra_width.empty())
				sprintf(bufr, "\\begin{floatingfigure}{%s}\n",
					pextra_width.c_str());
			else
				sprintf(bufr, "\\begin{floatingfigure}{%f\\textwidth}\n",
					atoi(pextra_widthp.c_str())/100.0);
			file += bufr;
		} else {
			file += "\\begin{figure}";
			if (!params->float_placement.empty()) { 
				file += '[';
				file += params->float_placement;
				file += "]\n";
			} else {
				file += '\n';
			}
		}
		break;
	case LyXParagraph::TAB:
		file += "\\begin{table}";
		if (!params->float_placement.empty()) { 
			file += '[';
			file += params->float_placement;
			file += "]\n";
		} else {
			file += '\n';
		}
		break;
	case LyXParagraph::WIDE_FIG:
		file += "\\begin{figure*}";
		if (!params->float_placement.empty()) { 
			file += '[';
			file += params->float_placement;
			file += "]\n";
		} else {
			file += '\n';
		}
		break;
	case LyXParagraph::WIDE_TAB:
		file += "\\begin{table*}";
		if (!params->float_placement.empty()) { 
			file += '[';
			file += params->float_placement;
			file += "]\n";
		} else {
			file += '\n';
		}
		break;
	case LyXParagraph::ALGORITHM:
		file += "\\begin{algorithm}\n";
		break;
	}
	texrow.newline();
   
	if (footnotekind != LyXParagraph::FOOTNOTE
	    || !footer_in_body) {
		// Process text for all floats except footnotes in body
		do {
			LyXLayout *style = lyxstyle.Style(GetCurrentTextClass(),
							  par->layout);
			if (par->IsDummy())
				lyxerr << "ERROR (LyXParagraph::TeXFootnote)"
				       << endl;
			if (style->isEnvironment()
			    || par->pextra_type == PEXTRA_MINIPAGE) { /* && !minipage_open ?? */
				// Allows the use of minipages within float environments.
				// Shouldn't be circular because we don't support
				// footnotes inside floats (yet). ARRae
				par = par->TeXEnvironment(file, texrow,
							  foot, foot_texrow,
							  foot_count);
			} else {
				par = par->TeXOnePar(file, texrow,
						     foot, foot_texrow,
						     foot_count);
			}
			
			if (par && !par->IsDummy() && par->depth > depth) {
				par = par->TeXDeeper(file, texrow,
						     foot, foot_texrow,
						     foot_count);
			}
		} while (par && par->footnoteflag != LyXParagraph::NO_FOOTNOTE);
	} else {
		// process footnotes > depth 0 or in environments separately
		// NOTE: Currently don't support footnotes within footnotes
		//       even though that is possible using the \footnotemark
		string dummy;
		TexRow dummy_texrow;
		int dummy_count = 0;
		do {
			LyXLayout *style = lyxstyle.Style(GetCurrentTextClass(),
							  par->layout);
			if (par->IsDummy())
				lyxerr << "ERROR (LyXParagraph::TeXFootnote)"
				       << endl;
			if (style->isEnvironment()
			    || par->pextra_type == PEXTRA_MINIPAGE) { /* && !minipage_open ?? */
				// Allows the use of minipages within float environments.
				// Shouldn't be circular because we don't support
				// footnotes inside floats (yet). ARRae
				par = par->TeXEnvironment(foot, foot_texrow,
							  dummy, dummy_texrow,
							  dummy_count);
			} else {
				par = par->TeXOnePar(foot, foot_texrow,
						     dummy, dummy_texrow,
						     dummy_count);
			}

			if (par && !par->IsDummy() && par->depth > depth) {
				par = par->TeXDeeper(foot, foot_texrow,
						     dummy, dummy_texrow,
						     dummy_count);
			}
		} while (par && par->footnoteflag != LyXParagraph::NO_FOOTNOTE);
		if (dummy_count) {
			lyxerr << "ERROR (LyXParagraph::TeXFootnote): "
				"Footnote in a Footnote -- not supported"
			       << endl;
		}
	}

	switch (footnotekind) {
	case LyXParagraph::FOOTNOTE:
		if (footer_in_body) {
			// This helps tell which of the multiple
			// footnotetexts an error was in.
			foot += "}%\n";
			foot_texrow.newline();
		} else {
			file += '}';
		}
		break;
	case LyXParagraph::MARGIN:
		file += '}';
		break;
	case LyXParagraph::FIG:
		if (pextra_type == PEXTRA_FLOATFLT
		    && (!pextra_width.empty()
			|| !pextra_widthp.empty()))
			file += "\\end{floatingfigure}";
		else
			file += "\\end{figure}";
		break;
	case LyXParagraph::TAB:
		file += "\\end{table}";
		break;
	case LyXParagraph::WIDE_FIG:
		file += "\\end{figure*}";
		break;
	case LyXParagraph::WIDE_TAB:
		file += "\\end{table*}";
		break;
	case LyXParagraph::ALGORITHM:
		file += "\\end{algorithm}";
		break;
	}

	if (footnotekind != LyXParagraph::FOOTNOTE
	    && footnotekind != LyXParagraph::MARGIN) {
		// we need to ensure that real floats like tables and figures
		// have their \end{} on a line of their own otherwise we can
		// get incorrect results when using the endfloat.sty package.
		file += "\n";
		texrow.newline();
	}

	lyxerr[Debug::LATEX] << "TeXFootnote...done " << par->next << endl;
	return par;
}


void LyXParagraph::SetPExtraType(int type, const char *width,const char *widthp)
{
    pextra_type = type;
    pextra_width = width;
    pextra_widthp = widthp;

    if (lyxstyle.Style(GetCurrentTextClass(), 
                       layout)->isEnvironment()) {
        LyXParagraph
            *par = this,
            *ppar = par;

        while (par && (par->layout == layout) && (par->depth == depth)) {
            ppar = par;
            par = par->Previous();
            if (par)
                par = par->FirstPhysicalPar();
            while (par && par->depth > depth) {
                par = par->Previous();
                if (par)
                    par = par->FirstPhysicalPar();
            }
        }
        par = ppar;
        while (par && (par->layout == layout) && (par->depth == depth)) {
            par->pextra_type = type;
            par->pextra_width = width;
            par->pextra_widthp = widthp;
            par = par->NextAfterFootnote();
            if (par && (par->depth > depth))
                par->SetPExtraType(type,width,widthp);
            while (par && ((par->depth > depth) || par->IsDummy()))
                par = par->NextAfterFootnote();
        }
    }
}

void LyXParagraph::UnsetPExtraType()
{
    if (pextra_type == PEXTRA_NONE)
        return;
    
    pextra_type = PEXTRA_NONE;
    pextra_width.clear();
    pextra_widthp.clear();

    if (lyxstyle.Style(GetCurrentTextClass(), 
                       layout)->isEnvironment()) {
        LyXParagraph
            *par = this,
            *ppar = par;

        while (par && (par->layout == layout) && (par->depth == depth)) {
            ppar = par;
            par = par->Previous();
            if (par)
                par = par->FirstPhysicalPar();
            while (par && par->depth > depth) {
                par = par->Previous();
                if (par)
                    par = par->FirstPhysicalPar();
            }
        }
        par = ppar;
        while (par && (par->layout == layout) && (par->depth == depth)) {
            par->pextra_type = PEXTRA_NONE;
            par->pextra_width.clear();
            par->pextra_widthp.clear();
            par = par->NextAfterFootnote();
            if (par && (par->depth > depth))
                par->UnsetPExtraType();
            while (par && ((par->depth > depth) || par->IsDummy()))
                par = par->NextAfterFootnote();
        }
    }
}
