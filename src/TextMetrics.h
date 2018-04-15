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

#include "Font.h"
#include "InsetList.h"
#include "LayoutEnums.h"
#include "ParagraphMetrics.h"

#include "support/types.h"

namespace lyx {

class BufferView;
class Cursor;
class CursorSlice;
class DocIterator;
class MetricsInfo;
class Text;

/// A map from a Text to the map of paragraphs metrics
class TextMetrics
{
public:
	/// Default constructor (only here for STL containers).
	TextMetrics() : bv_(0), text_(0), max_width_(0) {}
	/// The only useful constructor.
	TextMetrics(BufferView *, Text *);

	///
	bool contains(pit_type pit) const;
	///
	ParagraphMetrics const & parMetrics(pit_type) const;
	///
	std::pair<pit_type, ParagraphMetrics const *> first() const;
	///
	std::pair<pit_type, ParagraphMetrics const *> last() const;

	///
	Dimension const & dimension() const { return dim_; }
	///
	Point const & origin() const { return origin_; }


	/// compute text metrics.
	bool metrics(MetricsInfo & mi, Dimension & dim, int min_width = 0);

	///
	void newParMetricsDown();
	///
	void newParMetricsUp();

	/// The "nodraw" drawing stage for one single paragraph: set the
	/// positions of the insets contained this paragraph in metrics
	/// cache. Related to BufferView::updatePosCache.
	void updatePosCache(pit_type pit) const;

	/// Gets the fully instantiated font at a given position in a paragraph
	/// Basically the same routine as Paragraph::getFont() in Paragraph.cpp.
	/// The difference is that this one is used for displaying, and thus we
	/// are allowed to make cosmetic improvements. For instance make footnotes
	/// smaller. (Asger)
	Font displayFont(pit_type pit, pos_type pos) const;

	/// There are currently two font mechanisms in LyX:
	/// 1. The font attributes in a lyxtext, and
	/// 2. The inset-specific font properties, defined in an inset's
	/// metrics() and draw() methods and handed down the inset chain through
	/// the pi/mi parameters, and stored locally in a lyxtext in font_.
	/// This is where the two are integrated in the final fully realized
	/// font.
	void applyOuterFont(Font &) const;

	/// is this position in the paragraph right-to-left?
	bool isRTL(CursorSlice const & sl, bool boundary) const;
	/// is between pos-1 and pos an RTL<->LTR boundary?
	bool isRTLBoundary(pit_type pit,
	  pos_type pos) const;
	/// would be a RTL<->LTR boundary between pos and the given font?
	bool isRTLBoundary(pit_type pit,
	  pos_type pos, Font const & font) const;


	/// Rebreaks the given paragraph.
	/// \retval true if a full screen redraw is needed.
	/// \retval false if a single paragraph redraw is enough.
	bool redoParagraph(pit_type const pit);
	/// Clear cache of paragraph metrics
	void clear() { par_metrics_.clear(); }
	/// Is cache of paragraph metrics empty ?
	bool empty() const { return par_metrics_.empty(); }

	///
	int ascent() const { return dim_.asc; }
	///
	int descent() const { return dim_.des; }
	/// current text width.
	int width() const { return dim_.wid; }
	/// current text height.
	int height() const { return dim_.height(); }

	///
	int rightMargin(ParagraphMetrics const & pm) const;
	int rightMargin(pit_type const pit) const;

	///
	void draw(PainterInfo & pi, int x, int y) const;

	void drawParagraph(PainterInfo & pi, pit_type pit, int x, int y) const;

private:
	///
	ParagraphMetrics & parMetrics(pit_type, bool redo_paragraph);

	/**
	 * Returns the left beginning of the text.
	 * This information cannot be taken from the layout object, because
	 * in LaTeX the beginning of the text fits in some cases
	 * (for example sections) exactly the label-width.
	 */
	int leftMargin(pit_type pit, pos_type pos) const;
	int leftMargin(pit_type pit) const;

	/// the minimum space a manual label needs on the screen in pixels
	int labelFill(Row const & row) const;

	/// FIXME??
	int labelEnd(pit_type const pit) const;

	/// sets row.end to the pos value *after* which a row should break.
	/// for example, the pos after which isNewLine(pos) == true
	/// \return true when another row is required (after a newline)
	bool breakRow(Row & row, int right_margin) const;

	// Expand the alignment of row \param row in paragraph \param par
	LyXAlignment getAlign(Paragraph const & par, Row const & row) const;
	/** this calculates the specified parameters. needed when setting
	 * the cursor and when creating a visible row */
	void computeRowMetrics(Row & row, int width) const;

	/// Set the height of the row (without space above/below paragraph)
	void setRowHeight(Row & row) const;
	// Compute the space on top of a paragraph
	int parTopSpacing(pit_type pit) const;
	// Compute the space below a a paragraph
	int parBottomSpacing(pit_type pit) const;

	// Helper function for the other checkInsetHit method.
	InsetList::InsetTable * checkInsetHit(pit_type pit, int x, int y);


// Temporary public:
public:
	/// returns the position near the specified x-coordinate of the row.
	/// x is an absolute screen coord, it is set to the real beginning
	/// of this column. This takes in account horizontal cursor row scrolling.
	pos_type getPosNearX(Row const & row, int & x, bool & boundary) const;

	/// returns pos in given par at given x coord.
	pos_type x2pos(pit_type pit, int row, int x) const;

	// FIXME: is there a need for this?
	//int pos2x(pit_type pit, pos_type pos) const;

	/// returns the row near the specified y-coordinate in a given paragraph
	/// (relative to the screen). If assert_in_view is true, it is made sure
	/// that the row is on screen completely; this might change the given pit.
	Row const & getPitAndRowNearY(int & y, pit_type & pit,
		bool assert_in_view, bool up);

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
	\param assert_in_view if true the cursor will be set on a row
           that is completely visible
	\param up whether we are going up or down (only used when
           assert_in_view is true
	\retval inset is null if the cursor is positioned over normal
	       text in the current Text object. Otherwise it is the inset
	       that the cursor points to, like for Inset::editXY.
	*/
	/// FIXME: cleanup to use BufferView::getCoveringInset() and
	/// setCursorFromCoordinates() instead of checkInsetHit().
	Inset * editXY(Cursor & cur, int x, int y,
		bool assert_in_view = false, bool up = true);

	/// sets cursor only within this Text.
	/// x,y are screen coordinates
	void setCursorFromCoordinates(Cursor & cur, int x, int y);

	///
	int cursorX(CursorSlice const & cursor, bool boundary) const;
	///
	int cursorY(CursorSlice const & cursor, bool boundary) const;

	///
	bool cursorHome(Cursor & cur);
	///
	bool cursorEnd(Cursor & cur);
	///
	void deleteLineForward(Cursor & cur);

	/// is this row the last in the text?
	bool isLastRow(Row const & row) const;
	/// is this row the first in the text?
	bool isFirstRow(Row const & row) const;

	/// Returns an inset if inset was hit, or 0 if not.
	/// \warning This method is not recursive! It will return the
	/// outermost inset within this Text.
	/// \sa BufferView::getCoveringInset() to get the innermost inset.
	Inset * checkInsetHit(int x, int y);

	/// calculates the position of a completion popup
	void completionPosAndDim(Cursor const & cur, int & x, int & y,
		Dimension & dim) const;

private:
	friend class BufferView;

	/// The BufferView owner.
	BufferView * bv_;

	/// The text contents (the model).
	/// \todo FIXME: this should be const.
	Text * text_;

	/// A map from paragraph index number to paragraph metrics
	typedef std::map<pit_type, ParagraphMetrics> ParMetricsCache;
	///
	mutable ParMetricsCache par_metrics_;
	Dimension dim_;
	int max_width_;
	mutable Point origin_;

// temporary public:
public:
	/// our 'outermost' font.
	/// This is handed down from the surrounding
	/// inset through the pi/mi parameter (pi.base.font)
	/// It is used in applyOuterFont() and setCharFont() for reasons
	/// that are not clear... to hand hand the outermost language and
	/// also for char style apparently.
	Font font_;
};

/// return the default height of a row in pixels, considering font zoom
int defaultRowHeight();

} // namespace lyx

#endif // TEXT_METRICS_H
