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

#include <config.h>

#include "support/types.h"
#include <vector>

class Buffer;
class Paragraph;
class Row;
class LyXFont;


/// bidi stuff
struct  Bidi {
	///
	bool isBoundary(Buffer const &, Paragraph const & par,
			lyx::pos_type pos) const;
	///
	bool isBoundary(Buffer const &, Paragraph const & par,
				lyx::pos_type pos, LyXFont const & font) const;
	///
	lyx::pos_type log2vis(lyx::pos_type pos) const;
	/** Maps positions in the logical string to positions
	 *  in visual string.
	 */
	lyx::pos_type vis2log(lyx::pos_type pos) const;
	///
	lyx::pos_type level(lyx::pos_type pos) const;
	///
	bool inRange(lyx::pos_type pos) const;
	/// same_direction?
	bool same_direction() const;
	///
	void computeTables(Paragraph const & par,
			   Buffer const &, Row & row);
private: 
	/// 
	bool same_direction_;
	///
	std::vector<lyx::pos_type> log2vis_list_;
	/** Maps positions in the visual string to positions
	 *  in logical string.
	 */
	std::vector<lyx::pos_type> vis2log_list_;
	///
	std::vector<lyx::pos_type> levels_;
	///
	lyx::pos_type start_;
	///
	lyx::pos_type end_;
};

#endif // BIDI_H
