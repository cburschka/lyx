/**
 * \file FontList.cpp
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
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "FontList.h"

#include "support/lyxalgo.h"

using namespace std;

namespace lyx {


FontList::iterator FontList::fontIterator(pos_type pos)
{
	FontList::iterator it = list_.begin();
	FontList::iterator end = list_.end();
	for (; it != end; ++it) {
		if (it->pos() >= pos)
			break;
	}
	return it;
}


FontList::const_iterator FontList::fontIterator(pos_type pos) const
{
	FontList::const_iterator it = list_.begin();
	FontList::const_iterator end = list_.end();
	for (; it != end; ++it) {
		if (it->pos() >= pos)
			break;
	}
	return it;
}


Font const & FontList::get(pos_type pos)
{
	iterator end = list_.end();
	iterator it = fontIterator(pos);
	if (it != end && it->pos() == pos)
		return it->font_;

	static Font const dummy;
	return dummy;
}


void FontList::erase(pos_type pos)
{
	// Erase entries in the tables.
	iterator it = fontIterator(pos);
	iterator beg = list_.begin();
	if (it != list_.end() && it->pos() == pos
		&& (pos == 0 
			|| (it != list_.begin() && prev(it, 1)->pos() == pos - 1))) {

		// If it is a multi-character font
		// entry, we just make it smaller
		// (see update below), otherwise we
		// should delete it.
		unsigned int const i = it - list_.begin();
		list_.erase(it);
		if (i >= list_.size())
			return;
		it = list_.begin() + i;
		if (i > 0 && i < list_.size() &&
		    list_[i - 1].font() == list_[i].font()) {
			list_.erase(beg + i - 1);
			it = list_.begin() + i - 1;
		}
	}

	// Update all other entries
	iterator end = list_.end();
	for (; it != end; ++it)
		it->pos(it->pos() - 1);
}

void FontList::increasePosAfterPos(pos_type pos)
{
	List::iterator end = list_.end();
	List::iterator it = fontIterator(pos);
	for (; it != end; ++it)
		++it->pos_;
}


void FontList::decreasePosAfterPos(pos_type pos)
{
	List::iterator end = list_.end();
	List::iterator it = fontIterator(pos);
	for (; it != end; ++it)
		--it->pos_;
}


void FontList::setRange(pos_type startpos, pos_type endpos, Font const & font)
{
	// FIXME: Optimize!!!
	for (pos_type pos = startpos; pos != endpos; ++pos)
		set(pos, font);
}


void FontList::set(pos_type pos, Font const & font)
{
	// No need to simplify this because it will disappear
	// in a new kernel. (Asger)
	// Next search font table

	List::iterator it = fontIterator(pos);
	bool const found = it != list_.end();
	if (found && it->font() == font)
		// Font is already set.
		return;

	size_t const i = distance(list_.begin(), it);

	// Is position pos a beginning of a font block?
	bool const begin = pos == 0 || !found 
		|| (i > 0 && list_[i - 1].pos() == pos - 1);

	// Is position pos at the end of a font block?
	bool const end = found && list_[i].pos() == pos;

	if (!begin && !end) {
		// The general case: The block is split into 3 blocks
		list_.insert(list_.begin() + i,
				FontTable(pos - 1, list_[i].font()));
		list_.insert(list_.begin() + i + 1,
				FontTable(pos, font));
		return;
	}

	if (begin && end) {
		// A single char block
		if (i + 1 < list_.size() &&
		    list_[i + 1].font() == font) {
			// Merge the singleton block with the next block
			list_.erase(list_.begin() + i);
			if (i > 0 && list_[i - 1].font() == font)
				list_.erase(list_.begin() + i - 1);
		} else if (i > 0 && list_[i - 1].font() == font) {
			// Merge the singleton block with the previous block
			list_[i - 1].pos(pos);
			list_.erase(list_.begin() + i);
		} else
			list_[i].font(font);
	} else if (begin) {
		if (i > 0 && list_[i - 1].font() == font)
			list_[i - 1].pos(pos);
		else
			list_.insert(list_.begin() + i,
					FontTable(pos, font));
	} else if (end) {
		list_[i].pos(pos - 1);
		if (!(i + 1 < list_.size() &&
		      list_[i + 1].font() == font))
			list_.insert(list_.begin() + i + 1,
					FontTable(pos, font));
	}
}


FontSize FontList::highestInRange(pos_type startpos, pos_type endpos,
	FontSize def_size) const
{
	if (list_.empty())
		return def_size;

	List::const_iterator end_it = fontIterator(endpos);
	const_iterator const end = list_.end();
	if (end_it != end)
		++end_it;

	List::const_iterator cit = fontIterator(startpos);

	FontSize maxsize = FONT_SIZE_TINY;
	for (; cit != end_it; ++cit) {
		FontSize size = cit->font().fontInfo().size();
		if (size == FONT_SIZE_INHERIT)
			size = def_size;
		if (size > maxsize && size <= FONT_SIZE_HUGER)
			maxsize = size;
	}
	return maxsize;
}


void FontList::validate(LaTeXFeatures & features) const
{
	const_iterator fcit = list_.begin();
	const_iterator fend = list_.end();
	for (; fcit != fend; ++fcit)
		fcit->font().validate(features);
}

} // namespace lyx
