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

#include "paragraph.h"
#include "paragraph_pimpl.h"

#include "buffer.h"
#include "bufferparams.h"
#include "BufferView.h"
#include "changes.h"
#include "encoding.h"
#include "debug.h"
#include "gettext.h"
#include "language.h"
#include "latexrunparams.h"
#include "layout.h"
#include "lyxrc.h"
#include "paragraph_funcs.h"
#include "ParameterStruct.h"
#include "texrow.h"

#include "Lsstream.h"

#include "insets/insetbibitem.h"
#include "insets/insetoptarg.h"
#include "insets/insetenv.h"

#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/lyxmanip.h"
#include "support/FileInfo.h"
#include "support/LAssert.h"
#include "support/textutils.h"

#include <algorithm>
#include <fstream>
#include <csignal>
#include <ctime>

using namespace lyx::support;

using std::ostream;
using std::endl;
using std::fstream;
using std::ios;
using std::lower_bound;
using std::upper_bound;

using lyx::pos_type;


Paragraph::Paragraph()
	: pimpl_(new Paragraph::Pimpl(this))
{
	enumdepth = 0;
	itemdepth = 0;
	params().clear();
}


Paragraph::Paragraph(Paragraph const & lp)
	: pimpl_(new Paragraph::Pimpl(*lp.pimpl_, this))
{
	enumdepth = 0;
	itemdepth = 0;
	// this is because of the dummy layout of the paragraphs that
	// follow footnotes
	layout_ = lp.layout();

	// copy everything behind the break-position to the new paragraph
	insetlist = lp.insetlist;
	InsetList::iterator it = insetlist.begin();
	InsetList::iterator end = insetlist.end();
	for (; it != end; ++it) {
		// currently we hold Inset*, not InsetBase*
		it->inset = static_cast<InsetOld*>(it->inset->clone().release());
		// tell the new inset who is the boss now
		it->inset->parOwner(this);
	}
}


void Paragraph::operator=(Paragraph const & lp)
{
	// needed as we will destroy the pimpl_ before copying it
	if (&lp != this)
		return;
	lyxerr << "Paragraph::operator=()\n";
	delete pimpl_;
	pimpl_ = new Pimpl(*lp.pimpl_, this);

	enumdepth = lp.enumdepth;
	itemdepth = lp.itemdepth;
	// this is because of the dummy layout of the paragraphs that
	// follow footnotes
	layout_ = lp.layout();

	// copy everything behind the break-position to the new paragraph
	insetlist = lp.insetlist;
	InsetList::iterator it = insetlist.begin();
	InsetList::iterator end = insetlist.end();
	for (; it != end; ++it) {
		it->inset = static_cast<InsetOld*>(it->inset->clone().release());
		// tell the new inset who is the boss now
		it->inset->parOwner(this);
	}
}

// the destructor removes the new paragraph from the list
Paragraph::~Paragraph()
{
	delete pimpl_;
	//
	//lyxerr << "Paragraph::paragraph_id = "
	//       << Paragraph::paragraph_id << endl;
}


void Paragraph::write(Buffer const * buf, ostream & os,
			  BufferParams const & bparams,
			  depth_type & dth) const
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
	os << "\n\\begin_layout " << layout()->name() << '\n';

	params().write(os);

	LyXFont font1(LyXFont::ALL_INHERIT, bparams.language);

	Change running_change = Change(Change::UNCHANGED);
	lyx::time_type const curtime(lyx::current_time());

	int column = 0;
	for (pos_type i = 0; i < size(); ++i) {
		if (!i) {
			os << '\n';
			column = 0;
		}

		Change change = pimpl_->lookupChangeFull(i);
		Changes::lyxMarkChange(os, column, curtime, running_change, change);
		running_change = change;

		// Write font changes
		LyXFont font2 = getFontSettings(bparams, i);
		if (font2 != font1) {
			font2.lyxWriteChanges(font1, os);
			column = 0;
			font1 = font2;
		}

		value_type const c = getChar(i);
		switch (c) {
		case META_INSET:
		{
			InsetOld const * inset = getInset(i);
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
		case '\\':
			os << "\n\\backslash \n";
			column = 0;
			break;
		case '.':
			if (i + 1 < size() && getChar(i + 1) == ' ') {
				os << ".\n";
				column = 0;
			} else
				os << '.';
			break;
		default:
			if ((column > 70 && c == ' ')
			    || column > 79) {
				os << '\n';
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

	// to make reading work properly
	if (!size()) {
		running_change = pimpl_->lookupChange(0);
		Changes::lyxMarkChange(os, column, curtime,
			Change(Change::UNCHANGED), running_change);
	}
	Changes::lyxMarkChange(os, column, curtime,
		running_change, Change(Change::UNCHANGED));

	os << "\n\\end_layout\n";
}


void Paragraph::validate(LaTeXFeatures & features) const
{
	pimpl_->validate(features, *layout());
}


void Paragraph::eraseIntern(lyx::pos_type pos)
{
	pimpl_->eraseIntern(pos);
}


bool Paragraph::erase(pos_type pos)
{
	return pimpl_->erase(pos);
}


int Paragraph::erase(pos_type start, pos_type end)
{
	return pimpl_->erase(start, end);
}


bool Paragraph::checkInsertChar(LyXFont & font)
{
	if (pimpl_->inset_owner)
		return pimpl_->inset_owner->checkInsertChar(font);
	return true;
}


void Paragraph::insertChar(pos_type pos, Paragraph::value_type c)
{
	insertChar(pos, c, LyXFont(LyXFont::ALL_INHERIT));
}


void Paragraph::insertChar(pos_type pos, Paragraph::value_type c,
			   LyXFont const & font, Change change)
{
	pimpl_->insertChar(pos, c, font, change);
}


void Paragraph::insertInset(pos_type pos, InsetOld * inset)
{
	insertInset(pos, inset, LyXFont(LyXFont::ALL_INHERIT));
}


void Paragraph::insertInset(pos_type pos, InsetOld * inset, LyXFont const & font, Change change)
{
	pimpl_->insertInset(pos, inset, font, change);
}


bool Paragraph::insetAllowed(InsetOld::Code code)
{
	//lyxerr << "Paragraph::InsertInsetAllowed" << endl;
	if (pimpl_->inset_owner)
		return pimpl_->inset_owner->insetAllowed(code);
	return true;
}


InsetOld * Paragraph::getInset(pos_type pos)
{
	Assert(pos < size());
	return insetlist.get(pos);
}


InsetOld const * Paragraph::getInset(pos_type pos) const
{
	Assert(pos < size());
	return insetlist.get(pos);
}


// Gets uninstantiated font setting at position.
LyXFont const Paragraph::getFontSettings(BufferParams const & bparams,
					 pos_type pos) const
{
	Assert(pos <= size());

	Pimpl::FontList::const_iterator cit = pimpl_->fontlist.begin();
	Pimpl::FontList::const_iterator end = pimpl_->fontlist.end();
	for (; cit != end; ++cit)
		if (cit->pos() >= pos)
			break;

	if (cit != end)
		return cit->font();

	if (pos == size() && !empty())
		return getFontSettings(bparams, pos - 1);

	return LyXFont(LyXFont::ALL_INHERIT, getParLanguage(bparams));
}


lyx::pos_type Paragraph::getEndPosOfFontSpan(lyx::pos_type pos) const
{
	Assert(pos <= size());

	Pimpl::FontList::const_iterator cit = pimpl_->fontlist.begin();
	Pimpl::FontList::const_iterator end = pimpl_->fontlist.end();
	for (; cit != end; ++cit)
		if (cit->pos() >= pos)
			return cit->pos();

	// This should not happen, but if so, we take no chances.
	lyxerr << "Pararaph::getEndPosOfFontSpan: This should not happen!\n";
	return pos;
}


// Gets uninstantiated font setting at position 0
LyXFont const Paragraph::getFirstFontSettings() const
{
	if (!empty() && !pimpl_->fontlist.empty())
		return pimpl_->fontlist[0].font();

	return LyXFont(LyXFont::ALL_INHERIT);
}


// Gets the fully instantiated font at a given position in a paragraph
// This is basically the same function as LyXText::GetFont() in text2.C.
// The difference is that this one is used for generating the LaTeX file,
// and thus cosmetic "improvements" are disallowed: This has to deliver
// the true picture of the buffer. (Asger)
LyXFont const Paragraph::getFont(BufferParams const & bparams, pos_type pos,
				 LyXFont const & outerfont) const
{
	Assert(pos >= 0);

	LyXLayout_ptr const & lout = layout();

	pos_type const body_pos = beginningOfBody();

	LyXFont layoutfont;
	if (pos < body_pos)
		layoutfont = lout->labelfont;
	else
		layoutfont = lout->font;

	LyXFont tmpfont = getFontSettings(bparams, pos);
	tmpfont.realize(layoutfont);
	tmpfont.realize(outerfont);
	tmpfont.realize(bparams.getLyXTextClass().defaultfont());

	return tmpfont;
}


LyXFont const Paragraph::getLabelFont(BufferParams const & bparams,
				      LyXFont const & outerfont) const
{
	LyXFont tmpfont = layout()->labelfont;
	tmpfont.setLanguage(getParLanguage(bparams));
	tmpfont.realize(outerfont);
	tmpfont.realize(bparams.getLyXTextClass().defaultfont());
	return tmpfont;
}


LyXFont const Paragraph::getLayoutFont(BufferParams const & bparams,
				       LyXFont const & outerfont) const
{
	LyXFont tmpfont = layout()->font;
	tmpfont.setLanguage(getParLanguage(bparams));
	tmpfont.realize(outerfont);
	tmpfont.realize(bparams.getLyXTextClass().defaultfont());
	return tmpfont;
}


/// Returns the height of the highest font in range
LyXFont::FONT_SIZE
Paragraph::highestFontInRange(pos_type startpos, pos_type endpos,
			      LyXFont::FONT_SIZE const def_size) const
{
	if (pimpl_->fontlist.empty())
		return def_size;

	Pimpl::FontList::const_iterator end_it = pimpl_->fontlist.begin();
	Pimpl::FontList::const_iterator end = pimpl_->fontlist.end();
	for (; end_it != end; ++end_it) {
		if (end_it->pos() >= endpos)
			break;
	}

	if (end_it != end)
		++end_it;

	Pimpl::FontList::const_iterator cit = pimpl_->fontlist.begin();
	for (; cit != end; ++cit) {
		if (cit->pos() >= startpos)
			break;
	}

	LyXFont::FONT_SIZE maxsize = LyXFont::SIZE_TINY;
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
Paragraph::getUChar(BufferParams const & bparams, pos_type pos) const
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


void Paragraph::setFont(pos_type pos, LyXFont const & font)
{
	Assert(pos <= size());

	// First, reduce font against layout/label font
	// Update: The SetCharFont() routine in text2.C already
	// reduces font, so we don't need to do that here. (Asger)
	// No need to simplify this because it will disappear
	// in a new kernel. (Asger)
	// Next search font table

	Pimpl::FontList::iterator beg = pimpl_->fontlist.begin();
	Pimpl::FontList::iterator it = beg;
	Pimpl::FontList::iterator endit = pimpl_->fontlist.end();
	for (; it != endit; ++it) {
		if (it->pos() >= pos)
			break;
	}
	unsigned int i = std::distance(beg, it);
	bool notfound = (it == endit);

	if (!notfound && pimpl_->fontlist[i].font() == font)
		return;

	bool begin = pos == 0 || notfound ||
		(i > 0 && pimpl_->fontlist[i - 1].pos() == pos - 1);
	// Is position pos is a beginning of a font block?
	bool end = !notfound && pimpl_->fontlist[i].pos() == pos;
	// Is position pos is the end of a font block?
	if (begin && end) { // A single char block
		if (i + 1 < pimpl_->fontlist.size() &&
		    pimpl_->fontlist[i + 1].font() == font) {
			// Merge the singleton block with the next block
			pimpl_->fontlist.erase(pimpl_->fontlist.begin() + i);
			if (i > 0 && pimpl_->fontlist[i - 1].font() == font)
				pimpl_->fontlist.erase(pimpl_->fontlist.begin() + i - 1);
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


void Paragraph::makeSameLayout(Paragraph const & par)
{
	layout(par.layout());
	// move to pimpl?
	params() = par.params();
}


int Paragraph::stripLeadingSpaces()
{
	if (isFreeSpacing())
		return 0;

	int i = 0;
	while (!empty() && (isNewline(0) || isLineSeparator(0))) {
		pimpl_->eraseIntern(0);
		++i;
	}

	return i;
}


bool Paragraph::hasSameLayout(Paragraph const & par) const
{
	return
		par.layout() == layout() &&
		params().sameLayout(par.params());
}


Paragraph::depth_type Paragraph::getDepth() const
{
	return params().depth();
}


Paragraph::depth_type Paragraph::getMaxDepthAfter() const
{
	if (layout()->isEnvironment())
		return params().depth() + 1;
	else
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


void Paragraph::applyLayout(LyXLayout_ptr const & new_layout)
{
	layout(new_layout);
	params().labelWidthString(string());
	params().align(LYX_ALIGN_LAYOUT);
	params().spaceTop(VSpace(VSpace::NONE));
	params().spaceBottom(VSpace(VSpace::NONE));
	params().spacing(Spacing(Spacing::Default));
}


int Paragraph::beginningOfBody() const
{
	if (layout()->labeltype != LABEL_MANUAL)
		return 0;

	// Unroll the first two cycles of the loop
	// and remember the previous character to
	// remove unnecessary GetChar() calls
	pos_type i = 0;
	if (i < size() && !isNewline(i)) {
		++i;
		char previous_char = 0;
		char temp = 0;
		if (i < size()) {
			previous_char = getChar(i);
			if (!isNewline(i)) {
				++i;
				while (i < size() && previous_char != ' ') {
					temp = getChar(i);
					if (isNewline(i))
						break;

					++i;
					previous_char = temp;
				}
			}
		}
	}

	return i;
}


// returns -1 if inset not found
int Paragraph::getPositionOfInset(InsetOld const * inset) const
{
	// Find the entry.
	InsetList::const_iterator it = insetlist.begin();
	InsetList::const_iterator end = insetlist.end();
	for (; it != end; ++it)
		if (it->inset == inset)
			return it->pos;
	return -1;
}


InsetBibitem * Paragraph::bibitem() const
{
	InsetList::const_iterator it = insetlist.begin();
	if (it != insetlist.end() && it->inset->lyxCode() == InsetOld::BIBTEX_CODE)
		return static_cast<InsetBibitem *>(it->inset);
	return 0;
}



// This could go to ParagraphParameters if we want to
int Paragraph::startTeXParParams(BufferParams const & bparams,
				 ostream & os, bool moving_arg) const
{
	int column = 0;

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
	case LYX_ALIGN_RIGHT:
	case LYX_ALIGN_CENTER:
		if (moving_arg) {
			os << "\\protect";
			column = 8;
		}
		break;
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

	return column;
}


// This could go to ParagraphParameters if we want to
int Paragraph::endTeXParParams(BufferParams const & bparams,
			       ostream & os, bool moving_arg) const
{
	int column = 0;

	switch (params().align()) {
	case LYX_ALIGN_NONE:
	case LYX_ALIGN_BLOCK:
	case LYX_ALIGN_LAYOUT:
	case LYX_ALIGN_SPECIAL:
		break;
	case LYX_ALIGN_LEFT:
	case LYX_ALIGN_RIGHT:
	case LYX_ALIGN_CENTER:
		if (moving_arg) {
			os << "\\protect";
			column = 8;
		}
		break;
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
			column = 15;
		} else {
			os << "\\end{flushright}";
			column = 16;
		}
		break;
	case LYX_ALIGN_RIGHT:
		if (getParLanguage(bparams)->babel() != "hebrew") {
			os << "\\end{flushright}";
			column+= 16;
		} else {
			os << "\\end{flushleft}";
			column = 15;
		}
		break;
	case LYX_ALIGN_CENTER:
		os << "\\end{center}";
		column = 12;
		break;
	}
	return column;
}


// This one spits out the text of the paragraph
bool Paragraph::simpleTeXOnePar(Buffer const * buf,
				BufferParams const & bparams,
				LyXFont const & outerfont,
				ostream & os, TexRow & texrow,
				LatexRunParams const & runparams)
{
	lyxerr[Debug::LATEX] << "SimpleTeXOnePar...     " << this << endl;

	bool return_value = false;

	LyXLayout_ptr style;

	// well we have to check if we are in an inset with unlimited
	// lenght (all in one row) if that is true then we don't allow
	// any special options in the paragraph and also we don't allow
	// any environment other then "Standard" to be valid!
	bool asdefault =
		(inInset() && inInset()->forceDefaultParagraphs(inInset()));

	if (asdefault) {
		style = bparams.getLyXTextClass().defaultLayout();
	} else {
		style = layout();
	}

	LyXFont basefont;

	// Maybe we have to create a optional argument.
	pos_type body_pos;

	// FIXME: can we actually skip this check and just call
	// beginningOfBody() ??
	if (style->labeltype != LABEL_MANUAL) {
		body_pos = 0;
	} else {
		body_pos = beginningOfBody();
	}

	unsigned int column = 0;

	if (body_pos > 0) {
		os << '[';
		++column;
		basefont = getLabelFont(bparams, outerfont);
	} else {
		basefont = getLayoutFont(bparams, outerfont);
	}

	bool moving_arg = runparams.moving_arg;
	moving_arg |= style->needprotect;

	// Which font is currently active?
	LyXFont running_font(basefont);
	// Do we have an open font change?
	bool open_font = false;

	Change::Type running_change = Change::UNCHANGED;

	texrow.start(id(), 0);

	// if the paragraph is empty, the loop will not be entered at all
	if (empty()) {
		if (style->isCommand()) {
			os << '{';
			++column;
		}
		if (!asdefault)
			column += startTeXParParams(bparams, os, moving_arg);

	}

	for (pos_type i = 0; i < size(); ++i) {
		++column;
		// First char in paragraph or after label?
		if (i == body_pos) {
			if (body_pos > 0) {
				if (open_font) {
					column += running_font.latexWriteEndChanges(os, basefont, basefont);
					open_font = false;
				}
				basefont = getLayoutFont(bparams, outerfont);
				running_font = basefont;
				os << ']';
				++column;
			}
			if (style->isCommand()) {
				os << '{';
				++column;
			}

			if (!asdefault)
				column += startTeXParParams(bparams, os,
							    moving_arg);
		}

		value_type c = getChar(i);

		// Fully instantiated font
		LyXFont font = getFont(bparams, i, outerfont);

		LyXFont const last_font = running_font;

		// Spaces at end of font change are simulated to be
		// outside font change, i.e. we write "\textXX{text} "
		// rather than "\textXX{text }". (Asger)
		if (open_font && c == ' ' && i <= size() - 2) {
			LyXFont const & next_font = getFont(bparams, i + 1, outerfont);
			if (next_font != running_font
			    && next_font != font) {
				font = next_font;
			}
		}

		// We end font definition before blanks
		if (open_font &&
		    (font != running_font ||
		     font.language() != running_font.language()))
		{
			column += running_font.latexWriteEndChanges(os,
								    basefont,
								    (i == body_pos-1) ? basefont : font);
			running_font = basefont;
			open_font = false;
		}

		// Blanks are printed before start of fontswitch
		if (c == ' ') {
			// Do not print the separation of the optional argument
			if (i != body_pos - 1) {
				pimpl_->simpleTeXBlanks(os, texrow, i,
						       column, font, *style);
			}
		}

		// Do we need to change font?
		if ((font != running_font ||
		     font.language() != running_font.language()) &&
			i != body_pos - 1)
		{
			column += font.latexWriteStartChanges(os, basefont,
							      last_font);
			running_font = font;
			open_font = true;
		}

		Change::Type change = pimpl_->lookupChange(i);

		column += Changes::latexMarkChange(os, running_change, change);
		running_change = change;

		LatexRunParams rp = runparams;
		rp.moving_arg = moving_arg;
		rp.free_spacing = style->free_spacing;
		pimpl_->simpleTeXSpecialChars(buf, bparams,
					      os, texrow, runparams,
					      font, running_font,
					      basefont, outerfont, open_font,
					      running_change,
					      *style, i, column, c);
	}

	column += Changes::latexMarkChange(os,
			running_change, Change::UNCHANGED);

	// If we have an open font definition, we have to close it
	if (open_font) {
#ifdef FIXED_LANGUAGE_END_DETECTION
		if (next_) {
			running_font
				.latexWriteEndChanges(os, basefont,
						      next_->getFont(bparams,
						      0, outerfont));
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
	if (body_pos > 0 && body_pos == size()) {
		os << "]~";
		return_value = false;
	}

	if (!asdefault) {
		column += endTeXParParams(bparams, os, moving_arg);
	}

	lyxerr[Debug::LATEX] << "SimpleTeXOnePar...done " << this << endl;
	return return_value;
}




bool Paragraph::isHfill(pos_type pos) const
{
	return IsInsetChar(getChar(pos))
	       && getInset(pos)->lyxCode() == InsetOld::HFILL_CODE;
}


bool Paragraph::isInset(pos_type pos) const
{
	return IsInsetChar(getChar(pos));
}


bool Paragraph::isNewline(pos_type pos) const
{
	return IsInsetChar(getChar(pos))
	       && getInset(pos)->lyxCode() == InsetOld::NEWLINE_CODE;
}


bool Paragraph::isSeparator(pos_type pos) const
{
	return IsSeparatorChar(getChar(pos));
}


bool Paragraph::isLineSeparator(pos_type pos) const
{
	value_type const c = getChar(pos);
	return IsLineSeparatorChar(c)
		|| (IsInsetChar(c) && getInset(pos) &&
		getInset(pos)->isLineSeparator());
}


bool Paragraph::isKomma(pos_type pos) const
{
	return IsKommaChar(getChar(pos));
}


/// Used by the spellchecker
bool Paragraph::isLetter(pos_type pos) const
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


bool Paragraph::isWord(pos_type pos) const
{
	return IsWordChar(getChar(pos)) ;
}


Language const *
Paragraph::getParLanguage(BufferParams const & bparams) const
{
	if (!empty())
		return getFirstFontSettings().language();
#warning FIXME we should check the prev par as well (Lgb)
	return bparams.language;
}


bool Paragraph::isRightToLeftPar(BufferParams const & bparams) const
{
	return lyxrc.rtl_support
		&& getParLanguage(bparams)->RightToLeft()
		&& !(inInset() && inInset()->owner() &&
		     inInset()->owner()->lyxCode() == InsetOld::ERT_CODE);
}


void Paragraph::changeLanguage(BufferParams const & bparams,
			       Language const * from, Language const * to)
{
	for (pos_type i = 0; i < size(); ++i) {
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
	Pimpl::FontList::const_iterator cit = pimpl_->fontlist.begin();
	Pimpl::FontList::const_iterator end = pimpl_->fontlist.end();

	for (; cit != end; ++cit)
		if (cit->font().language() != ignore_language &&
		    cit->font().language() != latex_language &&
		    cit->font().language() != doc_language)
			return true;
	return false;
}


// Convert the paragraph to a string.
// Used for building the table of contents
string const Paragraph::asString(Buffer const * buffer, bool label) const
{
#if 0
	string s;
	if (label && !params().labelString().empty())
		s += params().labelString() + ' ';

	for (pos_type i = 0; i < size(); ++i) {
		value_type c = getChar(i);
		if (IsPrintable(c))
			s += c;
		else if (c == META_INSET &&
			 getInset(i)->lyxCode() == InsetOld::MATH_CODE) {
			ostringstream ost;
			getInset(i)->ascii(buffer, ost);
			s += subst(STRCONV(ost.str()),'\n',' ');
		}
	}

	return s;
#else
	// This should really be done by the caller and not here.
	string ret(asString(buffer, 0, size(), label));
	return subst(ret, '\n', ' ');
#endif
}


string const Paragraph::asString(Buffer const * buffer,
				 pos_type beg, pos_type end, bool label) const
{
	ostringstream os;

	if (beg == 0 && label && !params().labelString().empty())
		os << params().labelString() << ' ';

	for (pos_type i = beg; i < end; ++i) {
		value_type const c = getUChar(buffer->params, i);
		if (IsPrintable(c))
			os << c;
		else if (c == META_INSET)
			getInset(i)->ascii(buffer, os);
	}

	return STRCONV(os.str());
}


void Paragraph::setInsetOwner(UpdatableInset * inset)
{
	pimpl_->inset_owner = inset;
	InsetList::iterator it = insetlist.begin();
	InsetList::iterator end = insetlist.end();
	for (; it != end; ++it)
		if (it->inset)
			it->inset->setOwner(inset);
}


void Paragraph::deleteInsetsLyXText(BufferView * bv)
{
	insetlist.deleteInsetsLyXText(bv);
}


void Paragraph::resizeInsetsLyXText(BufferView * bv)
{
	insetlist.resizeInsetsLyXText(bv);
}


void Paragraph::setContentsFromPar(Paragraph const & par)
{
	pimpl_->setContentsFromPar(par);
}


void Paragraph::trackChanges(Change::Type type)
{
	pimpl_->trackChanges(type);
}


void Paragraph::untrackChanges()
{
	pimpl_->untrackChanges();
}


void Paragraph::cleanChanges()
{
	pimpl_->cleanChanges();
}


Change::Type Paragraph::lookupChange(lyx::pos_type pos) const
{
	Assert(!size() || pos < size());
	return pimpl_->lookupChange(pos);
}


Change const Paragraph::lookupChangeFull(lyx::pos_type pos) const
{
	Assert(!size() || pos < size());
	return pimpl_->lookupChangeFull(pos);
}


bool Paragraph::isChanged(pos_type start, pos_type end) const
{
	return pimpl_->isChanged(start, end);
}


bool Paragraph::isChangeEdited(pos_type start, pos_type end) const
{
	return pimpl_->isChangeEdited(start, end);
}


void Paragraph::setChange(lyx::pos_type pos, Change::Type type)
{
	pimpl_->setChange(pos, type);

}


void Paragraph::markErased()
{
	pimpl_->markErased();
}


void Paragraph::acceptChange(pos_type start, pos_type end)
{
	return pimpl_->acceptChange(start, end);
}


void Paragraph::rejectChange(pos_type start, pos_type end)
{
	return pimpl_->rejectChange(start, end);
}


lyx::pos_type Paragraph::size() const
{
	return pimpl_->size();
}


bool Paragraph::empty() const
{
	return pimpl_->empty();
}


Paragraph::value_type Paragraph::getChar(pos_type pos) const
{
	return pimpl_->getChar(pos);
}


int Paragraph::id() const
{
	return pimpl_->id_;
}


void Paragraph::id(int i)
{
	pimpl_->id_ = i;
}


LyXLayout_ptr const & Paragraph::layout() const
{
/*
	InsetOld * inset = inInset();
	if (inset && inset->lyxCode() == InsetOld::ENVIRONMENT_CODE)
		return static_cast<InsetEnvironment*>(inset)->layout();
*/
	return layout_;
}


void Paragraph::layout(LyXLayout_ptr const & new_layout)
{
	layout_ = new_layout;
}


UpdatableInset * Paragraph::inInset() const
{
	return pimpl_->inset_owner;
}


void Paragraph::clearContents()
{
	pimpl_->clear();
}

void Paragraph::setChar(pos_type pos, value_type c)
{
	pimpl_->setChar(pos, c);
}


ParagraphParameters & Paragraph::params()
{
	return pimpl_->params;
}


ParagraphParameters const & Paragraph::params() const
{
	return pimpl_->params;
}


bool Paragraph::isFreeSpacing() const
{
	if (layout()->free_spacing)
		return true;

	// for now we just need this, later should we need this in some
	// other way we can always add a function to InsetOld::() too.
	if (pimpl_->inset_owner && pimpl_->inset_owner->owner())
		return pimpl_->inset_owner->owner()->lyxCode() == InsetOld::ERT_CODE;
	return false;
}


bool Paragraph::allowEmpty() const
{
	if (layout()->keepempty)
		return true;
	if (pimpl_->inset_owner && pimpl_->inset_owner->owner())
		return pimpl_->inset_owner->owner()->lyxCode() == InsetOld::ERT_CODE;
	return false;
}
