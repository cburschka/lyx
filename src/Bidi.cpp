/**
 * \file Bidi.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Bidi.h"
#include "Buffer.h"
#include "BufferView.h"
#include "Cursor.h"
#include "Font.h"
#include "Row.h"
#include "LyXRC.h"
#include "Paragraph.h"


namespace lyx {


pos_type Bidi::log2vis(pos_type pos) const
{
	return (start_ == -1) ? pos : log2vis_list_[pos - start_];
}


pos_type Bidi::vis2log(pos_type pos) const
{
	return (start_ == -1) ? pos : vis2log_list_[pos - start_];
}


pos_type Bidi::level(pos_type pos) const
{
	return (start_ == -1) ? 0 : levels_[pos - start_];
}


bool Bidi::inRange(pos_type pos) const
{
	return start_ == -1 || (start_ <= pos && pos <= end_);
}


bool Bidi::same_direction() const
{
	return same_direction_;
}


void Bidi::computeTables(Paragraph const & par,
	Buffer const & buf, Row const & row)
{
	same_direction_ = true;

	if (par.inInset().forceLTR()) {
		start_ = -1;
		return;
	}

	start_ = row.pos();
	end_ = row.endpos() - 1;

	if (start_ > end_) {
		start_ = -1;
		return;
	}

	if (end_ + 2 - start_ >
	    static_cast<pos_type>(log2vis_list_.size())) {
		pos_type new_size =
			(end_ + 2 - start_ < 500) ?
			500 : 2 * (end_ + 2 - start_);
		log2vis_list_.resize(new_size);
		vis2log_list_.resize(new_size);
		levels_.resize(new_size);
	}

	vis2log_list_[end_ + 1 - start_] = -1;
	log2vis_list_[end_ + 1 - start_] = -1;

	BufferParams const & bufparams = buf.params();
	pos_type stack[2];
	bool const rtl_par = par.isRTL(bufparams);
	int lev = 0;
	bool rtl = false;
	bool rtl0 = false;
	pos_type const body_pos = par.beginOfBody();

	for (pos_type lpos = start_; lpos <= end_; ++lpos) {
		bool is_space = false;
		// We do not handle spaces around an RTL segment in a special way anymore.
		// Neither do we do so when generating the LaTeX, so setting is_space
		// to false makes the view in the GUI consistent with the output of LaTeX 
		// later. The old setting was:
		//bool is_space = par.isLineSeparator(lpos);
		// FIXME: once we're sure that this is what we really want, we should just
		// get rid of this variable...
		pos_type const pos =
			(is_space && lpos + 1 <= end_ &&
			 !par.isLineSeparator(lpos + 1) &&
			 !par.isEnvSeparator(lpos + 1) &&
			 !par.isNewline(lpos + 1))
			? lpos + 1 : lpos;

		Font const * font = &(par.getFontSettings(bufparams, pos));
		if (pos != lpos && 0 < lpos && rtl0 && font->isRightToLeft() &&
		    font->fontInfo().number() == FONT_ON &&
		    par.getFontSettings(bufparams, lpos - 1).fontInfo().number()
		    == FONT_ON) {
			font = &(par.getFontSettings(bufparams, lpos));
			is_space = false;
		}
		bool new_rtl = font->isVisibleRightToLeft();
		bool new_rtl0 = font->isRightToLeft();

		int new_level;

		if (lpos == body_pos - 1
		    && row.pos() < body_pos - 1
		    && is_space) {
			new_level = rtl_par ? 1 : 0;
			new_rtl0 = rtl_par;
			new_rtl = rtl_par;
		} else if (new_rtl0) {
			new_level = new_rtl ? 1 : 2;
		} else {
			new_level = rtl_par ? 2 : 0;
		}

		if (is_space && new_level >= lev) {
			new_level = lev;
			new_rtl = rtl;
			new_rtl0 = rtl0;
		}

		int new_level2 = new_level;

		if (lev == new_level && rtl0 != new_rtl0) {
			--new_level2;
			log2vis_list_[lpos - start_] = rtl ? 1 : -1;
		} else if (lev < new_level) {
			log2vis_list_[lpos - start_] = rtl ? -1 : 1;
			if (new_level > 0 && !rtl_par)
				same_direction_ = false;
		} else {
			log2vis_list_[lpos - start_] = new_rtl ? -1 : 1;
		}
		rtl = new_rtl;
		rtl0 = new_rtl0;
		levels_[lpos - start_] = new_level;

		while (lev > new_level2) {
			pos_type old_lpos = stack[--lev];
			int delta = lpos - old_lpos - 1;
			if (lev % 2)
				delta = -delta;
			log2vis_list_[lpos - start_] += delta;
			log2vis_list_[old_lpos - start_] += delta;
		}
		while (lev < new_level)
			stack[lev++] = lpos;
	}

	while (lev > 0) {
		pos_type const old_lpos = stack[--lev];
		int delta = end_ - old_lpos;
		if (lev % 2)
			delta = -delta;
		log2vis_list_[old_lpos - start_] += delta;
	}

	pos_type vpos = start_ - 1;
	for (pos_type lpos = start_; lpos <= end_; ++lpos) {
		vpos += log2vis_list_[lpos - start_];
		vis2log_list_[vpos - start_] = lpos;
		log2vis_list_[lpos - start_] = vpos;
	}
}


// This method requires a previous call to computeTables()
bool Bidi::isBoundary(Buffer const & buf, Paragraph const & par,
	pos_type pos) const
{
	if (pos == 0)
		return false;

	if (!inRange(pos - 1)) {
		// This can happen if pos is the first char of a row.
		// Returning false in this case is incorrect!
		return false;
	}

	bool const rtl = level(pos - 1) % 2;
	bool const rtl2 = inRange(pos)
		? level(pos) % 2
		: par.isRTL(buf.params());
	return rtl != rtl2;
}


bool Bidi::isBoundary(Buffer const & buf, Paragraph const & par,
	pos_type pos, Font const & font) const
{
	bool const rtl = font.isVisibleRightToLeft();
	bool const rtl2 = inRange(pos)
		? level(pos) % 2
		: par.isRTL(buf.params());
	return rtl != rtl2;
}

} // namespace lyx
