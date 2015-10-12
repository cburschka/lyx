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

#include "Changes.h"
#include "Row.h"

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
class Text;
class TextMetrics;

namespace frontend { class Painter; }

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
	void paintSeparator(double width, Font const & font);
	void paintForeignMark(double orig_x, Language const * lang, int desc = 0) const;
	void paintStringAndSel(Row::Element const & e);
	void paintMisspelledMark(double orig_x, Row::Element const & e) const;
	void paintChange(double orig_x , Font const & font, Change const & change) const;
	void paintAppendixStart(int y) const;
	void paintInset(Inset const * inset, Font const & font,
                    Change const & change, pos_type const pos);

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
