/**
 * \file paragraph.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 * \author John Levon
 * \author André Pönitz
 * \author Dekel Tsur
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "paragraph.h"
#include "paragraph_pimpl.h"

#include "buffer.h"
#include "bufferparams.h"
#include "counters.h"
#include "encoding.h"
#include "debug.h"
#include "gettext.h"
#include "language.h"
#include "lyxfont.h"
#include "lyxrc.h"
#include "lyxrow.h"
#include "outputparams.h"
#include "paragraph_funcs.h"
#include "ParagraphList_fwd.h"
#include "sgml.h"
#include "texrow.h"
#include "vspace.h"

#include "insets/insetbibitem.h"
#include "insets/insetoptarg.h"

#include "support/lstrings.h"
#include "support/textutils.h"
#include "support/tostr.h"

#include <boost/tuple/tuple.hpp>
#include <boost/bind.hpp>

#include <list>
#include <stack>
#include <sstream>

using lyx::pos_type;

using lyx::support::contains;
using lyx::support::subst;

using std::distance;
using std::endl;
using std::list;
using std::stack;
using std::string;
using std::ostream;
using std::ostringstream;


ParagraphList::ParagraphList()
{}


Paragraph::Paragraph()
	: y(0), height(0), begin_of_body_(0),
	  pimpl_(new Paragraph::Pimpl(this))
{
	//lyxerr << "sizeof Paragraph::Pimpl: " << sizeof(Paragraph::Pimpl) << endl;
	itemdepth = 0;
	params().clear();
}


Paragraph::Paragraph(Paragraph const & par)
	:	itemdepth(par.itemdepth), insetlist(par.insetlist),
		rows(par.rows), y(par.y), height(par.height),
		width(par.width), layout_(par.layout_),
		text_(par.text_), begin_of_body_(par.begin_of_body_),
	  pimpl_(new Paragraph::Pimpl(*par.pimpl_, this))
{
	//lyxerr << "Paragraph::Paragraph(Paragraph const&)" << endl;
	InsetList::iterator it = insetlist.begin();
	InsetList::iterator end = insetlist.end();
	for (; it != end; ++it)
		it->inset = it->inset->clone().release();
}


Paragraph & Paragraph::operator=(Paragraph const & par)
{
	// needed as we will destroy the pimpl_ before copying it
	if (&par != this) {
		itemdepth = par.itemdepth;

		insetlist = par.insetlist;
		InsetList::iterator it = insetlist.begin();
		InsetList::iterator end = insetlist.end();
		for (; it != end; ++it)
			it->inset = it->inset->clone().release();

		rows = par.rows;
		y = par.y;
		height = par.height;
		width = par.width;
		layout_ = par.layout();
		text_ = par.text_;
		begin_of_body_ = par.begin_of_body_;

		delete pimpl_;
		pimpl_ = new Pimpl(*par.pimpl_, this);
	}
	return *this;
}


Paragraph::~Paragraph()
{
	delete pimpl_;
	//
	//lyxerr << "Paragraph::paragraph_id = "
	//       << Paragraph::paragraph_id << endl;
}


void Paragraph::write(Buffer const & buf, ostream & os,
			  BufferParams const & bparams,
			  depth_type & dth) const
{
	// The beginning or end of a deeper (i.e. nested) area?
	if (dth != params().depth()) {
		if (params().depth() > dth) {
			while (params().depth() > dth) {
				os << "\n\\begin_deeper";
				++dth;
			}
		} else {
			while (params().depth() < dth) {
				os << "\n\\end_deeper";
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
			InsetBase const * inset = getInset(i);
			if (inset)
				if (inset->directWrite()) {
					// international char, let it write
					// code directly so it's shorter in
					// the file
					inset->write(buf, os);
				} else {
					os << "\n\\begin_inset ";
					inset->write(buf, os);
					os << "\n\\end_inset\n\n";
					column = 0;
				}
		}
		break;
		case '\\':
			os << "\n\\backslash\n";
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


void Paragraph::insert(pos_type start, string const & str,
		       LyXFont const & font)
{
	int size = str.size();
	for (int i = 0 ; i < size ; ++i)
		insertChar(start + i, str[i], font);
}


bool Paragraph::checkInsertChar(LyXFont &)
{
	return true;
}


void Paragraph::insertChar(pos_type pos, Paragraph::value_type c,
			   Change change)
{
	pimpl_->insertChar(pos, c, change);
}


void Paragraph::insertChar(pos_type pos, Paragraph::value_type c,
			   LyXFont const & font, Change change)
{
	pimpl_->insertChar(pos, c, change);
	setFont(pos, font);
}


void Paragraph::insertInset(pos_type pos, InsetBase * inset, Change change)
{
	pimpl_->insertInset(pos, inset, change);
}


void Paragraph::insertInset(pos_type pos, InsetBase * inset,
			    LyXFont const & font, Change change)
{
	pimpl_->insertInset(pos, inset, change);
	setFont(pos, font);
}


bool Paragraph::insetAllowed(InsetOld_code code)
{
	return !pimpl_->inset_owner || pimpl_->inset_owner->insetAllowed(code);
}


InsetBase * Paragraph::getInset(pos_type pos)
{
	BOOST_ASSERT(pos < size());
	return insetlist.get(pos);
}


InsetBase const * Paragraph::getInset(pos_type pos) const
{
	BOOST_ASSERT(pos < size());
	return insetlist.get(pos);
}


// Gets uninstantiated font setting at position.
LyXFont const Paragraph::getFontSettings(BufferParams const & bparams,
					 pos_type pos) const
{
	if (pos > size()) {
		lyxerr << " pos: " << pos << " size: " << size() << endl;
		BOOST_ASSERT(pos <= size());
	}

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
	BOOST_ASSERT(pos <= size());

	Pimpl::FontList::const_iterator cit = pimpl_->fontlist.begin();
	Pimpl::FontList::const_iterator end = pimpl_->fontlist.end();
	for (; cit != end; ++cit)
		if (cit->pos() >= pos)
			return cit->pos();

	// This should not happen, but if so, we take no chances.
	//lyxerr << "Paragraph::getEndPosOfFontSpan: This should not happen!"
	//      << endl;
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
	BOOST_ASSERT(pos >= 0);

	LyXLayout_ptr const & lout = layout();

	pos_type const body_pos = beginOfBody();

	LyXFont layoutfont;
	if (pos < body_pos)
		layoutfont = lout->labelfont;
	else
		layoutfont = lout->font;

	LyXFont font = getFontSettings(bparams, pos);
	font.realize(layoutfont);
	font.realize(outerfont);
	font.realize(bparams.getLyXTextClass().defaultfont());

	return font;
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
LyXFont_size
Paragraph::highestFontInRange(pos_type startpos, pos_type endpos,
			      LyXFont_size def_size) const
{
	if (pimpl_->fontlist.empty())
		return def_size;

	Pimpl::FontList::const_iterator end_it = pimpl_->fontlist.begin();
	Pimpl::FontList::const_iterator const end = pimpl_->fontlist.end();
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
	BOOST_ASSERT(pos <= size());

	// First, reduce font against layout/label font
	// Update: The setCharFont() routine in text2.C already
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
	unsigned int i = distance(beg, it);
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
	return par.layout() == layout() && params().sameLayout(par.params());
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
	params().spacing(Spacing(Spacing::Default));
}


pos_type Paragraph::beginOfBody() const
{
	return begin_of_body_;
}


void Paragraph::setBeginOfBody()
{
	if (layout()->labeltype != LABEL_MANUAL) {
		begin_of_body_ = 0;
		return;
	}

	// Unroll the first two cycles of the loop
	// and remember the previous character to
	// remove unnecessary getChar() calls
	pos_type i = 0;
	pos_type end = size();
	if (i < end && !isNewline(i)) {
		++i;
		char previous_char = 0;
		char temp = 0;
		if (i < end) {
			previous_char = text_[i];
			if (!isNewline(i)) {
				++i;
				while (i < end && previous_char != ' ') {
					temp = text_[i];
					if (isNewline(i))
						break;
					++i;
					previous_char = temp;
				}
			}
		}
	}

	begin_of_body_ = i;
}


// returns -1 if inset not found
int Paragraph::getPositionOfInset(InsetBase const * inset) const
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
	if (!insetlist.empty()) {
		InsetBase * inset = insetlist.begin()->inset;
		if (inset->lyxCode() == InsetBase::BIBTEX_CODE)
			return static_cast<InsetBibitem *>(inset);
	}
	return 0;
}


bool Paragraph::forceDefaultParagraphs() const
{
	return inInset() && inInset()->forceDefaultParagraphs(inInset());
}


bool Paragraph::autoBreakRows() const
{
	return inInset() && static_cast<InsetText *>(inInset())->getAutoBreakRows();
}


namespace {

// paragraphs inside floats need different alignment tags to avoid
// unwanted space

bool noTrivlistCentering(InsetBase::Code code)
{
	return code == InsetBase::FLOAT_CODE || code == InsetBase::WRAP_CODE;
}


string correction(string const & orig)
{
	if (orig == "flushleft")
		return "raggedright";
	if (orig == "flushright")
		return "raggedleft";
	if (orig == "center")
		return "centering";
	return orig;
}


string const corrected_env(string const & suffix, string const & env,
	InsetBase::Code code)
{
	string output = suffix + "{";
	if (noTrivlistCentering(code))
		output += correction(env);
	else
		output += env;
	return output + "}";
}

} // namespace anon


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
			column += 8;
		}
		break;
	}

	switch (params().align()) {
	case LYX_ALIGN_NONE:
	case LYX_ALIGN_BLOCK:
	case LYX_ALIGN_LAYOUT:
	case LYX_ALIGN_SPECIAL:
		break;
	case LYX_ALIGN_LEFT: {
		string output;
		if (getParLanguage(bparams)->babel() != "hebrew")
			output = corrected_env("\\begin", "flushleft", ownerCode());
		else
			output = corrected_env("\\begin", "flushright", ownerCode());
		os << output;
		column += output.size();
		break;
	} case LYX_ALIGN_RIGHT: {
		string output;
		if (getParLanguage(bparams)->babel() != "hebrew")
			output = corrected_env("\\begin", "flushright", ownerCode());
		else
			output = corrected_env("\\begin", "flushleft", ownerCode());
		os << output;
		column += output.size();
		break;
	} case LYX_ALIGN_CENTER: {
		string output;
		output = corrected_env("\\begin", "center", ownerCode());
		os << output;
		column += output.size();
		break;
	}
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
	case LYX_ALIGN_LEFT: {
		string output;
		if (getParLanguage(bparams)->babel() != "hebrew")
			output = corrected_env("\\par\\end", "flushleft", ownerCode());
		else
			output = corrected_env("\\par\\end", "flushright", ownerCode());
		os << output;
		column += output.size();
		break;
	} case LYX_ALIGN_RIGHT: {
		string output;
		if (getParLanguage(bparams)->babel() != "hebrew")
			output = corrected_env("\\par\\end", "flushright", ownerCode());
		else
			output = corrected_env("\\par\\end", "flushleft", ownerCode());
		os << output;
		column += output.size();
		break;
	} case LYX_ALIGN_CENTER: {
		string output;
		output = corrected_env("\\par\\end", "center", ownerCode());
		os << output;
		column += output.size();
		break;
	}
	}

	return column;
}


// This one spits out the text of the paragraph
bool Paragraph::simpleTeXOnePar(Buffer const & buf,
				BufferParams const & bparams,
				LyXFont const & outerfont,
				ostream & os, TexRow & texrow,
				OutputParams const & runparams) const
{
	lyxerr[Debug::LATEX] << "SimpleTeXOnePar...     " << this << endl;

	bool return_value = false;

	LyXLayout_ptr style;

	// well we have to check if we are in an inset with unlimited
	// length (all in one row) if that is true then we don't allow
	// any special options in the paragraph and also we don't allow
	// any environment other then "Standard" to be valid!
	bool asdefault = forceDefaultParagraphs();

	if (asdefault) {
		style = bparams.getLyXTextClass().defaultLayout();
	} else {
		style = layout();
	}

	LyXFont basefont;

	// Maybe we have to create a optional argument.
	pos_type body_pos = beginOfBody();
	unsigned int column = 0;

	if (body_pos > 0) {
		// the optional argument is kept in curly brackets in
		// case it contains a ']'
		os << "[{";
		column += 2;
		basefont = getLabelFont(bparams, outerfont);
	} else {
		basefont = getLayoutFont(bparams, outerfont);
	}

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
			column += startTeXParParams(bparams, os, 
						    runparams.moving_arg);
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
				os << "}] ";
				column +=3;
			}
			if (style->isCommand()) {
				os << '{';
				++column;
			}

			if (!asdefault)
				column += startTeXParParams(bparams, os,
							    runparams.moving_arg);
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
			if (next_font != running_font && next_font != font) {
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

		OutputParams rp = runparams;
		rp.free_spacing = style->free_spacing;
		rp.local_language = font.language()->babel();
		rp.intitle = style->intitle;
		pimpl_->simpleTeXSpecialChars(buf, bparams,
					      os, texrow, rp,
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
		column += endTeXParParams(bparams, os, runparams.moving_arg);
	}

	lyxerr[Debug::LATEX] << "SimpleTeXOnePar...done " << this << endl;
	return return_value;
}


namespace {

// checks, if newcol chars should be put into this line
// writes newline, if necessary.
void sgmlLineBreak(ostream & os, string::size_type & colcount,
			  string::size_type newcol)
{
	colcount += newcol;
	if (colcount > lyxrc.ascii_linelen) {
		os << "\n";
		colcount = newcol; // assume write after this call
	}
}

enum PAR_TAG {
	PAR_NONE=0,
	TT = 1,
	SF = 2,
	BF = 4,
	IT = 8,
	SL = 16,
	EM = 32
};


string tag_name(PAR_TAG const & pt) {
	switch (pt) {
	case PAR_NONE: return "!-- --";
	case TT: return "tt";
	case SF: return "sf";
	case BF: return "bf";
	case IT: return "it";
	case SL: return "sl";
	case EM: return "em";
	}
	return "";
}


inline
void operator|=(PAR_TAG & p1, PAR_TAG const & p2)
{
	p1 = static_cast<PAR_TAG>(p1 | p2);
}


inline
void reset(PAR_TAG & p1, PAR_TAG const & p2)
{
	p1 = static_cast<PAR_TAG>(p1 & ~p2);
}

} // anon


// Handle internal paragraph parsing -- layout already processed.
void Paragraph::simpleLinuxDocOnePar(Buffer const & buf,
				     ostream & os,
				     LyXFont const & outerfont,
				     OutputParams const & runparams,
				     lyx::depth_type /*depth*/) const
{
	LyXLayout_ptr const & style = layout();

	string::size_type char_line_count = 5;     // Heuristic choice ;-)

	// gets paragraph main font
	LyXFont font_old;
	bool desc_on;
	if (style->labeltype == LABEL_MANUAL) {
		font_old = style->labelfont;
		desc_on = true;
	} else {
		font_old = style->font;
		desc_on = false;
	}

	LyXFont::FONT_FAMILY family_type = LyXFont::ROMAN_FAMILY;
	LyXFont::FONT_SERIES series_type = LyXFont::MEDIUM_SERIES;
	LyXFont::FONT_SHAPE  shape_type  = LyXFont::UP_SHAPE;
	bool is_em = false;

	stack<PAR_TAG> tag_state;
	// parsing main loop
	for (pos_type i = 0; i < size(); ++i) {

		PAR_TAG tag_close = PAR_NONE;
		list < PAR_TAG > tag_open;

		LyXFont const font = getFont(buf.params(), i, outerfont);

		if (font_old.family() != font.family()) {
			switch (family_type) {
			case LyXFont::SANS_FAMILY:
				tag_close |= SF;
				break;
			case LyXFont::TYPEWRITER_FAMILY:
				tag_close |= TT;
				break;
			default:
				break;
			}

			family_type = font.family();

			switch (family_type) {
			case LyXFont::SANS_FAMILY:
				tag_open.push_back(SF);
				break;
			case LyXFont::TYPEWRITER_FAMILY:
				tag_open.push_back(TT);
				break;
			default:
				break;
			}
		}

		if (font_old.series() != font.series()) {
			switch (series_type) {
			case LyXFont::BOLD_SERIES:
				tag_close |= BF;
				break;
			default:
				break;
			}

			series_type = font.series();

			switch (series_type) {
			case LyXFont::BOLD_SERIES:
				tag_open.push_back(BF);
				break;
			default:
				break;
			}

		}

		if (font_old.shape() != font.shape()) {
			switch (shape_type) {
			case LyXFont::ITALIC_SHAPE:
				tag_close |= IT;
				break;
			case LyXFont::SLANTED_SHAPE:
				tag_close |= SL;
				break;
			default:
				break;
			}

			shape_type = font.shape();

			switch (shape_type) {
			case LyXFont::ITALIC_SHAPE:
				tag_open.push_back(IT);
				break;
			case LyXFont::SLANTED_SHAPE:
				tag_open.push_back(SL);
				break;
			default:
				break;
			}
		}
		// handle <em> tag
		if (font_old.emph() != font.emph()) {
			if (font.emph() == LyXFont::ON) {
				tag_open.push_back(EM);
				is_em = true;
			}
			else if (is_em) {
				tag_close |= EM;
				is_em = false;
			}
		}

		list < PAR_TAG > temp;
		while (!tag_state.empty() && tag_close) {
			PAR_TAG k =  tag_state.top();
			tag_state.pop();
			os << "</" << tag_name(k) << '>';
			if (tag_close & k)
				reset(tag_close,k);
			else
				temp.push_back(k);
		}

		for(list< PAR_TAG >::const_iterator j = temp.begin();
		    j != temp.end(); ++j) {
			tag_state.push(*j);
			os << '<' << tag_name(*j) << '>';
		}

		for(list< PAR_TAG >::const_iterator j = tag_open.begin();
		    j != tag_open.end(); ++j) {
			tag_state.push(*j);
			os << '<' << tag_name(*j) << '>';
		}

		char c = getChar(i);


		if (c == Paragraph::META_INSET) {
			getInset(i)->linuxdoc(buf, os, runparams);
			font_old = font;
			continue;
		}

		if (style->latexparam() == "CDATA") {
			// "TeX"-Mode on == > SGML-Mode on.
			if (c != '\0')
				os << c;
			++char_line_count;
		} else {
			bool ws;
			string str;
			boost::tie(ws, str) = sgml::escapeChar(c);
			if (ws && !isFreeSpacing()) {
				// in freespacing mode, spaces are
				// non-breaking characters
				if (desc_on) { // if char is ' ' then...
					++char_line_count;
					sgmlLineBreak(os, char_line_count, 6);
					os << "</tag>";
					desc_on = false;
				} else  {
					sgmlLineBreak(os, char_line_count, 1);
					os << c;
				}
			} else {
				os << str;
				char_line_count += str.length();
			}
		}
		font_old = font;
	}

	while (!tag_state.empty()) {
		os << "</" << tag_name(tag_state.top()) << '>';
		tag_state.pop();
	}

	// resets description flag correctly
	if (desc_on) {
		// <tag> not closed...
		sgmlLineBreak(os, char_line_count, 6);
		os << "</tag>";
	}
}


bool Paragraph::emptyTag() const
{
	for (pos_type i = 0; i < size(); ++i) {
		if (isInset(i)) {
			InsetBase const * inset = getInset(i);
			InsetBase::Code lyx_code = inset->lyxCode();
			if (lyx_code != InsetBase::TOC_CODE and
			    lyx_code != InsetBase::INCLUDE_CODE and
			    lyx_code != InsetBase::GRAPHICS_CODE and
			    lyx_code != InsetBase::ERT_CODE and
			    lyx_code != InsetBase::FLOAT_CODE and
			    lyx_code != InsetBase::TABULAR_CODE) {
				return false;
			}
		} else {
			char c = getChar(i);
			if(c!= ' ' and c!= '\t')
				return false;
		}

	}
	return true;
}


string Paragraph::getID(Buffer const & buf, OutputParams const & runparams) const
{
	for (pos_type i = 0; i < size(); ++i) {
		if (isInset(i)) {
			InsetBase const * inset = getInset(i);
			InsetBase::Code lyx_code = inset->lyxCode();
			if (lyx_code == InsetBase::LABEL_CODE) {
				string const id = static_cast<InsetCommand const *>(inset)->getContents();
				return "id=\"" + sgml::cleanID(buf, runparams, id) + "\"";
			}
		}

	}
	return string();
}


pos_type Paragraph::getFirstWord(Buffer const & buf, ostream & os, OutputParams const & runparams) const
{
	pos_type i;
	for (i = 0; i < size(); ++i) {
		if (isInset(i)) {
			InsetBase const * inset = getInset(i);
			inset->docbook(buf, os, runparams);
		} else {
			char c = getChar(i);
			if (c == ' ')
				break;
			bool ws;
			string str;
			boost::tie(ws, str) = sgml::escapeChar(c);

			os << str;
		}
	}
	return i;
}


bool Paragraph::onlyText(Buffer const & buf, LyXFont const & outerfont, pos_type initial) const
{
	LyXLayout_ptr const & style = layout();
	LyXFont font_old;

	for (pos_type i = initial; i < size(); ++i) {
		LyXFont font = getFont(buf.params(), i, outerfont);
		if (isInset(i))
			return false;
		if ( i != initial and font != font_old)
			return false;
		font_old = font;
	}

	return true;
}


void Paragraph::simpleDocBookOnePar(Buffer const & buf,
				    ostream & os,
				    OutputParams const & runparams,
				    LyXFont const & outerfont,
				    pos_type initial) const
{
	bool emph_flag = false;

	LyXLayout_ptr const & style = layout();
	LyXFont font_old =
		style->labeltype == LABEL_MANUAL ? style->labelfont : style->font;

	if (style->pass_thru and not onlyText(buf, outerfont, initial))
		os << "]]>";
	// parsing main loop
	for (pos_type i = initial; i < size(); ++i) {
		LyXFont font = getFont(buf.params(), i, outerfont);

		// handle <emphasis> tag
		if (font_old.emph() != font.emph()) {
			if (font.emph() == LyXFont::ON) {
				os << "<emphasis>";
				emph_flag = true;
			} else if (i != initial) {
				os << "</emphasis>";
				emph_flag = false;
			}
		}

		if (isInset(i)) {
			InsetBase const * inset = getInset(i);
			inset->docbook(buf, os, runparams);
		} else {
			char c = getChar(i);
			bool ws;
			string str;
			boost::tie(ws, str) = sgml::escapeChar(c);

			if (style->pass_thru)
				os << c;
			else
				os << str;
		}
		font_old = font;
	}

	if (emph_flag) {
		os << "</emphasis>";
	}

	if (style->free_spacing)
		os << '\n';
	if (style->pass_thru and not onlyText(buf, outerfont, initial))
		os << "<![CDATA[";
}


namespace {

/// return true if the char is a meta-character for an inset
inline
bool IsInsetChar(char c)
{
	return (c == Paragraph::META_INSET);
}

} // namespace anon



bool Paragraph::isHfill(pos_type pos) const
{
	return isInset(pos)
		&& getInset(pos)->lyxCode() == InsetBase::HFILL_CODE;
}


bool Paragraph::isNewline(pos_type pos) const
{
	return isInset(pos)
		&& getInset(pos)->lyxCode() == InsetBase::NEWLINE_CODE;
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
	if (isInset(pos))
		return getInset(pos)->isLetter();
	value_type const c = getChar(pos);
	return !(IsSeparatorChar(c)
		  || IsKommaChar(c)
		  || IsInsetChar(c));
}


Language const *
Paragraph::getParLanguage(BufferParams const & bparams) const
{
	if (!empty())
		return getFirstFontSettings().language();
#ifdef WITH_WARNINGS
#warning FIXME we should check the prev par as well (Lgb)
#endif
	return bparams.language;
}


bool Paragraph::isRightToLeftPar(BufferParams const & bparams) const
{
	return lyxrc.rtl_support
		&& getParLanguage(bparams)->RightToLeft()
		&& ownerCode() != InsetBase::ERT_CODE;
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


bool Paragraph::isMultiLingual(BufferParams const & bparams) const
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
string const Paragraph::asString(Buffer const & buffer, bool label) const
{
	OutputParams runparams;
	return asString(buffer, runparams, label);
}


string const Paragraph::asString(Buffer const & buffer,
				 OutputParams const & runparams,
				 bool label) const
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
			 getInset(i)->lyxCode() == InsetBase::MATH_CODE) {
			ostringstream os;
			getInset(i)->plaintext(buffer, os, runparams);
			s += subst(STRCONV(os.str()),'\n',' ');
		}
	}

	return s;
#else
	// This should really be done by the caller and not here.
	string ret = asString(buffer, runparams, 0, size(), label);
	return subst(ret, '\n', ' ');
#endif
}


string const Paragraph::asString(Buffer const & buffer,
				 pos_type beg, pos_type end, bool label) const
{

	OutputParams const runparams;
	return asString(buffer, runparams, beg, end, label);
}


string const Paragraph::asString(Buffer const & buffer,
				 OutputParams const & runparams,
				 pos_type beg, pos_type end, bool label) const
{
	ostringstream os;

	if (beg == 0 && label && !params().labelString().empty())
		os << params().labelString() << ' ';

	for (pos_type i = beg; i < end; ++i) {
		value_type const c = getUChar(buffer.params(), i);
		if (IsPrintable(c))
			os << c;
		else if (c == META_INSET)
			getInset(i)->plaintext(buffer, os, runparams);
	}

	return os.str();
}


void Paragraph::setInsetOwner(UpdatableInset * inset)
{
	pimpl_->inset_owner = inset;
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
	BOOST_ASSERT(empty() || pos < size());
	return pimpl_->lookupChange(pos);
}


Change const Paragraph::lookupChangeFull(lyx::pos_type pos) const
{
	BOOST_ASSERT(empty() || pos < size());
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


int Paragraph::id() const
{
	return pimpl_->id_;
}


LyXLayout_ptr const & Paragraph::layout() const
{
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


InsetBase::Code Paragraph::ownerCode() const
{
	return pimpl_->inset_owner
		? pimpl_->inset_owner->lyxCode() : InsetBase::NO_CODE;
}


void Paragraph::clearContents()
{
	text_.clear();
}


void Paragraph::setChar(pos_type pos, value_type c)
{
	text_[pos] = c;
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
	// other way we can always add a function to InsetBase too.
	return ownerCode() == InsetBase::ERT_CODE;
}


bool Paragraph::allowEmpty() const
{
	if (layout()->keepempty)
		return true;
	return ownerCode() == InsetBase::ERT_CODE;
}


RowList::iterator Paragraph::getRow(pos_type pos)
{
	RowList::iterator rit = rows.end();
	RowList::iterator const begin = rows.begin();

	for (--rit; rit != begin && rit->pos() > pos; --rit)
		;

	return rit;
}


RowList::const_iterator Paragraph::getRow(pos_type pos) const
{
	RowList::const_iterator rit = rows.end();
	RowList::const_iterator const begin = rows.begin();

	for (--rit; rit != begin && rit->pos() > pos; --rit)
		;

	return rit;
}


size_t Paragraph::row(pos_type pos) const
{
	RowList::const_iterator rit = rows.end();
	RowList::const_iterator const begin = rows.begin();

	for (--rit; rit != begin && rit->pos() > pos; --rit)
		;

	return rit - begin;
}


unsigned char Paragraph::transformChar(unsigned char c, pos_type pos) const
{
	if (!Encodings::is_arabic(c))
		if (lyxrc.font_norm_type == LyXRC::ISO_8859_6_8 && IsDigit(c))
			return c + (0xb0 - '0');
		else
			return c;

	unsigned char const prev_char = pos > 0 ? getChar(pos - 1) : ' ';
	unsigned char next_char = ' ';

	for (pos_type i = pos + 1, end = size(); i < end; ++i) {
		unsigned char const par_char = getChar(i);
		if (!Encodings::IsComposeChar_arabic(par_char)) {
			next_char = par_char;
			break;
		}
	}

	if (Encodings::is_arabic(next_char)) {
		if (Encodings::is_arabic(prev_char) &&
			!Encodings::is_arabic_special(prev_char))
			return Encodings::TransformChar(c, Encodings::FORM_MEDIAL);
		else
			return Encodings::TransformChar(c, Encodings::FORM_INITIAL);
	} else {
		if (Encodings::is_arabic(prev_char) &&
			!Encodings::is_arabic_special(prev_char))
			return Encodings::TransformChar(c, Encodings::FORM_FINAL);
		else
			return Encodings::TransformChar(c, Encodings::FORM_ISOLATED);
	}
}
