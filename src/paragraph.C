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
#pragma implementation
#endif

#include <algorithm>
#include <fstream>
#include <csignal>

#include "paragraph.h"
#include "paragraph_pimpl.h"
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
#include "support/lstrings.h"
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
// this is a bad idea, but how can Paragraph find its buffer to get
// parameters? (JMarc)

extern string bibitemWidest(Buffer const *);

// this is a minibuffer

namespace {

char minibuffer_char;
LyXFont minibuffer_font;
Inset * minibuffer_inset;

} // namespace anon


extern BufferView * current_view;


Paragraph::Paragraph()
		: pimpl_(new Paragraph::Pimpl(this))
{
	for (int i = 0; i < 10; ++i) setCounter(i , 0);
	next_ = 0;
	previous_ = 0;
	enumdepth = 0;
	itemdepth = 0;
        bibkey = 0; // ale970302
	clear();
}


// This konstruktor inserts the new paragraph in a list.
Paragraph::Paragraph(Paragraph * par)
		: pimpl_(new Paragraph::Pimpl(this))
{
	for (int i = 0; i < 10; ++i)
		setCounter(i, 0);
	enumdepth = 0;
	itemdepth = 0;

	// double linked list begin
	next_ = par->next_;
	if (next_)
		next_->previous_ = this;
	previous_ = par;
	previous_->next_ = this;
	// end

        bibkey = 0; // ale970302        
    
	clear();
}


Paragraph::Paragraph(Paragraph const & lp, bool same_ids)
	: pimpl_(new Paragraph::Pimpl(*lp.pimpl_, this, same_ids))
{
	for (int i = 0; i < 10; ++i)
		setCounter(i , 0);
	enumdepth = 0;
	itemdepth = 0;
	next_ = 0;
	previous_ = 0;

	// this is because of the dummy layout of the paragraphs that
	// follow footnotes
	layout = lp.layout;

        // ale970302
	if (lp.bibkey) {
		bibkey = static_cast<InsetBibKey *>
			(lp.bibkey->clone(*current_view->buffer()));
	} else {
		bibkey = 0;
	}
	
	// copy everything behind the break-position to the new paragraph

	insetlist = lp.insetlist;
	for (InsetList::iterator it = insetlist.begin();
	     it != insetlist.end(); ++it)
	{
		it->inset = it->inset->clone(*current_view->buffer(), same_ids);
	}
}


// the destructor removes the new paragraph from the list
Paragraph::~Paragraph()
{
	if (previous_)
		previous_->next_ = next_;
	if (next_)
		next_->previous_ = previous_;

	for (InsetList::iterator it = insetlist.begin();
	     it != insetlist.end(); ++it) {
		delete it->inset;
	}

        // ale970302
	delete bibkey;

	delete pimpl_;
	//
	//lyxerr << "Paragraph::paragraph_id = "
	//       << Paragraph::paragraph_id << endl;
}


void Paragraph::writeFile(Buffer const * buf, ostream & os,
                          BufferParams const & bparams,
                          depth_type dth) const
{
	// The beginning or end of a deeper (i.e. nested) area?
	if (dth != params().depth()) {
		if (params().depth() > dth) {
			while (params().depth() > dth) {
				os << "\n\\begin_deeper ";
				++dth;
			}
		} else {
			while (params().depth() < dth) {
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
	if (params().spaceTop().kind() != VSpace::NONE)
		os << "\\added_space_top "
		   << params().spaceTop().asLyXCommand() << " ";
	if (params().spaceBottom().kind() != VSpace::NONE)
		os << "\\added_space_bottom "
		   << params().spaceBottom().asLyXCommand() << " ";
	
	// Maybe the paragraph has special spacing
	params().spacing().writeFile(os, true);
	
	// The labelwidth string used in lists.
	if (!params().labelWidthString().empty())
		os << "\\labelwidthstring "
		   << params().labelWidthString() << '\n';
	
	// Lines above or below?
	if (params().lineTop())
		os << "\\line_top ";
	if (params().lineBottom())
		os << "\\line_bottom ";
	
	// Pagebreaks above or below?
	if (params().pagebreakTop())
		os << "\\pagebreak_top ";
	if (params().pagebreakBottom())
		os << "\\pagebreak_bottom ";
	
	// Start of appendix?
	if (params().startOfAppendix())
		os << "\\start_of_appendix ";
	
	// Noindent?
	if (params().noindent())
		os << "\\noindent ";
	
	// Alignment?
	if (params().align() != LYX_ALIGN_LAYOUT) {
		int h = 0;
		switch (params().align()) {
		case LYX_ALIGN_LEFT: h = 1; break;
		case LYX_ALIGN_RIGHT: h = 2; break;
		case LYX_ALIGN_CENTER: h = 3; break;
		default: h = 0; break;
		}
		os << "\\align " << string_align[h] << " ";
	}
	
	// bibitem  ale970302
	if (bibkey)
		bibkey->write(buf, os);
	
	LyXFont font1(LyXFont::ALL_INHERIT, bparams.language);
	
	int column = 0;
	for (size_type i = 0; i < size(); ++i) {
		if (!i) {
			os << "\n";
			column = 0;
		}
		
		// Write font changes
		LyXFont font2 = getFontSettings(bparams, i);
		if (font2 != font1) {
#ifndef INHERIT_LANGUAGE
			font2.lyxWriteChanges(font1, os);
#else
			font2.lyxWriteChanges(font1, bparams.language, os);
#endif
			column = 0;
			font1 = font2;
		}
		
		value_type const c = getChar(i);
		switch (c) {
		case META_INSET:
		{
			Inset const * inset = getInset(i);
			if (inset)
				if (inset->directWrite()) {
					// international char, let it write
					// code directly so it's shorter in
					// the file
					inset->write(buf, os);
				} else {
					os << "\n\\begin_inset ";
					inset->write(buf, os);
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
			if (i + 1 < size() && getChar(i + 1) == ' ') {
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
				lyxerr << "ERROR (Paragraph::writeFile):"
					" NULL char in structure." << endl;
			++column;
			break;
		}
	}
	
	// now write the next paragraph
	if (next_)
		next_->writeFile(buf, os, bparams, dth);
}


void Paragraph::validate(LaTeXFeatures & features) const
{
	BufferParams const & bparams = features.bufferParams();

	// check the params.
	if (params().lineTop() || params().lineBottom())
		features.require("lyxline");
	if (!params().spacing().isDefault())
		features.require("setspace");
	
	// then the layouts
	features.useLayout(getLayout());

	// then the fonts
	Language const * doc_language = bparams.language;
	
	for (Pimpl::FontList::const_iterator cit = pimpl_->fontlist.begin();
	     cit != pimpl_->fontlist.end(); ++cit) {
		if (cit->font().noun() == LyXFont::ON) {
			lyxerr[Debug::LATEX] << "font.noun: "
					     << cit->font().noun()
					     << endl;
			features.require("noun");
			lyxerr[Debug::LATEX] << "Noun enabled. Font: "
					     << cit->font().stateText(0)
					     << endl;
		}
		switch (cit->font().color()) {
		case LColor::none:
		case LColor::inherit:
		case LColor::ignore:
			// probably we should put here all interface colors used for
			// font displaying! For now I just add this ones I know of (Jug)
		case LColor::latex:
		case LColor::note:
			break;
		default:
			features.require("color");
			lyxerr[Debug::LATEX] << "Color enabled. Font: "
					     << cit->font().stateText(0)
					     << endl;
		}

		Language const * language = cit->font().language();
		if (language->babel() != doc_language->babel() &&
		    language != ignore_language &&
#ifdef INHERIT_LANGUAGE
		    language != inherit_language &&
#endif
		    language != latex_language)
		{
			features.useLanguage(language);
			lyxerr[Debug::LATEX] << "Found language "
					     << language->babel() << endl;
		}
	}

	// then the insets
	LyXLayout const & layout =
                textclasslist.Style(bparams.textclass,
                                    getLayout());

	for (InsetList::const_iterator cit = insetlist.begin();
	     cit != insetlist.end(); ++cit) {
		if (cit->inset) {
			cit->inset->validate(features);
			if (layout.needprotect &&
			    cit->inset->lyxCode() == Inset::FOOT_CODE)
				features.require("NeedLyXFootnoteCode");
		}
	}
}


// First few functions needed for cut and paste and paragraph breaking.
void Paragraph::copyIntoMinibuffer(Buffer const & buffer,
				      Paragraph::size_type pos) const
{
	BufferParams bparams = buffer.params;

	minibuffer_char = getChar(pos);
	minibuffer_font = getFontSettings(bparams, pos);
	minibuffer_inset = 0;
	if (minibuffer_char == Paragraph::META_INSET) {
		if (getInset(pos)) {
			minibuffer_inset = getInset(pos)->clone(buffer);
		} else {
			minibuffer_inset = 0;
			minibuffer_char = ' ';
			// This reflects what GetInset() does (ARRae)
		}
	}
}


void Paragraph::cutIntoMinibuffer(BufferParams const & bparams,
				     Paragraph::size_type pos)
{
	minibuffer_char = getChar(pos);
	minibuffer_font = getFontSettings(bparams, pos);
	minibuffer_inset = 0;
	if (minibuffer_char == Paragraph::META_INSET) {
		if (getInset(pos)) {
			minibuffer_inset = getInset(pos);
			// This is a little hack since I want exactly
			// the inset, not just a clone. Otherwise
			// the inset would be deleted when calling Erase(pos)
			// find the entry
			InsetTable search_elem(pos, 0);
			InsetList::iterator it =
				lower_bound(insetlist.begin(),
					    insetlist.end(),
					    search_elem, Pimpl::matchIT());
			if (it != insetlist.end() && it->pos == pos)
				it->inset = 0;
		} else {
			minibuffer_inset = 0;
			minibuffer_char = ' ';
			// This reflects what GetInset() does (ARRae)
		}

	}

	// Erase(pos); now the caller is responsible for that.
}


bool Paragraph::insertFromMinibuffer(Paragraph::size_type pos)
{
	if ((minibuffer_char == Paragraph::META_INSET) &&
	    !insetAllowed(minibuffer_inset->lyxCode()))
		return false;
	if (minibuffer_char == Paragraph::META_INSET)
		insertInset(pos, minibuffer_inset, minibuffer_font);
	else {
		LyXFont f = minibuffer_font;
		if (checkInsertChar(f))
			insertChar(pos, minibuffer_char, f);
	}
	return true;
}

// end of minibuffer



void Paragraph::clear()
{
	params().clear();
	
	layout = 0;
	bibkey = 0;
}


void Paragraph::erase(Paragraph::size_type pos)
{
	pimpl_->erase(pos);
}


bool Paragraph::checkInsertChar(LyXFont & font)
{
	if (pimpl_->inset_owner)
		return pimpl_->inset_owner->checkInsertChar(font);
	return true;
}


void Paragraph::insertChar(Paragraph::size_type pos, Paragraph::value_type c)
{
	LyXFont const f(LyXFont::ALL_INHERIT);
	insertChar(pos, c, f);
}


void Paragraph::insertChar(Paragraph::size_type pos, Paragraph::value_type c,
                           LyXFont const & font)
{
	pimpl_->insertChar(pos, c, font);
}


void Paragraph::insertInset(Paragraph::size_type pos, Inset * inset)
{
	LyXFont const f(LyXFont::ALL_INHERIT);
	insertInset(pos, inset, f);
}


void Paragraph::insertInset(Paragraph::size_type pos, Inset * inset,
                            LyXFont const & font)
{
	pimpl_->insertInset(pos, inset, font);
}


bool Paragraph::insetAllowed(Inset::Code code)
{
	//lyxerr << "Paragraph::InsertInsetAllowed" << endl;
	
	if (pimpl_->inset_owner)
		return pimpl_->inset_owner->insetAllowed(code);
	return true;
}


Inset * Paragraph::getInset(Paragraph::size_type pos)
{
	lyx::Assert(pos < size());

	// Find the inset.
	InsetTable search_inset(pos, 0);
	InsetList::iterator it = lower_bound(insetlist.begin(),
					     insetlist.end(),
					     search_inset, Pimpl::matchIT());
	if (it != insetlist.end() && it->pos == pos)
		return it->inset;

	lyxerr << "ERROR (Paragraph::getInset): "
		"Inset does not exist: " << pos << endl;
	//::raise(SIGSTOP);
	
	// text[pos] = ' '; // WHY!!! does this set the pos to ' '????
	// Did this commenting out introduce a bug? So far I have not
	// see any, please enlighten me. (Lgb)
	// My guess is that since the inset does not exist, we might
	// as well replace it with a space to prevent craches. (Asger)
	return 0;
}


Inset const * Paragraph::getInset(Paragraph::size_type pos) const
{
	lyx::Assert(pos < size());

	// Find the inset.
	InsetTable search_inset(pos, 0);
	InsetList::const_iterator cit = lower_bound(insetlist.begin(),
						    insetlist.end(),
						    search_inset, Pimpl::matchIT());
	if (cit != insetlist.end() && cit->pos == pos)
		return cit->inset;

	lyxerr << "ERROR (Paragraph::GetInset): "
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
LyXFont const Paragraph::getFontSettings(BufferParams const & bparams,
                                         Paragraph::size_type pos) const
{
	lyx::Assert(pos <= size());
	
	Pimpl::FontTable search_font(pos, LyXFont());
	Pimpl::FontList::const_iterator cit = lower_bound(pimpl_->fontlist.begin(),
						   pimpl_->fontlist.end(),
						   search_font, Pimpl::matchFT());
	LyXFont retfont;
	if (cit != pimpl_->fontlist.end()) {
		retfont = cit->font();
	} else if (pos == size() && size()) {
		retfont = getFontSettings(bparams, pos - 1);
	} else
		retfont = LyXFont(LyXFont::ALL_INHERIT, getParLanguage(bparams));
#ifdef INHERIT_LANGUAGE
	if (retfont.language() == inherit_language)
		retfont.setLanguage(bparams.language);
#endif

	return retfont;
}


// Gets uninstantiated font setting at position 0
LyXFont const Paragraph::getFirstFontSettings() const
{
	if (size() > 0) {
		if (!pimpl_->fontlist.empty())
			return pimpl_->fontlist[0].font();
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
LyXFont const Paragraph::getFont(BufferParams const & bparams,
				 Paragraph::size_type pos) const
{
	lyx::Assert(pos >= 0);
	
	LyXLayout const & layout =
		textclasslist.Style(bparams.textclass, 
				    getLayout());
	Paragraph::size_type main_body = 0;
	if (layout.labeltype == LABEL_MANUAL)
		main_body = beginningOfMainBody();

	LyXFont layoutfont;
	if (pos < main_body)
		layoutfont = layout.labelfont;
	else
		layoutfont = layout.font;
	
	LyXFont tmpfont = getFontSettings(bparams, pos);
#ifndef INHERIT_LANGUAGE
	tmpfont.realize(layoutfont);
#else
	tmpfont.realize(layoutfont, bparams.language);
#endif

	return pimpl_->realizeFont(tmpfont, bparams);
}


LyXFont const Paragraph::getLabelFont(BufferParams const & bparams) const
{
	LyXLayout const & layout =
		textclasslist.Style(bparams.textclass, getLayout());
	
	LyXFont tmpfont = layout.labelfont;
	tmpfont.setLanguage(getParLanguage(bparams));

	return pimpl_->realizeFont(tmpfont, bparams);
}


LyXFont const Paragraph::getLayoutFont(BufferParams const & bparams) const
{
	LyXLayout const & layout =
		textclasslist.Style(bparams.textclass, 
				    getLayout());

	LyXFont tmpfont = layout.font;
	tmpfont.setLanguage(getParLanguage(bparams));

	return pimpl_->realizeFont(tmpfont, bparams);
}


/// Returns the height of the highest font in range
LyXFont::FONT_SIZE
Paragraph::highestFontInRange(Paragraph::size_type startpos,
                              Paragraph::size_type endpos,
			      LyXFont::FONT_SIZE const def_size) const
{
	if (pimpl_->fontlist.empty())
		return def_size;

	LyXFont::FONT_SIZE maxsize = LyXFont::SIZE_TINY;
	Pimpl::FontTable end_search(endpos, LyXFont());
	Pimpl::FontList::const_iterator end_it =
		lower_bound(pimpl_->fontlist.begin(),
		            pimpl_->fontlist.end(),
		            end_search, Pimpl::matchFT());
	if (end_it != pimpl_->fontlist.end())
		++end_it;

	Pimpl::FontTable start_search(startpos, LyXFont());
	Pimpl::FontList::const_iterator cit =
		lower_bound(pimpl_->fontlist.begin(),
			    pimpl_->fontlist.end(),
			    start_search, Pimpl::matchFT());
	
	for (; cit != end_it; ++cit) {
		LyXFont::FONT_SIZE size = cit->font().size();
		if (size == LyXFont::INHERIT_SIZE)
			size = def_size;
		if (size > maxsize && size <= LyXFont::SIZE_HUGER)
			maxsize = size;
	}
	return maxsize;
}


Paragraph::value_type
Paragraph::getUChar(BufferParams const & bparams,
		       Paragraph::size_type pos) const
{
	value_type c = getChar(pos);
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
	if (uc != c && getFontSettings(bparams, pos).isRightToLeft())
		return uc;
	else
		return c;
}


void Paragraph::setFont(Paragraph::size_type pos,
			   LyXFont const & font)
{
	lyx::Assert(pos <= size());

	// First, reduce font against layout/label font
	// Update: The SetCharFont() routine in text2.C already
	// reduces font, so we don't need to do that here. (Asger)
	// No need to simplify this because it will disappear
	// in a new kernel. (Asger)
	// Next search font table

	Pimpl::FontTable search_font(pos, LyXFont());
	Pimpl::FontList::iterator it = lower_bound(pimpl_->fontlist.begin(),
					    pimpl_->fontlist.end(),
					    search_font, Pimpl::matchFT());
	unsigned int i = it - pimpl_->fontlist.begin();
	bool notfound = it == pimpl_->fontlist.end();

	if (!notfound && pimpl_->fontlist[i].font() == font)
		return;

	bool begin = pos == 0 || notfound ||
		(i > 0 && pimpl_->fontlist[i-1].pos() == pos - 1);
	// Is position pos is a beginning of a font block?
	bool end = !notfound && pimpl_->fontlist[i].pos() == pos;
	// Is position pos is the end of a font block?
	if (begin && end) { // A single char block
		if (i + 1 < pimpl_->fontlist.size() &&
		    pimpl_->fontlist[i + 1].font() == font) {
			// Merge the singleton block with the next block
			pimpl_->fontlist.erase(pimpl_->fontlist.begin() + i);
			if (i > 0 && pimpl_->fontlist[i - 1].font() == font)
				pimpl_->fontlist.erase(pimpl_->fontlist.begin() + i-1);
		} else if (i > 0 && pimpl_->fontlist[i - 1].font() == font) {
			// Merge the singleton block with the previous block
			pimpl_->fontlist[i - 1].pos(pos);
			pimpl_->fontlist.erase(pimpl_->fontlist.begin() + i);
		} else
			pimpl_->fontlist[i].font(font);
	} else if (begin) {
		if (i > 0 && pimpl_->fontlist[i - 1].font() == font)
			pimpl_->fontlist[i - 1].pos(pos);
		else
			pimpl_->fontlist.insert(pimpl_->fontlist.begin() + i,
					Pimpl::FontTable(pos, font));
	} else if (end) {
		pimpl_->fontlist[i].pos(pos - 1);
		if (!(i + 1 < pimpl_->fontlist.size() &&
		      pimpl_->fontlist[i + 1].font() == font))
			pimpl_->fontlist.insert(pimpl_->fontlist.begin() + i + 1,
					Pimpl::FontTable(pos, font));
	} else { // The general case. The block is splitted into 3 blocks
		pimpl_->fontlist.insert(pimpl_->fontlist.begin() + i, 
				Pimpl::FontTable(pos - 1, pimpl_->fontlist[i].font()));
		pimpl_->fontlist.insert(pimpl_->fontlist.begin() + i + 1,
				Pimpl::FontTable(pos, font));
	}
}



void Paragraph::next(Paragraph * p)
{
	next_ = p;
}


// This function is able to hide closed footnotes.
Paragraph * Paragraph::next()
{
	return next_;
}


Paragraph const * Paragraph::next() const
{
	return next_;
}


void Paragraph::previous(Paragraph * p)
{
	previous_ = p;
}


// This function is able to hide closed footnotes.
Paragraph * Paragraph::previous()
{
	return previous_;
}


// This function is able to hide closed footnotes.
Paragraph const * Paragraph::previous() const
{
	return previous_;
}


void Paragraph::breakParagraph(BufferParams const & bparams,
				  Paragraph::size_type pos,
				  int flag)
{
	// create a new paragraph
	Paragraph * tmp = new Paragraph(this);
	// remember to set the inset_owner
	tmp->setInsetOwner(inInset());
	
	// this is an idea for a more userfriendly layout handling, I will
	// see what the users say
	
	// layout stays the same with latex-environments
	if (flag) {
		tmp->setOnlyLayout(layout);
		tmp->setLabelWidthString(params().labelWidthString());
	}
	
	if (size() > pos || !size() || flag == 2) {
		tmp->setOnlyLayout(layout);
		tmp->params().align(params().align());
		tmp->setLabelWidthString(params().labelWidthString());
		
		tmp->params().lineBottom(params().lineBottom());
		params().lineBottom(false);
		tmp->params().pagebreakBottom(params().pagebreakBottom());
		params().pagebreakBottom(false);
		tmp->params().spaceBottom(params().spaceBottom());
		params().spaceBottom(VSpace(VSpace::NONE));
		
		tmp->params().depth(params().depth());
		tmp->params().noindent(params().noindent());
		
		// copy everything behind the break-position
		// to the new paragraph
		size_type pos_end = pimpl_->size() - 1;
		size_type i = pos;
		size_type j = pos;
		for (; i <= pos_end; ++i) {
			cutIntoMinibuffer(bparams, i);
			if (tmp->insertFromMinibuffer(j - pos))
				++j;
		}
		for (i = pos_end; i >= pos; --i) {
			erase(i);
		}
	}
	
	// just an idea of me
	if (!pos) {
		tmp->params().lineTop(params().lineTop());
		tmp->params().pagebreakTop(params().pagebreakTop());
		tmp->params().spaceTop(params().spaceTop());
		tmp->bibkey = bibkey;
		clear();
		// layout stays the same with latex-environments
		if (flag) {
			setOnlyLayout(tmp->layout);
			setLabelWidthString(tmp->params().labelWidthString());
			params().depth(tmp->params().depth());
		}
	}
}
	

void Paragraph::makeSameLayout(Paragraph const * par)
{
	layout = par->layout;
	// move to pimpl?
	params() = par->params();
}


int Paragraph::stripLeadingSpaces(LyXTextClassList::size_type tclass) 
{
	if (textclasslist.Style(tclass, getLayout()).free_spacing)
		return 0;
	
	int i = 0;
	while (size()
	       && (isNewline(0) || isLineSeparator(0))){
		erase(0);
		++i;
	}

	return i;
}


bool Paragraph::hasSameLayout(Paragraph const * par) const
{
	return 
		par->layout == layout &&
		params().sameLayout(par->params());
}


void Paragraph::breakParagraphConservative(BufferParams const & bparams,
					   Paragraph::size_type pos)
{
	// create a new paragraph
	Paragraph * tmp = new Paragraph(this);
	tmp->makeSameLayout(this);

	// When can pos > Last()?
	// I guess pos == Last() is possible.
	if (size() > pos) {
		// copy everything behind the break-position to the new
		// paragraph
		size_type pos_end = pimpl_->size() - 1;

		//size_type i = pos;
		//size_type j = pos;
		for (size_type i = pos, j = pos; i <= pos_end; ++i) {
			cutIntoMinibuffer(bparams, i);
			if (tmp->insertFromMinibuffer(j - pos))
				++j;
		}
		
		for (size_type k = pos_end; k >= pos; --k) {
			erase(k);
		}
	}
}
   

// Be carefull, this does not make any check at all.
// This method has wrong name, it combined this par with the next par.
// In that sense it is the reverse of break paragraph. (Lgb)
void Paragraph::pasteParagraph(BufferParams const & bparams)
{
	// copy the next paragraph to this one
	Paragraph * the_next = next();
   
	// first the DTP-stuff
	params().lineBottom(the_next->params().lineBottom());
	params().spaceBottom(the_next->params().spaceBottom());
	params().pagebreakBottom(the_next->params().pagebreakBottom());

	size_type pos_end = the_next->pimpl_->size() - 1;
	size_type pos_insert = size();

	// ok, now copy the paragraph
	for (size_type i = 0, j = 0; i <= pos_end; ++i) {
		the_next->cutIntoMinibuffer(bparams, i);
		if (insertFromMinibuffer(pos_insert + j))
			++j;
	}
   
	// delete the next paragraph
	Paragraph * ppar = the_next->previous_;
	Paragraph * npar = the_next->next_;
	delete the_next;
	ppar->next(npar);
}


int Paragraph::getEndLabel(BufferParams const & bparams) const
{
	Paragraph const * par = this;
	depth_type par_depth = getDepth();
	while (par) {
		LyXTextClass::LayoutList::size_type layout = par->getLayout();
		int const endlabeltype =
			textclasslist.Style(bparams.textclass,
					    layout).endlabeltype;
		if (endlabeltype != END_LABEL_NO_LABEL) {
			if (!next_)
				return endlabeltype;

			depth_type const next_depth = next_->getDepth();
			if (par_depth > next_depth ||
			    (par_depth == next_depth
			     && layout != next_->getLayout()))
				return endlabeltype;
			break;
		}
		if (par_depth == 0)
			break;
		par = par->outerHook();
		if (par)
			par_depth = par->getDepth();
	}
	return END_LABEL_NO_LABEL;
}


Paragraph::depth_type Paragraph::getDepth() const
{
	return params().depth();
}


char Paragraph::getAlign() const
{
	return params().align();
}


string const & Paragraph::getLabelstring() const
{
	return params().labelString();
}


int Paragraph::getFirstCounter(int i) const
{
	return pimpl_->counter_[i];
}


// the next two functions are for the manual labels
string const Paragraph::getLabelWidthString() const
{
	if (!params().labelWidthString().empty())
		return params().labelWidthString();
	else
		return _("Senseless with this layout!");
}


void Paragraph::setLabelWidthString(string const & s)
{
	params().labelWidthString(s);
}


void Paragraph::setOnlyLayout(LyXTextClass::size_type new_layout)
{
	layout = new_layout;
}


void Paragraph::setLayout(LyXTextClass::size_type new_layout)
{
        layout = new_layout;
	params().labelWidthString(string());
	params().align(LYX_ALIGN_LAYOUT);
	params().spaceTop(VSpace(VSpace::NONE));
	params().spaceBottom(VSpace(VSpace::NONE));
	params().spacing(Spacing(Spacing::Default));
}


// if the layout of a paragraph contains a manual label, the beginning of the 
// main body is the beginning of the second word. This is what the par-
// function returns. If the layout does not contain a label, the main
// body always starts with position 0. This differentiation is necessary,
// because there cannot be a newline or a blank <= the beginning of the 
// main body in TeX.

int Paragraph::beginningOfMainBody() const
{
	// Unroll the first two cycles of the loop
	// and remember the previous character to
	// remove unnecessary GetChar() calls
	size_type i = 0;
	if (i < size()
	    && getChar(i) != Paragraph::META_NEWLINE) {
		++i;
		char previous_char = 0;
		char temp = 0; 
		if (i < size()
		    && (previous_char = getChar(i)) != Paragraph::META_NEWLINE) {
			// Yes, this  ^ is supposed to be "= " not "=="
			++i;
			while (i < size()
			       && previous_char != ' '
			       && (temp = getChar(i)) != Paragraph::META_NEWLINE) {
				++i;
				previous_char = temp;
			}
		}
	}

	return i;
}


Paragraph * Paragraph::depthHook(depth_type depth)
{
	Paragraph * newpar = this;
  
	do {
		newpar = newpar->previous();
	} while (newpar && newpar->getDepth() > depth);
   
	if (!newpar) {
		if (previous() || getDepth())
			lyxerr << "ERROR (Paragraph::DepthHook): "
				"no hook." << endl;
		newpar = this;
	}

	return newpar;
}


Paragraph const * Paragraph::depthHook(depth_type depth) const
{
	Paragraph const * newpar = this;
   
	do {
		newpar = newpar->previous();
	} while (newpar && newpar->getDepth() > depth);
   
	if (!newpar) {
		if (previous() || getDepth())
			lyxerr << "ERROR (Paragraph::DepthHook): "
				"no hook." << endl;
		newpar = this;
	}

	return newpar;
}

Paragraph * Paragraph::outerHook()
{
	if(!getDepth())
		return 0;
	return depthHook(depth_type(getDepth() - 1));
}

Paragraph const * Paragraph::outerHook() const
{
	if(!getDepth())
		return 0;
	return depthHook(depth_type(getDepth() - 1));
}

int Paragraph::autoDeleteInsets()
{
	int count = 0;
	InsetList::size_type index = 0;
	while (index < insetlist.size()) {
		if (insetlist[index].inset && insetlist[index].inset->autoDelete()) {
			erase(insetlist[index].pos); 
			// Erase() calls to insetlist.erase(&insetlist[index])
			// so index shouldn't be increased.
			++count;
		} else
			++index;
	}
	return count;
}


Paragraph::inset_iterator
Paragraph::InsetIterator(Paragraph::size_type pos)
{
	InsetTable search_inset(pos, 0);
	InsetList::iterator it = lower_bound(insetlist.begin(),
					     insetlist.end(),
					     search_inset, Pimpl::matchIT());
	return inset_iterator(it);
}


// returns -1 if inset not found
int Paragraph::getPositionOfInset(Inset * inset) const
{
	// Find the entry.
	for (InsetList::const_iterator cit = insetlist.begin();
	     cit != insetlist.end(); ++cit) {
		if (cit->inset == inset) {
			return cit->pos;
		}
	}
	if (inset == bibkey)
		return 0;

	return -1;
}


Paragraph * Paragraph::TeXOnePar(Buffer const * buf,
				       BufferParams const & bparams,
				       ostream & os, TexRow & texrow,
				       bool moving_arg)
{
	lyxerr[Debug::LATEX] << "TeXOnePar...     " << this << endl;
	LyXLayout const & style =
		textclasslist.Style(bparams.textclass,
				    layout);

	bool further_blank_line = false;

	if (params().startOfAppendix()) {
		os << "\\appendix\n";
		texrow.newline();
	}

	if (!params().spacing().isDefault()
	    && (!previous() || !previous()->hasSameLayout(this))) {
		os << params().spacing().writeEnvirBegin() << "\n";
		texrow.newline();
	}
	
	if (tex_code_break_column && style.isCommand()){
		os << '\n';
		texrow.newline();
	}

	if (params().pagebreakTop()) {
		os << "\\newpage";
		further_blank_line = true;
	}
	if (params().spaceTop().kind() != VSpace::NONE) {
		os << params().spaceTop().asLatexCommand(bparams);
		further_blank_line = true;
	}
      
	if (params().lineTop()) {
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
	Language const * previous_language = previous()
		? previous()->getParLanguage(bparams) : doc_language;

	if (language->babel() != previous_language->babel()
	    // check if we already put language command in TeXEnvironment()
	    && !(textclasslist.Style(bparams.textclass, layout).isEnvironment()
		 && (!previous() || previous()->layout != layout ||
		     previous()->params().depth() != params().depth()))) {

		if (!lyxrc.language_command_end.empty() &&
		    previous_language->babel() != doc_language->babel()) {
			os << subst(lyxrc.language_command_end, "$$lang",
				    previous_language->babel())
			   << endl;
			texrow.newline();
		}

		if (lyxrc.language_command_end.empty() ||
		    language->babel() != doc_language->babel()) {
			os << subst(lyxrc.language_command_begin, "$$lang",
				    language->babel())
			   << endl;
			texrow.newline();
		}
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
			bibkey->latex(buf, os, false, false);
		} else
			os << "\\item ";
		break;
	case LATEX_LIST_ENVIRONMENT:
		os << "\\item ";
		break;
	default:
		break;
	}

	bool need_par = simpleTeXOnePar(buf, bparams, os, texrow, moving_arg);
 
	// Make sure that \\par is done with the font of the last
	// character if this has another size as the default.
	// This is necessary because LaTeX (and LyX on the screen)
	// calculates the space between the baselines according
	// to this font. (Matthias)
	//
	// Is this really needed ? (Dekel)
	// We do not need to use to change the font for the last paragraph
	// or for a command.
	LyXFont const font =
		(size() == 0
		 ? getLayoutFont(bparams)
		 : getFont(bparams, size() - 1));

	bool is_command = textclasslist.Style(bparams.textclass,
					      getLayout()).isCommand();
	if (style.resfont.size() != font.size() && next_ && !is_command) {
		if (!need_par)
			os << "{";
		os << "\\" << font.latexSize() << " \\par}";
	} else if (need_par) {
		os << "\\par}";
	} else if (is_command)
		os << "}";

	switch (style.latextype) {
	case LATEX_ITEM_ENVIRONMENT:
	case LATEX_LIST_ENVIRONMENT:
                if (next_ && (params().depth() < next_->params().depth())) {
                        os << '\n';
                        texrow.newline();
                }
		break;
	case LATEX_ENVIRONMENT:
		// if its the last paragraph of the current environment
		// skip it otherwise fall through
		if (next_
		    && (next_->layout != layout
			|| next_->params().depth() != params().depth()))
			break;
	default:
		// we don't need it for the last paragraph!!!
		if (next_) {
			os << '\n';
			texrow.newline();
		}
	}
	
	further_blank_line = false;
	if (params().lineBottom()) {
		os << "\\lyxline{\\" << font.latexSize() << '}';
		further_blank_line = true;
	}

	if (params().spaceBottom().kind() != VSpace::NONE) {
		os << params().spaceBottom().asLatexCommand(bparams);
		further_blank_line = true;
	}
      
	if (params().pagebreakBottom()) {
		os << "\\newpage";
		further_blank_line = true;
	}

	if (further_blank_line){
		os << '\n';
		texrow.newline();
	}

	if (!params().spacing().isDefault()
	    && (!next_ || !next_->hasSameLayout(this))) {
		os << params().spacing().writeEnvirEnd() << "\n";
		texrow.newline();
	}
	
	// we don't need it for the last paragraph!!!
	if (next_) {
		os << '\n';
		texrow.newline();
	} else {
		// Since \selectlanguage write the language to the aux file,
		// we need to reset the language at the end of footnote or
		// float.

		if (language->babel() != doc_language->babel()) {
			if (lyxrc.language_command_end.empty())
				os << subst(lyxrc.language_command_begin,
					    "$$lang",
					    doc_language->babel())
				   << endl;
			else
				os << subst(lyxrc.language_command_end,
					    "$$lang",
					    language->babel())
				   << endl;
			texrow.newline();
		}
	}

	lyxerr[Debug::LATEX] << "TeXOnePar...done " << next_ << endl;
	return next_;
}


// This one spits out the text of the paragraph
bool Paragraph::simpleTeXOnePar(Buffer const * buf,
				   BufferParams const & bparams,
				   ostream & os, TexRow & texrow,
				   bool moving_arg)
{
	lyxerr[Debug::LATEX] << "SimpleTeXOnePar...     " << this << endl;

	bool return_value = false;

	LyXLayout const & style =
		textclasslist.Style(bparams.textclass,
				    getLayout());
	LyXFont basefont;

	// Maybe we have to create a optional argument.
	size_type main_body;
	if (style.labeltype != LABEL_MANUAL)
		main_body = 0;
	else
		main_body = beginningOfMainBody();

	int column = 0;

	if (main_body > 0) {
		os << '[';
		++column;
		basefont = getLabelFont(bparams);
	} else {
		basefont = getLayoutFont(bparams);
	}

	if (main_body >= 0
	    && !pimpl_->size()) {
		if (style.isCommand()) {
			os << '{';
			++column;
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
				basefont = getLayoutFont(bparams);
				running_font = basefont;
				os << ']';
				++column;
			}
			if (style.isCommand()) {
				os << '{';
				++column;
			}

			if (params().noindent()) {
				os << "\\noindent ";
				column += 10;
			}
			switch (params().align()) {
			case LYX_ALIGN_NONE:
			case LYX_ALIGN_BLOCK:
			case LYX_ALIGN_LAYOUT:
			case LYX_ALIGN_SPECIAL:
				break;
			case LYX_ALIGN_LEFT:
				if (getParLanguage(bparams)->babel() != "hebrew") {
					os << "\\begin{flushleft}";
					column += 17;
				} else {
					os << "\\begin{flushright}";
					column += 18;
				}
				break;
			case LYX_ALIGN_RIGHT:
				if (getParLanguage(bparams)->babel() != "hebrew") {
					os << "\\begin{flushright}";
					column += 18;
				} else {
					os << "\\begin{flushleft}";
					column += 17;
				}
				break;
			case LYX_ALIGN_CENTER:
				os << "\\begin{center}";
				column += 14;
				break;
			}	 
		}

		value_type c = getChar(i);

		// Fully instantiated font
		LyXFont font = getFont(bparams, i);

		LyXFont const last_font = running_font;

		// Spaces at end of font change are simulated to be
		// outside font change, i.e. we write "\textXX{text} "
		// rather than "\textXX{text }". (Asger)
		if (open_font && c == ' ' && i <= size() - 2) {
			LyXFont const next_font = getFont(bparams, i + 1);
			if (next_font != running_font
			    && next_font != font) {
				font = next_font;
			}
		}
		
		// We end font definition before blanks
		if (font != running_font && open_font) {
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
				pimpl_->simpleTeXBlanks(os, texrow, i,
						       column, font, style);
			}
		}

		// Do we need to change font?
		if (font != running_font && i != main_body - 1) {
			column += font.latexWriteStartChanges(os, basefont,
							      last_font);
			running_font = font;
			open_font = true;
		}

		if (c == Paragraph::META_NEWLINE) {
			// newlines are handled differently here than
			// the default in SimpleTeXSpecialChars().
			if (!style.newline_allowed) {
				os << '\n';
			} else {
				if (open_font) {
					column += running_font.latexWriteEndChanges(os, basefont, basefont);
					open_font = false;
				}
				basefont = getLayoutFont(bparams);
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
			pimpl_->simpleTeXSpecialChars(buf, bparams,
						      os, texrow, moving_arg,
						      font, running_font, 
						      basefont, open_font, 
						      style, i, column, c);
		}
	}

	// If we have an open font definition, we have to close it
	if (open_font) {
#ifdef FIXED_LANGUAGE_END_DETECTION
		if (next_) {
			running_font
				.latexWriteEndChanges(os, basefont,
				                      next_->getFont(bparams,
				                      0));
		} else {
			running_font.latexWriteEndChanges(os, basefont,
			                                  basefont);
		}
#else
#ifdef WITH_WARNINGS
//#warning For now we ALWAYS have to close the foreign font settings if they are
//#warning there as we start another \selectlanguage with the next paragraph if
//#warning we are in need of this. This should be fixed sometime (Jug)
#endif
		running_font.latexWriteEndChanges(os, basefont,  basefont);
#endif
	}

	// Needed if there is an optional argument but no contents.
	if (main_body > 0 && main_body == size()) {
		os << "]~";
		return_value = false;
	}

	switch (params().align()) {
	case LYX_ALIGN_NONE:
	case LYX_ALIGN_BLOCK:
	case LYX_ALIGN_LAYOUT:
	case LYX_ALIGN_SPECIAL:
		break;
	case LYX_ALIGN_LEFT:
		if (getParLanguage(bparams)->babel() != "hebrew") {
			os << "\\end{flushleft}";
			column+= 15;
		} else {
			os << "\\end{flushright}";
			column+= 16;
		}
		break;
	case LYX_ALIGN_RIGHT:
		if (getParLanguage(bparams)->babel() != "hebrew") {
			os << "\\end{flushright}";
			column+= 16;
		} else {
			os << "\\end{flushleft}";
			column+= 15;
		}
		break;
	case LYX_ALIGN_CENTER:
		os << "\\end{center}";
		column+= 12;
		break;
	}	 

	lyxerr[Debug::LATEX] << "SimpleTeXOnePar...done " << this << endl;
	return return_value;
}


bool Paragraph::sgmlConvertChar(char c, string & sgml_string)
{
	bool retval = false;
	switch (c) {
	case Paragraph::META_HFILL:
		sgml_string.erase();
		break;
	case Paragraph::META_NEWLINE:
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


Paragraph * Paragraph::TeXEnvironment(Buffer const * buf,
					    BufferParams const & bparams,
					    ostream & os, TexRow & texrow)
{
	lyxerr[Debug::LATEX] << "TeXEnvironment...     " << this << endl;

	LyXLayout const & style =
		textclasslist.Style(bparams.textclass,
				    layout);

	Language const * language = getParLanguage(bparams);
	Language const * doc_language = bparams.language;
	Language const * previous_language = previous_
		? previous_->getParLanguage(bparams) : doc_language;
	if (language->babel() != previous_language->babel()) {

		if (!lyxrc.language_command_end.empty() &&
		    previous_language->babel() != doc_language->babel()) {
			os << subst(lyxrc.language_command_end, "$$lang",
				    previous_language->babel())
			   << endl;
			texrow.newline();
		}

		if (lyxrc.language_command_end.empty() ||
		    language->babel() != doc_language->babel()) {
			os << subst(lyxrc.language_command_begin, "$$lang",
				    language->babel())
			   << endl;
			texrow.newline();
		}
	}

	if (style.isEnvironment()){
		if (style.latextype == LATEX_LIST_ENVIRONMENT) {
			os << "\\begin{" << style.latexname() << "}{"
			   << params().labelWidthString() << "}\n";
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
	Paragraph * par = this;
	do {
		par = par->TeXOnePar(buf, bparams,
				     os, texrow, false);

		if (par && par->params().depth() > params().depth()) {
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
			par = par->pimpl_->TeXDeeper(buf, bparams, os, texrow);
		}
	} while (par
		 && par->layout == layout
		 && par->params().depth() == params().depth());
 
	if (style.isEnvironment()) {
		os << "\\end{" << style.latexname() << "}\n";
	}

	lyxerr[Debug::LATEX] << "TeXEnvironment...done " << par << endl;
	return par;  // ale970302
}


bool Paragraph::isHfill(size_type pos) const
{
	return IsHfillChar(getChar(pos));
}


bool Paragraph::isInset(size_type pos) const
{
	return IsInsetChar(getChar(pos));
}


bool Paragraph::isNewline(size_type pos) const
{
	return pos >= 0 && IsNewlineChar(getChar(pos));
}


bool Paragraph::isSeparator(size_type pos) const
{
	return IsSeparatorChar(getChar(pos));
}


bool Paragraph::isLineSeparator(size_type pos) const
{
	return IsLineSeparatorChar(getChar(pos));
}


bool Paragraph::isKomma(size_type pos) const
{
	return IsKommaChar(getChar(pos));
}


/// Used by the spellchecker
bool Paragraph::isLetter(Paragraph::size_type pos) const
{
	value_type const c = getChar(pos);
	if (IsLetterChar(c))
		return true;
	if (isInset(pos)) 
		return getInset(pos)->isLetter();
	// We want to pass the ' and escape chars to ispell
	string const extra = lyxrc.isp_esc_chars + '\'';
	return contains(extra, c);
}
 
 
bool Paragraph::isWord(size_type pos ) const
{
	return IsWordChar(getChar(pos)) ;
}


Language const *
Paragraph::getParLanguage(BufferParams const & bparams) const 
{
	if (size() > 0) {
#ifndef INHERIT_LANGUAGE
		return getFirstFontSettings().language();
#else
		Language const * lang = getFirstFontSettings().language();
#warning We should make this somewhat better, any ideas? (Jug)
		if (lang == inherit_language || lang == ignore_language)
			lang = bparams.language;
		return lang;
#endif
	} else if (previous_)
		return previous_->getParLanguage(bparams);
	else
		return bparams.language;
}


bool Paragraph::isRightToLeftPar(BufferParams const & bparams) const
{
	return lyxrc.rtl_support
		&& getParLanguage(bparams)->RightToLeft();
}


void Paragraph::changeLanguage(BufferParams const & bparams,
				  Language const * from, Language const * to)
{
	for (size_type i = 0; i < size(); ++i) {
		LyXFont font = getFontSettings(bparams, i);
		if (font.language() == from) {
			font.setLanguage(to);
			setFont(i, font);
		}
	}
}


bool Paragraph::isMultiLingual(BufferParams const & bparams)
{
	Language const * doc_language =	bparams.language;
	for (Pimpl::FontList::const_iterator cit = pimpl_->fontlist.begin();
	     cit != pimpl_->fontlist.end(); ++cit)
		if (cit->font().language() != ignore_language &&
		    cit->font().language() != latex_language &&
#ifdef INHERIT_LANGUAGE
			cit->font().language() != inherit_language &&
#endif
			cit->font().language() != doc_language)
			return true;
	return false;
}


// Convert the paragraph to a string.
// Used for building the table of contents
string const Paragraph::asString(Buffer const * buffer, bool label)
{
	BufferParams const & bparams = buffer->params;
	string s;
	if (label && !params().labelString().empty())
		s += params().labelString() + ' ';
	string::size_type const len = s.size();

	for (Paragraph::size_type i = 0; i < size(); ++i) {
		value_type c = getChar(i);
		if (IsPrintable(c))
			s += c;
		else if (c == META_INSET &&
			 getInset(i)->lyxCode() == Inset::MATH_CODE) {
			ostringstream ost;
			getInset(i)->ascii(buffer, ost);
			s += subst(ost.str().c_str(),'\n',' ');
		}
	}

	if (isRightToLeftPar(bparams))
		reverse(s.begin() + len,s.end());

	return s;
}


string const Paragraph::asString(Buffer const * buffer, 
				 Paragraph::size_type beg,
				 Paragraph::size_type end,
				 bool label)
{
	ostringstream ost;

	if (beg == 0 && label && !params().labelString().empty())
		ost << params().labelString() << ' ';

	for (Paragraph::size_type i = beg; i < end; ++i) {
		value_type const c = getUChar(buffer->params, i);
		if (IsPrintable(c))
			ost << c;
		else if (c == META_NEWLINE)
			ost << '\n';
		else if (c == META_HFILL)
			ost << '\t'; 
		else if (c == META_INSET) {
			getInset(i)->ascii(buffer, ost);
		}
	}

	return ost.str().c_str();
}


void Paragraph::setInsetOwner(Inset * i)
{
	pimpl_->inset_owner = i;
	for (InsetList::const_iterator cit = insetlist.begin();
	     cit != insetlist.end(); ++cit) {
		if (cit->inset)
			cit->inset->setOwner(i);
	}
}


void Paragraph::deleteInsetsLyXText(BufferView * bv)
{
	// then the insets
	for (InsetList::const_iterator cit = insetlist.begin();
	     cit != insetlist.end(); ++cit) {
		if (cit->inset) {
			if (cit->inset->isTextInset()) {
				static_cast<UpdatableInset *>
					(cit->inset)->deleteLyXText(bv, true);
			}
		}
	}
}


void Paragraph::resizeInsetsLyXText(BufferView * bv)
{
	// then the insets
	for (InsetList::const_iterator cit = insetlist.begin();
	     cit != insetlist.end(); ++cit)
	{
		if (cit->inset) {
			if (cit->inset->isTextInset()) {
				static_cast<UpdatableInset *>
					(cit->inset)->resizeLyXText(bv, true);
			}
		}
	}
}


void Paragraph::setContentsFromPar(Paragraph * par)
{
	pimpl_->setContentsFromPar(par);
}


Paragraph::size_type Paragraph::size() const
{
	return pimpl_->size();
}


Paragraph::value_type
Paragraph::getChar(Paragraph::size_type pos) const
{
	return pimpl_->getChar(pos);
}


int Paragraph::id() const
{
	return pimpl_->id_;
}


void  Paragraph::id(int id_arg)
{
	pimpl_->id_ = id_arg;
}


LyXTextClass::LayoutList::size_type Paragraph::getLayout() const
{
	return layout;
}


bool Paragraph::isFirstInSequence() const
{
	Paragraph const * dhook = depthHook(getDepth());
	return (dhook == this
		|| dhook->getLayout() != getLayout()
		|| dhook->getDepth() != getDepth());
}


Inset * Paragraph::inInset() const
{
	return pimpl_->inset_owner;
}


void Paragraph::clearContents()
{
	pimpl_->clear();
}


void Paragraph::setCounter(int i, int v)
{
	pimpl_->counter_[i] = v;
}


int Paragraph::getCounter(int i) const
{
	return pimpl_->counter_[i];
}


void Paragraph::incCounter(int i)
{
	pimpl_->counter_[i]++;
}


void Paragraph::setChar(size_type pos, value_type c)
{
	pimpl_->setChar(pos, c);
}


Paragraph::inset_iterator::inset_iterator(Paragraph::InsetList::iterator const & iter)
 : it(iter) 
{}


Paragraph::inset_iterator Paragraph::inset_iterator_begin()
{
	return inset_iterator(insetlist.begin());
}


Paragraph::inset_iterator Paragraph::inset_iterator_end()
{
	return inset_iterator(insetlist.end());
}


ParagraphParameters & Paragraph::params()
{
	return pimpl_->params;
}


ParagraphParameters const & Paragraph::params() const
{
	return pimpl_->params;
}


Paragraph * Paragraph::getParFromID(int id) const
{
	return pimpl_->getParFromID(id);
}
