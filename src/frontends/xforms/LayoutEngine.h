// -*- C++ -*-
/**
 * \file LayoutEngine.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 *
 * A generic layout engine that draws heavily on GTK+.
 */

#ifndef LAYOUT_ENGINE_H
#define LAYOUT_ENGINE_H

#include "forms_fwd.h"
#include <list>
#include <map>

namespace lyx {
namespace frontend {

class Box;

class BoxList {
public:
	typedef std::list<Box> Container;
	typedef Container::size_type size_type;
	typedef Container::iterator iterator;
	typedef Container::const_iterator const_iterator;

	bool empty() const;
	size_type size() const;

	void clear();
	Box & push_back(Box const &);

	iterator begin();
	iterator end();

	const_iterator begin() const;
	const_iterator end() const;

	iterator erase(iterator where);
	iterator erase(iterator begin, iterator end);

private:
	Container data_;
};


class Box {
public:
	typedef unsigned int dimension_t;

	/** \param min_w the minimum allowed width of the box.
	 *  \param min_h the minimum allowed height of the box.
	 */
	Box(dimension_t min_w, dimension_t min_h);

	void setMinimumDimensions(dimension_t min_w, dimension_t min_h);

	/** \name Child Orientation
	 *  The enum prescribes whether children are aligned
	 *  horizontally or vertically.
	 */
	//@{
	enum Orientation {
		Vertical,
		Horizontal
	};

	Orientation orientation() const;
	void set(Orientation);

	/// Initially set to Vertical
	static Orientation defaultOrientation();
	static void setDefault(Orientation);
	//@}

	/** \name Packing
	 *  Do the children receive extra space when the parent grows?
	 */
	//@{
	enum Packing {
		Shrink,
		Expand
	};

	Packing packing() const;
	void set(Packing);

	/// Initially set to Shrink
	static Packing defaultPacking();
	static void setDefault(Packing);

	/** returns true if this Box or any of its children have
	 *  packing() == Expand.
	 */
	bool expandable() const;
	//@}

	/** \name Prefered Visibility
	 *  If the parent container is visible, should this Box be
	 *  visible or not?
	 */
	//@{
	enum PreferedVisibility {
		Visible,
		Invisible
	};

	PreferedVisibility preferedVisibility() const;
	/// If \pv == Invisible, also calls hide().
	void set(PreferedVisibility pv);
	//@}

	/** \name Actual Visibility
	 */
	//@{
	bool visible() const;
	/// Does nothing if preferedVisibility() == Invisible.
	void show();
	/// Always hides.
	void hide();
	//@}

	BoxList & children();
	BoxList const & children() const;

	dimension_t width() const;
	dimension_t height() const;
	dimension_t xorigin() const;
	dimension_t yorigin() const;

	void updateMetrics();

private:
	void shrinkMetrics();
	void expandMetrics(dimension_t x, dimension_t y,
			   dimension_t w, dimension_t h);
	void expandHbox(dimension_t x, dimension_t y,
			dimension_t w, dimension_t h);
	void expandVbox(dimension_t x, dimension_t y,
			dimension_t w, dimension_t h);

	static Orientation default_orientation_;
	static Packing default_packing_;

	BoxList children_;
	bool visible_;
	dimension_t min_w_;
	dimension_t min_h_;
	dimension_t w_;
	dimension_t h_;
	dimension_t x_;
	dimension_t y_;
	Orientation orientation_;
	Packing packing_;
	PreferedVisibility prefered_visibility_;
};


class WidgetMap {
public:
	typedef Box::dimension_t dimension_t;

	/// \returns the just-added Box.
	Box & add(FL_OBJECT * widget, BoxList & container,
		  dimension_t min_w, dimension_t min_h);
	void updateMetrics() const;

private:
	typedef std::map<FL_OBJECT *, Box *> DataMap;
	DataMap widgets_;
};


/** Embed \c ob in \c container inside a border of width \c bw.
 *  Thereafter, hand control of its metrics to \c widgets.
 *  \returns the Box containing \c ob.
 */
Box & embed(FL_OBJECT * ob, BoxList & container, WidgetMap & widgets, int bw);

} // namespace frontend
} // namespace lyx

#endif // NOT LAYOUT_ENGINE_H
