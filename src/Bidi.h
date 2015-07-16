// -*- C++ -*-
/**
 * \file Bidi.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef BIDI_H
#define BIDI_H

#include "support/types.h"

#include <vector>


namespace lyx {

class Buffer;
class Cursor;
class Paragraph;
class Row;
class Font;


/// bidi stuff
class Bidi {
public:
	///
	bool isBoundary(Buffer const &, Paragraph const & par,
			pos_type pos) const;
	///
	bool isBoundary(Buffer const &, Paragraph const & par,
				pos_type pos, Font const & font) const;
	/** Maps positions in the visual string to positions
	 *  in logical string.
	 */
	pos_type log2vis(pos_type pos) const;
	/** Maps positions in the logical string to positions
	 *  in visual string.
	 */
	pos_type vis2log(pos_type pos) const;
	/* 
	 * The text direction at logical position \param pos.
	 * Possible values are
	 *  0: left-to-right text
	 *  1: right-to-left text
	 *  2: left-to-right text in right-to-left language (aka numbers)
	 */
	pos_type level(pos_type pos) const;
	/// Is logical position covered by this row?
	bool inRange(pos_type pos) const;
	/// Is the whole row in the same direction as the paragraph?
	bool same_direction() const;
	/// Compute the data for visual positions. 
	void computeTables(Paragraph const & par,
			   Buffer const &, Row const & row);
private:
	///
	bool same_direction_;
	///
	std::vector<pos_type> log2vis_list_;
	/** Maps positions in the visual string to positions
	 *  in logical string.
	 */
	std::vector<pos_type> vis2log_list_;
	///
	std::vector<pos_type> levels_;
	///
	pos_type start_;
	///
	pos_type end_;
};

} // namespace lyx

#endif // BIDI_H
