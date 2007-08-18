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

#include "rowpainter.h"

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


ParagraphMetrics::ParagraphMetrics(Paragraph const & par): par_(&par)
{
}


ParagraphMetrics & ParagraphMetrics::operator=(
	ParagraphMetrics const & pm)
{
	rows_ = pm.rows_;
	dim_ = pm.dim_;
	par_ = pm.par_;
	return *this;
}


size_type ParagraphMetrics::calculateRowSignature(Row const & row)
{
	boost::crc_32_type crc;
	for (pos_type i = row.pos(); i < row.endpos(); ++i) {
		char_type const b[] = { par_->getChar(i) };
		crc.process_bytes(b, 1);
	}
	return crc.checksum();
}


void ParagraphMetrics::updateRowChangeStatus()
{
	size_t const size = rows_.size();
	row_change_status_.resize(size);
	row_signature_.resize(size);

	for (size_t i = 0; i != size; ++i) {
		// Row signature; has row changed since last update?
		size_type const row_sig = calculateRowSignature(rows_[i]);
		row_change_status_[i] = row_signature_[i] != row_sig;
		row_signature_[i] = row_sig;
	}
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

} // namespace lyx
