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

#ifdef __GNUG__
#pragma implementation "lyxparagraph.h"
#endif

#include <algorithm>
#include <fstream>

#include "lyxparagraph.h"
#include "support/textutils.h"
#include "lyxrc.h"
#include "layout.h"
#include "tex-strings.h"
#include "bufferparams.h"
#include "support/FileInfo.h"
#include "support/LAssert.h"
#include "debug.h"
#include "LaTeXFeatures.h"
#include "insets/insetinclude.h"
#include "insets/insetbib.h"
#include "support/filetools.h"
#include "lyx_gui_misc.h"
#include "texrow.h"
#include "support/lyxmanip.h"

using std::ostream;
using std::endl;
using std::fstream;
using std::ios;

int tex_code_break_column = 72;  // needs non-zero initialization. set later.
// this is a bad idea, but how can LyXParagraph find its buffer to get
// parameters? (JMarc)
extern BufferView * current_view;


// ale970405
extern string bibitemWidthest(Painter &);

// this is a minibuffer
static char minibuffer_char;
static LyXFont minibuffer_font;
static Inset * minibuffer_inset;


// Initialization of the counter for the paragraph id's,
// declared in lyxparagraph.h
unsigned int LyXParagraph::paragraph_id = 0;


LyXParagraph::LyXParagraph()
{
	text.reserve(500); // is this number too big?

	for (int i = 0; i < 10; ++i) setCounter(i , 0);
	appendix = false;
	enumdepth = 0;
	itemdepth = 0;
	next = 0;
	previous = 0;
	footnoteflag = LyXParagraph::NO_FOOTNOTE;
	footnotekind = LyXParagraph::FOOTNOTE; // should not be needed
	
	align = LYX_ALIGN_BLOCK;

	/* table stuff -- begin*/ 
	table = 0;
	/* table stuff -- end*/ 
	id_ = paragraph_id++;
        bibkey = 0; // ale970302
	Clear();
}


// This konstruktor inserts the new paragraph in a list.
LyXParagraph::LyXParagraph(LyXParagraph * par)
{
	text.reserve(500);
	par->text.resize(par->text.size());

	for (int i = 0; i < 10; ++i) setCounter(i, 0);
	appendix = false;
	enumdepth = 0;
	itemdepth = 0;
	// double linked list begin
	next = par->next;
	if (next)
		next->previous = this;
	previous = par;
	previous->next = this;
	// end
	footnoteflag = LyXParagraph::NO_FOOTNOTE;
	footnotekind = LyXParagraph::FOOTNOTE;
	
	/* table stuff -- begin*/ 
	table = 0;
	/* table stuff -- end*/ 
	id_ = paragraph_id++;

        bibkey = 0; // ale970302        
    
	Clear();
}


void LyXParagraph::writeFile(ostream & os, BufferParams const & params,
			     char footflag, char dth) const
{
	LyXFont font1, font2;
	int column = 0;
	int h = 0;
	char c = 0;

	if (footnoteflag != LyXParagraph::NO_FOOTNOTE
	    || !previous
	    || previous->footnoteflag == LyXParagraph::NO_FOOTNOTE){
		
		// The beginning or the end of a footnote environment?
		if (footflag != footnoteflag) {
			footflag = footnoteflag;
			if (footflag) {
				os << "\n\\begin_float "
				   << string_footnotekinds[footnotekind]
				   << " ";
			} else {
				os << "\n\\end_float ";
			}
		}

		// The beginning or end of a deeper (i.e. nested) area?
		if (dth != depth) {
			if (depth > dth) {
				while (depth > dth) {
					os << "\n\\begin_deeper ";
					++dth;
				}
			} else {
				while (depth < dth) {
					os << "\n\\end_deeper ";
					--dth;
				}
			}
		}

		// First write the layout
		os << "\n\\layout "
		   << textclasslist.NameOfLayout(params.textclass, layout)
		   << "\n";

		// Maybe some vertical spaces.
		if (added_space_top.kind() != VSpace::NONE)
			os << "\\added_space_top "
			   << added_space_top.asLyXCommand() << " ";
		if (added_space_bottom.kind() != VSpace::NONE)
			os << "\\added_space_bottom "
			   << added_space_bottom.asLyXCommand() << " ";
			
		// The labelwidth string used in lists.
		if (!labelwidthstring.empty())
			os << "\\labelwidthstring "
			   << labelwidthstring << '\n';

		// Lines above or below?
		if (line_top)
			os << "\\line_top ";
		if (line_bottom)
			os << "\\line_bottom ";

		// Pagebreaks above or below?
		if (pagebreak_top)
			os << "\\pagebreak_top ";
		if (pagebreak_bottom)
			os << "\\pagebreak_bottom ";
			
		// Start of appendix?
		if (start_of_appendix)
			os << "\\start_of_appendix ";

		// Noindent?
		if (noindent)
			os << "\\noindent ";
			
		// Alignment?
		if (align != LYX_ALIGN_LAYOUT) {
			switch (align) {
			case LYX_ALIGN_LEFT: h = 1; break;
			case LYX_ALIGN_RIGHT: h = 2; break;
			case LYX_ALIGN_CENTER: h = 3; break;
			default: h = 0; break;
			}
			os << "\\align " << string_align[h] << " ";
		}
                if (pextra_type != PEXTRA_NONE) {
                        os << "\\pextra_type " << pextra_type;
                        if (pextra_type == PEXTRA_MINIPAGE) {
				os << " \\pextra_alignment "
				   << pextra_alignment;
				if (pextra_hfill)
					os << " \\pextra_hfill "
					   << pextra_hfill;
				if (pextra_start_minipage)
					os << " \\pextra_start_minipage "
					   << pextra_start_minipage;
                        }
                        if (!pextra_width.empty()) {
				os << " \\pextra_width "
				   << VSpace(pextra_width).asLyXCommand();
                        } else if (!pextra_widthp.empty()) {
				os << " \\pextra_widthp "
				   << pextra_widthp;
                        }
			os << '\n';
                }
	} else {
   		// Dummy layout. This means that a footnote ended.
		os << "\n\\end_float ";
		footflag = LyXParagraph::NO_FOOTNOTE;
	}
		
	// It might be a table.
	if (table){
		os << "\\LyXTable\n";
		table->Write(os);
	}

	// bibitem  ale970302
	if (bibkey)
		bibkey->Write(os);

	font1 = LyXFont(LyXFont::ALL_INHERIT,params.language_info);

	column = 0;
	for (size_type i = 0; i < size(); ++i) {
		if (!i) {
			os << "\n";
			column = 0;
		}
		
		// Write font changes
		font2 = GetFontSettings(i);
		if (font2 != font1) {
			font2.lyxWriteChanges(font1, os);
			column = 0;
			font1 = font2;
		}

		c = GetChar(i);
		switch (c) {
		case META_INSET:
		{
			Inset const * inset = GetInset(i);
			if (inset)
				if (inset->DirectWrite()) {
					// international char, let it write
					// code directly so it's shorter in
					// the file
					inset->Write(os);
				} else {
					os << "\n\\begin_inset ";
					inset->Write(os);
					os << "\n\\end_inset \n\n";
					column = 0;
				}
		}
		break;
		case META_NEWLINE: 
			os << "\n\\newline \n";
			column = 0;
			break;
		case META_HFILL: 
			os << "\n\\hfill \n";
			column = 0;
			break;
#if 0
		case META_PROTECTED_SEPARATOR: 
			os << "\n\\protected_separator \n";
			column = 0;
			break;
#endif
		case '\\':
			os << "\n\\backslash \n";
			column = 0;
			break;
		case '.':
			if (i + 1 < size() && GetChar(i + 1) == ' ') {
				os << ".\n";
				column = 0;
			} else
				os << ".";
			break;
		default:
			if ((column > 70 && c == ' ')
			    || column > 79) {
				os << "\n";
				column = 0;
			}
			// this check is to amend a bug. LyX sometimes
			// inserts '\0' this could cause problems.
			if (c != '\0')
				os << c;
			else
				lyxerr << "ERROR (LyXParagraph::writeFile):"
					" NULL char in structure." << endl;
			++column;
			break;
		}
	}

	// now write the next paragraph
	if (next)
		next->writeFile(os, params, footflag, dth);
}


void LyXParagraph::validate(LaTeXFeatures & features) const
{
	// this will be useful later
	LyXLayout const & layout =
		textclasslist.Style(current_view->buffer()->params.textclass, 
				    GetLayout());
	
	// check the params.
	if (line_top || line_bottom)
		features.lyxline = true;
	
	// then the layouts
	features.layout[GetLayout()] = true;

	// then the fonts
	Language const * doc_language = current_view->buffer()->params.language_info;
	for (FontList::const_iterator cit = fontlist.begin();
	     cit != fontlist.end(); ++cit) {
		if ((*cit).font.noun() == LyXFont::ON) {
			lyxerr[Debug::LATEX] << "font.noun: "
					     << (*cit).font.noun()
					     << endl;
			features.noun = true;
			lyxerr[Debug::LATEX] << "Noun enabled. Font: "
					     << (*cit).font.stateText()
					     << endl;
		}
		switch ((*cit).font.color()) {
		case LColor::none:
		case LColor::inherit:
		case LColor::ignore:
			break;
		default:
			features.color = true;
			lyxerr[Debug::LATEX] << "Color enabled. Font: "
					     << (*cit).font.stateText()
					     << endl;
		}
		Language const * language = (*cit).font.language();
		if (language != doc_language && language != default_language) {
			features.UsedLanguages.insert(language);
			lyxerr[Debug::LATEX] << "Found language "
					     << language->lang << endl;
		}		
	}

	// then the insets
	for (InsetList::const_iterator cit = insetlist.begin();
	     cit != insetlist.end(); ++cit) {
		if ((*cit).inset)
			(*cit).inset->Validate(features);
	}

        if (table && table->IsLongTable())
		features.longtable = true;
        if (pextra_type == PEXTRA_INDENT)
                features.LyXParagraphIndent = true;
        if (pextra_type == PEXTRA_FLOATFLT)
                features.floatflt = true;
        if (layout.needprotect 
	    && next && next->footnoteflag != LyXParagraph::NO_FOOTNOTE)
		features.NeedLyXFootnoteCode = true;
        if ((current_view->buffer()->params.paragraph_separation == BufferParams::PARSEP_INDENT) &&
            (pextra_type == LyXParagraph::PEXTRA_MINIPAGE))
		features.NeedLyXMinipageIndent = true;
        if (table && table->NeedRotating())
		features.rotating = true;
	if (footnoteflag != NO_FOOTNOTE && footnotekind == ALGORITHM)
		features.algorithm = true;
}


// First few functions needed for cut and paste and paragraph breaking.
void LyXParagraph::CopyIntoMinibuffer(LyXParagraph::size_type pos) const
{
	minibuffer_char = GetChar(pos);
	minibuffer_font = GetFontSettings(pos);
	minibuffer_inset = 0;
	if (minibuffer_char == LyXParagraph::META_INSET) {
		if (GetInset(pos)) {
			minibuffer_inset = GetInset(pos)->Clone();
		} else {
			minibuffer_inset = 0;
			minibuffer_char = ' ';
			// This reflects what GetInset() does (ARRae)
		}
	}
}


void LyXParagraph::CutIntoMinibuffer(LyXParagraph::size_type pos)
{
	minibuffer_char = GetChar(pos);
	minibuffer_font = GetFontSettings(pos);
	minibuffer_inset = 0;
	if (minibuffer_char == LyXParagraph::META_INSET) {
		if (GetInset(pos)) {
			minibuffer_inset = GetInset(pos);
			// This is a little hack since I want exactly
			// the inset, not just a clone. Otherwise
			// the inset would be deleted when calling Erase(pos)
			// find the entry
			InsetList::iterator it = lower_bound(insetlist.begin(),
							     insetlist.end(),
							     InsetTable(pos,0));
			if (it != insetlist.end() && (*it).pos == pos)
				(*it).inset = 0;
		} else {
			minibuffer_inset = 0;
			minibuffer_char = ' ';
			// This reflects what GetInset() does (ARRae)
		}

	}

	// Erase(pos); now the caller is responsible for that.
}


void LyXParagraph::InsertFromMinibuffer(LyXParagraph::size_type pos)
{
	InsertChar(pos, minibuffer_char);
	SetFont(pos, minibuffer_font);
	if (minibuffer_char == LyXParagraph::META_INSET)
		InsertInset(pos, minibuffer_inset);
}

// end of minibuffer



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


// the destructor removes the new paragraph from the list
LyXParagraph::~LyXParagraph()
{
	if (previous)
		previous->next = next;
	if (next)
		next->previous = previous;

	for (InsetList::iterator it = insetlist.begin();
	     it != insetlist.end(); ++it) {
		delete (*it).inset;
	}

	/* table stuff -- begin*/ 
	delete table;
	/* table stuff -- end*/ 

        // ale970302
	delete bibkey;
}


void LyXParagraph::Erase(LyXParagraph::size_type pos)
{
	// > because last is the next unused position, and you can 
	// use it if you want
	if (pos > size()) {
		if (next && next->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE) 
			NextAfterFootnote()->Erase(pos - text.size() - 1);
		else 
			lyxerr.debug() << "ERROR (LyXParagraph::Erase): "
				"position does not exist." << endl;
		return;
	}
	if (pos < size()) { // last is free for insertation, but should be empty
		// if it is an inset, delete the inset entry 
		if (text[pos] == LyXParagraph::META_INSET) {
			// find the entry
			InsetList::iterator it = lower_bound(insetlist.begin(),
							     insetlist.end(),
							     InsetTable(pos,0));
			if (it != insetlist.end() && (*it).pos == pos) {
				delete (*it).inset;
				insetlist.erase(it);
			}
		}
		text.erase(text.begin() + pos);
		// Erase entries in the tables.
		for (FontList::iterator it = fontlist.begin();
		     it != fontlist.end(); ++it) {
			if (pos >= (*it).pos && pos <= (*it).pos_end) {
				if ((*it).pos == (*it).pos_end) {
					// If it is a multi-character font
					// entry, we just make it smaller
					// (see update below), otherwise we
					// should delete it.
					fontlist.erase(it);
					break;
				}
			}
		}
		// Update all other entries.
		for (FontList::iterator it = fontlist.begin();
		     it != fontlist.end(); ++it) {
			if ((*it).pos > pos)
				(*it).pos--;
			if ((*it).pos_end >= pos)
				(*it).pos_end--;
		}

		// Update the inset table.
		for (InsetList::iterator it = upper_bound(insetlist.begin(),
							  insetlist.end(),
							  InsetTable(pos,0));
		     it != insetlist.end(); ++it)
			--(*it).pos;
	} else {
		lyxerr << "ERROR (LyXParagraph::Erase): "
			"can't erase non-existant char." << endl;
	}
}


void LyXParagraph::InsertChar(LyXParagraph::size_type pos, char c)
{
	// > because last is the next unused position, and you can 
	// use it if you want
	if (pos > size()) {
		if (next
		    && next->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE) 
			NextAfterFootnote()->InsertChar(pos - text.size() - 1,
							c);
		else 
			lyxerr.debug() << "ERROR (LyXParagraph::InsertChar): "
				"position does not exist." << endl;
		return;
	}
	text.insert(text.begin() + pos, c);
	// Update the font table.
	for (FontList::iterator it = fontlist.begin();
	     it != fontlist.end(); ++it) {
		if ((*it).pos >= pos)
			(*it).pos++;
		if ((*it).pos_end >= pos)
			(*it).pos_end++;
	}
   
	// Update the inset table.
	for (InsetList::iterator it = lower_bound(insetlist.begin(),
						  insetlist.end(),
						  InsetTable(pos,0));
	     it != insetlist.end(); ++it)
		++(*it).pos;
}


void LyXParagraph::InsertInset(LyXParagraph::size_type pos,
			       Inset * inset)
{
	// > because last is the next unused position, and you can 
	// use it if you want
	if (pos > size()) {
		if (next
		    && next->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE) 
			NextAfterFootnote()
				->InsertInset(pos - text.size() - 1, inset);
		else 
			lyxerr << "ERROR (LyXParagraph::InsertInset): " 
				"position does not exist: " << pos << endl;
		return;
	}
	if (text[pos] != LyXParagraph::META_INSET) {
		lyxerr << "ERROR (LyXParagraph::InsertInset): "
			"there is no LyXParagraph::META_INSET" << endl;
		return;
	}

	if (inset) {
		// Add a new entry in the inset table.
		InsetList::iterator it = lower_bound(insetlist.begin(),
						     insetlist.end(),
						     InsetTable(pos,0));
		if (it != insetlist.end() && (*it).pos == pos)
			lyxerr << "ERROR (LyXParagraph::InsertInset): "
				"there is an inset in position: " << pos << endl;
		else
			insetlist.insert(it,InsetTable(pos,inset));
	}
}


Inset * LyXParagraph::GetInset(LyXParagraph::size_type pos)
{
	if (pos >= size()) {
		if (next
		    && next->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE) 
			return NextAfterFootnote()
				->GetInset(pos - text.size() - 1);
		else { 
		        lyxerr << "ERROR (LyXParagraph::GetInset): "
				"position does not exist: "
			       << pos << endl;
		}
		return 0;
	}
	// Find the inset.
	InsetList::iterator it = lower_bound(insetlist.begin(),
					     insetlist.end(),
					     InsetTable(pos,0));
	if (it != insetlist.end() && (*it).pos == pos)
		return (*it).inset;

	lyxerr << "ERROR (LyXParagraph::GetInset): "
		"Inset does not exist: " << pos << endl;
	// text[pos] = ' '; // WHY!!! does this set the pos to ' '????
	// Did this commenting out introduce a bug? So far I have not
	// see any, please enlighten me. (Lgb)
	// My guess is that since the inset does not exist, we might
	// as well replace it with a space to prevent craches. (Asger)
	return 0;
}


Inset const * LyXParagraph::GetInset(LyXParagraph::size_type pos) const
{
	if (pos >= size()) {
		if (next
		    && next->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE) 
			return NextAfterFootnote()
				->GetInset(pos - text.size() - 1);
		else { 
		        lyxerr << "ERROR (LyXParagraph::GetInset): "
				"position does not exist: "
			       << pos << endl;
		}
		return 0;
	}
	// Find the inset.
	InsetList::const_iterator cit = lower_bound(insetlist.begin(),
						    insetlist.end(),
						    InsetTable(pos,0));
	if (cit != insetlist.end() && (*cit).pos == pos)
		return (*cit).inset;

	lyxerr << "ERROR (LyXParagraph::GetInset): "
		"Inset does not exist: " << pos << endl;
	//text[pos] = ' '; // WHY!!! does this set the pos to ' '????
	// Did this commenting out introduce a bug? So far I have not
	// see any, please enlighten me. (Lgb)
	// My guess is that since the inset does not exist, we might
	// as well replace it with a space to prevent craches. (Asger)
	return 0;
}


// Gets uninstantiated font setting at position.
// Optimized after profiling. (Asger)
LyXFont LyXParagraph::GetFontSettings(LyXParagraph::size_type pos) const
{
	if (pos < size()) {
#ifdef SORTED_FONT_LIST
		for (FontList::const_iterator cit = fontlist.begin();
		     cit != fontlist.end() && pos <= (*cit).pos_end; ++cit)
			if (pos >= (*cit).pos)
				return (*cit).font;
#else
		for (FontList::const_iterator cit = fontlist.begin();
		     cit != fontlist.end(); ++cit)
			if (pos >= (*cit).pos && pos <= (*cit).pos_end)
				return (*cit).font;
#endif
	}
	// > because last is the next unused position, and you can 
	// use it if you want
	else if (pos > size()) {
		if (next
		    && next->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE) 
			return NextAfterFootnote()
				->GetFontSettings(pos - text.size() - 1);
		else {
			// Why is it an error to ask for the font of a
			// position that does not exist? Would it be
			// enough for this to be enabled on debug?
			// We want strict error checking, but it's ok to only
			// have it when debugging. (Asger)
			lyxerr << "ERROR (LyXParagraph::GetFontSettings): "
				"position does not exist. "
			       << pos << " (" << static_cast<int>(pos)
			       << ")" << endl;
		}
	} else if (pos > 0) {
		return GetFontSettings(pos - 1);
	} else // pos = size() = 0
		return LyXFont(LyXFont::ALL_INHERIT,getParLanguage());

	return LyXFont(LyXFont::ALL_INHERIT);
}

// Gets uninstantiated font setting at position 0
LyXFont LyXParagraph::GetFirstFontSettings() const
{
	if (size() > 0) {
#ifdef SORTED_FONT_LIST
		if (!fontlist.empty() && fontlist.front().pos == 0)
			return fontlist.front().font;
#else
		for (FontList::const_iterator cit = fontlist.begin();
		     cit != fontlist.end(); ++cit)
			if (0 >= (*cit).pos && 0 <= (*cit).pos_end)
				return (*cit).font;
#endif
	} else if (next && next->footnoteflag != LyXParagraph::NO_FOOTNOTE) 
		return NextAfterFootnote()->GetFirstFontSettings();
	return LyXFont(LyXFont::ALL_INHERIT);
}

// Gets the fully instantiated font at a given position in a paragraph
// This is basically the same function as LyXText::GetFont() in text2.C.
// The difference is that this one is used for generating the LaTeX file,
// and thus cosmetic "improvements" are disallowed: This has to deliver
// the true picture of the buffer. (Asger)
// If position is -1, we get the layout font of the paragraph.
// If position is -2, we get the font of the manual label of the paragraph.
LyXFont LyXParagraph::getFont(LyXParagraph::size_type pos) const
{
	LyXFont tmpfont;
	LyXLayout const & layout =
		textclasslist.Style(current_view->buffer()->params.textclass, 
				    GetLayout());
	LyXParagraph::size_type main_body = 0;
	if (layout.labeltype == LABEL_MANUAL)
		main_body = BeginningOfMainBody();

	if (pos >= 0){
		LyXFont layoutfont;
		if (pos < main_body)
			layoutfont = layout.labelfont;
		else
			layoutfont = layout.font;
		tmpfont = GetFontSettings(pos);
		tmpfont.realize(layoutfont);
	} else {
		// process layoutfont for pos == -1 and labelfont for pos < -1
		if (pos == -1)
			tmpfont = layout.font;
		else
			tmpfont = layout.labelfont;
		tmpfont.setLanguage(getParLanguage());
	}

	// check for environment font information
	char par_depth = GetDepth();
	LyXParagraph const * par = this;
	while (par && par_depth && !tmpfont.resolved()) {
		par = par->DepthHook(par_depth - 1);
		if (par) {
			tmpfont.realize(textclasslist.
					Style(current_view->buffer()->params.textclass,
					      par->GetLayout()).font);
			par_depth = par->GetDepth();
		}
	}

	tmpfont.realize(textclasslist
			.TextClass(current_view->buffer()->params.textclass)
			.defaultfont());
	return tmpfont;
}


/// Returns the height of the highest font in range
LyXFont::FONT_SIZE
LyXParagraph::HighestFontInRange(LyXParagraph::size_type startpos,
				 LyXParagraph::size_type endpos) const
{
	LyXFont::FONT_SIZE maxsize = LyXFont::SIZE_TINY;
	for (FontList::const_iterator cit = fontlist.begin();
	     cit != fontlist.end(); ++cit) {
		if (startpos <= (*cit).pos_end && endpos >= (*cit).pos) {
			LyXFont::FONT_SIZE size = (*cit).font.size();
			if (size > maxsize && size <= LyXFont::SIZE_HUGER)
				maxsize = size;
		}
	}
	return maxsize;
}


char LyXParagraph::GetChar(LyXParagraph::size_type pos)
{
	Assert(pos >= 0);

	if (pos < size()) {
		return text[pos];
	}
	// > because last is the next unused position, and you can 
	// use it if you want
	else if (pos > size()) {
		if (next && next->footnoteflag != LyXParagraph::NO_FOOTNOTE) 
			return NextAfterFootnote()
				->GetChar(pos - text.size() - 1);
		else {
			lyxerr << "ERROR (LyXParagraph::GetChar): "
				"position does not exist."
			       << pos << " (" << static_cast<int>(pos)
			       << ")\n";
			// Assert(false); // This triggers sometimes...
			// Why?
		}
		return '\0';
	} else {
		// We should have a footnote environment.
		if (!next || next->footnoteflag == LyXParagraph::NO_FOOTNOTE) {
			// Notice that LyX does request the
			// last char from time to time. (Asger)
			//lyxerr << "ERROR (LyXParagraph::GetChar): "
			//	"expected footnote." << endl;
			return '\0';
		}
		switch (next->footnotekind) {
		case LyXParagraph::FOOTNOTE:
			return LyXParagraph::META_FOOTNOTE;
		case LyXParagraph::MARGIN:
			return LyXParagraph::META_MARGIN;
		case LyXParagraph::FIG:
		case LyXParagraph::WIDE_FIG:
			return LyXParagraph::META_FIG;
		case LyXParagraph::TAB:
		case LyXParagraph::WIDE_TAB:
			return LyXParagraph::META_TAB;
		case LyXParagraph::ALGORITHM:
			return LyXParagraph::META_ALGORITHM;
		}
		return '\0'; // to shut up gcc
	}
}


char LyXParagraph::GetChar(LyXParagraph::size_type pos) const
{
	Assert(pos >= 0);

	if (pos < size()) {
		return text[pos];
	}
	// > because last is the next unused position, and you can 
	// use it if you want
	else if (pos > size()) {
		if (next && next->footnoteflag != LyXParagraph::NO_FOOTNOTE) 
			return NextAfterFootnote()
				->GetChar(pos - text.size() - 1);
		else {
			lyxerr << "ERROR (LyXParagraph::GetChar const): "
				"position does not exist."
			       << pos << " (" << static_cast<int>(pos)
			       << ")\n";
			Assert(false);
		}
		return '\0';
	} else {
		// We should have a footnote environment.
		if (!next || next->footnoteflag == LyXParagraph::NO_FOOTNOTE) {
			// Notice that LyX does request the
			// last char from time to time. (Asger)
			//lyxerr << "ERROR (LyXParagraph::GetChar): "
			//	"expected footnote." << endl;
			return '\0';
		}
		switch (next->footnotekind) {
		case LyXParagraph::FOOTNOTE:
			return LyXParagraph::META_FOOTNOTE;
		case LyXParagraph::MARGIN:
			return LyXParagraph::META_MARGIN;
		case LyXParagraph::FIG:
		case LyXParagraph::WIDE_FIG:
			return LyXParagraph::META_FIG;
		case LyXParagraph::TAB:
		case LyXParagraph::WIDE_TAB:
			return LyXParagraph::META_TAB;
		case LyXParagraph::ALGORITHM:
			return LyXParagraph::META_ALGORITHM;
		}
		return '\0'; // to shut up gcc
	}
}


// return an string of the current word, and the end of the word in lastpos.
string LyXParagraph::GetWord(LyXParagraph::size_type & lastpos) const
{
	Assert(lastpos>=0);

	// the current word is defined as starting at the first character
	// from the immediate left of lastpospos which meets the definition
	// of IsLetter(), continuing to the last character to the right
	// of this meeting IsLetter.

    	string theword;

	// grab a word
		
	// move back until we have a letter

	//there's no real reason to have firstpos & lastpos as
	//separate variables as this is written, but maybe someon
	// will want to return firstpos in the future.

	//since someone might have typed a punctuation first
	int firstpos = lastpos;
 	
	while ((firstpos >= 0) && !IsLetter(firstpos))
		--firstpos;

	// now find the beginning by looking for a nonletter
	
	while ((firstpos>= 0) && IsLetter(firstpos))
		--firstpos;

	// the above is now pointing to the preceeding non-letter
	++firstpos;
	lastpos = firstpos;

	// so copy characters into theword  until we get a nonletter
	// note that this can easily exceed lastpos, wich means
	// that if used in the middle of a word, the whole word
	// is included

	while (IsLetter(lastpos)) theword += GetChar(lastpos++);
	
	return theword;

}

 
LyXParagraph::size_type LyXParagraph::Last() const
{
	if (next && next->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE)
		return text.size() + NextAfterFootnote()->Last() + 1;
	// the 1 is the symbol
	// for the footnote
	else
		return text.size();
}


LyXParagraph * LyXParagraph::ParFromPos(LyXParagraph::size_type pos)
{
	// > because last is the next unused position, and you can 
	// use it if you want
	if (pos > size()) {
		if (next
		    && next->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE) 
			return NextAfterFootnote()
				->ParFromPos(pos - text.size() - 1);
		else 
			lyxerr << "ERROR (LyXParagraph::ParFromPos): "
				"position does not exist." << endl;
		return this;
	} else
		return this;
}


int LyXParagraph::PositionInParFromPos(LyXParagraph::size_type pos) const
{
	// > because last is the next unused position, and you can 
	// use it if you want
	if (pos > size()) {
		if (next
		    && next->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE) 
			return NextAfterFootnote()
				->PositionInParFromPos(pos - text.size() - 1);
		else 
			lyxerr <<
				"ERROR (LyXParagraph::PositionInParFromPos): "
				"position does not exist." << endl;
		return pos;
	}
	else
		return pos;
}


void LyXParagraph::SetFont(LyXParagraph::size_type pos,
			   LyXFont const & font)
{
	// > because last is the next unused position, and you can 
	// use it if you want
	if (pos > size()) {
		if (next &&
		    next->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE) {
			NextAfterFootnote()->SetFont(pos - text.size() - 1,
						     font);
		} else {
			lyxerr << "ERROR (LyXParagraph::SetFont): "
				"position does not exist." << endl;
		}
		return;
	}
	LyXFont patternfont(LyXFont::ALL_INHERIT);

	// First, reduce font against layout/label font
	// Update: The SetCharFont() routine in text2.C already
	// reduces font, so we don't need to do that here. (Asger)
	// No need to simplify this because it will disappear
	// in a new kernel. (Asger)
	// Next search font table
	FontList::iterator tmp = fontlist.begin();
	for (; tmp != fontlist.end(); ++tmp) {
		if (pos >= (*tmp).pos && pos <= (*tmp).pos_end) {
			break;
		}
	}
	if (tmp == fontlist.end()) { // !found
		// if we did not find a font entry, but if the font at hand
		// is the same as default, we just forget it
		if (font == patternfont) return;

		// ok, we did not find a font entry. But maybe there is exactly
		// the needed font ientry one position left
		FontList::iterator tmp2 = fontlist.begin();
		for (; tmp2 != fontlist.end(); ++tmp2) {
			if (pos - 1 >= (*tmp2).pos
			    && pos - 1 <= (*tmp2).pos_end)
				break;
		}
		if (tmp2 != fontlist.end()) {
			// ok there is one, maybe it is exactly
			// the needed font
			if ((*tmp2).font == font) {
				// put the position under the font
				(*tmp2).pos_end++;
				return;
			}
		}
		// Add a new entry in the
		// fontlist for the position
		FontTable ft;
		ft.pos = pos;
		ft.pos_end = pos;
		ft.font = font; // or patternfont
		// It seems that using font instead of patternfont here
		// fixes all the problems. This also surfaces a "bug" in
		// the old code.
		fontlist.insert(fontlist.begin(), ft);
	} else if ((*tmp).pos != (*tmp).pos_end) { // we found a font entry. maybe we have to split it and create a new one.

// more than one character
		if (pos == (*tmp).pos) {
			// maybe we should enlarge the left fonttable
			FontList::iterator tmp2 = fontlist.begin();
			for (; tmp2 != fontlist.end(); ++tmp2) {
				if (pos - 1 >= (*tmp2).pos
				    && pos - 1 <= (*tmp2).pos_end)
					break;
			}
			// Is there is one, and is it exactly
			// the needed font?
			if (tmp2 != fontlist.end() &&
			    (*tmp2).font == font) {
				// Put the position under the font
				(*tmp2).pos_end++;
				(*tmp).pos++;
				return;
			}

			// Add a new entry in the
			// fontlist for the position
			FontTable ft;
			ft.pos = pos + 1;
			ft.pos_end = (*tmp).pos_end;
			ft.font = (*tmp).font;
			(*tmp).pos_end = pos;
			(*tmp).font = font;
			fontlist.insert(fontlist.begin(), ft);
		} else if (pos == (*tmp).pos_end) {
			// Add a new entry in the
			// fontlist for the position
			FontTable ft;
			ft.pos = (*tmp).pos;
			ft.pos_end = (*tmp).pos_end - 1;
			ft.font = (*tmp).font;
			(*tmp).pos = (*tmp).pos_end;
			(*tmp).font = font;
			fontlist.insert(fontlist.begin(), ft);
		} else {
			// Add a new entry in the
			// fontlist for the position
			FontTable ft;
			ft.pos = (*tmp).pos;
			ft.pos_end = pos - 1;
			ft.font = (*tmp).font;
			
			FontTable ft2;
			ft2.pos = pos + 1;
			ft2.pos_end = (*tmp).pos_end;
			ft2.font = (*tmp).font;
			
			(*tmp).pos = pos;
			(*tmp).pos_end = pos;
			(*tmp).font = font;
			
			fontlist.insert(fontlist.begin(), ft);
			fontlist.insert(fontlist.begin(), ft2);
		}
	} else {
		(*tmp).font = font;
	}
}

   
// This function is able to hide closed footnotes.
LyXParagraph * LyXParagraph::Next()
{
	if (next && next->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE) {
		LyXParagraph * tmp = next;
		while (tmp
		       && tmp->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE)
			tmp = tmp->next;
		if (tmp && tmp->footnoteflag != LyXParagraph::CLOSED_FOOTNOTE) 
			return tmp->Next(); /* there can be more than one
					       footnote in a logical
					       paragraph */
		else
			return next;  // This should never happen!
	} else
		return next;
}


LyXParagraph * LyXParagraph::NextAfterFootnote()
{
	if (next && next->footnoteflag != LyXParagraph::NO_FOOTNOTE) {
		LyXParagraph * tmp = next;
		while (tmp && tmp->footnoteflag != LyXParagraph::NO_FOOTNOTE)
			tmp = tmp->next;
		if (tmp && tmp->footnoteflag != LyXParagraph::CLOSED_FOOTNOTE) 
			return tmp;   /* there can be more than one footnote
					 in a logical paragraph */
		else
			return next;  // This should never happen!
	} else
		return next;
}


LyXParagraph const * LyXParagraph::NextAfterFootnote() const
{
	if (next && next->footnoteflag != LyXParagraph::NO_FOOTNOTE) {
		LyXParagraph * tmp = next;
		while (tmp && tmp->footnoteflag != LyXParagraph::NO_FOOTNOTE)
			tmp = tmp->next;
		if (tmp && tmp->footnoteflag != LyXParagraph::CLOSED_FOOTNOTE) 
			return tmp;   /* there can be more than one footnote
					 in a logical paragraph */
		else
			return next;  // This should never happen!
	} else
		return next;
}


LyXParagraph * LyXParagraph::PreviousBeforeFootnote()
{
	LyXParagraph * tmp;
	if (previous && previous->footnoteflag != LyXParagraph::NO_FOOTNOTE) {
		tmp = previous;
		while (tmp && tmp->footnoteflag != LyXParagraph::NO_FOOTNOTE)
			tmp = tmp->previous;
		if (tmp && tmp->footnoteflag != LyXParagraph::CLOSED_FOOTNOTE) 
			return tmp;    /* there can be more than one footnote
					  in a logical paragraph */
		else
			return previous;  // This should never happen!
	} else
		return previous;
}


LyXParagraph * LyXParagraph::LastPhysicalPar()
{
	if (footnoteflag != LyXParagraph::NO_FOOTNOTE)
		return this;
   
	LyXParagraph * tmp = this;
	while (tmp->next
	       && tmp->next->footnoteflag != LyXParagraph::NO_FOOTNOTE)
		tmp = tmp->NextAfterFootnote();
   
	return tmp;
}

LyXParagraph const * LyXParagraph::LastPhysicalPar() const
{
	if (footnoteflag != LyXParagraph::NO_FOOTNOTE)
		return this;
   
	LyXParagraph const * tmp = this;
	while (tmp->next
	       && tmp->next->footnoteflag != LyXParagraph::NO_FOOTNOTE)
		tmp = tmp->NextAfterFootnote();
   
	return tmp;  
}

LyXParagraph * LyXParagraph::FirstPhysicalPar()
{
	if (!IsDummy())
		return this;
	LyXParagraph * tmppar = this;

	while (tmppar &&
	       (tmppar->IsDummy()
		|| tmppar->footnoteflag != LyXParagraph::NO_FOOTNOTE))
		tmppar = tmppar->previous;
   
	if (!tmppar) {
		return this;
	} else
		return tmppar;
}


LyXParagraph const * LyXParagraph::FirstPhysicalPar() const
{
	if (!IsDummy())
		return this;
	LyXParagraph const * tmppar = this;

	while (tmppar &&
	       (tmppar->IsDummy()
		|| tmppar->footnoteflag != LyXParagraph::NO_FOOTNOTE))
		tmppar = tmppar->previous;
   
	if (!tmppar) {
		return this;
	} else
		return tmppar;
}


// This function is able to hide closed footnotes.
LyXParagraph * LyXParagraph::Previous()
{
	LyXParagraph * tmp = previous;
	if (!tmp)
		return tmp;
   
	if (tmp->previous
	    && tmp->previous->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE) {
		tmp = tmp->previous;
		while (tmp
		       && tmp->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE)
			tmp = tmp->previous;
		if (tmp && tmp->footnoteflag != LyXParagraph::CLOSED_FOOTNOTE) 
			return tmp->next->Previous();	

		else
			return previous; 
	} else
		return previous;
}


// This function is able to hide closed footnotes.
LyXParagraph const * LyXParagraph::Previous() const
{
	LyXParagraph * tmp = previous;
	if (!tmp)
		return tmp;
   
	if (tmp->previous
	    && tmp->previous->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE) {
		tmp = tmp->previous;
		while (tmp
		       && tmp->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE)
			tmp = tmp->previous;
		if (tmp && tmp->footnoteflag != LyXParagraph::CLOSED_FOOTNOTE) 
			return tmp->next->Previous();	

		else
			return previous; 
	} else
		return previous;
}


void LyXParagraph::BreakParagraph(LyXParagraph::size_type pos,
				  int flag)
{
	size_type i, pos_end, pos_first;
	// create a new paragraph
	LyXParagraph * par = ParFromPos(pos);
	LyXParagraph * firstpar = FirstPhysicalPar();
   
	LyXParagraph * tmp = new LyXParagraph(par);
	
	tmp->footnoteflag = footnoteflag;
	tmp->footnotekind = footnotekind;
   
	// this is an idea for a more userfriendly layout handling, I will
	// see what the users say
   
	// layout stays the same with latex-environments
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
   
		// copy everything behind the break-position
		// to the new paragraph
		pos_first = 0;
		while (ParFromPos(pos_first) != par)
			++pos_first;

		pos_end = pos_first + par->text.size() - 1;
		// The constructor has already reserved 500 elements
		//if (pos_end > pos)
		//	tmp->text.reserve(pos_end - pos);

		for (i = pos; i <= pos_end; ++i) {
			par->CutIntoMinibuffer(i - pos_first);
			tmp->InsertFromMinibuffer(i - pos);
		}
		tmp->text.resize(tmp->text.size());
		for (i = pos_end; i >= pos; --i)
			par->Erase(i - pos_first);

		par->text.resize(par->text.size());
	}

	// just an idea of me
	if (!pos) {
		tmp->line_top = firstpar->line_top;
		tmp->pagebreak_top = firstpar->pagebreak_top;
		tmp->added_space_top = firstpar->added_space_top;
		tmp->bibkey = firstpar->bibkey;
		firstpar->Clear();
		// layout stays the same with latex-environments
		if (flag) {
			firstpar->SetOnlyLayout(tmp->layout);
			firstpar->SetLabelWidthString(tmp->labelwidthstring);
			firstpar->depth = tmp->depth;
		}
	}
}


void LyXParagraph::MakeSameLayout(LyXParagraph const * par)
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


LyXParagraph * LyXParagraph::FirstSelfrowPar()
{
	LyXParagraph * tmppar = this;
	while (tmppar && (
		(tmppar->IsDummy()
		 && tmppar->previous->footnoteflag == 
		 LyXParagraph::CLOSED_FOOTNOTE)
		|| tmppar->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE))
		tmppar = tmppar->previous;
   
	if (!tmppar)
		return this;  // This should never happen!
	else
		return tmppar;
}


LyXParagraph * LyXParagraph::Clone() const
{
	// create a new paragraph
	LyXParagraph * result = new LyXParagraph;
   
	result->MakeSameLayout(this);

	// this is because of the dummy layout of the paragraphs that
	// follow footnotes
	result->layout = layout;
   
	/* table stuff -- begin*/ 
	if (table)
		result->table = table->Clone();
	else
		result->table = 0;
	/* table stuff -- end*/ 
   
        // ale970302
        result->bibkey = (bibkey) ? new InsetBibKey(bibkey): 0;
               
    
	// copy everything behind the break-position to the new paragraph

	result->text = text;
	result->fontlist = fontlist;
	result->insetlist = insetlist;
	for (InsetList::iterator it = result->insetlist.begin();
	     it != result->insetlist.end(); ++it)
		(*it).inset = (*it).inset->Clone();
	return result;
}


bool LyXParagraph::HasSameLayout(LyXParagraph const * par) const
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


void LyXParagraph::BreakParagraphConservative(LyXParagraph::size_type pos)
{
	// create a new paragraph
	LyXParagraph * par = ParFromPos(pos);

	LyXParagraph * tmp = new LyXParagraph(par);
   
	tmp->MakeSameLayout(par);

	// When can pos < Last()?
	// I guess pos == Last() is possible.
	if (Last() > pos) {
		// copy everything behind the break-position to the new
		// paragraph
		size_type pos_first = 0;
		while (ParFromPos(pos_first) != par)
			++pos_first;
		size_type pos_end = pos_first + par->text.size() - 1;
		// make sure there is enough memory for the now larger
		// paragraph. This is not neccessary, because
		// InsertFromMinibuffer will enlarge the memory (it uses
		// InsertChar of course). But doing it by hand
		// is MUCH faster! (only one time, not thousend times!!)
		// Not needed since the constructor aleady have
		// reserved 500 elements in text.
		//if (pos_end > pos)
		//	tmp->text.reserve(pos_end - pos);

		for (size_type i = pos; i <= pos_end; ++i) {
			par->CutIntoMinibuffer(i - pos_first);
			tmp->InsertFromMinibuffer(i - pos);
		}
		tmp->text.resize(tmp->text.size());
		for (size_type i = pos_end; i >= pos; --i)
			par->Erase(i - pos_first);

		par->text.resize(par->text.size());
	}
}
   

// Be carefull, this does not make any check at all.
void LyXParagraph::PasteParagraph()
{
	// copy the next paragraph to this one
	LyXParagraph * the_next = Next();
   
	LyXParagraph * firstpar = FirstPhysicalPar();
   
	// first the DTP-stuff
	firstpar->line_bottom = the_next->line_bottom;
	firstpar->added_space_bottom = the_next->added_space_bottom;
	firstpar->pagebreak_bottom = the_next->pagebreak_bottom;

	size_type pos_end = the_next->text.size() - 1;
	size_type pos_insert = Last();

	// ok, now copy the paragraph
	for (size_type i = 0; i <= pos_end; ++i) {
		the_next->CutIntoMinibuffer(i);
		InsertFromMinibuffer(pos_insert + i);
	}
   
	// delete the next paragraph
	delete the_next;
}


void LyXParagraph::OpenFootnote(LyXParagraph::size_type pos)
{
	LyXParagraph * par = ParFromPos(pos);
	par = par->next;
	while (par && par->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE) {
		par->footnoteflag = LyXParagraph::OPEN_FOOTNOTE;
		par = par->next;
	}
}


void LyXParagraph::CloseFootnote(LyXParagraph::size_type pos)
{
	LyXParagraph * par = ParFromPos(pos);
	par = par->next;
	while (par && par->footnoteflag == LyXParagraph::OPEN_FOOTNOTE) {
		par->footnoteflag = LyXParagraph::CLOSED_FOOTNOTE;
		par = par->next;
	}
}

int LyXParagraph::GetEndLabel() const
{
	LyXParagraph const * par = this;
	int par_depth = GetDepth();
	while (par) {
		LyXTextClass::LayoutList::size_type layout = par->GetLayout();
		int endlabeltype =
			textclasslist.Style(current_view->buffer()->params.textclass,
					    layout).endlabeltype;
		if (endlabeltype != END_LABEL_NO_LABEL) {
			LyXParagraph const * last = this;
			if( footnoteflag == NO_FOOTNOTE)
				last = LastPhysicalPar();
			else if (next->footnoteflag == NO_FOOTNOTE)
				return endlabeltype;

			if (!last || !last->next)
				return endlabeltype;

			int next_depth = last->next->GetDepth();
			if (par_depth > next_depth ||
			    (par_depth == next_depth && layout != last->next->GetLayout() ))
				return endlabeltype;
			break;
		}
		if (par_depth == 0)
			break;
		par = par->DepthHook(par_depth - 1);
		if (par)
			par_depth = par->GetDepth();
	}
	return END_LABEL_NO_LABEL;
}

LyXTextClass::size_type LyXParagraph::GetLayout() const
{
	return FirstPhysicalPar()->layout;
}


char LyXParagraph::GetDepth() const
{
	return FirstPhysicalPar()->depth;
}


char LyXParagraph::GetAlign() const
{
	return FirstPhysicalPar()->align;
}


string LyXParagraph::GetLabelstring() const
{
	return FirstPhysicalPar()->labelstring;
}


int LyXParagraph::GetFirstCounter(int i) const
{
	return FirstPhysicalPar()->counter_[i];
}


// the next two functions are for the manual labels
string LyXParagraph::GetLabelWidthString() const
{
	if (!FirstPhysicalPar()->labelwidthstring.empty())
		return FirstPhysicalPar()->labelwidthstring;
	else
		return _("Senseless with this layout!");
}


void LyXParagraph::SetLabelWidthString(string const & s)
{
	LyXParagraph * par = FirstPhysicalPar();

	par->labelwidthstring = s;
}


void LyXParagraph::SetOnlyLayout(LyXTextClass::size_type new_layout)
{
	LyXParagraph * par = FirstPhysicalPar();
	LyXParagraph * ppar = 0;
	LyXParagraph * npar = 0;

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
                                            p1.c_str(), p2.c_str());
                }
                if ((par->pextra_type == PEXTRA_NONE) &&
                    npar && (npar->pextra_type != PEXTRA_NONE)) {
                        string
                                p1 = npar->pextra_width,
                                p2 = npar->pextra_widthp;
                        npar->SetPExtraType(npar->pextra_type,
                                            p1.c_str(), p2.c_str());
                }
        }
}


void LyXParagraph::SetLayout(LyXTextClass::size_type new_layout)
{
	LyXParagraph
		* par = FirstPhysicalPar(),
		* ppar = 0,
		* npar = 0;

        par->layout = new_layout;
	par->labelwidthstring.clear();
	par->align = LYX_ALIGN_LAYOUT;
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
                                            p1.c_str(), p2.c_str());
                }
                if ((par->pextra_type == PEXTRA_NONE) &&
                    npar && (npar->pextra_type != PEXTRA_NONE)) {
                        string
                                p1 = npar->pextra_width,
                                p2 = npar->pextra_widthp;
                        npar->SetPExtraType(npar->pextra_type,
                                            p1.c_str(), p2.c_str());
                }
        }
}


// if the layout of a paragraph contains a manual label, the beginning of the 
// main body is the beginning of the second word. This is what the par-
// function returns. If the layout does not contain a label, the main
// body always starts with position 0. This differentiation is necessary,
// because there cannot be a newline or a blank <= the beginning of the 
// main body in TeX.

int LyXParagraph::BeginningOfMainBody() const
{
	if (FirstPhysicalPar() != this)
		return -1;
   
	// Unroll the first two cycles of the loop
	// and remember the previous character to
	// remove unnecessary GetChar() calls
	size_type i = 0;
	if (i < size()
	    && GetChar(i) != LyXParagraph::META_NEWLINE
		) {
		++i;
		char previous_char = 0, temp = 0; 
		if (i < size()
		    && (previous_char = GetChar(i)) != LyXParagraph::META_NEWLINE) {
			// Yes, this  ^ is supposed to be "= " not "=="
			++i;
			while (i < size()
			       && previous_char != ' '
			       && (temp = GetChar(i)) != LyXParagraph::META_NEWLINE) {
				++i;
				previous_char = temp;
			}
		}
	}

	if (i == 0 && i == size() &&
	    !(footnoteflag == LyXParagraph::NO_FOOTNOTE
	      && next && next->footnoteflag != LyXParagraph::NO_FOOTNOTE))
		++i;			       /* the cursor should not jump  
						* to the main body if there
						* is nothing in! */
	return i;
}


LyXParagraph * LyXParagraph::DepthHook(int deth)
{
	LyXParagraph * newpar = this;
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


LyXParagraph const * LyXParagraph::DepthHook(int deth) const
{
	LyXParagraph const * newpar = this;
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
	int count = 0;
	unsigned int i = 0;
	while (i < insetlist.size()) {
		if (insetlist[i].inset && insetlist[i].inset->AutoDelete()) {
			Erase(insetlist[i].pos); 
			// Erase() calls to insetlist.erase(&insetlist[i])
			// so i shouldn't be increased.
			++count;
		} else
			++i;
	}
	return count;
}


Inset * LyXParagraph::ReturnNextInsetPointer(LyXParagraph::size_type & pos)
{
	InsetList::iterator it = lower_bound(insetlist.begin(),
					     insetlist.end(),
					     InsetTable(pos, 0));
	if (it != insetlist.end()) {
		pos = (*it).pos;
		return (*it).inset;
	}
	return 0;
}


// returns -1 if inset not found
int LyXParagraph::GetPositionOfInset(Inset * inset) const
{
	// Find the entry.
	for (InsetList::const_iterator cit = insetlist.begin();
	     cit != insetlist.end(); ++cit) {
		if ((*cit).inset == inset) {
			return (*cit).pos;
		}
	}
	// Think about footnotes.
	if (footnoteflag == LyXParagraph::NO_FOOTNOTE 
	    && next && next->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE) {
		int further = 
			NextAfterFootnote()->GetPositionOfInset(inset);
		if (further != -1)
			return text.size() + 1 + further;
	}
	return -1;
}


LyXParagraph * LyXParagraph::TeXOnePar(ostream & os, TexRow & texrow,
				       ostream & foot,
				       TexRow & foot_texrow,
				       int & foot_count)
{
	lyxerr[Debug::LATEX] << "TeXOnePar...     " << this << endl;
	LyXLayout const & style =
		textclasslist.Style(current_view->buffer()->params.textclass,
				    layout);

	bool further_blank_line = false;
	if (IsDummy())
		lyxerr << "ERROR (LyXParagraph::TeXOnePar) is dummy." << endl;

	if (start_of_appendix) {
		os << "\\appendix\n";
		texrow.newline();
	}

	if (tex_code_break_column && style.isCommand()){
		os << '\n';
		texrow.newline();
	}

	if (pagebreak_top) {
		os << "\\newpage";
		further_blank_line = true;
	}
	if (added_space_top.kind() != VSpace::NONE) {
		os << added_space_top.asLatexCommand(current_view->buffer()->params);
		further_blank_line = true;
	}
      
	if (line_top) {
		os << "\\lyxline{\\" << getFont(0).latexSize() << '}'
		   << "\\vspace{-1\\parskip}";
		further_blank_line = true;
	}

	if (further_blank_line){
		os << '\n';
		texrow.newline();
	}

	Language const * language = getParLanguage();
	Language const * doc_language = current_view->buffer()->params.language_info;
	if (language != doc_language) {
		os << subst(lyxrc.language_command_begin, "$$lang",
			    language->lang)
		   << endl;
		texrow.newline();
	}
	
	switch (style.latextype) {
	case LATEX_COMMAND:
		os << '\\'
		   << style.latexname()
		   << style.latexparam();
		break;
	case LATEX_ITEM_ENVIRONMENT:
	        if (bibkey) {
			bibkey->Latex(os, false, false);
		} else
			os << "\\item ";
		break;
	case LATEX_LIST_ENVIRONMENT:
		os << "\\item ";
		break;
	default:
		break;
	}

	bool need_par = SimpleTeXOnePar(os, texrow);
 
	// Spit out footnotes
	LyXParagraph * par = next;
	if (lyxrc.rtl_support) {
		if (next && next->footnoteflag != LyXParagraph::NO_FOOTNOTE
		    && next->footnoteflag != footnoteflag) {
			LyXParagraph * p = 0;
			bool is_rtl = GetFontSettings(size()-1).isRightToLeft();
			if ( (p = NextAfterFootnote()) != 0 &&
			     p->GetFontSettings(0).isRightToLeft() != is_rtl)
				is_rtl = GetFontSettings(0).isRightToLeft();
			while (par &&
			       par->footnoteflag != LyXParagraph::NO_FOOTNOTE &&
			       par->footnoteflag != footnoteflag) {
				LyXDirection dir = (is_rtl)
					? LYX_DIR_RIGHT_TO_LEFT 
					: LYX_DIR_LEFT_TO_RIGHT;
				par = par->TeXFootnote(os, texrow, foot,
						       foot_texrow, foot_count,
						       dir);
				par->SimpleTeXOnePar(os, texrow);
				is_rtl = par->GetFontSettings(par->size()-1).isRightToLeft();
				if (par->next &&
				    par->next->footnoteflag != LyXParagraph::NO_FOOTNOTE &&
				    (p = par->NextAfterFootnote()) != 0 &&
				    p->GetFontSettings(0).isRightToLeft() != is_rtl)
					is_rtl = GetFontSettings(0).isRightToLeft();
				par = par->next;
			}
		}
	} else {
		while (par && par->footnoteflag != LyXParagraph::NO_FOOTNOTE
		       && par->footnoteflag != footnoteflag) {
			par = par->TeXFootnote(os, texrow,
					       foot, foot_texrow, foot_count,
					       LYX_DIR_LEFT_TO_RIGHT);
			par->SimpleTeXOnePar(os, texrow);
			par = par->next;
		}
	}

	// Make sure that \\par is done with the font of the last
	// character if this has another size as the default.
	// This is necessary because LaTeX (and LyX on the screen)
	// calculates the space between the baselines according
	// to this font. (Matthias)
	LyXFont font = getFont(Last() - 1);
	if (need_par) {
		if (style.resfont.size() != font.size()) {
			os << '\\'
			   << font.latexSize()
			   << ' ';
		}
		os << "\\par}";
	} else if (textclasslist.Style(current_view->buffer()->params.textclass,
				       GetLayout()).isCommand()){
		if (style.resfont.size() != font.size()) {
			os << '\\'
			   << font.latexSize()
			   << ' ';
		}
		os << '}';
	} else if (style.resfont.size() != font.size()){
		os << "{\\" << font.latexSize() << " \\par}";
	}

	if (language != doc_language) {
		os << endl 
		   << subst(lyxrc.language_command_end, "$$lang",
			    doc_language->lang);
	}
	
	switch (style.latextype) {
	case LATEX_ITEM_ENVIRONMENT:
	case LATEX_LIST_ENVIRONMENT:
                if (par && (depth < par->depth)) {
                        os << '\n';
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
			os << '\n';
			texrow.newline();
		}
	}
	
	further_blank_line = false;
	if (line_bottom) {
		os << "\\lyxline{\\" << getFont(Last() - 1).latexSize() << '}';
		further_blank_line = true;
	}

	if (added_space_bottom.kind() != VSpace::NONE) {
		os << added_space_bottom.asLatexCommand(current_view->buffer()->params);
		further_blank_line = true;
	}
      
	if (pagebreak_bottom) {
		os << "\\newpage";
		further_blank_line = true;
	}

	if (further_blank_line){
		os << '\n';
		texrow.newline();
	}

	if (!(footnoteflag != LyXParagraph::NO_FOOTNOTE && par &&
              par->footnoteflag == LyXParagraph::NO_FOOTNOTE)) {
		os << '\n';
		texrow.newline();
	}

	lyxerr[Debug::LATEX] << "TeXOnePar...done " << par << endl;
	return par;
}


// This one spits out the text of the paragraph
bool LyXParagraph::SimpleTeXOnePar(ostream & os, TexRow & texrow)
{
	lyxerr[Debug::LATEX] << "SimpleTeXOnePar...     " << this << endl;

	if (table)
		return SimpleTeXOneTablePar(os, texrow);

	bool return_value = false;

	LyXLayout const & style =
		textclasslist.Style(current_view->buffer()->params.textclass,
				    GetLayout());
	LyXFont basefont, last_font;

	// Maybe we have to create a optional argument.
	size_type main_body;
	if (style.labeltype != LABEL_MANUAL)
		main_body = 0;
	else
		main_body = BeginningOfMainBody();

	if (main_body > 0) {
		os << '[';
		basefont = getFont(-2); // Get label font
	} else {
		basefont = getFont(-1); // Get layout font
	}

	int column = 0;

	if (main_body >= 0
	    && !text.size()
	    && !IsDummy()) {
		if (style.isCommand()) {
			os << '{';
			++column;
		} else if (align != LYX_ALIGN_LAYOUT) {
			os << '{';
			++column;
			return_value = true;
		}
	}
 
	// Which font is currently active?
	LyXFont running_font(basefont);
	// Do we have an open font change?
	bool open_font = false;

	texrow.start(this, 0);

	for (size_type i = 0; i < size(); ++i) {
		++column;
		// First char in paragraph or after label?
		if (i == main_body && !IsDummy()) {
			if (main_body > 0) {
				if (open_font) {
					column += running_font.latexWriteEndChanges(os, basefont, basefont);
					open_font = false;
				}
				basefont = getFont(-1); // Now use the layout font
				running_font = basefont;
				os << ']';
				++column;
			}
			if (style.isCommand()) {
				os << '{';
				++column;
			} else if (align != LYX_ALIGN_LAYOUT) {
				os << "{\\par";
				column += 4;
				return_value = true;
			}

			if (noindent) {
				os << "\\noindent ";
				column += 10;
			}
			switch (align) {
			case LYX_ALIGN_NONE:
			case LYX_ALIGN_BLOCK:
			case LYX_ALIGN_LAYOUT:
			case LYX_ALIGN_SPECIAL:
				break;
			case LYX_ALIGN_LEFT:
				if (getParLanguage()->lang != "hebrew") {
					os << "\\raggedright ";
					column+= 13;
				} else {
					os << "\\raggedleft ";
					column+= 12;
				}
				break;
			case LYX_ALIGN_RIGHT:
				if (getParLanguage()->lang != "hebrew") {
					os << "\\raggedleft ";
					column+= 12;
				} else {
					os << "\\raggedright ";
					column+= 13;
				}
				break;
			case LYX_ALIGN_CENTER:
				os << "\\centering ";
				column+= 11;
				break;
			}	 
		}

		int c = GetChar(i);

		// Fully instantiated font
		LyXFont font = getFont(i);
		LyXParagraph * p = 0;
		if (i == 0 && previous && 
		    previous->footnoteflag != LyXParagraph::NO_FOOTNOTE &&
		    (p = PreviousBeforeFootnote()) != 0)
			last_font = p->getFont(p->size()-1);
		else
			last_font = running_font;

		// Spaces at end of font change are simulated to be
		// outside font change, i.e. we write "\textXX{text} "
		// rather than "\textXX{text }". (Asger)
		if (open_font && c == ' ' && i <= size() - 2 
		    && !getFont(i+1).equalExceptLatex(running_font) 
		    && !getFont(i+1).equalExceptLatex(font)) {
			font = getFont(i + 1);
		}
		// We end font definition before blanks
		if (!font.equalExceptLatex(running_font) && open_font) {
			column += running_font.latexWriteEndChanges(os,
								    basefont,
								    (i == main_body-1) ? basefont : font);
			running_font = basefont;
			open_font = false;
		}

		// Blanks are printed before start of fontswitch
		if (c == ' '){
			// Do not print the separation of the optional argument
			if (i != main_body - 1) {
				SimpleTeXBlanks(os, texrow, i,
						column, font, style);
			}
		}

		// Do we need to change font?
		if (!font.equalExceptLatex(running_font)
		    && i != main_body-1) {
			column += font.latexWriteStartChanges(os, basefont,
							      last_font);
			running_font = font;
			open_font = true;
		}

		if (c == LyXParagraph::META_NEWLINE) {
			// newlines are handled differently here than
			// the default in SimpleTeXSpecialChars().
			if (!style.newline_allowed
			    || font.latex() == LyXFont::ON) {
				os << '\n';
			} else {
				if (open_font) {
					column += running_font.latexWriteEndChanges(os, basefont, basefont);
					open_font = false;
				}
				basefont = getFont(-1);
				running_font = basefont;
				if (font.family() == 
				    LyXFont::TYPEWRITER_FAMILY) {
					os << "~";
				}
				os << "\\\\\n";
			}
			texrow.newline();
			texrow.start(this, i + 1);
			column = 0;
		} else {
			SimpleTeXSpecialChars(os, texrow,
					      font, running_font, basefont,
					      open_font, style, i, column, c);
		}
	}

	// If we have an open font definition, we have to close it
	if (open_font) {
		LyXParagraph * p = 0;
		if (next && next->footnoteflag != LyXParagraph::NO_FOOTNOTE
		    && (p =  NextAfterFootnote()) != 0)
			running_font.latexWriteEndChanges(os, basefont,
							  p->getFont(0));
		else
			running_font.latexWriteEndChanges(os, basefont, basefont);
	}

	// Needed if there is an optional argument but no contents.
	if (main_body > 0 && main_body == size()) {
		os << "]~";
		return_value = false;
	}

	lyxerr[Debug::LATEX] << "SimpleTeXOnePar...done " << this << endl;
	return return_value;
}


// This one spits out the text of a table paragraph
bool LyXParagraph::SimpleTeXOneTablePar(ostream & os, TexRow & texrow)
{
	lyxerr[Debug::LATEX] << "SimpleTeXOneTablePar...     " << this << endl;
   
	bool return_value = false;

	LyXLayout const & style = 
		textclasslist.Style(current_view->buffer()->params.textclass,
				    GetLayout());
 
	int column = 0;
	if (!IsDummy()) { // it is dummy if it is in a float!!!
		if (style.isCommand()) {
			os << '{';
			++column;
		} else if (align != LYX_ALIGN_LAYOUT) {
			os << '{';
			++column;
			return_value = true;
		}
		if (noindent) {
			os << "\\noindent ";
			column += 10;
		}
		switch (align) {
		case LYX_ALIGN_NONE:
		case LYX_ALIGN_BLOCK:
		case LYX_ALIGN_LAYOUT:
		case LYX_ALIGN_SPECIAL: break;
		case LYX_ALIGN_LEFT:
			os << "\\raggedright ";
			column+= 13;
			break;
		case LYX_ALIGN_RIGHT:
			os << "\\raggedleft ";
			column+= 12;
			break;
		case LYX_ALIGN_CENTER:
			os << "\\centering ";
			column+= 11;
			break;
		}
	}

	LyXFont basefont = getFont(-1); // Get layout font
	// Which font is currently active?
	LyXFont running_font = basefont;
	LyXFont last_font;
	// Do we have an open font change?
	bool open_font = false;
	int current_cell_number = -1;
	int tmp = table->TexEndOfCell(os, current_cell_number);
	for (; tmp > 0 ; --tmp)
		texrow.newline();
	
	texrow.start(this, 0);

	for (size_type i = 0; i < size(); ++i) {
		char c = GetChar(i);
		if (table->IsContRow(current_cell_number + 1)) {
			if (c == LyXParagraph::META_NEWLINE)
				++current_cell_number;
			continue;
		}
		++column;
		
		// Fully instantiated font
		LyXFont font = getFont(i);
		last_font = running_font;

		// Spaces at end of font change are simulated to be
		// outside font change.
		// i.e. we write "\textXX{text} " rather than
		// "\textXX{text }". (Asger)
		if (open_font && c == ' ' && i <= size() - 2
		    && getFont(i + 1) != running_font
		    && getFont(i + 1) != font) {
			font = getFont(i + 1);
		}

		// We end font definition before blanks
		if (font != running_font && open_font) {
			column += running_font.latexWriteEndChanges(os,
								    basefont,
								    font);
			running_font = basefont;
			open_font = false;
		}
		// Blanks are printed before start of fontswitch
		if (c == ' '){
			SimpleTeXBlanks(os, texrow, i, column, font, style);
		}
		// Do we need to change font?
		if (font != running_font) {
			column += font.latexWriteStartChanges(os, basefont,
							      last_font);
			running_font = font;
			open_font = true;
		}
		// Do we need to turn on LaTeX mode?
		if (font.latex() != running_font.latex()) {
			if (font.latex() == LyXFont::ON
			    && style.needprotect) {
				os << "\\protect ";
				column += 9;
			}
		}
		if (c == LyXParagraph::META_NEWLINE) {
			// special case for inside a table
			// different from default case in
			// SimpleTeXSpecialChars()
			if (open_font) {
				column += running_font
					.latexWriteEndChanges(os, basefont,
							      basefont);
				open_font = false;
			}
			basefont = getFont(-1);
			running_font = basefont;
			++current_cell_number;
			if (table->CellHasContRow(current_cell_number) >= 0) {
				TeXContTableRows(os, i + 1,
						 current_cell_number,
						 column, texrow);
			}
			// if this cell follow only ContRows till end don't
			// put the EndOfCell because it is put after the
			// for(...)
			if (table->ShouldBeVeryLastCell(current_cell_number)) {
				--current_cell_number;
				break;
			}
			int tmp = table->TexEndOfCell(os,
						      current_cell_number);
			if (tmp > 0) {
				column = 0;
			} else if (tmp < 0) {
				tmp = -tmp;
			}
			for (; tmp--;) {
				texrow.newline();
			}
			texrow.start(this, i + 1);
		} else {
			SimpleTeXSpecialChars(os, texrow,
					      font, running_font, basefont,
					      open_font, style, i, column, c);
		}
	}

	// If we have an open font definition, we have to close it
	if (open_font) {
		running_font.latexWriteEndChanges(os, basefont, basefont);
	}
	++current_cell_number;
	tmp = table->TexEndOfCell(os, current_cell_number);
	for (; tmp > 0; --tmp)
		texrow.newline();
	lyxerr[Debug::LATEX] << "SimpleTeXOneTablePar...done " << this << endl;
	return return_value;
}


// This one spits out the text off ContRows in tables
bool LyXParagraph::TeXContTableRows(ostream & os,
				    LyXParagraph::size_type i,
				    int current_cell_number,
				    int & column, TexRow & texrow)
{
	lyxerr[Debug::LATEX] << "TeXContTableRows...     " << this << endl;
	if (!table)
		return false;
    
	char c;
   
	bool return_value = false;
	LyXLayout const & style =
		textclasslist.Style(current_view->buffer()->params.textclass,
				    GetLayout());
	LyXFont basefont = getFont(-1); // Get layout font
	LyXFont last_font;
	// Which font is currently active?
	LyXFont running_font = basefont;
	// Do we have an open font change?
	bool open_font = false;

	size_type lastpos = i;
	int cell = table->CellHasContRow(current_cell_number);
	++current_cell_number;
	while(cell >= 0) {
		// first find the right position
		i = lastpos;
		for (; (i < size()) && (current_cell_number<cell); ++i) {
			c = GetChar(i);
			if (c == LyXParagraph::META_NEWLINE)
				++current_cell_number;
		}
		lastpos = i;
		c = GetChar(i);
		if (table->Linebreaks(table->FirstVirtualCell(cell))) {
			os << " \\\\\n";
			texrow.newline();
			column = 0;
		} else if ((c != ' ') && (c != LyXParagraph::META_NEWLINE)) {
			os << ' ';
		}

		for (; i < size()
			     && (c = GetChar(i)) != LyXParagraph::META_NEWLINE;
		     ++i) {
			++column;

			// Fully instantiated font
			LyXFont font = getFont(i);
			last_font = running_font;

			// Spaces at end of font change are simulated to
			// be outside font change. i.e. we write
			// "\textXX{text} " rather than "\textXX{text }".
			// (Asger)
			if (open_font && c == ' ' && i <= size() - 2 
			    && getFont(i + 1) != running_font
			    && getFont(i + 1) != font) {
				font = getFont(i + 1);
			}

			// We end font definition before blanks
			if (font != running_font && open_font) {
				column += running_font.latexWriteEndChanges(os, basefont, font);
				running_font = basefont;
				open_font = false;
			}
			// Blanks are printed before start of fontswitch
			if (c == ' '){
				SimpleTeXBlanks(os, texrow, i,
						column, font, style);
			}
			// Do we need to change font?
			if (font != running_font) {
				column +=
					font.latexWriteStartChanges(os,
								    basefont,
								    last_font);
				running_font = font;
				open_font = true;
			}
			// Do we need to turn on LaTeX mode?
			if (font.latex() != running_font.latex()) {
				if (font.latex() == LyXFont::ON
				    && style.needprotect) {
					os << "\\protect ";
					column += 9;
				}
			}
			SimpleTeXSpecialChars(os, texrow, font,
					      running_font, basefont,
					      open_font, style, i, column, c);
		}
		// If we have an open font definition, we have to close it
		if (open_font) {
			running_font.latexWriteEndChanges(os, basefont,
							  basefont);
			open_font = false;
		}
		basefont = getFont(-1);
		running_font = basefont;
		cell = table->CellHasContRow(current_cell_number);
	}
	lyxerr[Debug::LATEX] << "TeXContTableRows...done " << this << endl;
	return return_value;
}


bool LyXParagraph::linuxDocConvertChar(char c, string & sgml_string)
{
	bool retval = false;
	switch (c) {
	case LyXParagraph::META_HFILL:
		sgml_string.clear();
		break;
#if 0
	case LyXParagraph::META_PROTECTED_SEPARATOR: 
		sgml_string = ' ';
		break;
#endif
	case LyXParagraph::META_NEWLINE:
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
	case '\0': // Ignore :-)
		sgml_string.clear();
		break;
	default:
		sgml_string = c;
		break;
	}
	return retval;
}


void LyXParagraph::SimpleDocBookOneTablePar(ostream & os, string & extra,
					    int & desc_on, int depth) 
{
	if (!table) return;
	lyxerr[Debug::LATEX] << "SimpleDocbookOneTablePar... " << this << endl;
	int column = 0;
	LyXFont font1, font2;
	char c;
	Inset * inset;
	size_type main_body;
	bool emph_flag = false;
	
	LyXLayout const & style =
		textclasslist.Style(current_view->buffer()->params.textclass,
				    GetLayout());
	
	if (style.labeltype != LABEL_MANUAL)
		main_body = 0;
	else
		main_body = BeginningOfMainBody();
	
	// Gets paragraph main font.
	if (main_body > 0)
		font1 = style.labelfont;
	else
		font1 = style.font;
	
	int char_line_count = depth;
	os << newlineAndDepth(depth);
	if (footnoteflag == LyXParagraph::NO_FOOTNOTE) {
		os << "<INFORMALTABLE>"
		   << newlineAndDepth(++depth);
	}
	int current_cell_number = -1;
	int tmp = table->DocBookEndOfCell(os, current_cell_number, depth);
	
	// Parsing main loop.
	for (size_type i = 0; i < size(); ++i) {
		c = GetChar(i);
		if (table->IsContRow(current_cell_number+1)) {
			if (c == LyXParagraph::META_NEWLINE)
				++current_cell_number;
			continue;
		}
		++column;
		
		// Fully instantiated font
		font2 = getFont(i);
		
		// Handle <emphasis> tag.
		if (font1.emph() != font2.emph() && i) {
			if (font2.emph() == LyXFont::ON) {
				os << "<emphasis>";
				emph_flag= true;
			} else if (emph_flag) {
				os << "</emphasis>";
				emph_flag= false;
			}
		}
		if (c == LyXParagraph::META_NEWLINE) {
			// We have only to control for emphasis open here!
			if (emph_flag) {
				os << "</emphasis>";
				emph_flag= false;
			}
			font1 = font2 = getFont(-1);
			++current_cell_number;
			if (table->CellHasContRow(current_cell_number) >= 0) {
				DocBookContTableRows(os, extra, desc_on, i + 1,
						     current_cell_number,
						     column);
			}
			// if this cell follow only ContRows till end don't
			// put the EndOfCell because it is put after the
			// for(...)
			if (table->ShouldBeVeryLastCell(current_cell_number)) {
				--current_cell_number;
				break;
			}
			tmp = table->DocBookEndOfCell(os,
						      current_cell_number,
						      depth);
			
			if (tmp > 0)
				column = 0;
		} else if (c == LyXParagraph::META_INSET) {
			inset = GetInset(i);
#ifdef HAVE_SSTREAM
			std::ostringstream ost;
			inset->DocBook(ost);
			string tmp_out = ost.str().c_str();
#else
			ostrstream ost;
			inset->DocBook(ost);
			ost << '\0';
			char * ctmp = ost.str();
			string tmp_out(ctmp);
			delete [] ctmp;
#endif
			//
			// This code needs some explanation:
			// Two insets are treated specially
			//   label if it is the first element in a
			//   command paragraph
			//         desc_on == 3
			//   graphics inside tables or figure floats
			//   can't go on
			//   title (the equivalente in latex for this
			//   case is caption
			//   and title should come first
			//         desc_on == 4
			//
			if(desc_on != 3 || i != 0) {
				if(tmp_out[0] == '@') {
					if(desc_on == 4)
						extra += frontStrip(tmp_out,
								    '@');
					else
						os << frontStrip(tmp_out,
								 '@');
				} else
					os << tmp_out;
			}
		} else if (font2.latex() == LyXFont::ON) {
			// "TeX"-Mode on == > SGML-Mode on.
			if (c != '\0')
				os << c;
			++char_line_count;
		} else {
			string sgml_string;
			if (linuxDocConvertChar(c, sgml_string) 
			    && !style.free_spacing) {
				// in freespacing mode, spaces are
				// non-breaking characters
				// char is ' '
				if (desc_on == 1) {
					++char_line_count;
					os << '\n'
					   << "</term><listitem><para>";
					desc_on = 2;
				} else  {
					os << c;
				}
			} else {
				os << sgml_string;
			}
		}
		font1 = font2;
	}
	
	// Needed if there is an optional argument but no contents.
	if (main_body > 0 && main_body == size()) {
		font1 = style.font;
	}

	if (emph_flag) {
	        os << "</emphasis>";
	}
	
	++current_cell_number;
	tmp = table->DocBookEndOfCell(os, current_cell_number, depth);
	// Resets description flag correctly.
	switch(desc_on){
	case 1:
		// <term> not closed...
		os << "</term>";
		break;
	}
	if (footnoteflag == LyXParagraph::NO_FOOTNOTE)
		os << "</INFORMALTABLE>";
	os << '\n';
	lyxerr[Debug::LATEX] << "SimpleDocbookOneTablePar...done "
			     << this << endl;
}


void LyXParagraph::DocBookContTableRows(ostream & os, string & extra,
                                        int & desc_on,
					LyXParagraph::size_type i,
                                        int current_cell_number, int &column) 

{
	if (!table) return;
	
	lyxerr[Debug::LATEX] << "DocBookContTableRows... " << this << endl;

	LyXFont font2;
	char c;
	Inset * inset;
	//string emph = "emphasis";
	bool emph_flag = false;
	int char_line_count = 0;
	
	LyXLayout const & style =
		textclasslist.Style(current_view->buffer()->params.textclass,
				    GetLayout());
	
	size_type main_body;
	if (style.labeltype != LABEL_MANUAL)
		main_body = 0;
	else
		main_body = BeginningOfMainBody();
	
	// Gets paragraph main font.
	LyXFont font1;
	if (main_body > 0)
		font1 = style.labelfont;
	else
		font1 = style.font;
	
	size_type lastpos = i;
	int cell = table->CellHasContRow(current_cell_number);
	++current_cell_number;
	while(cell >= 0) {
		// first find the right position
		i = lastpos;
		for (; i < size() && current_cell_number < cell; ++i) {
			c = GetChar(i);
			if (c == LyXParagraph::META_NEWLINE)
				++current_cell_number;
		}
		lastpos = i;
		c = GetChar(i);
		// I don't know how to handle this so I comment it
                // for the moment (Jug)
//             if (table->Linebreaks(table->FirstVirtualCell(cell))) {
//                     file += " \\\\\n";
//                     column = 0;
//             } else
		if ((c != ' ') && (c != LyXParagraph::META_NEWLINE)) {
			os << ' ';
		}

		for (; i < size()
			     && (c = GetChar(i)) != LyXParagraph::META_NEWLINE;
		     ++i) {
			++column;
			
			// Fully instantiated font
			font2 = getFont(i);
			
			// Handle <emphasis> tag.
			if (font1.emph() != font2.emph() && i) {
				if (font2.emph() == LyXFont::ON) {
					os << "<emphasis>";
					emph_flag= true;
				} else if (emph_flag) {
					os << "</emphasis>";
					emph_flag= false;
				}
			}
			if (c == LyXParagraph::META_INSET) {
				inset = GetInset(i);
#ifdef HAVE_SSTREAM
				std::ostringstream ost;
				inset->DocBook(ost);
				string tmp_out = ost.str().c_str();
#else
				ostrstream ost;
				inset->DocBook(ost);
				ost << '\0';
				char * ctmp = ost.str();
				string tmp_out(ctmp);
				delete [] ctmp;
#endif
				//
				// This code needs some explanation:
				// Two insets are treated specially
				//   label if it is the first element in a
				//   command paragraph
				//       desc_on == 3
				//   graphics inside tables or figure floats
				//   can't go on title (the equivalente in
				//   latex for this case is caption and title
				//   should come first
				//       desc_on == 4
				//
				if(desc_on != 3 || i != 0) {
					if(tmp_out[0] == '@') {
						if(desc_on == 4)
							extra += frontStrip(tmp_out, '@');
						else
							os << frontStrip(tmp_out, '@');
					} else
						os << tmp_out;
				}
			} else if (font2.latex() == LyXFont::ON) {
				// "TeX"-Mode on == > SGML-Mode on.
				if (c!= '\0')
					os << c;
				++char_line_count;
			} else {
				string sgml_string;
				if (linuxDocConvertChar(c, sgml_string) 
				    && !style.free_spacing) {
					// in freespacing mode, spaces are
					// non-breaking characters
					// char is ' '
					if (desc_on == 1) {
						++char_line_count;
						os << '\n'
						   << "</term><listitem><para>";
						desc_on = 2;
					} else  {
						os << c;
					}
				} else {
					os << sgml_string;
				}
			}
		}
		// we have only to control for emphasis open here!
		if (emph_flag) {
			os << "</emphasis>";
			emph_flag= false;
		}
		font1 = font2 = getFont(-1);
		cell = table->CellHasContRow(current_cell_number);
	}
	lyxerr[Debug::LATEX] << "DocBookContTableRows...done " << this << endl;
}


void LyXParagraph::SimpleTeXBlanks(ostream & os, TexRow & texrow,
				   LyXParagraph::size_type const i,
				   int & column, LyXFont const & font,
				   LyXLayout const & style)
{
	if (column > tex_code_break_column
	    && i 
	    && GetChar(i - 1) != ' '
	    && (i < size() - 1)
	    // In LaTeX mode, we don't want to
	    // break lines since some commands
	    // do not like this
	    && ! (font.latex() == LyXFont::ON)
	    // same in FreeSpacing mode
	    && !style.free_spacing
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
			os << " %\n";
		} else {
			os << '\n';
		}
		texrow.newline();
		texrow.start(this, i + 1);
		column = 0;
	} else if (font.latex() == LyXFont::OFF) {
		if (style.free_spacing) {
			os << '~';
		} else {
			os << ' ';
		}
	}
}


void LyXParagraph::SimpleTeXSpecialChars(ostream & os, TexRow & texrow,
					 LyXFont & font,
					 LyXFont & running_font,
					 LyXFont & basefont,
					 bool & open_font,
					 LyXLayout const & style,
					 LyXParagraph::size_type & i,
					 int & column, char const c)
{
	// Two major modes:  LaTeX or plain
	// Handle here those cases common to both modes
	// and then split to handle the two modes separately.
	switch (c) {
	case LyXParagraph::META_INSET: {
		Inset * inset = GetInset(i);
		if (inset) {
			bool close = false;
			int len = os.tellp();
			if ((inset->LyxCode() == Inset::GRAPHICS_CODE
			     || inset->LyxCode() == Inset::MATH_CODE
			     || inset->LyxCode() == Inset::URL_CODE)
			    && running_font.isRightToLeft()) {
				os << "\\L{";
				close = true;
			} else if (inset->LyxCode() == Inset::NUMBER_CODE
				   && running_font.isRightToLeft()) {
				os << "{\\beginL ";
				close = true;
			}

			int tmp = inset->Latex(os, style.isCommand(),
					       style.free_spacing);

			if (close)
				if (inset->LyxCode() == Inset::NUMBER_CODE)
					os << "\\endL}";
				else
					os << "}";

			if (tmp) {
				column = 0;
			} else {
				column += os.tellp() - len;
			}
			for (; tmp--;) {
				texrow.newline();
			}
		}
	}
	break;

	case LyXParagraph::META_NEWLINE:
		if (open_font) {
			column += running_font.latexWriteEndChanges(os,
								    basefont,
								    basefont);
			open_font = false;
		}
		basefont = getFont(-1);
		running_font = basefont;
		break;

	case LyXParagraph::META_HFILL: 
		os << "\\hfill{}";
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
#if 0
			case LyXParagraph::META_PROTECTED_SEPARATOR: 
			        os << ' ';
			        break;
#endif
			default:
				// make sure that we will not print
				// error generating chars to the tex
				// file. This test would not be needed
				// if it were done in the buffer
				// itself.
				if (c != '\0') {
					os << c;
				}
				break;
			}
		} else {
			// Plain mode (i.e. not LaTeX)
			switch (c) {
#if 0
			case LyXParagraph::META_PROTECTED_SEPARATOR: 
			        os << '~';
			        break;
#endif
			case '\\': 
				os << "\\textbackslash{}";
				column += 15;
				break;
		
			case '°': case '±': case '²': case '³':  
			case '×': case '÷': case '¹': case 'ª':
			case 'º': case '¬': case 'µ':
				if (current_view->buffer()->params.inputenc == "latin1") {
					os << "\\ensuremath{"
					   << c
					   << '}';
					column += 13;
				} else {
					os << c;
				}
				break;

			case '|': case '<': case '>':
				// In T1 encoding, these characters exist
				if (lyxrc.fontenc == "T1") {
					os << c;
					//... but we should avoid ligatures
					if ((c == '>' || c == '<')
					    && i <= size() - 2
					    && GetChar(i + 1) == c){
						os << "\\textcompwordmark{}";
						column += 19;
					}
					break;
				}
				// Typewriter font also has them
				if (font.family() == LyXFont::TYPEWRITER_FAMILY) {
					os << c;
					break;
				} 
				// Otherwise, we use what LaTeX
				// provides us.
				switch(c) {
				case '<':
					os << "\\textless{}";
					column += 10;
					break;
				case '>':
					os << "\\textgreater{}";
					column += 13;
					break;
				case '|':
					os << "\\textbar{}";
					column += 9;
					break;
				}
				break;

			case '-': // "--" in Typewriter mode -> "-{}-"
				if (i <= size() - 2
				    && GetChar(i + 1) == '-'
				    && font.family() == LyXFont::TYPEWRITER_FAMILY) {
					os << "-{}";
					column += 2;
				} else {
					os << '-';
				}
				break;

			case '\"': 
				os << "\\char`\\\"{}";
				column += 9;
				break;

			case '£':
				if (current_view->buffer()->params.inputenc == "default") {
					os << "\\pounds{}";
					column += 8;
				} else {
					os << c;
				}
				break;

			case '$': case '&':
			case '%': case '#': case '{':
			case '}': case '_':
				os << '\\' << c;
				column += 1;
				break;

			case '~':
				os << "\\textasciitilde{}";
				column += 16;
				break;

			case '^':
				os << "\\textasciicircum{}";
				column += 17;
				break;

			case '*': case '[': case ']':
				// avoid being mistaken for optional arguments
				os << '{' << c << '}';
				column += 2;
				break;

			case ' ':
				// Blanks are printed before font switching.
				// Sure? I am not! (try nice-latex)
				// I am sure it's correct. LyX might be smarter
				// in the future, but for now, nothing wrong is
				// written. (Asger)
				break;

			default:
				/* idea for labels --- begin*/
				// Check for "LyX"
				if (c ==  'L'
				    && i <= size() - 3
				    && font.family() != LyXFont::TYPEWRITER_FAMILY
				    && GetChar(i + 1) == 'y'
				    && GetChar(i + 2) == 'X') {
					os << "\\LyX{}";
					i += 2;
					column += 5;
				}
				// Check for "TeX"
				else if (c == 'T'
					 && i <= size() - 3
					 && font.family() != LyXFont::TYPEWRITER_FAMILY
					 && GetChar(i + 1) == 'e'
					 && GetChar(i + 2) == 'X') {
					os << "\\TeX{}";
					i += 2;
					column += 5;
				}
				// Check for "LaTeX2e"
				else if (c == 'L'
					 && i <= size() - 7
					 && font.family() != LyXFont::TYPEWRITER_FAMILY
					 && GetChar(i + 1) == 'a'
					 && GetChar(i + 2) == 'T'
					 && GetChar(i + 3) == 'e'
					 && GetChar(i + 4) == 'X'
					 && GetChar(i + 5) == '2'
					 && GetChar(i + 6) == 'e') {
					os << "\\LaTeXe{}";
					i += 6;
					column += 8;
				}
				// Check for "LaTeX"
				else if (c == 'L'
					 && i <= size() - 5
					 && font.family() != LyXFont::TYPEWRITER_FAMILY
					 && GetChar(i + 1) == 'a'
					 && GetChar(i + 2) == 'T'
					 && GetChar(i + 3) == 'e'
					 && GetChar(i + 4) == 'X') {
					os << "\\LaTeX{}";
					i += 4;
					column += 7;
					/* idea for labels --- end*/ 
				} else if (c != '\0') {
					os << c;
				}
				break;
			}
		}
	}
}


#if 0
bool LyXParagraph::RoffContTableRows(ostream & os,
				     LyXParagraph::size_type i,
				     int actcell)
{
	if (!table)
		return false;

	LyXFont font1(LyXFont::ALL_INHERIT);
	LyXFont	font2;
	Inset * inset;
	char c;

	string fname2 = TmpFileName(string(), "RAT2");
	int lastpos = i;
	int cell = table->CellHasContRow(actcell);
	++actcell;
	while(cell >= 0) {
		// first find the right position
		i = lastpos;
		for (; i < size() && actcell < cell; ++i) {
			c = GetChar(i);
			if (c == LyXParagraph::META_NEWLINE)
				++actcell;
		}
		lastpos = i;
		c = GetChar(i);
		if ((c != ' ') && (c != LyXParagraph::META_NEWLINE))
			os << " ";
		for (; i < size()
			     && (c = GetChar(i)) != LyXParagraph::META_NEWLINE;
		     ++i) {
			font2 = GetFontSettings(i);
			if (font1.latex() != font2.latex()) {
				if (font2.latex() != LyXFont::OFF)
					continue;
			}
			c = GetChar(i);
			switch (c) {
			case LyXParagraph::META_INSET:
				if ((inset = GetInset(i))) {
#if 1
#ifdef HAVE_SSTREAM
					stringstream ss(ios::in | ios::out);
					inset->Latex(ss, -1);
					ss.seekp(0);
					ss.get(c);
					while (!ss) {
						if (c == '\\')
							os << "\\\\";
						else
							os << c;
						ss.get(c);
					}
#else
					strstream ss;
					inset->Latex(ss, -1);
					ss.seekp(0);
					ss.get(c);
					while (!ss) {
						if (c == '\\')
							os << "\\\\";
						else
							os << c;
						ss.get(c);
					}
					delete [] ss.str();
#endif
#else
					fstream fs(fname2.c_str(),
						   ios::in|ios::out);
					if (!fs) {
						WriteAlert(_("LYX_ERROR:"),
							   _("Cannot open temporary file:"),
							   fname2);
						return false;
					}
					inset->Latex(fs, -1);
					fs.seekp(0);
					fs.get(c);
					while (!fs) {
						if (c == '\\')
							os << "\\\\";
						else
							os << c;
						fs.get(c);
					}
					fs.close();
#endif
				}
				break;
			case LyXParagraph::META_NEWLINE:
				break;
			case LyXParagraph::META_HFILL: 
				break;
#if 0
			case LyXParagraph::META_PROTECTED_SEPARATOR:
				break;
#endif
			case '\\': 
				os << "\\\\";
				break;
			default:
				if (c != '\0')
					os << c;
				else
					lyxerr.debug() << "RoffAsciiTable: "
						"NULL char in structure."
						       << endl;
				break;
			}
		}
		cell = table->CellHasContRow(actcell);
	}
	return true;
}
#endif


LyXParagraph * LyXParagraph::TeXDeeper(ostream & os, TexRow & texrow,
				       ostream & foot,
				       TexRow & foot_texrow,
				       int & foot_count)
{
	lyxerr[Debug::LATEX] << "TeXDeeper...     " << this << endl;
	LyXParagraph * par = this;

	while (par && par->depth == depth) {
		if (par->IsDummy())
			lyxerr << "ERROR (LyXParagraph::TeXDeeper)" << endl;
		if (textclasslist.Style(current_view->buffer()->params.textclass, 
					par->layout).isEnvironment()
		    || par->pextra_type != PEXTRA_NONE) {
			par = par->TeXEnvironment(os, texrow,
						  foot, foot_texrow,
						  foot_count);
		} else {
			par = par->TeXOnePar(os, texrow,
					     foot, foot_texrow,
					     foot_count);
		}
	}
	lyxerr[Debug::LATEX] << "TeXDeeper...done " << par << endl;

	return par;
}


LyXParagraph * LyXParagraph::TeXEnvironment(ostream & os, TexRow & texrow,
					    ostream & foot,
					    TexRow & foot_texrow,
					    int & foot_count)
{
	bool eindent_open = false;
	bool foot_this_level = false;
	// flags when footnotetext should be appended to file.
        static bool minipage_open = false;
        static int minipage_open_depth = 0;
	char par_sep = current_view->buffer()->params.paragraph_separation;
    
	lyxerr[Debug::LATEX] << "TeXEnvironment...     " << this << endl;
	if (IsDummy())
		lyxerr << "ERROR (LyXParagraph::TeXEnvironment)" << endl;

	LyXLayout const & style =
		textclasslist.Style(current_view->buffer()->params.textclass,
				    layout);
       
	if (pextra_type == PEXTRA_INDENT) {
		if (!pextra_width.empty()) {
			os << "\\begin{LyXParagraphIndent}{"
			   << pextra_width << "}\n";
		} else {
			//float ib = atof(pextra_widthp.c_str())/100;
			// string can't handle floats at present (971109)
			// so I'll do a conversion by hand knowing that
			// the limits are 0.0 to 1.0. ARRae.
			os << "\\begin{LyXParagraphIndent}{";
			switch (pextra_widthp.length()) {
			case 3:
				os << "1.00";
				break;
			case 2:
				os << "0."
				   << pextra_widthp;
				break;
			case 1:
				os << "0.0"
				   << pextra_widthp;
			}
			os << "\\columnwidth}\n";
		}
		texrow.newline();
		eindent_open = true;
	}
	if ((pextra_type == PEXTRA_MINIPAGE) && !minipage_open) {
		if (pextra_hfill && Previous() &&
		    (Previous()->pextra_type == PEXTRA_MINIPAGE)) {
			os << "\\hfill{}\n";
			texrow.newline();
		}
		if (par_sep == BufferParams::PARSEP_INDENT) {
			os << "{\\setlength\\parindent{0pt}\n";
			texrow.newline();
		}
		os << "\\begin{minipage}";
		switch(pextra_alignment) {
		case MINIPAGE_ALIGN_TOP:
			os << "[t]";
			break;
		case MINIPAGE_ALIGN_MIDDLE:
			os << "[m]";
			break;
		case MINIPAGE_ALIGN_BOTTOM:
			os << "[b]";
			break;
		}
		if (!pextra_width.empty()) {
			os << '{' << pextra_width << "}\n";
		} else {
			//float ib = atof(par->pextra_width.c_str())/100;
			// string can't handle floats at present
			// so I'll do a conversion by hand knowing that
			// the limits are 0.0 to 1.0. ARRae.
			os << '{';
			switch (pextra_widthp.length()) {
			case 3:
				os << "1.00";
				break;
			case 2:
				os << "0."
				   << pextra_widthp;
				break;
			case 1:
				os << "0.0"
				   << pextra_widthp;
			}
			os << "\\columnwidth}\n";
		}
		texrow.newline();
		if (par_sep == BufferParams::PARSEP_INDENT) {
			os << "\\setlength\\parindent{\\LyXMinipageIndent}\n";
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
	if (style.isEnvironment()){
		if (style.latextype == LATEX_LIST_ENVIRONMENT) {
#ifdef FANCY_FOOTNOTE_CODE
			if (foot_count < 0) {
				// flag that footnote[mark][text] should be
				// used for any footnotes from now on
				foot_count = 0;
				foot_this_level = true;
			}
#endif
			os << "\\begin{" << style.latexname() << "}{"
			   << labelwidthstring << "}\n";
		} else if (style.labeltype == LABEL_BIBLIO) {
			// ale970405
			os << "\\begin{" << style.latexname() << "}{"
			   << bibitemWidthest(current_view->painter())
			   << "}\n";
		} else if (style.latextype == LATEX_ITEM_ENVIRONMENT) {
#ifdef FANCY_FOOTNOTE_CODE
			if (foot_count < 0) {
				// flag that footnote[mark][text] should be
				// used for any footnotes from now on
				foot_count = 0;
				foot_this_level = true;
			}
#endif
			os << "\\begin{" << style.latexname() << '}'
			   << style.latexparam() << '\n';
		} else 
			os << "\\begin{" << style.latexname() << '}'
			   << style.latexparam() << '\n';
		texrow.newline();
	}
	LyXParagraph * par = this;
	do {
		par = par->TeXOnePar(os, texrow,
				     foot, foot_texrow, foot_count);

                if (minipage_open && par && !style.isEnvironment() &&
                    (par->pextra_type == PEXTRA_MINIPAGE) &&
                    par->pextra_start_minipage) {
			os << "\\end{minipage}\n";
			texrow.newline();
			if (par_sep == BufferParams::PARSEP_INDENT) {
				os << "}\n";
				texrow.newline();
			}
			minipage_open = false;
                }
		if (par && par->depth > depth) {
			if (textclasslist.Style(current_view->buffer()->params.textclass,
						par->layout).isParagraph()
			    && !par->table
			    // Thinko!
			    // How to handle this? (Lgb)
			    //&& !suffixIs(os, "\n\n")
				) {
				// There should be at least one '\n' already
				// but we need there to be two for Standard 
				// paragraphs that are depth-increment'ed to be
				// output correctly.  However, tables can
				// also be paragraphs so don't adjust them.
				// ARRae
				// Thinkee:
				// Will it ever harm to have one '\n' too
				// many? i.e. that we sometimes will have
				// three in a row. (Lgb)
				os << '\n';
				texrow.newline();
			}
			par = par->TeXDeeper(os, texrow,
					     foot, foot_texrow, foot_count);
		}
		if (par && par->layout == layout && par->depth == depth &&
		    (par->pextra_type == PEXTRA_MINIPAGE) && !minipage_open) {
			if (par->pextra_hfill && par->Previous() &&
			    (par->Previous()->pextra_type == PEXTRA_MINIPAGE)){
				os << "\\hfill{}\n";
                                texrow.newline();
                        }
			if (par_sep == BufferParams::PARSEP_INDENT) {
				os << "{\\setlength\\parindent{0pt}\n";
				texrow.newline();
			}
			os << "\\begin{minipage}";
			switch(par->pextra_alignment) {
			case MINIPAGE_ALIGN_TOP:
				os << "[t]";
				break;
			case MINIPAGE_ALIGN_MIDDLE:
				os << "[m]";
				break;
			case MINIPAGE_ALIGN_BOTTOM:
				os << "[b]";
				break;
			}
			if (!par->pextra_width.empty()) {
				os << '{' << par->pextra_width << "}\n";
			} else {
				//float ib = atof(par->pextra_widthp.c_str())/100;
				// string can't handle floats at present
				// so I'll do a conversion by hand knowing that
				// the limits are 0.0 to 1.0. ARRae.
				os << '{';
				switch (par->pextra_widthp.length()) {
				case 3:
					os << "1.00";
					break;
				case 2:
					os << "0." << par->pextra_widthp;
					break;
				case 1:
					os << "0.0" << par->pextra_widthp;
				}
				os << "\\columnwidth}\n";
			}
			texrow.newline();
			if (par_sep == BufferParams::PARSEP_INDENT) {
				os << "\\setlength\\parindent{\\LyXMinipageIndent}\n";
				texrow.newline();
			}
			minipage_open = true;
                        minipage_open_depth = par->depth;
		}
	} while (par
		 && par->layout == layout
		 && par->depth == depth
		 && par->pextra_type == pextra_type);
 
	if (style.isEnvironment()) {
		os << "\\end{" << style.latexname() << '}';
		// maybe this should go after the minipage closes?
		if (foot_this_level) {
			if (foot_count >= 1) {
				if (foot_count > 1) {
					os << "\\addtocounter{footnote}{-"
					   << foot_count - 1
					   << '}';
				}
				os << foot;
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
                os << "\\end{minipage}\n";
		texrow.newline();
                if (par_sep == BufferParams::PARSEP_INDENT) {
                        os << "}\n";
			texrow.newline();
                }
                if (par && par->pextra_type != PEXTRA_MINIPAGE) {
                        os << "\\medskip\n\n";
			texrow.newline();
			texrow.newline();
                }
                minipage_open = false;
        }
	if (eindent_open) {
		os << "\\end{LyXParagraphIndent}\n";
		texrow.newline();
	}
        if (!(par && (par->pextra_type == PEXTRA_MINIPAGE) 
	      && par->pextra_hfill)) {
                os << '\n';
		texrow.newline();
	}
	lyxerr[Debug::LATEX] << "TeXEnvironment...done " << par << endl;
	return par;  // ale970302
}


LyXParagraph * LyXParagraph::TeXFootnote(ostream & os, TexRow & texrow,
					 ostream & foot, TexRow & foot_texrow,
					 int & foot_count,
					 LyXDirection parent_direction)
{
	lyxerr[Debug::LATEX] << "TeXFootnote...  " << this << endl;
	if (footnoteflag == LyXParagraph::NO_FOOTNOTE)
		lyxerr << "ERROR (LyXParagraph::TeXFootnote): "
			"No footnote!" << endl;

	LyXParagraph * par = this;
	LyXLayout const & style =
		textclasslist.Style(current_view->buffer()->params.textclass, 
				    previous->GetLayout());
	
	if (style.needprotect && footnotekind != LyXParagraph::FOOTNOTE){
		lyxerr << "ERROR (LyXParagraph::TeXFootnote): "
			"Float other than footnote in command"
			" with moving argument is illegal" << endl;
	}

	if (footnotekind != LyXParagraph::FOOTNOTE
	    && footnotekind != LyXParagraph::MARGIN
	    && os.tellp()
	    // Thinko
	    // How to solve this?
	    //&& !suffixIs(file, '\n')
		) {
		// we need to ensure that real floats like tables and figures
		// have their \begin{} on a new line otherwise we can get
		// incorrect results when using the endfloat.sty package
		// especially if two floats follow one another.  ARRae 981022
		// NOTE: if the file is length 0 it must have just been
		//       written out so we assume it ended with a '\n'
		// Thinkee:
		// As far as I can see there is never any harm in writing
		// a '\n' too much. Please tell me if I am wrong. (Lgb)
		os << '\n';
		texrow.newline();
	}

	bool need_closing = false;
	LyXDirection direction = getParDirection();
	if (direction != parent_direction) {
		if (direction == LYX_DIR_LEFT_TO_RIGHT)
			os << "\\L{";
		else
			os << "\\R{";
		need_closing = true;
	}
	
	BufferParams * params = &current_view->buffer()->params;
	bool footer_in_body = true;
	switch (footnotekind) {
	case LyXParagraph::FOOTNOTE:
		if (style.intitle) {
			os << "\\thanks{\n";
			footer_in_body = false;
		} else {
			if (foot_count == -1) {
				// we're at depth 0 so we can use:
				os << "\\footnote{%\n";
				footer_in_body = false;
			} else {
				os << "\\footnotemark{}%\n";
				if (foot_count) {
					// we only need this when there are
					// multiple footnotes
					os << "\\stepcounter{footnote}";
				}
				os << "\\footnotetext{%\n";
				foot_texrow.start(this, 0);
				foot_texrow.newline();
				++foot_count;
			}
		}
		break;
	case LyXParagraph::MARGIN:
		os << "\\marginpar{\n";
		break;
	case LyXParagraph::FIG:
		if (pextra_type == PEXTRA_FLOATFLT
		    && (!pextra_width.empty()
			|| !pextra_widthp.empty())) {
			if (!pextra_width.empty())
				os << "\\begin{floatingfigure}{"
				   << pextra_width << "}\n";
			else
				os << "\\begin{floatingfigure}{"
				   << atoi(pextra_widthp.c_str())/100.0
				   << "\\textwidth}\n";
		} else {
			os << "\\begin{figure}";
			if (!params->float_placement.empty()) { 
				os << '[' << params->float_placement << "]\n";
			} else {
				os << '\n';
			}
		}
		break;
	case LyXParagraph::TAB:
		os << "\\begin{table}";
		if (!params->float_placement.empty()) { 
			os << '[' << params->float_placement << "]\n";
		} else {
			os << '\n';
		}
		break;
	case LyXParagraph::WIDE_FIG:
		os << "\\begin{figure*}";
		if (!params->float_placement.empty()) { 
			os << '[' << params->float_placement << "]\n";
		} else {
			os << '\n';
		}
		break;
	case LyXParagraph::WIDE_TAB:
		os << "\\begin{table*}";
		if (!params->float_placement.empty()) { 
			os << '[' << params->float_placement << "]\n";
		} else {
			os << '\n';
		}
		break;
	case LyXParagraph::ALGORITHM:
		os << "\\begin{algorithm}\n";
		break;
	}
	texrow.newline();
   
 	if (footnotekind != LyXParagraph::FOOTNOTE
	    || !footer_in_body) {
		// Process text for all floats except footnotes in body
		do {
			LyXLayout const & style =
				textclasslist
				.Style(current_view->buffer()->params
				       .textclass,
				       par->layout);
			if (par->IsDummy())
				lyxerr << "ERROR (LyXParagraph::TeXFootnote)"
				       << endl;
			if (style.isEnvironment()
			    || par->pextra_type == PEXTRA_MINIPAGE) { /* && !minipage_open ?? */
				// Allows the use of minipages within float
				// environments. Shouldn't be circular because
				// we don't support footnotes inside
				// floats (yet). ARRae
				par = par->TeXEnvironment(os, texrow,
							  foot, foot_texrow,
							  foot_count);
			} else {
				par = par->TeXOnePar(os, texrow,
						     foot, foot_texrow,
						     foot_count);
			}
			
			if (par && !par->IsDummy() && par->depth > depth) {
				par = par->TeXDeeper(os, texrow,
						     foot, foot_texrow,
						     foot_count);
			}
		} while (par && par->footnoteflag != LyXParagraph::NO_FOOTNOTE);
	} else {
		// process footnotes > depth 0 or in environments separately
		// NOTE: Currently don't support footnotes within footnotes
		//       even though that is possible using the \footnotemark
#ifdef HAVE_SSTREAM
		std::ostringstream dummy;
#else
		ostrstream dummy;
#endif
		TexRow dummy_texrow;
		int dummy_count = 0;
		do {
			LyXLayout const & style =
				textclasslist
				.Style(current_view->buffer()->params
				       .textclass,
				       par->layout);
			if (par->IsDummy())
				lyxerr << "ERROR (LyXParagraph::TeXFootnote)"
				       << endl;
			if (style.isEnvironment()
			    || par->pextra_type == PEXTRA_MINIPAGE) { /* && !minipage_open ?? */
				// Allows the use of minipages within float
				// environments. Shouldn't be circular because
				// we don't support footnotes inside
				// floats (yet). ARRae
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
		} while (par
			 && par->footnoteflag != LyXParagraph::NO_FOOTNOTE);
		if (dummy_count) {
			lyxerr << "ERROR (LyXParagraph::TeXFootnote): "
				"Footnote in a Footnote -- not supported"
			       << endl;
		}
#ifndef HAVE_OSTREAM
		delete [] dummy.str();
#endif
	}

	switch (footnotekind) {
	case LyXParagraph::FOOTNOTE:
		if (footer_in_body) {
			// This helps tell which of the multiple
			// footnotetexts an error was in.
			foot << "}%\n";
			foot_texrow.newline();
		} else {
			os << '}';
		}
		break;
	case LyXParagraph::MARGIN:
		os << '}';
		break;
	case LyXParagraph::FIG:
		if (pextra_type == PEXTRA_FLOATFLT
		    && (!pextra_width.empty()
			|| !pextra_widthp.empty()))
			os << "\\end{floatingfigure}";
		else
			os << "\\end{figure}";
		break;
	case LyXParagraph::TAB:
		os << "\\end{table}";
		break;
	case LyXParagraph::WIDE_FIG:
		os << "\\end{figure*}";
		break;
	case LyXParagraph::WIDE_TAB:
	        os << "\\end{table*}";
		break;
	case LyXParagraph::ALGORITHM:
		os << "\\end{algorithm}";
		break;
	}

	if (need_closing)
		os << "}";

	if (footnotekind != LyXParagraph::FOOTNOTE
	    && footnotekind != LyXParagraph::MARGIN) {
		// we need to ensure that real floats like tables and figures
		// have their \end{} on a line of their own otherwise we can
		// get incorrect results when using the endfloat.sty package.
		os << "\n";
		texrow.newline();
	}

	lyxerr[Debug::LATEX] << "TeXFootnote...done " << par->next << endl;
	return par;
}


bool LyXParagraph::IsDummy() const
{
	return (footnoteflag == LyXParagraph::NO_FOOTNOTE && previous
		&& previous->footnoteflag != LyXParagraph::NO_FOOTNOTE);
}


void LyXParagraph::SetPExtraType(int type, char const * width,
				 char const * widthp)
{
	pextra_type = type;
	pextra_width = width;
	pextra_widthp = widthp;

	if (textclasslist.Style(current_view->buffer()->params.textclass, 
				layout).isEnvironment()) {
		LyXParagraph
			* par = this,
			* ppar = par;

		while (par && (par->layout == layout)
		       && (par->depth == depth)) {
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
		while (par && (par->layout == layout)
		       && (par->depth == depth)) {
			par->pextra_type = type;
			par->pextra_width = width;
			par->pextra_widthp = widthp;
			par = par->NextAfterFootnote();
			if (par && (par->depth > depth))
				par->SetPExtraType(type, width, widthp);
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

	if (textclasslist.Style(current_view->buffer()->params.textclass, 
				layout).isEnvironment()) {
		LyXParagraph
			* par = this,
			* ppar = par;

		while (par && (par->layout == layout)
		       && (par->depth == depth)) {
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
		while (par && (par->layout == layout)
		       && (par->depth == depth)) {
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


bool LyXParagraph::IsHfill(size_type pos) const
{
	return IsHfillChar(GetChar(pos));
}


bool LyXParagraph::IsInset(size_type pos) const
{
	return IsInsetChar(GetChar(pos));
}


bool LyXParagraph::IsFloat(size_type pos) const
{
	return IsFloatChar(GetChar(pos));
}


bool LyXParagraph::IsNewline(size_type pos) const
{
	return pos >= 0 && IsNewlineChar(GetChar(pos));
}


bool LyXParagraph::IsSeparator(size_type pos) const
{
	return IsSeparatorChar(GetChar(pos));
}


bool LyXParagraph::IsLineSeparator(size_type pos) const
{
	return IsLineSeparatorChar(GetChar(pos));
}


bool LyXParagraph::IsKomma(size_type pos) const
{
	return IsKommaChar(GetChar(pos));
}


/// Used by the spellchecker
bool LyXParagraph::IsLetter(LyXParagraph::size_type pos) const
{
	unsigned char c = GetChar(pos);
	if (IsLetterChar(c))
		return true;
	// '\0' is not a letter, allthough every string contains "" (below)
	if( c == '\0')
		return false;
	// We want to pass the ' and escape chars to ispell
	string extra = lyxrc.isp_esc_chars + '\'';
	char ch[2];
	ch[0] = c;
	ch[1] = 0;
	return contains(extra, ch);
}
 
 
bool LyXParagraph::IsWord(size_type pos ) const
{
	return IsWordChar(GetChar(pos)) ;
}

Language const * LyXParagraph::getParLanguage() const 
{
	if (!table && size() > 0)
		return FirstPhysicalPar()->GetFirstFontSettings().language();
	else if (previous)
		return previous->getParLanguage();
	else
		return current_view->buffer()->params.language_info;
}

Language const * LyXParagraph::getLetterLanguage(size_type pos) const
{
	return GetFontSettings(pos).language();
}

LyXDirection LyXParagraph::getParDirection() const
{
	if (!lyxrc.rtl_support || table)
		return LYX_DIR_LEFT_TO_RIGHT;
	else if (getParLanguage()->RightToLeft)
		return LYX_DIR_RIGHT_TO_LEFT;
	else
		return LYX_DIR_LEFT_TO_RIGHT;
}

LyXDirection
LyXParagraph::getLetterDirection(LyXParagraph::size_type pos) const
{
	if (!lyxrc.rtl_support)
		return LYX_DIR_LEFT_TO_RIGHT;
	else if (table && IsNewline(pos))
		return LYX_DIR_LEFT_TO_RIGHT;

	bool is_rtl =  GetFontSettings(pos).isVisibleRightToLeft();
	if (IsLineSeparator(pos) && 0 < pos && pos < Last() - 1
	    && !IsLineSeparator(pos + 1)
	    && !(table && IsNewline(pos + 1))
	    && ( GetFontSettings(pos - 1).isVisibleRightToLeft() != is_rtl
		 || GetFontSettings(pos + 1).isVisibleRightToLeft() != is_rtl))
		return getParDirection();
	else
		return (is_rtl) ? LYX_DIR_RIGHT_TO_LEFT
			: LYX_DIR_LEFT_TO_RIGHT;
}
