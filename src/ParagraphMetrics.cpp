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
#include "Language.h"
#include "LaTeXFeatures.h"
#include "Layout.h"
#include "Font.h"
#include "LyXRC.h"
#include "Row.h"
#include "OutputParams.h"
#include "paragraph_funcs.h"
#include "sgml.h"
#include "TextClass.h"
#include "TexRow.h"
#include "VSpace.h"

#include "frontends/FontMetrics.h"

#include "insets/InsetBibitem.h"
#include "insets/InsetOptArg.h"

#include "support/lassert.h"
#include "support/convert.h"
#include "support/debug.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/textutils.h"

#include <boost/bind.hpp>
#include <boost/crc.hpp>

#include <algorithm>
#include <list>
#include <stack>
#include <sstream>

using namespace std;
using namespace lyx::support;

namespace lyx {


ParagraphMetrics::ParagraphMetrics(Paragraph const & par) :
	position_(-1), par_(&par)
{}


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


size_t ParagraphMetrics::computeRowSignature(Row const & row,
		BufferParams const & bparams) const
{
	boost::crc_32_type crc;
	for (pos_type i = row.pos(); i < row.endpos(); ++i) {
		char_type const b[] = { par_->getChar(i) };
		crc.process_bytes(b, sizeof(char_type));
		if (bparams.trackChanges) {
			Change change = par_->lookupChange(i);
			char_type const b[] = { change.type };
			// 1 byte is enough to encode Change::Type
			crc.process_bytes(b, 1);
		}			
	}

	Dimension const & d = row.dimension();
	char_type const b[] = { row.sel_beg, row.sel_end, d.wid, d.asc, d.des};
	// Each of the variable to process is 4 bytes: 4x5 = 20
	crc.process_bytes(b, 20);

	return crc.checksum();
}


void ParagraphMetrics::setPosition(int position)
{
	position_ = position;
}


Dimension const & ParagraphMetrics::insetDimension(Inset const * inset) const
{
	InsetDims::const_iterator it = inset_dims_.find(inset);
	if (it != inset_dims_.end())
		return it->second;

	static Dimension dummy;
	return dummy;
}


void ParagraphMetrics::setInsetDimension(Inset const * inset,
		Dimension const & dim)
{
	inset_dims_[inset] = dim;
}


Row & ParagraphMetrics::getRow(pos_type pos, bool boundary)
{
	LASSERT(!rows().empty(), /**/);

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
	LASSERT(!rows().empty(), /**/);

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
	LASSERT(!rows().empty(), /**/);

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

int ParagraphMetrics::rightMargin(BufferView const & bv) const
{
	BufferParams const & params = bv.buffer().params();
	DocumentClass const & tclass = params.documentClass();
	frontend::FontMetrics const & fm = theFontMetrics(params.getFont());
	int const r_margin =
		bv.rightMargin()
		+ fm.signedWidth(tclass.rightmargin())
		+ fm.signedWidth(par_->layout().rightmargin)
		* 4 / (par_->getDepth() + 4);

	return r_margin;
}


int ParagraphMetrics::singleWidth(pos_type pos, Font const & font) const
{
	// The most special cases are handled first.
	if (Inset const * inset = par_->getInset(pos))
		return insetDimension(inset).wid;

	char_type c = par_->getChar(pos);

	if (!isPrintable(c))
		return theFontMetrics(font).width(c);

	Language const * language = font.language();
	if (language->rightToLeft()) {
		if (language->lang() == "arabic_arabtex" ||
			language->lang() == "arabic_arabi" ||
			language->lang() == "farsi") {
				if (Encodings::isArabicComposeChar(c))
					return 0;
				c = par_->transformChar(c, pos);
		} else if (language->lang() == "hebrew" &&
				Encodings::isHebrewComposeChar(c)) {
			return 0;	
		}
	}
	return theFontMetrics(font).width(c);
}


bool ParagraphMetrics::hfillExpansion(Row const & row, pos_type pos) const
{
	if (!par_->isHfill(pos))
		return false;

	LASSERT(pos >= row.pos() && pos < row.endpos(), /**/);

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
	if (par_->layout().margintype != MARGIN_MANUAL && pos < par_->beginOfBody())
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
