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

#include "paragraph_pimpl.h"
#include "texrow.h"
#include "language.h"
#include "bufferparams.h"
#include "encoding.h"
#include "lyxrc.h"
#include "debug.h"
#include "support/LAssert.h"

extern int tex_code_break_column;


// Initialization of the counter for the paragraph id's,
unsigned int Paragraph::Pimpl::paragraph_id = 0;

// Initialize static member.
ShareContainer<LyXFont> Paragraph::Pimpl::FontTable::container;


Paragraph::Pimpl::Pimpl(Paragraph * owner)
	: owner_(owner)
{
	inset_owner = 0;
	id_ = paragraph_id++;
}


Paragraph::Pimpl::Pimpl(Pimpl const & p, Paragraph * owner, bool same_ids)
	: params(p.params), owner_(owner)
{
	inset_owner = p.inset_owner;
	text = p.text;
	fontlist = p.fontlist;
	if (same_ids)
		id_ = p.id_;
	else
		id_ = paragraph_id++;
}


void Paragraph::Pimpl::clear()
{
	text.clear();
}


void Paragraph::Pimpl::setContentsFromPar(Paragraph const * par)
{
	lyx::Assert(par);
	text = par->pimpl_->text;
}


Paragraph::value_type Paragraph::Pimpl::getChar(pos_type pos) const
{
	lyx::Assert(pos <= size());
	// This is stronger, and I belive that this is the assertion
	// that we should really use. (Lgb)
	//Assert(pos < size());

	// Then this has no meaning. (Lgb)
	if (!size() || pos == size()) return '\0';
	
	return text[pos];
}


void Paragraph::Pimpl::setChar(pos_type pos, value_type c)
{
	text[pos] = c;
}


void Paragraph::Pimpl::insertChar(pos_type pos, value_type c,
                                  LyXFont const & font)
{
	lyx::Assert(pos <= size());

	text.insert(text.begin() + pos, c);

	// Update the font table.
	FontTable search_font(pos, LyXFont());
	for (FontList::iterator it = std::lower_bound(fontlist.begin(),
	                                              fontlist.end(),
	                                              search_font, matchFT());
	     it != fontlist.end(); ++it)
	{
		it->pos(it->pos() + 1);
	}
   
	// Update the inset table.
	InsetTable search_inset(pos, 0);
	for (InsetList::iterator it = std::lower_bound(owner_->insetlist.begin(),
	                                               owner_->insetlist.end(),
	                                               search_inset, matchIT());
	     it != owner_->insetlist.end(); ++it)
	{
		++it->pos;
	}
	owner_->setFont(pos, font);
}


void Paragraph::Pimpl::insertInset(pos_type pos,
				   Inset * inset, LyXFont const & font)
{
	lyx::Assert(inset);
	lyx::Assert(pos <= size());
	
	insertChar(pos, META_INSET, font);
	lyx::Assert(text[pos] == META_INSET);
	
	// Add a new entry in the inset table.
	InsetTable search_inset(pos, 0);
	InsetList::iterator it = std::lower_bound(owner_->insetlist.begin(),
						  owner_->insetlist.end(),
						  search_inset, matchIT());
	if (it != owner_->insetlist.end() && it->pos == pos) {
		lyxerr << "ERROR (Paragraph::InsertInset): "
			"there is an inset in position: " << pos << std::endl;
	} else {
		owner_->insetlist.insert(it, InsetTable(pos, inset));
	}
	
	if (inset_owner)
		inset->setOwner(inset_owner);
}


void Paragraph::Pimpl::erase(pos_type pos)
{
	lyx::Assert(pos < size());
	// if it is an inset, delete the inset entry 
	if (text[pos] == Paragraph::META_INSET) {
		// find the entry
		InsetTable search_inset(pos, 0);
		InsetList::iterator it =
			std::lower_bound(owner_->insetlist.begin(),
					 owner_->insetlist.end(),
					 search_inset, matchIT());
		if (it != owner_->insetlist.end() && it->pos == pos) {
			delete it->inset;
			owner_->insetlist.erase(it);
		}
	}
	
	text.erase(text.begin() + pos);
	
	// Erase entries in the tables.
	FontTable search_font(pos, LyXFont());
	
	FontList::iterator it =
		std::lower_bound(fontlist.begin(),
			    fontlist.end(),
			    search_font, matchFT());
	if (it != fontlist.end() && it->pos() == pos &&
	    (pos == 0 || 
	     (it != fontlist.begin() 
	      && boost::prior(it)->pos() == pos - 1))) {
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
		it->pos(it->pos() - 1);
	
	// Update the inset table.
	InsetTable search_inset(pos, 0);
	InsetList::iterator lend = owner_->insetlist.end();
	for (InsetList::iterator it =
		     std::upper_bound(owner_->insetlist.begin(),
				      lend,
				      search_inset, matchIT());
	     it != lend; ++it)
		--it->pos;
}


void Paragraph::Pimpl::simpleTeXBlanks(std::ostream & os, TexRow & texrow,
				       pos_type const i,
				       int & column, LyXFont const & font,
				       LyXLayout const & style)
{
	if (style.pass_thru) return;
	if (column > tex_code_break_column
	    && i 
	    && getChar(i - 1) != ' '
	    && (i < size() - 1)
	    // same in FreeSpacing mode
	    && !style.free_spacing
	    // In typewriter mode, we want to avoid 
	    // ! . ? : at the end of a line
	    && !(font.family() == LyXFont::TYPEWRITER_FAMILY
		 && (getChar(i - 1) == '.'
		     || getChar(i - 1) == '?' 
		     || getChar(i - 1) == ':'
		     || getChar(i - 1) == '!'))) {
		if (tex_code_break_column == 0) {
			// in batchmode we need LaTeX to still
			// see it as a space not as an extra '\n'
			os << " %\n";
		} else {
			os << '\n';
		}
		texrow.newline();
		texrow.start(owner_, i + 1);
		column = 0;
	} else if (style.free_spacing) {
		os << '~';
	} else {
		os << ' ';
	}
}


bool Paragraph::Pimpl::isTextAt(string const & str, pos_type pos)
{
	for (int i=0; i < str.length(); ++i) {
		if (pos + i >= size())
			return false;
		if (str[i] != getChar(pos + i))
			return false;
	}
	return true;
}

 
void Paragraph::Pimpl::simpleTeXSpecialChars(Buffer const * buf,
					     BufferParams const & bparams,
					     std::ostream & os,
					     TexRow & texrow,
					     bool moving_arg,
					     LyXFont & font,
					     LyXFont & running_font,
					     LyXFont & basefont,
					     bool & open_font,
					     LyXLayout const & style,
					     pos_type & i,
					     int & column,
					     value_type const c)
{
	if (style.pass_thru) {
		if (c != '\0') os << c;
		return;
	}
	// Two major modes:  LaTeX or plain
	// Handle here those cases common to both modes
	// and then split to handle the two modes separately.
	switch (c) {
	case Paragraph::META_INSET: {
		Inset * inset = owner_->getInset(i);
		if (inset) {
			bool close = false;
			int const len = os.tellp();
			//ostream::pos_type const len = os.tellp();
			if ((inset->lyxCode() == Inset::GRAPHICS_CODE
			     || inset->lyxCode() == Inset::MATH_CODE
			     || inset->lyxCode() == Inset::URL_CODE)
			    && running_font.isRightToLeft()) {
				os << "\\L{";
				close = true;
			}

			int tmp = inset->latex(buf, os, moving_arg,
					       style.free_spacing);

			if (close)
				os << "}";

			if (tmp) {
				for (int j = 0; j < tmp; ++j) {
					texrow.newline();
				}
				texrow.start(owner_, i + 1);
				column = 0;
			} else {
				column += int(os.tellp()) - len;
			}
		}
	}
	break;

	case Paragraph::META_NEWLINE:
		if (open_font) {
			column += running_font.latexWriteEndChanges(os,
								    basefont,
								    basefont);
			open_font = false;
		}
		basefont = owner_->getLayoutFont(bparams);
		running_font = basefont;
		break;

	case Paragraph::META_HFILL: 
		os << "\\hfill{}";
		column += 7;
		break;

	default:
		// And now for the special cases within each mode

		switch (c) {
		case '\\': 
			os << "\\textbackslash{}";
			column += 15;
			break;
			
		case '°': case '±': case '²': case '³':  
		case '×': case '÷': case '¹': case 'ª':
		case 'º': case '¬': case 'µ':
			if ((bparams.inputenc == "latin1" ||
			     bparams.inputenc == "latin9") ||
			    (bparams.inputenc == "auto" &&
			     (font.language()->encoding()->LatexName()
			      == "latin1" ||
			      font.language()->encoding()->LatexName()
			      == "latin9"))) {
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
				    && getChar(i + 1) == c) {
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
			    && getChar(i + 1) == '-'
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
 
			// I assume this is hack treating typewriter as verbatim
			if (font.family() == LyXFont::TYPEWRITER_FAMILY) {
				if (c != '\0') {
					os << c;
				}
				break;
			}
				
			if (isTextAt("LyX", i)) {
				os << "\\LyX{}";
				i += 2;
				column += 5;
			} else if (isTextAt("TeX", i)) {
				os << "\\TeX{}";
				i += 2;
				column += 5;
			} else if (isTextAt("LaTeX2e", i)) {
				os << "\\LaTeXe{}";
				i += 6;
				column += 8;
			} else if (isTextAt("LaTeX", i)) {
				os << "\\LaTeX{}";
				i += 4;
				column += 7;
			// do we really try to print out '\0' ?
			} else if (c != '\0') {
				os << c;
			}
			break;
		}
	}
}



Paragraph * Paragraph::Pimpl::TeXDeeper(Buffer const * buf,
					BufferParams const & bparams,
					std::ostream & os, TexRow & texrow)
{
	lyxerr[Debug::LATEX] << "TeXDeeper...     " << this << std::endl;
	Paragraph * par = owner_;

	while (par && par->params().depth() == owner_->params().depth()) {
		if (textclasslist.Style(bparams.textclass, 
					par->layout).isEnvironment()) {
			par = par->TeXEnvironment(buf, bparams,
						  os, texrow);
		} else {
			par = par->TeXOnePar(buf, bparams,
					     os, texrow, false);
		}
	}
	lyxerr[Debug::LATEX] << "TeXDeeper...done " << par << std::endl;

	return par;
}


Paragraph * Paragraph::Pimpl::getParFromID(int id) const
{
	InsetList::const_iterator cit = owner_->insetlist.begin();
	InsetList::const_iterator lend = owner_->insetlist.end();
	Paragraph * result;
	for (; cit != lend; ++cit) {
		if ((result = cit->inset->getParFromID(id)))
			return result;
	}
	return 0;
}


LyXFont const Paragraph::Pimpl::realizeFont(LyXFont const & font,
					    BufferParams const & bparams) const
{
	LyXFont tmpfont(font);
	
	// check for environment font information
	char par_depth = owner_->getDepth();
	Paragraph const * par = owner_;
	while (par && par->getDepth() && !tmpfont.resolved()) {
		par = par->outerHook();
		if (par) {
			tmpfont.realize(textclasslist.
					Style(bparams.textclass,
					      par->getLayout()).font
#ifdef INHERIT_LANGUAGE
					, bparams.language
#endif
					);
			par_depth = par->getDepth();
		}
	}

	tmpfont.realize(textclasslist.TextClass(bparams.textclass)
			.defaultfont()
#ifdef INHERIT_LANGUAGE
		, bparams.language
#endif
		);
	return tmpfont;	
}
