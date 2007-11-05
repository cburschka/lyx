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

#include "Paragraph.h"

namespace lyx {

/**
 * Each paragraph is broken up into a number of rows on the screen.
 * This is a list of such on-screen rows, ordered from the top row
 * downwards.
 */
typedef std::vector<Row> RowList;

class MetricsInfo;
class PainterInfo;

/// Helper class for Paragraph Metrics.
class ParagraphMetrics  {
public:
	/// Default constructor (only here for STL containers).
	ParagraphMetrics(): par_(0) {};
	/// The only useful constructor.
	ParagraphMetrics(Paragraph const & par);

	/// Copy operator.
	/// Important note: We don't copy \c row_change_status_ and
	/// \c row_signature_ because those are updated externally with
	/// \c updateRowChangeStatus() in TextMetrics::redoParagraph().
	ParagraphMetrics & operator=(ParagraphMetrics const &);

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
	unsigned int height() const { return dim_.height(); }
	/// total width of paragraph, may differ from workwidth
	unsigned int width() const { return dim_.width(); }
	/// ascend of paragraph above baseline
	unsigned int ascent() const { return dim_.ascent(); }
	/// descend of paragraph below baseline
	unsigned int descent() const { return dim_.descent(); }
	/// Text updates the rows using this access point
	RowList & rows() { return rows_; }
	/// The painter and others use this
	RowList const & rows() const { return rows_; }
	/// The painter and others use this
	std::vector<bool> const & rowChangeStatus() const
	{ return row_change_status_; }
	///
	void updateRowChangeStatus(BufferParams const &) const;
	///
	int rightMargin(Buffer const & buffer) const;

	/// dump some information to lyxerr
	void dump() const;

private:
	///
	typedef std::vector<size_type> RowSignature;
	///
	size_type calculateRowSignature(Row const &, BufferParams const &) const;
	///
	mutable RowList rows_;
	///
	mutable RowSignature row_signature_;
	///
	mutable std::vector<bool> row_change_status_;
	/// cached dimensions of paragraph
	Dimension dim_;
	///
	Paragraph const * par_;
};

} // namespace lyx

#endif // PARAGRAPH_METRICS_H
