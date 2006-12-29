// -*- C++ -*-
/**
 * \file TextMetrics.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef TEXT_METRICS_H
#define TEXT_METRICS_H

#include "ParagraphMetrics.h"

#include "support/types.h"

#include <map>


namespace lyx {

class BufferView;
class LyXText;
class MetricsInfo;

/// A map from a LyXText to the map of paragraphs metrics
class TextMetrics
{
public:
	/// Default constructor (only here for STL containers).
	TextMetrics(): text_(0) {}
	/// The only useful constructor.
	TextMetrics(BufferView *, LyXText *);
	///
	ParagraphMetrics const & parMetrics(pit_type) const;
	///
	Dimension const & dimension() const;
	Dimension const & dimension();

	/// compute text metrics.
	bool metrics(MetricsInfo & mi, Dimension & dim);

	/// Rebreaks the given paragraph.
	/// \retval true if a full screen redraw is needed.
	/// \retval false if a single paragraph redraw is enough.
	bool redoParagraph(pit_type const pit);

	///
	int ascent() const { return dim_.asc; }
	///
	int descent() const { return dim_.des; }
	/// current text width.
	int width() const { return dim_.wid; }
	/// current text heigth.
	int height() const { return dim_.height(); }

	///
	int maxWidth() const { return max_width_; }

	///
	int rightMargin(ParagraphMetrics const & pm) const;
	int rightMargin(pit_type const pit) const;

	/** this calculates the specified parameters. needed when setting
	 * the cursor and when creating a visible row */
	RowMetrics computeRowMetrics(pit_type pit, Row const & row) const;


private:
	/// 
	ParagraphMetrics & parMetrics(pit_type, bool redo_paragraph);

	/// the minimum space a manual label needs on the screen in pixels
	int labelFill(Paragraph const & par, Row const & row) const;

	/// The BufferView owner.
	BufferView * bv_;

	/// The text contents (the model).
	/// \todo FIXME: this should be const.
	LyXText * text_;

	bool main_text_;
	/// A map from paragraph index number to paragraph metrics
	typedef std::map<pit_type, ParagraphMetrics> ParMetricsCache;
	///
	mutable ParMetricsCache par_metrics_;
	Dimension dim_;
	int max_width_;
};

/// return the default height of a row in pixels, considering font zoom
int defaultRowHeight();

} // namespace lyx

#endif // TEXT_METRICS_H
