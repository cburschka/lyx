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
#include <csignal>

#include "lyxparagraph.h"
#include "support/textutils.h"
#include "lyxrc.h"
#include "layout.h"
#include "language.h"
#include "tex-strings.h"
#include "buffer.h"
#include "bufferparams.h"
#include "support/FileInfo.h"
#include "support/LAssert.h"
#include "debug.h"
#include "LaTeXFeatures.h"
#include "insets/insetinclude.h"
#include "insets/insetbib.h"
#include "insets/insettext.h"
#include "support/filetools.h"
#include "lyx_gui_misc.h"
#include "texrow.h"
#include "support/lyxmanip.h"
#include "BufferView.h"
#include "encoding.h"
#include "ParameterStruct.h"
#include "gettext.h"

using std::ostream;
using std::endl;
using std::fstream;
using std::ios;
using std::lower_bound;
using std::upper_bound;
using std::reverse;

int tex_code_break_column = 72;  // needs non-zero initialization. set later.
// this is a bad idea, but how can LyXParagraph find its buffer to get
// parameters? (JMarc)

extern string bibitemWidest(Buffer const *);

// this is a minibuffer

namespace {

char minibuffer_char;
LyXFont minibuffer_font;
Inset * minibuffer_inset;

} // namespace anon


extern BufferView * current_view;

// Initialization of the counter for the paragraph id's,
// declared in lyxparagraph.h
unsigned int LyXParagraph::paragraph_id = 0;

// Initialize static member.
ShareContainer<LyXFont> LyXParagraph::FontTable::container;


LyXParagraph::LyXParagraph()
{
	text.reserve(500); // is this number too big?
	for (int i = 0; i < 10; ++i) setCounter(i , 0);
	enumdepth = 0;
	itemdepth = 0;
	next_ = 0;
	previous_ = 0;
	inset_owner = 0;
	id_ = paragraph_id++;
        bibkey = 0; // ale970302
	Clear();
}


// This konstruktor inserts the new paragraph in a list.
LyXParagraph::LyXParagraph(LyXParagraph * par)
{
	text.reserve(500);
	par->fitToSize();
	
	for (int i = 0; i < 10; ++i) setCounter(i, 0);
	enumdepth = 0;
	itemdepth = 0;

	// double linked list begin
	next_ = par->next_;
	if (next_)
		next_->previous_ = this;
	previous_ = par;
	previous_->next_ = this;
	// end
	
	inset_owner = 0;
	id_ = paragraph_id++;

        bibkey = 0; // ale970302        
    
	Clear();
}


LyXParagraph::LyXParagraph(LyXParagraph const & lp)
{
	for (int i = 0; i < 10; ++i) setCounter(i , 0);
	enumdepth = 0;
	itemdepth = 0;
	next_ = 0;
	previous_ = 0;
	id_ = paragraph_id++;
	Clear();
   
	MakeSameLayout(&lp);

	// this is because of the dummy layout of the paragraphs that
	// follow footnotes
	layout = lp.layout;

	inset_owner = lp.inset_owner;
   
        // ale970302
	if (lp.bibkey)
		bibkey = static_cast<InsetBibKey *>
			(lp.bibkey->Clone(*current_view->buffer()));
	else
		bibkey = 0;
	
	// copy everything behind the break-position to the new paragraph

	text = lp.text;
	fontlist = lp.fontlist;
	insetlist = lp.insetlist;
	for (InsetList::iterator it = insetlist.begin();
	     it != insetlist.end(); ++it)
		it->inset = it->inset->Clone(*current_view->buffer());
}


// the destructor removes the new paragraph from the list
LyXParagraph::~LyXParagraph()
{
	if (previous_)
		previous_->next_ = next_;
	if (next_)
		next_->previous_ = previous_;

	for (InsetList::iterator it = insetlist.begin();
	     it != insetlist.end(); ++it) {
		delete (*it).inset;
	}

        // ale970302
	delete bibkey;
	//
	//lyxerr << "LyXParagraph::paragraph_id = "
	//       << LyXParagraph::paragraph_id << endl;
}


void LyXParagraph::writeFile(Buffer const * buf, ostream & os,
			     BufferParams const & bparams,
			     char footflag, char dth) const
{
		// The beginning or end of a deeper (i.e. nested) area?
		if (dth != params.depth()) {
			if (params.depth() > dth) {
				while (params.depth() > dth) {
					os << "\n\\begin_deeper ";
					++dth;
				}
			} else {
				while (params.depth() < dth) {
					os << "\n\\end_deeper ";
					--dth;
				}
			}
		}

		// First write the layout
		os << "\n\\layout "
		   << textclasslist.NameOfLayout(bparams.textclass, layout)
		   << "\n";

		// Maybe some vertical spaces.
		if (params.spaceTop().kind() != VSpace::NONE)
			os << "\\added_space_top "
			   << params.spaceTop().asLyXCommand() << " ";
		if (params.spaceBottom().kind() != VSpace::NONE)
			os << "\\added_space_bottom "
			   << params.spaceBottom().asLyXCommand() << " ";

		// Maybe the paragraph has special spacing
		params.spacing().writeFile(os, true);
		
		// The labelwidth string used in lists.
		if (!params.labelWidthString().empty())
			os << "\\labelwidthstring "
			   << params.labelWidthString() << '\n';

		// Lines above or below?
		if (params.lineTop())
			os << "\\line_top ";
		if (params.lineBottom())
			os << "\\line_bottom ";

		// Pagebreaks above or below?
		if (params.pagebreakTop())
			os << "\\pagebreak_top ";
		if (params.pagebreakBottom())
			os << "\\pagebreak_bottom ";
			
		// Start of appendix?
		if (params.startOfAppendix())
			os << "\\start_of_appendix ";

		// Noindent?
		if (params.noindent())
			os << "\\noindent ";
			
		// Alignment?
		if (params.align() != LYX_ALIGN_LAYOUT) {
			int h = 0;
			switch (params.align()) {
			case LYX_ALIGN_LEFT: h = 1; break;
			case LYX_ALIGN_RIGHT: h = 2; break;
			case LYX_ALIGN_CENTER: h = 3; break;
			default: h = 0; break;
			}
			os << "\\align " << string_align[h] << " ";
		}

	// bibitem  ale970302
	if (bibkey)
		bibkey->Write(buf, os);

	LyXFont font1(LyXFont::ALL_INHERIT, bparams.language);

	int column = 0;
	for (size_type i = 0; i < size(); ++i) {
		if (!i) {
			os << "\n";
			column = 0;
		}
		
		// Write font changes
		LyXFont font2 = GetFontSettings(bparams, i);
		if (font2 != font1) {
			font2.lyxWriteChanges(font1, os);
			column = 0;
			font1 = font2;
		}

		value_type const c = GetChar(i);
		switch (c) {
		case META_INSET:
		{
			Inset const * inset = GetInset(i);
			if (inset)
				if (inset->DirectWrite()) {
					// international char, let it write
					// code directly so it's shorter in
					// the file
					inset->Write(buf, os);
				} else {
					os << "\n\\begin_inset ";
					inset->Write(buf, os);
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
	if (next_)
		next_->writeFile(buf, os, bparams, footflag, dth);
}


void LyXParagraph::validate(LaTeXFeatures & features) const
{
	BufferParams const & bparams = features.bufferParams();

	// check the params.
	if (params.lineTop() || params.lineBottom())
		features.lyxline = true;
	if (!params.spacing().isDefault())
		features.setspace = true;
	
	// then the layouts
	features.layout[GetLayout()] = true;

	// then the fonts
	Language const * doc_language = bparams.language;
	
	for (FontList::const_iterator cit = fontlist.begin();
	     cit != fontlist.end(); ++cit) {
		if ((*cit).font().noun() == LyXFont::ON) {
			lyxerr[Debug::LATEX] << "font.noun: "
					     << (*cit).font().noun()
					     << endl;
			features.noun = true;
			lyxerr[Debug::LATEX] << "Noun enabled. Font: "
					     << (*cit).font().stateText(0)
					     << endl;
		}
		switch ((*cit).font().color()) {
		case LColor::none:
		case LColor::inherit:
		case LColor::ignore:
			break;
		default:
			features.color = true;
			lyxerr[Debug::LATEX] << "Color enabled. Font: "
					     << (*cit).font().stateText(0)
					     << endl;
		}

		Language const * language = (*cit).font().language();
		if (language->babel() != doc_language->babel()) {
			features.UsedLanguages.insert(language);
			lyxerr[Debug::LATEX] << "Found language "
					     << language->babel() << endl;
		}
	}

	// then the insets
	for (InsetList::const_iterator cit = insetlist.begin();
	     cit != insetlist.end(); ++cit) {
		if ((*cit).inset)
			(*cit).inset->Validate(features);
	}
}


// First few functions needed for cut and paste and paragraph breaking.
void LyXParagraph::CopyIntoMinibuffer(Buffer const & buffer,
				      LyXParagraph::size_type pos) const
{
	BufferParams bparams = buffer.params;

	minibuffer_char = GetChar(pos);
	minibuffer_font = GetFontSettings(bparams, pos);
	minibuffer_inset = 0;
	if (minibuffer_char == LyXParagraph::META_INSET) {
		if (GetInset(pos)) {
			minibuffer_inset = GetInset(pos)->Clone(buffer);
		} else {
			minibuffer_inset = 0;
			minibuffer_char = ' ';
			// This reflects what GetInset() does (ARRae)
		}
	}
}


void LyXParagraph::CutIntoMinibuffer(BufferParams const & bparams,
				     LyXParagraph::size_type pos)
{
	minibuffer_char = GetChar(pos);
	minibuffer_font = GetFontSettings(bparams, pos);
	minibuffer_inset = 0;
	if (minibuffer_char == LyXParagraph::META_INSET) {
		if (GetInset(pos)) {
			minibuffer_inset = GetInset(pos);
			// This is a little hack since I want exactly
			// the inset, not just a clone. Otherwise
			// the inset would be deleted when calling Erase(pos)
			// find the entry
			InsetTable search_elem(pos, 0);
			InsetList::iterator it =
				lower_bound(insetlist.begin(),
					    insetlist.end(),
					    search_elem, matchIT());
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


bool LyXParagraph::InsertFromMinibuffer(LyXParagraph::size_type pos)
{
	if ((minibuffer_char == LyXParagraph::META_INSET) &&
	    !InsertInsetAllowed(minibuffer_inset))
		return false;
	if (minibuffer_char == LyXParagraph::META_INSET)
		InsertInset(pos, minibuffer_inset, minibuffer_font);
	else
		InsertChar(pos, minibuffer_char, minibuffer_font);
	return true;
}

// end of minibuffer



void LyXParagraph::Clear()
{
	params.clear();
	
	layout = 0;
	bibkey = 0;
}


void LyXParagraph::Erase(LyXParagraph::size_type pos)
{
	lyx::Assert(pos < size());
		// if it is an inset, delete the inset entry 
		if (text[pos] == LyXParagraph::META_INSET) {
			// find the entry
			InsetTable search_inset(pos, 0);
			InsetList::iterator it =
				lower_bound(insetlist.begin(),
					    insetlist.end(),
					    search_inset, matchIT());
			if (it != insetlist.end() && (*it).pos == pos) {
				delete (*it).inset;
				insetlist.erase(it);
			}
		}

		text.erase(text.begin() + pos);

		// Erase entries in the tables.
		FontTable search_font(pos, LyXFont());
		
		FontList::iterator it =
			lower_bound(fontlist.begin(),
				    fontlist.end(),
				    search_font, matchFT());
		if (it != fontlist.end() && (*it).pos() == pos &&
		    (pos == 0 || 
		     (it != fontlist.begin() && (*(it - 1)).pos() == pos - 1))) {
			// If it is a multi-character font
			// entry, we just make it smaller
			// (see update below), otherwise we
			// should delete it.
			unsigned int const i = it - fontlist.begin();
			fontlist.erase(fontlist.begin() + i);
			it = fontlist.begin() + i;
			if (i > 0 && i < fontlist.size() &&
			    fontlist[i - 1].font() == fontlist[i].font()) {
				fontlist.erase(fontlist.begin() + i - 1);
				it = fontlist.begin() + i - 1;
			}
		}

		// Update all other entries.
		FontList::iterator fend = fontlist.end();
		for (; it != fend; ++it)
			(*it).pos((*it).pos() - 1);

		// Update the inset table.
		InsetTable search_inset(pos, 0);
		InsetList::iterator lend = insetlist.end();
		for (InsetList::iterator it =
			     upper_bound(insetlist.begin(),
					 lend,
					 search_inset, matchIT());
		     it != lend; ++it)
			--(*it).pos;
}


void LyXParagraph::InsertChar(LyXParagraph::size_type pos,
			      LyXParagraph::value_type c)
{
	LyXFont const f(LyXFont::ALL_INHERIT);
	InsertChar(pos, c, f);
}


void LyXParagraph::InsertChar(LyXParagraph::size_type pos,
			      LyXParagraph::value_type c,
			      LyXFont const & font)
{
	lyx::Assert(pos <= size());

	text.insert(text.begin() + pos, c);

	// Update the font table.
	FontTable search_font(pos, LyXFont());
	for (FontList::iterator it = lower_bound(fontlist.begin(),
						 fontlist.end(),
						 search_font, matchFT());
	     it != fontlist.end(); ++it)
		(*it).pos((*it).pos() + 1);
   
	// Update the inset table.
	InsetTable search_inset(pos, 0);
	for (InsetList::iterator it = lower_bound(insetlist.begin(),
						  insetlist.end(),
						  search_inset, matchIT());
	     it != insetlist.end(); ++it)
		++(*it).pos;

	SetFont(pos, font);
}


void LyXParagraph::InsertInset(LyXParagraph::size_type pos,
			       Inset * inset)
{
	LyXFont const f(LyXFont::ALL_INHERIT);
	InsertInset(pos, inset, f);
}


void LyXParagraph::InsertInset(LyXParagraph::size_type pos,
			       Inset * inset, LyXFont const & font)
{
	lyx::Assert(inset);
	lyx::Assert(pos <= size());
	
	InsertChar(pos, META_INSET, font);
	lyx::Assert(text[pos] == META_INSET);
	
	// Add a new entry in the inset table.
	InsetTable search_inset(pos, 0);
	InsetList::iterator it = lower_bound(insetlist.begin(),
					     insetlist.end(),
					     search_inset, matchIT());
	if (it != insetlist.end() && (*it).pos == pos)
		lyxerr << "ERROR (LyXParagraph::InsertInset): "
			"there is an inset in position: " << pos << endl;
	else
		insetlist.insert(it, InsetTable(pos, inset));
	if (inset_owner)
		inset->setOwner(inset_owner);
}


bool LyXParagraph::InsertInsetAllowed(Inset * inset)
{
	//lyxerr << "LyXParagraph::InsertInsetAllowed" << endl;
	
	if (inset_owner)
		return inset_owner->InsertInsetAllowed(inset);
	return true;
}


Inset * LyXParagraph::GetInset(LyXParagraph::size_type pos)
{
	lyx::Assert(pos < size());

	// Find the inset.
	InsetTable search_inset(pos, 0);
	InsetList::iterator it = lower_bound(insetlist.begin(),
					     insetlist.end(),
					     search_inset, matchIT());
	if (it != insetlist.end() && (*it).pos == pos)
		return (*it).inset;

	lyxerr << "ERROR (LyXParagraph::GetInset): "
		"Inset does not exist: " << pos << endl;
	//::raise(SIGSTOP);
	
	// text[pos] = ' '; // WHY!!! does this set the pos to ' '????
	// Did this commenting out introduce a bug? So far I have not
	// see any, please enlighten me. (Lgb)
	// My guess is that since the inset does not exist, we might
	// as well replace it with a space to prevent craches. (Asger)
	return 0;
}


Inset const * LyXParagraph::GetInset(LyXParagraph::size_type pos) const
{
	lyx::Assert(pos < size());

	// Find the inset.
	InsetTable search_inset(pos, 0);
	InsetList::const_iterator cit = lower_bound(insetlist.begin(),
						    insetlist.end(),
						    search_inset, matchIT());
	if (cit != insetlist.end() && (*cit).pos == pos)
		return (*cit).inset;

	lyxerr << "ERROR (LyXParagraph::GetInset): "
		"Inset does not exist: " << pos << endl;
	//::raise(SIGSTOP);
	//text[pos] = ' '; // WHY!!! does this set the pos to ' '????
	// Did this commenting out introduce a bug? So far I have not
	// see any, please enlighten me. (Lgb)
	// My guess is that since the inset does not exist, we might
	// as well replace it with a space to prevent craches. (Asger)
	return 0;
}


// Gets uninstantiated font setting at position.
LyXFont const LyXParagraph::GetFontSettings(BufferParams const & bparams,
					    LyXParagraph::size_type pos) const
{
	lyx::Assert(pos <= size());
	
	FontTable search_font(pos, LyXFont());
	FontList::const_iterator cit = lower_bound(fontlist.begin(),
						   fontlist.end(),
						   search_font, matchFT());
	if (cit != fontlist.end())
		return (*cit).font();
	
	if (pos == size() && size())
		return GetFontSettings(bparams, pos - 1);
	
	return LyXFont(LyXFont::ALL_INHERIT, getParLanguage(bparams));
}


// Gets uninstantiated font setting at position 0
LyXFont const LyXParagraph::GetFirstFontSettings() const
{
	if (size() > 0) {
		if (!fontlist.empty())
			return fontlist[0].font();
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
LyXFont const LyXParagraph::getFont(BufferParams const & bparams,
			      LyXParagraph::size_type pos) const
{
	LyXFont tmpfont;
	LyXLayout const & layout =
		textclasslist.Style(bparams.textclass, 
				    GetLayout());
	LyXParagraph::size_type main_body = 0;
	if (layout.labeltype == LABEL_MANUAL)
		main_body = BeginningOfMainBody();

	if (pos >= 0) {
		LyXFont layoutfont;
		if (pos < main_body)
			layoutfont = layout.labelfont;
		else
			layoutfont = layout.font;
		tmpfont = GetFontSettings(bparams, pos);
		tmpfont.realize(layoutfont);
	} else {
		// process layoutfont for pos == -1 and labelfont for pos < -1
		if (pos == -1)
			tmpfont = layout.font;
		else
			tmpfont = layout.labelfont;
		tmpfont.setLanguage(getParLanguage(bparams));
	}

	// check for environment font information
	char par_depth = GetDepth();
	LyXParagraph const * par = this;
	while (par && par_depth && !tmpfont.resolved()) {
		par = par->DepthHook(par_depth - 1);
		if (par) {
			tmpfont.realize(textclasslist.
					Style(bparams.textclass,
					      par->GetLayout()).font);
			par_depth = par->GetDepth();
		}
	}

	tmpfont.realize(textclasslist
			.TextClass(bparams.textclass)
			.defaultfont());
	return tmpfont;
}


/// Returns the height of the highest font in range
LyXFont::FONT_SIZE
LyXParagraph::HighestFontInRange(LyXParagraph::size_type startpos,
				 LyXParagraph::size_type endpos) const
{
	LyXFont::FONT_SIZE maxsize = LyXFont::SIZE_TINY;
	if (fontlist.empty())
		return maxsize;

	FontTable end_search(endpos, LyXFont());
	FontList::const_iterator end_it = lower_bound(fontlist.begin(),
						      fontlist.end(),
						      end_search, matchFT());
	if (end_it != fontlist.end())
		++end_it;

	FontTable start_search(startpos, LyXFont());
	for (FontList::const_iterator cit =
		     lower_bound(fontlist.begin(),
				 fontlist.end(),
				 start_search, matchFT());
	     cit != end_it; ++cit) {
		LyXFont::FONT_SIZE size = (*cit).font().size();
		if (size > maxsize && size <= LyXFont::SIZE_HUGER)
			maxsize = size;
	}
	return maxsize;
}


LyXParagraph::value_type
LyXParagraph::GetUChar(BufferParams const & bparams,
		       LyXParagraph::size_type pos) const
{
	value_type c = GetChar(pos);
	if (!lyxrc.rtl_support)
		return c;

	value_type uc = c;
	switch (c) {
	case '(':
		uc = ')';
		break;
	case ')':
		uc = '(';
		break;
	case '[':
		uc = ']';
		break;
	case ']':
		uc = '[';
		break;
	case '{':
		uc = '}';
		break;
	case '}':
		uc = '{';
		break;
	case '<':
		uc = '>';
		break;
	case '>':
		uc = '<';
		break;
	}
	if (uc != c && GetFontSettings(bparams, pos).isRightToLeft())
		return uc;
	else
		return c;
}


// return an string of the current word, and the end of the word in lastpos.
string const LyXParagraph::GetWord(LyXParagraph::size_type & lastpos) const
{
	lyx::Assert(lastpos >= 0);

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


void LyXParagraph::SetFont(LyXParagraph::size_type pos,
			   LyXFont const & font)
{
	lyx::Assert(pos <= size());

	// First, reduce font against layout/label font
	// Update: The SetCharFont() routine in text2.C already
	// reduces font, so we don't need to do that here. (Asger)
	// No need to simplify this because it will disappear
	// in a new kernel. (Asger)
	// Next search font table

	FontTable search_font(pos, LyXFont());
	FontList::iterator it = lower_bound(fontlist.begin(),
					    fontlist.end(),
					    search_font, matchFT());
	unsigned int i = it - fontlist.begin();
	bool notfound = it == fontlist.end();

	if (!notfound && fontlist[i].font() == font)
		return;

	bool begin = pos == 0 || notfound ||
		(i > 0 && fontlist[i-1].pos() == pos - 1);
	// Is position pos is a beginning of a font block?
	bool end = !notfound && fontlist[i].pos() == pos;
	// Is position pos is the end of a font block?
	if (begin && end) { // A single char block
		if (i + 1 < fontlist.size() &&
		    fontlist[i + 1].font() == font) {
			// Merge the singleton block with the next block
			fontlist.erase(fontlist.begin() + i);
			if (i > 0 && fontlist[i - 1].font() == font)
				fontlist.erase(fontlist.begin() + i-1);
		} else if (i > 0 && fontlist[i - 1].font() == font) {
			// Merge the singleton block with the previous block
			fontlist[i - 1].pos(pos);
			fontlist.erase(fontlist.begin() + i);
		} else
			fontlist[i].font(font);
	} else if (begin) {
		if (i > 0 && fontlist[i - 1].font() == font)
			fontlist[i - 1].pos(pos);
		else
			fontlist.insert(fontlist.begin() + i,
					FontTable(pos, font));
	} else if (end) {
		fontlist[i].pos(pos - 1);
		if (!(i + 1 < fontlist.size() &&
		      fontlist[i + 1].font() == font))
			fontlist.insert(fontlist.begin() + i + 1,
					FontTable(pos, font));
	} else { // The general case. The block is splitted into 3 blocks
		fontlist.insert(fontlist.begin() + i, 
				FontTable(pos - 1, fontlist[i].font()));
		fontlist.insert(fontlist.begin() + i + 1,
				FontTable(pos, font));
	}
}



void LyXParagraph::next(LyXParagraph * p)
{
	next_ = p;
}


// This function is able to hide closed footnotes.
LyXParagraph * LyXParagraph::next()
{
	return next_;
}


LyXParagraph const * LyXParagraph::next() const
{
	return next_;
}


void LyXParagraph::previous(LyXParagraph * p)
{
	previous_ = p;
}


// This function is able to hide closed footnotes.
LyXParagraph * LyXParagraph::previous()
{
	return previous_;
}


// This function is able to hide closed footnotes.
LyXParagraph const * LyXParagraph::previous() const
{
	return previous_;
}


void LyXParagraph::BreakParagraph(BufferParams const & bparams,
				  LyXParagraph::size_type pos,
				  int flag)
{
	// create a new paragraph
	LyXParagraph * tmp = new LyXParagraph(this);
	
	// this is an idea for a more userfriendly layout handling, I will
	// see what the users say
	
	// layout stays the same with latex-environments
	if (flag) {
		tmp->SetOnlyLayout(layout);
		tmp->SetLabelWidthString(params.labelWidthString());
	}
	
	if (size() > pos || !size() || flag == 2) {
		tmp->SetOnlyLayout(layout);
		tmp->params.align(params.align());
		tmp->SetLabelWidthString(params.labelWidthString());
		
		tmp->params.lineBottom(params.lineBottom());
		params.lineBottom(false);
		tmp->params.pagebreakBottom(params.pagebreakBottom());
		params.pagebreakBottom(false);
		tmp->params.spaceBottom(params.spaceBottom());
		params.spaceBottom(VSpace(VSpace::NONE));
		
		tmp->params.depth(params.depth());
		tmp->params.noindent(params.noindent());
		
		// copy everything behind the break-position
		// to the new paragraph
		size_type pos_end = text.size() - 1;
		size_type i = pos;
		size_type j = pos;
		for (; i <= pos_end; ++i) {
			CutIntoMinibuffer(bparams, i);
			if (tmp->InsertFromMinibuffer(j - pos))
				++j;
		}
		tmp->fitToSize();
		for (i = pos_end; i >= pos; --i)
			Erase(i);
		
		fitToSize();
	}
	
	// just an idea of me
	if (!pos) {
		tmp->params.lineTop(params.lineTop());
		tmp->params.pagebreakTop(params.pagebreakTop());
		tmp->params.spaceTop(params.spaceTop());
		tmp->bibkey = bibkey;
		Clear();
		// layout stays the same with latex-environments
		if (flag) {
			SetOnlyLayout(tmp->layout);
			SetLabelWidthString(tmp->params.labelWidthString());
			params.depth(tmp->params.depth());
		}
	}
}
	

void LyXParagraph::MakeSameLayout(LyXParagraph const * par)
{
	layout = par->layout;
	params.makeSame(par->params);
	
	// This can be changed after NEW_INSETS is in effect. (Lgb)
	SetLabelWidthString(par->params.labelWidthString());
}


int LyXParagraph::StripLeadingSpaces(LyXTextClassList::size_type tclass) 
{
	if (textclasslist.Style(tclass, GetLayout()).free_spacing)
		return 0;
	
	int i = 0;
	while (size()
	       && (IsNewline(0) || IsLineSeparator(0))){
		Erase(0);
		++i;
	}

	return i;
}


#if 0
LyXParagraph * LyXParagraph::Clone() const
{
	// create a new paragraph
	LyXParagraph * result = new LyXParagraph;
   
	result->MakeSameLayout(this);

	// this is because of the dummy layout of the paragraphs that
	// follow footnotes
	result->layout = layout;

	result->inset_owner = inset_owner;
   
        // ale970302
	if (bibkey)
		result->bibkey = static_cast<InsetBibKey *>
			         (bibkey->Clone(*current_view->buffer()));
	else
		result->bibkey = 0;
    
	// copy everything behind the break-position to the new paragraph

	result->text = text;
	result->fontlist = fontlist;
	result->insetlist = insetlist;
	for (InsetList::iterator it = result->insetlist.begin();
	     it != result->insetlist.end(); ++it)
		(*it).inset = (*it).inset->Clone(*current_view->buffer());
	return result;
}
#endif


bool LyXParagraph::HasSameLayout(LyXParagraph const * par) const
{
	return 
		par->layout == layout &&
		params.sameLayout(par->params);
}


void LyXParagraph::BreakParagraphConservative(BufferParams const & bparams,
					      LyXParagraph::size_type pos)
{
	// create a new paragraph
	LyXParagraph * tmp = new LyXParagraph(this);
	tmp->MakeSameLayout(this);

	// When can pos > Last()?
	// I guess pos == Last() is possible.
	if (size() > pos) {
		// copy everything behind the break-position to the new
		// paragraph
		size_type pos_end = text.size() - 1;

		size_type i, j;
		for (i = j = pos; i <= pos_end; ++i) {
			CutIntoMinibuffer(bparams, i);
			if (tmp->InsertFromMinibuffer(j - pos))
				++j;
		}

		tmp->fitToSize();
		
		for (size_type i = pos_end; i >= pos; --i)
			Erase(i);

		fitToSize();
	}
}
   

// Be carefull, this does not make any check at all.
// This method has wrong name, it combined this par with the next par.
// In that sense it is the reverse of break paragraph. (Lgb)
void LyXParagraph::PasteParagraph(BufferParams const & bparams)
{
	// copy the next paragraph to this one
	LyXParagraph * the_next = next();
   
	// first the DTP-stuff
	params.lineBottom(the_next->params.lineBottom());
	params.spaceBottom(the_next->params.spaceBottom());
	params.pagebreakBottom(the_next->params.pagebreakBottom());

	size_type pos_end = the_next->text.size() - 1;
	size_type pos_insert = size();

	// ok, now copy the paragraph
	size_type i, j;
	for (i = j = 0; i <= pos_end; ++i) {
		the_next->CutIntoMinibuffer(bparams, i);
		if (InsertFromMinibuffer(pos_insert + j))
			++j;
	}
   
	// delete the next paragraph
	LyXParagraph * ppar = the_next->previous_;
	LyXParagraph * npar = the_next->next_;
	delete the_next;
	ppar->next(npar);
}


int LyXParagraph::GetEndLabel(BufferParams const & bparams) const
{
	LyXParagraph const * par = this;
	int par_depth = GetDepth();
	while (par) {
		LyXTextClass::LayoutList::size_type layout = par->GetLayout();
		int const endlabeltype =
			textclasslist.Style(bparams.textclass,
					    layout).endlabeltype;
		if (endlabeltype != END_LABEL_NO_LABEL) {
			if (!next_)
				return endlabeltype;

			int const next_depth = next_->GetDepth();
			if (par_depth > next_depth ||
			    (par_depth == next_depth
			     && layout != next_->GetLayout()))
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


char LyXParagraph::GetDepth() const
{
	return params.depth();
}


char LyXParagraph::GetAlign() const
{
	return params.align();
}


string const & LyXParagraph::GetLabelstring() const
{
	return params.labelString();
}


int LyXParagraph::GetFirstCounter(int i) const
{
	return counter_[i];
}


// the next two functions are for the manual labels
string const LyXParagraph::GetLabelWidthString() const
{
	if (!params.labelWidthString().empty())
		return params.labelWidthString();
	else
		return _("Senseless with this layout!");
}


void LyXParagraph::SetLabelWidthString(string const & s)
{
	params.labelWidthString(s);
}


void LyXParagraph::SetOnlyLayout(LyXTextClass::size_type new_layout)
{
	layout = new_layout;
}


void LyXParagraph::SetLayout(LyXTextClass::size_type new_layout)
{
        layout = new_layout;
	params.labelWidthString(string());
	params.align(LYX_ALIGN_LAYOUT);
	params.spaceTop(VSpace(VSpace::NONE));
	params.spaceBottom(VSpace(VSpace::NONE));
	params.spacing(Spacing(Spacing::Default));
}


// if the layout of a paragraph contains a manual label, the beginning of the 
// main body is the beginning of the second word. This is what the par-
// function returns. If the layout does not contain a label, the main
// body always starts with position 0. This differentiation is necessary,
// because there cannot be a newline or a blank <= the beginning of the 
// main body in TeX.

int LyXParagraph::BeginningOfMainBody() const
{
	// Unroll the first two cycles of the loop
	// and remember the previous character to
	// remove unnecessary GetChar() calls
	size_type i = 0;
	if (i < size()
	    && GetChar(i) != LyXParagraph::META_NEWLINE) {
		++i;
		char previous_char = 0;
		char temp = 0; 
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

	return i;
}


LyXParagraph * LyXParagraph::DepthHook(int deth)
{
	LyXParagraph * newpar = this;
	if (deth < 0)
		return 0;
   
	do {
		newpar = newpar->previous();
	} while (newpar && newpar->GetDepth() > deth);
   
	if (!newpar) {
		if (previous() || GetDepth())
			lyxerr << "ERROR (LyXParagraph::DepthHook): "
				"no hook." << endl;
		newpar = this;
	}

	return newpar;
}


LyXParagraph const * LyXParagraph::DepthHook(int deth) const
{
	LyXParagraph const * newpar = this;
	if (deth < 0)
		return 0;
   
	do {
		newpar = newpar->previous();
	} while (newpar && newpar->GetDepth() > deth);
   
	if (!newpar) {
		if (previous() || GetDepth())
			lyxerr << "ERROR (LyXParagraph::DepthHook): "
				"no hook." << endl;
		newpar = this;
	}

	return newpar;
}


int LyXParagraph::AutoDeleteInsets()
{
	int count = 0;
	InsetList::size_type index = 0;
	while (index < insetlist.size()) {
		if (insetlist[index].inset && insetlist[index].inset->AutoDelete()) {
			Erase(insetlist[index].pos); 
			// Erase() calls to insetlist.erase(&insetlist[index])
			// so index shouldn't be increased.
			++count;
		} else
			++index;
	}
	return count;
}


LyXParagraph::inset_iterator
LyXParagraph::InsetIterator(LyXParagraph::size_type pos)
{
	InsetTable search_inset(pos, 0);
	InsetList::iterator it = lower_bound(insetlist.begin(),
					     insetlist.end(),
					     search_inset, matchIT());
	return inset_iterator(it);
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
	if (inset == bibkey)
		return 0;

	return -1;
}


LyXParagraph * LyXParagraph::TeXOnePar(Buffer const * buf,
				       BufferParams const & bparams,
				       ostream & os, TexRow & texrow,
				       bool moving_arg)
{
	lyxerr[Debug::LATEX] << "TeXOnePar...     " << this << endl;
	LyXLayout const & style =
		textclasslist.Style(bparams.textclass,
				    layout);

	bool further_blank_line = false;

	if (params.startOfAppendix()) {
		os << "\\appendix\n";
		texrow.newline();
	}

	if (!params.spacing().isDefault()
	    && (!previous() || !previous()->HasSameLayout(this))) {
		os << params.spacing().writeEnvirBegin() << "\n";
		texrow.newline();
	}
	
	if (tex_code_break_column && style.isCommand()){
		os << '\n';
		texrow.newline();
	}

	if (params.pagebreakTop()) {
		os << "\\newpage";
		further_blank_line = true;
	}
	if (params.spaceTop().kind() != VSpace::NONE) {
		os << params.spaceTop().asLatexCommand(bparams);
		further_blank_line = true;
	}
      
	if (params.lineTop()) {
		os << "\\lyxline{\\" << getFont(bparams, 0).latexSize() << '}'
		   << "\\vspace{-1\\parskip}";
		further_blank_line = true;
	}

	if (further_blank_line){
		os << '\n';
		texrow.newline();
	}

	Language const * language = getParLanguage(bparams);
	Language const * doc_language = bparams.language;
	Language const * previous_language = previous_
		? previous_->getParLanguage(bparams) : doc_language;
	if (language->babel() != doc_language->babel() &&
	    language->babel() != previous_language->babel()) {
		os << subst(lyxrc.language_command_begin, "$$lang",
			    language->babel())
		   << endl;
		texrow.newline();
	}

	if (bparams.inputenc == "auto" &&
	    language->encoding() != previous_language->encoding()) {
		os << "\\inputencoding{"
		   << language->encoding()->LatexName()
		   << "}" << endl;
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
			bibkey->Latex(buf, os, false, false);
		} else
			os << "\\item ";
		break;
	case LATEX_LIST_ENVIRONMENT:
		os << "\\item ";
		break;
	default:
		break;
	}

	bool need_par = SimpleTeXOnePar(buf, bparams, os, texrow, moving_arg);
 
	// Make sure that \\par is done with the font of the last
	// character if this has another size as the default.
	// This is necessary because LaTeX (and LyX on the screen)
	// calculates the space between the baselines according
	// to this font. (Matthias)
	//
	// Is this really needed ? (Dekel)
	// We do not need to use to change the font for the last paragraph
	// or for a command.
	LyXFont font = getFont(bparams, size() - 1);

	bool is_command = textclasslist.Style(bparams.textclass,
					      GetLayout()).isCommand();
	if (style.resfont.size() != font.size() && next_ && !is_command) {
		if (!need_par)
			os << "{";
		os << "\\" << font.latexSize() << " \\par}";
	} else if (need_par) {
		os << "\\par}";
	} else if (is_command)
		os << "}";

	if (language->babel() != doc_language->babel() &&
	    (!next_
	     || next_->getParLanguage(bparams)->babel() != language->babel())) {
		os << endl 
		   << subst(lyxrc.language_command_end, "$$lang",
			    doc_language->babel());
	}
	
	switch (style.latextype) {
	case LATEX_ITEM_ENVIRONMENT:
	case LATEX_LIST_ENVIRONMENT:
                if (next_ && (params.depth() < next_->params.depth())) {
                        os << '\n';
                        texrow.newline();
                }
		break;
	case LATEX_ENVIRONMENT:
		// if its the last paragraph of the current environment
		// skip it otherwise fall through
		if (next_
		    && (next_->layout != layout
			|| next_->params.depth() != params.depth()))
			break;
	default:
		// we don't need it for the last paragraph!!!
		// or for tables in floats
		//   -- effectively creates a \par where there isn't one which
		//      breaks a \subfigure or \subtable.
		if (next_) {
			os << '\n';
			texrow.newline();
		}
	}
	
	further_blank_line = false;
	if (params.lineBottom()) {
		os << "\\lyxline{\\" << getFont(bparams,
						size() - 1).latexSize() << '}';
		further_blank_line = true;
	}

	if (params.spaceBottom().kind() != VSpace::NONE) {
		os << params.spaceBottom().asLatexCommand(bparams);
		further_blank_line = true;
	}
      
	if (params.pagebreakBottom()) {
		os << "\\newpage";
		further_blank_line = true;
	}

	if (further_blank_line){
		os << '\n';
		texrow.newline();
	}

	if (!params.spacing().isDefault()
	    && (!next_ || !next_->HasSameLayout(this))) {
		os << params.spacing().writeEnvirEnd() << "\n";
		texrow.newline();
	}
	
	// we don't need it for the last paragraph!!!
	if (next_) {
		os << '\n';
		texrow.newline();
	}

	lyxerr[Debug::LATEX] << "TeXOnePar...done " << next_ << endl;
	return next_;
}


// This one spits out the text of the paragraph
bool LyXParagraph::SimpleTeXOnePar(Buffer const * buf,
				   BufferParams const & bparams,
				   ostream & os, TexRow & texrow,
				   bool moving_arg)
{
	lyxerr[Debug::LATEX] << "SimpleTeXOnePar...     " << this << endl;

	bool return_value = false;

	LyXLayout const & style =
		textclasslist.Style(bparams.textclass,
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
		basefont = getFont(bparams, -2); // Get label font
	} else {
		basefont = getFont(bparams, -1); // Get layout font
	}

	int column = 0;

	if (main_body >= 0
	    && !text.size()) {
		if (style.isCommand()) {
			os << '{';
			++column;
		} else if (params.align() != LYX_ALIGN_LAYOUT) {
			os << '{';
			++column;
			return_value = true;
		}
	}

	moving_arg |= style.needprotect;
 
	// Which font is currently active?
	LyXFont running_font(basefont);
	// Do we have an open font change?
	bool open_font = false;

	texrow.start(this, 0);

	for (size_type i = 0; i < size(); ++i) {
		++column;
		// First char in paragraph or after label?
		if (i == main_body) {
			if (main_body > 0) {
				if (open_font) {
					column += running_font.latexWriteEndChanges(os, basefont, basefont);
					open_font = false;
				}
				basefont = getFont(bparams, -1); // Now use the layout font
				running_font = basefont;
				os << ']';
				++column;
			}
			if (style.isCommand()) {
				os << '{';
				++column;
			} else if (params.align() != LYX_ALIGN_LAYOUT && next_) {
				// We do not need \par here (Dekel)
				// os << "{\\par";
				os << "{";
				++column;
				return_value = true;
			}

			if (params.noindent()) {
				os << "\\noindent ";
				column += 10;
			}
			switch (params.align()) {
			case LYX_ALIGN_NONE:
			case LYX_ALIGN_BLOCK:
			case LYX_ALIGN_LAYOUT:
			case LYX_ALIGN_SPECIAL:
				break;
			case LYX_ALIGN_LEFT:
				if (moving_arg)
					os << "\\protect";
				if (getParLanguage(bparams)->babel() != "hebrew") {
					os << "\\raggedright ";
					column+= 13;
				} else {
					os << "\\raggedleft ";
					column+= 12;
				}
				break;
			case LYX_ALIGN_RIGHT:
				if (moving_arg)
					os << "\\protect";
				if (getParLanguage(bparams)->babel() != "hebrew") {
					os << "\\raggedleft ";
					column+= 12;
				} else {
					os << "\\raggedright ";
					column+= 13;
				}
				break;
			case LYX_ALIGN_CENTER:
				if (moving_arg)
					os << "\\protect";
				os << "\\centering ";
				column+= 11;
				break;
			}	 
		}

		value_type c = GetChar(i);

		// Fully instantiated font
		LyXFont font = getFont(bparams, i);

		last_font = running_font;

		// Spaces at end of font change are simulated to be
		// outside font change, i.e. we write "\textXX{text} "
		// rather than "\textXX{text }". (Asger)
		if (open_font && c == ' ' && i <= size() - 2 
		    && !getFont(bparams, i + 1).equalExceptLatex(running_font) 
		    && !getFont(bparams, i + 1).equalExceptLatex(font)) {
			font = getFont(bparams, i + 1);
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
		if (c == ' ') {
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
				basefont = getFont(bparams, -1);
				running_font = basefont;
				if (font.family() == 
				    LyXFont::TYPEWRITER_FAMILY) {
					os << "~";
				}
				if (moving_arg)
					os << "\\protect ";
				os << "\\\\\n";
			}
			texrow.newline();
			texrow.start(this, i + 1);
			column = 0;
		} else {
			SimpleTeXSpecialChars(buf, bparams,
					      os, texrow, moving_arg,
					      font, running_font, basefont,
					      open_font, style, i, column, c);
		}
	}

	// If we have an open font definition, we have to close it
	if (open_font) {
		if (next_) {
			running_font
				.latexWriteEndChanges(os, basefont,
						      next_->getFont(bparams,
								     0));
		} else {
			running_font.latexWriteEndChanges(os, basefont,
							  basefont);
		}
	}

	// Needed if there is an optional argument but no contents.
	if (main_body > 0 && main_body == size()) {
		os << "]~";
		return_value = false;
	}

	lyxerr[Debug::LATEX] << "SimpleTeXOnePar...done " << this << endl;
	return return_value;
}


bool LyXParagraph::linuxDocConvertChar(char c, string & sgml_string)
{
	bool retval = false;
	switch (c) {
	case LyXParagraph::META_HFILL:
		sgml_string.erase();
		break;
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
		sgml_string.erase();
		break;
	default:
		sgml_string = c;
		break;
	}
	return retval;
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


void LyXParagraph::SimpleTeXSpecialChars(Buffer const * buf,
					 BufferParams const & bparams,
					 ostream & os, TexRow & texrow,
					 bool moving_arg,
					 LyXFont & font,
					 LyXFont & running_font,
					 LyXFont & basefont,
					 bool & open_font,
					 LyXLayout const & style,
					 LyXParagraph::size_type & i,
					 int & column,
					 LyXParagraph::value_type const c)
{
	// Two major modes:  LaTeX or plain
	// Handle here those cases common to both modes
	// and then split to handle the two modes separately.
	switch (c) {
	case LyXParagraph::META_INSET: {
		Inset * inset = GetInset(i);
		if (inset) {
			bool close = false;
			int const len = os.tellp();
			//ostream::pos_type const len = os.tellp();
			if ((inset->LyxCode() == Inset::GRAPHICS_CODE
			     || inset->LyxCode() == Inset::MATH_CODE
			     || inset->LyxCode() == Inset::URL_CODE)
			    && running_font.isRightToLeft()) {
				os << "\\L{";
				close = true;
			}

			int tmp = inset->Latex(buf, os, moving_arg,
					       style.free_spacing);

			if (close)
				os << "}";

			if (tmp) {
				column = 0;
			} else {
				column += int(os.tellp()) - len;
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
		basefont = getFont(bparams, -1);
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
			case '\\': 
				os << "\\textbackslash{}";
				column += 15;
				break;
		
			case '°': case '±': case '²': case '³':  
			case '×': case '÷': case '¹': case 'ª':
			case 'º': case '¬': case 'µ':
				if (bparams.inputenc == "latin1" ||
				    (bparams.inputenc == "auto" &&
				     font.language()->encoding()->LatexName()
				     == "latin1")) {
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
					    && GetChar(i + 1) == c) {
						//os << "\\textcompwordmark{}";
						// Jean-Marc, have a look at
						// this. I think this works
						// equally well:
						os << "\\,{}";
						// Lgb
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
				switch (c) {
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
				if (bparams.inputenc == "default") {
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


LyXParagraph * LyXParagraph::TeXDeeper(Buffer const * buf,
				       BufferParams const & bparams,
				       ostream & os, TexRow & texrow)
{
	lyxerr[Debug::LATEX] << "TeXDeeper...     " << this << endl;
	LyXParagraph * par = this;

	while (par && par->params.depth() == params.depth()) {
		if (textclasslist.Style(bparams.textclass, 
					par->layout).isEnvironment()) {
			par = par->TeXEnvironment(buf, bparams,
						  os, texrow);
		} else {
			par = par->TeXOnePar(buf, bparams,
					     os, texrow, false);
		}
	}
	lyxerr[Debug::LATEX] << "TeXDeeper...done " << par << endl;

	return par;
}


LyXParagraph * LyXParagraph::TeXEnvironment(Buffer const * buf,
					    BufferParams const & bparams,
					    ostream & os, TexRow & texrow)
{
	lyxerr[Debug::LATEX] << "TeXEnvironment...     " << this << endl;

	LyXLayout const & style =
		textclasslist.Style(bparams.textclass,
				    layout);

	if (style.isEnvironment()){
		if (style.latextype == LATEX_LIST_ENVIRONMENT) {
			os << "\\begin{" << style.latexname() << "}{"
			   << params.labelWidthString() << "}\n";
		} else if (style.labeltype == LABEL_BIBLIO) {
			// ale970405
			os << "\\begin{" << style.latexname() << "}{"
			   <<  bibitemWidest(buf)
			   << "}\n";
		} else if (style.latextype == LATEX_ITEM_ENVIRONMENT) {
			os << "\\begin{" << style.latexname() << '}'
			   << style.latexparam() << '\n';
		} else 
			os << "\\begin{" << style.latexname() << '}'
			   << style.latexparam() << '\n';
		texrow.newline();
	}
	LyXParagraph * par = this;
	do {
		par = par->TeXOnePar(buf, bparams,
				     os, texrow, false);

		if (par && par->params.depth() > params.depth()) {
			if (textclasslist.Style(bparams.textclass,
						par->layout).isParagraph()
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
			par = par->TeXDeeper(buf, bparams, os, texrow);
		}
	} while (par
		 && par->layout == layout
		 && par->params.depth() == params.depth());
 
	if (style.isEnvironment()) {
		os << "\\end{" << style.latexname() << "}\n";
	}

	lyxerr[Debug::LATEX] << "TeXEnvironment...done " << par << endl;
	return par;  // ale970302
}


bool LyXParagraph::IsHfill(size_type pos) const
{
	return IsHfillChar(GetChar(pos));
}


bool LyXParagraph::IsInset(size_type pos) const
{
	return IsInsetChar(GetChar(pos));
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
	value_type const c = GetChar(pos);
	if (IsLetterChar(c))
		return true;
	// '\0' is not a letter, allthough every string contains "" (below)
	if (c == '\0')
		return false;
	// We want to pass the ' and escape chars to ispell
	string const extra = lyxrc.isp_esc_chars + '\'';
	char ch[2] = { c, 0 };
	return contains(extra, ch);
}
 
 
bool LyXParagraph::IsWord(size_type pos ) const
{
	return IsWordChar(GetChar(pos)) ;
}


Language const *
LyXParagraph::getParLanguage(BufferParams const & bparams) const 
{
	if (size() > 0)
		return GetFirstFontSettings().language();
	else if (previous_)
		return previous_->getParLanguage(bparams);
	else
		return bparams.language;
}


bool LyXParagraph::isRightToLeftPar(BufferParams const & bparams) const
{
	return lyxrc.rtl_support
		&& getParLanguage(bparams)->RightToLeft();
}


void LyXParagraph::ChangeLanguage(BufferParams const & bparams,
				  Language const * from, Language const * to)
{
	for (size_type i = 0; i < size(); ++i) {
		LyXFont font = GetFontSettings(bparams, i);
		if (font.language() == from) {
			font.setLanguage(to);
			SetFont(i, font);
		}
	}
}


bool LyXParagraph::isMultiLingual(BufferParams const & bparams)
{
	Language const * doc_language =	bparams.language;
	for (FontList::const_iterator cit = fontlist.begin();
	     cit != fontlist.end(); ++cit)
		if ((*cit).font().language() != doc_language)
			return true;
	return false;
}


// Convert the paragraph to a string.
// Used for building the table of contents
string const LyXParagraph::String(Buffer const * buffer, bool label)
{
	BufferParams const & bparams = buffer->params;
	string s;
	if (label && !params.labelString().empty())
		s += params.labelString() + ' ';
	string::size_type const len = s.size();

	for (LyXParagraph::size_type i = 0; i < size(); ++i) {
		value_type c = GetChar(i);
		if (IsPrintable(c))
			s += c;
		else if (c == META_INSET &&
			 GetInset(i)->LyxCode() == Inset::MATH_CODE) {
			std::ostringstream ost;
			GetInset(i)->Ascii(buffer, ost);
			s += subst(ost.str().c_str(),'\n',' ');
		}
	}

	if (isRightToLeftPar(bparams))
		reverse(s.begin() + len,s.end());

	return s;
}


string const LyXParagraph::String(Buffer const * buffer, 
			    LyXParagraph::size_type beg,
			    LyXParagraph::size_type end)
{
	string s;

	if (beg == 0 && !params.labelString().empty())
		s += params.labelString() + ' ';

	for (LyXParagraph::size_type i = beg; i < end; ++i) {
		value_type c = GetUChar(buffer->params, i);
		if (IsPrintable(c))
			s += c;
		else if (c == META_INSET) {
			std::ostringstream ost;
			GetInset(i)->Ascii(buffer, ost);
			s += ost.str().c_str();
		}
	}

	return s;
}


void LyXParagraph::SetInsetOwner(Inset * i)
{
	inset_owner = i;
	for (InsetList::const_iterator cit = insetlist.begin();
	     cit != insetlist.end(); ++cit) {
		if ((*cit).inset)
			(*cit).inset->setOwner(i);
	}
}


void LyXParagraph::deleteInsetsLyXText(BufferView * bv)
{
	// then the insets
	for (InsetList::const_iterator cit = insetlist.begin();
	     cit != insetlist.end(); ++cit) {
		if ((*cit).inset) {
			if ((*cit).inset->IsTextInset()) {
				static_cast<UpdatableInset *>
					((*cit).inset)->deleteLyXText(bv);
			}
		}
	}
}


void LyXParagraph::resizeInsetsLyXText(BufferView * bv)
{
	// then the insets
	for (InsetList::const_iterator cit = insetlist.begin();
	     cit != insetlist.end(); ++cit) {
		if ((*cit).inset) {
			if ((*cit).inset->IsTextInset()) {
				static_cast<UpdatableInset *>
					((*cit).inset)->resizeLyXText(bv);
			}
		}
	}
}


void LyXParagraph::fitToSize()
{
	TextContainer tmp(text.begin(), text.end());
	text.swap(tmp);
}


void LyXParagraph::setContentsFromPar(LyXParagraph * par)
{
	text = par->text;
}
