/**
 * \file Paragraph.cpp
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

#include "ParagraphMetrics.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Counters.h"
#include "Encoding.h"
#include "debug.h"
#include "gettext.h"
#include "Language.h"
#include "LaTeXFeatures.h"
#include "Font.h"
#include "LyXRC.h"
#include "Row.h"
#include "OutputParams.h"
#include "paragraph_funcs.h"
#include "sgml.h"
#include "TexRow.h"
#include "VSpace.h"

#include "frontends/FontMetrics.h"

#include "insets/InsetBibitem.h"
#include "insets/InsetOptArg.h"

#include "support/lstrings.h"
#include "support/textutils.h"
#include "support/convert.h"
#include "support/unicode.h"

#include <boost/bind.hpp>
#include <boost/crc.hpp>

#include <algorithm>
#include <list>
#include <stack>
#include <sstream>


namespace lyx {

using lyx::support::contains;
using lyx::support::rsplit;
using support::subst;

using std::distance;
using std::endl;
using std::list;
using std::stack;
using std::string;
using std::ostream;
using std::ostringstream;


ParagraphMetrics::ParagraphMetrics(Paragraph const & par): par_(&par), position_(-1)
{
}


ParagraphMetrics & ParagraphMetrics::operator=(
	ParagraphMetrics const & pm)
{
	rows_ = pm.rows_;
	dim_ = pm.dim_;
	par_ = pm.par_;
	position_ = pm.position_;
	return *this;
}


void ParagraphMetrics::reset(Paragraph const & par)
{
	par_ = &par;
	dim_ = Dimension();
	//position_ = -1;
}


void ParagraphMetrics::computeRowSignature(Row & row,
		BufferParams const & bparams)
{
	boost::crc_32_type crc;
	for (pos_type i = row.pos(); i < row.endpos(); ++i) {
		char_type const b[] = { par_->getChar(i) };
		crc.process_bytes(b, 1);
		if (bparams.trackChanges) {
			Change change = par_->lookupChange(i);
			char_type const b[] = { change.type };
			crc.process_bytes(b, 1);
		}			
	}
	row.setCrc(crc.checksum());
}


void ParagraphMetrics::setPosition(int position)
{
	position_ = position;
}


Row & ParagraphMetrics::getRow(pos_type pos, bool boundary)
{
	BOOST_ASSERT(!rows().empty());

	// If boundary is set we should return the row on which
	// the character before is inside.
	if (pos > 0 && boundary)
		--pos;

	RowList::iterator rit = rows_.end();
	RowList::iterator const begin = rows_.begin();

	for (--rit; rit != begin && rit->pos() > pos; --rit)
		;

	return *rit;
}


Row const & ParagraphMetrics::getRow(pos_type pos, bool boundary) const
{
	BOOST_ASSERT(!rows().empty());

	// If boundary is set we should return the row on which
	// the character before is inside.
	if (pos > 0 && boundary)
		--pos;

	RowList::const_iterator rit = rows_.end();
	RowList::const_iterator const begin = rows_.begin();

	for (--rit; rit != begin && rit->pos() > pos; --rit)
		;

	return *rit;
}


size_t ParagraphMetrics::pos2row(pos_type pos) const
{
	BOOST_ASSERT(!rows().empty());

	RowList::const_iterator rit = rows_.end();
	RowList::const_iterator const begin = rows_.begin();

	for (--rit; rit != begin && rit->pos() > pos; --rit)
		;

	return rit - begin;
}


void ParagraphMetrics::dump() const
{
	lyxerr << "Paragraph::dump: rows.size(): " << rows_.size() << endl;
	for (size_t i = 0; i != rows_.size(); ++i) {
		lyxerr << "  row " << i << ":   ";
		rows_[i].dump();
	}
}

int ParagraphMetrics::rightMargin(Buffer const & buffer) const
{
	BufferParams const & params = buffer.params();
	TextClass const & tclass = params.getTextClass();
	frontend::FontMetrics const & fm = theFontMetrics(params.getFont());
	int const r_margin =
		lyx::rightMargin()
		+ fm.signedWidth(tclass.rightmargin())
		+ fm.signedWidth(par_->layout()->rightmargin)
		* 4 / (par_->getDepth() + 4);

	return r_margin;
}


int ParagraphMetrics::singleWidth(pos_type pos, Font const & font) const
{
	char_type c = par_->getChar(pos);

	// The most special cases are handled first.
	if (c == Paragraph::META_INSET)
		return par_->getInset(pos)->width();

	if (!isPrintable(c))
		return theFontMetrics(font).width(c);

	Language const * language = font.language();
	if (language->rightToLeft()) {
		if (language->lang() == "arabic_arabtex" ||
			language->lang() == "arabic_arabi" ||
			language->lang() == "farsi") {
				if (Encodings::isComposeChar_arabic(c))
					return 0;
				c = par_->transformChar(c, pos);
		} else if (language->lang() == "hebrew" &&
			Encodings::isComposeChar_hebrew(c))
			return 0;
	}
	return theFontMetrics(font).width(c);
}


bool ParagraphMetrics::hfillExpansion(Row const & row, pos_type pos) const
{
	if (!par_->isHfill(pos))
		return false;

	BOOST_ASSERT(pos >= row.pos() && pos < row.endpos());

	// expand at the end of a row only if there is another hfill on the same row
	if (pos == row.endpos() - 1) {
		for (pos_type i = row.pos(); i < pos; i++) {
			if (par_->isHfill(i))
				return true;
		}
		return false;
	}

	// expand at the beginning of a row only if it is the first row of a paragraph
	if (pos == row.pos()) {
		return pos == 0;
	}

	// do not expand in some labels
	if (par_->layout()->margintype != MARGIN_MANUAL && pos < par_->beginOfBody())
		return false;

	// if there is anything between the first char of the row and
	// the specified position that is neither a newline nor an hfill,
	// the hfill will be expanded, otherwise it won't
	for (pos_type i = row.pos(); i < pos; i++) {
		if (!par_->isNewline(i) && !par_->isHfill(i))
			return true;
	}
	return false;
}

} // namespace lyx
