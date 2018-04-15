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
#include "sgml.h"
#include "TextClass.h"
#include "TexRow.h"

#include "frontends/FontMetrics.h"

#include "insets/InsetBibitem.h"
#include "insets/InsetArgument.h"

#include "support/lassert.h"
#include "support/debug.h"
#include "support/ExceptionMessage.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/textutils.h"

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


void ParagraphMetrics::setPosition(int position)
{
	position_ = position;
}


Row & ParagraphMetrics::getRow(pos_type pos, bool boundary)
{
	LBUFERR(!rows().empty());

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
	LBUFERR(!rows().empty());

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
	LBUFERR(!rows().empty());

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
		lyxerr << "  row " << i << ":   " << rows_[i];
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


// FIXME: this code seems bogus. Audit and rewrite (see bug #9860).
bool ParagraphMetrics::hfillExpansion(Row const & row, pos_type pos) const
{
	if (!par_->isHfill(pos))
		return false;

	LASSERT(pos >= row.pos() && pos < row.endpos(), return false);

	// expand at the end of a row only if there is another hfill on the same row
	if (pos == row.endpos() - 1) {
		for (pos_type i = row.pos(); i < pos; i++) {
			if (par_->isHfill(i))
				return true;
		}
		return false;
	}

	// expand at the beginning of a row only if it is the first row of a paragraph
	if (pos == row.pos())
		return pos == 0;

	// do not expand in some labels
	if (par_->layout().margintype != MARGIN_MANUAL && pos < par_->beginOfBody())
		return false;

	// if there is anything between the first char of the row and
	// the specified position that is neither a newline nor an hfill,
	// the hfill will be expanded, otherwise it won't
	for (pos_type i = row.pos(); i < pos; i++) {
		if (!par_->isNewline(i) && !par_->isEnvSeparator(i) && !par_->isHfill(i))
			return true;
	}
	return false;
}

} // namespace lyx
