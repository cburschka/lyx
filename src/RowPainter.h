// -*- C++ -*-
/**
 * \file RowPainter.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author various
 * \author John Levon
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef ROWPAINTER_H
#define ROWPAINTER_H

#include "Bidi.h"
#include "Changes.h"

#include "support/types.h"

namespace lyx {

class BufferView;
class Font;
class FontInfo;
class Inset;
class Language;
class PainterInfo;
class Paragraph;
class ParagraphList;
class ParagraphMetrics;
class Row;
class Text;
class TextMetrics;

namespace frontend { class Painter; }

/**
 * FIXME: Re-implement row painting using row elements.
 *
 * This is not difficult in principle, but the code is intricate and
 * needs some careful analysis. The first thing that needs to be done
 * is to break row elements with the same criteria. Currently breakRow
 * does not consider on-the-fly spell-checking, but it is not clear to
 * me that it is required. Moreover, this thing would only work if we
 * are sure that the Row object is up-to-date when drawing happens.
 * This depends on the update machinery.
 *
 * This would allow to get rid of the Bidi class.
 */


/**
 * A class used for painting an individual row of text.
 * FIXME: get rid of that class.
 */
class RowPainter {
public:
	/// initialise and run painter
	RowPainter(PainterInfo & pi, Text const & text,
		pit_type pit, Row const & row, int x, int y);

	/// paint various parts
	/// FIXME: transfer to TextMetrics
	void paintAppendix() const;
	void paintDepthBar() const;
	void paintChangeBar() const;
	void paintTooLargeMarks(bool const left, bool const right) const;
	void paintFirst() const;
	void paintLast();
	void paintText();
	void paintOnlyInsets();
	void paintSelection() const;

private:
	void paintSeparator(double orig_x, double width, FontInfo const & font);
	void paintForeignMark(double orig_x, Language const * lang, int desc = 0) const;
	void paintTextAndSel(docstring const & str, Font const & font,
                         Change const & change,
                         pos_type start_pos, pos_type end_pos);
	void paintMisspelledMark(double orig_x,
	                         docstring const & str, Font const & font,
	                         pos_type pos, bool changed) const;
	int paintAppendixStart(int y) const;
	void paintFromPos(pos_type & vpos, bool changed);
	void paintInset(Inset const * inset, pos_type const pos);
	void paintInlineCompletion(Font const & font);

	/// return left margin
	int leftMargin() const;

	/// return the label font for this row
	FontInfo labelFont() const;

	///
	void paintLabel() const;
	///
	void paintTopLevelLabel() const;


	/// contains painting related information.
	PainterInfo & pi_;

	/// Text for the row
	Text const & text_;
	TextMetrics const & text_metrics_;
	ParagraphList const & pars_;

	/// The row to paint
	Row const & row_;

	/// Row's paragraph
	pit_type const pit_;
	Paragraph const & par_;
	ParagraphMetrics const & pm_;

	/// bidi cache
	Bidi bidi_;

	/// row changed? (change tracking)
	Change const change_;

	// Looks ugly - is
	double const xo_;
	int const yo_;    // current baseline
	double x_;
	int width_;
	int solid_line_thickness_;
	int solid_line_offset_;
	int dotted_line_thickness_;
	int dotted_line_offset_;
};

} // namespace lyx

#endif // ROWPAINTER_H
