/**
 * \file rowpainter.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author various
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef ROWPAINTER_H
#define ROWPAINTER_H

#include <config.h>

#include "LString.h"
#include "support/types.h"

class LyXText;
class BufferView;
class Row;
class Paragraph;
class Painter;
class LyXFont;
class VSpace;

/**
 * A class used for painting an individual row of text.
 */
class RowPainter {
public:
	/// initialise painter
	RowPainter(BufferView const & bv, LyXText const & text, Row const & row);

	/// paint the row. Returns true if CHANGED_IN_DRAW (e.g. image was loaded)
	bool paint(int y_offset, int x_offset, int y, bool cleared = false);

private:
	// paint various parts
	bool paintBackground();
	void paintSelection();
	void paintAppendix();
	void paintDepthBar();
	void paintChangeBar();
	void paintFirst();
	void paintLast();
	void paintForeignMark(float const orig_x, LyXFont const & orig_font);
	void paintHebrewComposeChar(lyx::pos_type & vpos);
	void paintArabicComposeChar(lyx::pos_type & vpos);
	void paintChars(lyx::pos_type & vpos, bool hebrew, bool arabic);
	int paintPageBreak(string const & label, int y);
	int paintLengthMarker(string const & prefix, VSpace const & vsp, int start);
	/// returns true when CHANGED_IN_DRAW
	bool paintText();
	/// returns true when CHANGED_IN_DRAW
	bool paintFromPos(lyx::pos_type & vpos);
	/// returns true when CHANGED_IN_DRAW
	bool paintInset(lyx::pos_type const pos);

	/// return left margin
	int leftMargin() const;

	/// return the font at the given pos
	LyXFont const getFont(lyx::pos_type pos) const;

	/// return the label font for this row
	LyXFont const getLabelFont() const;

	char const transformChar(char c, lyx::pos_type pos) const;

	/// return pixel width for the given pos
	int singleWidth(lyx::pos_type pos) const;
	int singleWidth(lyx::pos_type pos, char c) const;

	/// bufferview to paint on
	BufferView const & bv_;

	/// Painter to use
	Painter & pain_;

	/// LyXText for the row
	LyXText const & text_;

	/// The row to paint
	Row const & row_;

	/// Row's paragraph
	Paragraph const & par_;

	// Looks ugly - is
	int xo_;
	int yo_;
	float x_;
	int y_;
	bool cleared_;
	int width_;
	float separator_;
	float hfill_;
	float label_hfill_;
};

/// return the pixel height of a space marker before/after a par
int getLengthMarkerHeight(BufferView const & bv, VSpace const & vsp);

#endif // ROWPAINTER_H
