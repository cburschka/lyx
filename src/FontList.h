/**
 * \file FontList.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FONT_LIST_H
#define FONT_LIST_H

#include "Font.h"

#include "support/types.h"

#include <vector>

namespace lyx {

/** A font entry covers a range of positions. Notice that the
    entries in the list are inserted in random order.
    I don't think it's worth the effort to implement a more effective
    datastructure, because the number of different fonts in a paragraph
    is limited. (Asger)
    Nevertheless, I decided to store fontlist_ using a sorted vector:
    fontlist_ = { {pos_1,font_1} , {pos_2,font_2} , ... } where
    pos_1 < pos_2 < ..., font_{i-1} != font_i for all i,
    and font_i covers the chars in positions pos_{i-1}+1,...,pos_i
    (font_1 covers the chars 0,...,pos_1) (Dekel)
*/
class FontTable
{
public:
	///
	FontTable(pos_type p, Font const & f)
		: pos_(p), font_(f)
	{}
	///
	pos_type pos() const { return pos_; }
	///
	void pos(pos_type p) { pos_ = p; }
	///
	Font const & font() const { return font_; }
	///
	void font(Font const & f) { font_ = f;}

private:
	friend class FontList;
	/// End position of paragraph this font attribute covers
	pos_type pos_;
	/** Font. Interpretation of the font values:
	If a value is Font::INHERIT_*, it means that the font
	attribute is inherited from either the layout of this
	paragraph or, in the case of nested paragraphs, from the
	layout in the environment one level up until completely
	resolved.
	The values Font::IGNORE_* and FONT_TOGGLE are NOT
	allowed in these font tables.
	*/
	Font font_;
};

class LaTeXFeatures;

///
class FontList
{
public:
	typedef std::vector<FontTable> List;
	///
	typedef List::iterator iterator;
	///
	typedef List::const_iterator const_iterator;
	///
	iterator begin() { return list_.begin(); }
	///
	iterator end() { return list_.end(); }
	///
	const_iterator begin() const { return list_.begin(); }
	///
	const_iterator end() const { return list_.end(); }
	///
	bool empty() const { return list_.empty(); }
	///
	void clear() { list_.clear(); }
	///
	void erase(pos_type pos);
	///
	iterator fontIterator(pos_type pos);
	///
	const_iterator fontIterator(pos_type pos) const;
	///
	Font const & get(pos_type pos);
	///
	void set(pos_type pos, Font const & font);
	///
	void setRange(
		pos_type startpos,
		pos_type endpos,
		Font const & font);
	///
	void increasePosAfterPos(pos_type pos);
	///
	void decreasePosAfterPos(pos_type pos);

	/// Returns the height of the highest font in range
	FontSize highestInRange(
		pos_type startpos,
		pos_type endpos,
		FontSize def_size
		) const;

	///
	void validate(LaTeXFeatures & features) const;

private:
	///
	List list_;
};

} // namespace lyx

#endif
