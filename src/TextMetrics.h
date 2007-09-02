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

#include <boost/utility.hpp>
#include <boost/tuple/tuple.hpp>

#include <map>


namespace lyx {

class BufferView;
class DocIterator;
class MetricsInfo;
class Text;

/// A map from a Text to the map of paragraphs metrics
class TextMetrics
{
public:
	/// Default constructor (only here for STL containers).
	TextMetrics(): text_(0) {}
	/// The only useful constructor.
	TextMetrics(BufferView *, Text *);
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
	/// Clear cache of paragraph metrics
	void clear() { par_metrics_.clear(); }

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
	int singleWidth(pit_type const pit,	pos_type pos) const;

	///
	int rightMargin(ParagraphMetrics const & pm) const;
	int rightMargin(pit_type const pit) const;

	/** this calculates the specified parameters. needed when setting
	 * the cursor and when creating a visible row */
	void computeRowMetrics(pit_type pit, Row & row) const;

	///
	void draw(PainterInfo & pi, int x, int y) const;
	/// draw textselection
	void drawSelection(PainterInfo & pi, int x, int y) const;
	
	void drawParagraph(PainterInfo & pi, pit_type pit, int x, int y) const;

private:
	///
	ParagraphMetrics & parMetrics(pit_type, bool redo_paragraph);

	/// the minimum space a manual label needs on the screen in pixels
	int labelFill(pit_type const pit, Row const & row) const;

	/// FIXME??
	int labelEnd(pit_type const pit) const;

	/// sets row.end to the pos value *after* which a row should break.
	/// for example, the pos after which isNewLine(pos) == true
	pit_type rowBreakPoint(
		int width,
		pit_type const pit,
		pit_type first
		) const;

	/// sets row.width to the minimum space a row needs on the screen in pixel
	int rowWidth(
		int right_margin,
		pit_type const pit,
		pos_type const first,
		pos_type const end
		) const;

	/// Calculate and set the height of the row
	boost::tuple<int, int> rowHeight(
		pit_type const pit,
		pos_type const first,
		pos_type const end
		) const;

	/// draw selection for a single row
	void drawRowSelection(PainterInfo & pi, int x, Row const & row,
		DocIterator const & beg, DocIterator const & end, 
		bool drawOnBegMargin, bool drawOnEndMargin) const;

// Temporary public:
public:
	/// returns the column near the specified x-coordinate of the row.
	/// x is an absolute screen coord, it is set to the real beginning
	/// of this column.
	pos_type getColumnNearX(pit_type pit, Row const & row, int & x,
		bool & boundary) const;

	/// returns pos in given par at given x coord.
	pos_type x2pos(pit_type pit, int row, int x) const;

	// FIXME: is there a need for this?
	//int pos2x(pit_type pit, pos_type pos) const;

	/** returns row near the specified
	  * y-coordinate in given paragraph (relative to the screen).
	  */
	Row const & getRowNearY(int y,
		pit_type pit) const;

	/// returns the paragraph number closest to screen y-coordinate.
	/// This method uses the BufferView CoordCache to locate the
	/// paragraph. The y-coodinate is allowed to be off-screen and
	/// the CoordCache will be automatically updated if needed. This is
	/// the reason why we need a non const BufferView.
	pit_type getPitNearY(int y);

	/// sets cursor recursively descending into nested editable insets
	/**
	\return the inset pointer if x,y is covering that inset
	\param x,y are absolute screen coordinates.
	\retval inset is non-null if the cursor is positionned inside
	*/
	/// FIXME: cleanup to use BufferView::getCoveringInset() and
	/// setCursorFromCoordinates() instead of checkInsetHit().
	Inset * editXY(Cursor & cur, int x, int y);

	/// sets cursor only within this Text.
	/// x,y are screen coordinates
	void setCursorFromCoordinates(Cursor & cur, int x, int y);

	///
	int cursorX(CursorSlice const & cursor,
		bool boundary) const;
	///
	int cursorY(CursorSlice const & cursor,
		bool boundary) const;

	///
	void cursorPrevious(Cursor & cur);
	///
	void cursorNext(Cursor & cur);
	///
	bool cursorHome(Cursor & cur);
	///
	bool cursorEnd(Cursor & cur);
	///
	void deleteLineForward(Cursor & cur);

	/// Returns an inset if inset was hit, or 0 if not.
	/// \warning This method is not recursive! It will return the
	/// outermost inset within this Text.
	/// \sa BufferView::getCoveringInset() to get the innermost inset.
	Inset * checkInsetHit(int x, int y);

	/**
	 * Returns the left beginning of the text.
	 * This information cannot be taken from the layout object, because
	 * in LaTeX the beginning of the text fits in some cases
	 * (for example sections) exactly the label-width.
	 */
	int leftMargin(int max_width, pit_type pit, pos_type pos) const;
	int leftMargin(int max_width, pit_type pit) const;

private:

	/// The BufferView owner.
	BufferView * bv_;

	/// The text contents (the model).
	/// \todo FIXME: this should be const.
	Text * text_;

	bool main_text_;
	/// A map from paragraph index number to paragraph metrics
	typedef std::map<pit_type, ParagraphMetrics> ParMetricsCache;
	///
	mutable ParMetricsCache par_metrics_;
	Dimension dim_;
	int max_width_;

	/// FIXME: transfer this code in CoordCache here.
	/*
	/// A map from paragraph index number to screen point
	typedef std::map<pit_type, Point> InnerParPosCache;
	/// A map from a Text to the map of paragraphs to screen points
	typedef std::map<Text const *, InnerParPosCache> ParPosCache;
	/// Paragraph grouped by owning text
	ParPosCache pars_;
	*/
};

/// return the default height of a row in pixels, considering font zoom
int defaultRowHeight();

} // namespace lyx

#endif // TEXT_METRICS_H
