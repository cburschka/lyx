// -*- C++ -*-
/**
 * \file ParagraphMetrics.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author Lars Gullik Bjønnes
 * \author John Levon
 * \author André Pönitz
 * \author Jürgen Vigna
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef PARAGRAPH_METRICS_H
#define PARAGRAPH_METRICS_H

#include "Dimension.h"
#include "Row.h"

#include <map>
#include <vector>

namespace lyx {

/**
 * Each paragraph is broken up into a number of rows on the screen.
 * This is a list of such on-screen rows, ordered from the top row
 * downwards.
 */
typedef std::vector<Row> RowList;

class Buffer;
class BufferView;
class BufferParams;
class Font;
class Inset;
class Paragraph;
class MetricsInfo;
class PainterInfo;

/// Helper class for paragraph metrics.
class ParagraphMetrics {
public:
	/// Default constructor (only here for STL containers).
	ParagraphMetrics() : position_(0), par_(0) {}
	/// The only useful constructor.
	explicit ParagraphMetrics(Paragraph const & par);

	/// Copy operator.
	ParagraphMetrics & operator=(ParagraphMetrics const &);

	void reset(Paragraph const & par);

	///
	Row & getRow(pos_type pos, bool boundary);
	///
	Row const & getRow(pos_type pos, bool boundary) const;
	///
	size_t pos2row(pos_type pos) const;

	/// BufferView::redoParagraph updates this
	Dimension const & dim() const { return dim_; }
	Dimension & dim() { return dim_; }
	/// total height of paragraph
	int height() const { return dim_.height(); }
	/// total width of paragraph, may differ from workwidth
	int width() const { return dim_.width(); }
	/// ascend of paragraph above baseline
	int ascent() const { return dim_.ascent(); }
	/// descend of paragraph below baseline
	int descent() const { return dim_.descent(); }
	/// Text updates the rows using this access point
	RowList & rows() { return rows_; }
	/// The painter and others use this
	RowList const & rows() const { return rows_; }
	///
	int rightMargin(BufferView const & bv) const;

	/// dump some information to lyxerr
	void dump() const;

	///
	bool hfillExpansion(Row const & row, pos_type pos) const;

	/// 
	size_t computeRowSignature(Row const &, BufferParams const & bparams) const;

	///
	int position() const { return position_; }
	void setPosition(int position);

private:
	///
	int position_;
	///
	mutable RowList rows_;
	/// cached dimensions of paragraph
	Dimension dim_;
	///
	Paragraph const * par_;
};

} // namespace lyx

#endif // PARAGRAPH_METRICS_H
