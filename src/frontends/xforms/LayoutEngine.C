// -*- C++ -*-
/**
 * \file LayoutEngine.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 *
 * A generic layout engine.
 *
 * Draws heavily on the GTK+ files gtkbox.[ch], gtkhbox.[ch],
 * for both inspiration and implementation,
 * and from which this notice is taken:
 *
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/.
 */

#include "LayoutEngine.h"
#include "lyx_forms.h"
#include <boost/assert.hpp>


namespace lyx {
namespace frontend {


bool BoxList::empty() const
{
	return data_.empty();
}


BoxList::size_type BoxList::size() const
{
	return data_.size();
}


void BoxList::clear()
{
	data_.clear();
}


Box & BoxList::push_back(Box const & box)
{
	data_.push_back(box);
	return data_.back();
}


BoxList::iterator BoxList::begin()
{
	return data_.begin();
}


BoxList::iterator BoxList::end()
{
	return data_.end();
}


BoxList::const_iterator BoxList::begin() const
{
	return data_.begin();
}


BoxList::const_iterator BoxList::end() const
{
	return data_.end();
}


BoxList::iterator BoxList::erase(iterator where)
{
	return data_.erase(where);
}


BoxList::iterator BoxList::erase(iterator begin, iterator end)
{
	return data_.erase(begin, end);
}


Box::Orientation Box::default_orientation_ = Box::Vertical;
Box::Packing Box::default_packing_ = Box::Shrink;


Box::Box(dimension_t min_w, dimension_t min_h)
	: visible_(true),
	  min_w_(min_w),
	  min_h_(min_h),
	  w_(min_w),
	  h_(min_h),
	  x_(0),
	  y_(0),
	  orientation_(default_orientation_),
	  packing_(default_packing_),
	  prefered_visibility_(Visible)
{}


void Box::setMinimumDimensions(dimension_t min_w, dimension_t min_h)
{
	min_w_ = min_w;
	min_h_ = min_h;
}


Box::Orientation Box::orientation() const
{
	return orientation_;
}


void Box::set(Orientation o)
{
	orientation_ = o;
}


Box::Orientation Box::defaultOrientation()
{
	return default_orientation_;
}


void Box::setDefault(Orientation o)
{
	default_orientation_ = o;
}


Box::Packing Box::packing() const
{
	return packing_;
}


void Box::set(Packing p)
{
	packing_ = p;
}


Box::Packing Box::defaultPacking()
{
	return default_packing_;
}


void Box::setDefault(Packing p)
{
	default_packing_ = p;
}


bool Box::expandable() const
{
	if (!visible_)
		return false;

	if (packing_ == Expand)
		return true;

	BoxList::const_iterator it = children_.begin();
	BoxList::const_iterator const end = children_.end();
	for (; it != end; ++it) {
		if (it->visible() && it->packing() == Expand)
			return true;
	}

	return false;
}


Box::PreferedVisibility Box::preferedVisibility() const
{
	return prefered_visibility_;
}


void Box::set(PreferedVisibility pv)
{
	prefered_visibility_ = pv;
	if (pv == Invisible && visible_)
		hide();
}


bool Box::visible() const
{
	return visible_;
}


void Box::show()
{
	if (prefered_visibility_ == Invisible)
		return;

	visible_ = true;

	BoxList::iterator it = children_.begin();
	BoxList::iterator const end = children_.end();
	for (; it != end; ++it)
		it->show();
}


void Box::hide()
{
	visible_ = false;

	BoxList::iterator it = children_.begin();
	BoxList::iterator const end = children_.end();
	for (; it != end; ++it)
		it->hide();
}


BoxList & Box::children()
{
	return children_;
}


BoxList const & Box::children() const
{
	return children_;
}


Box::dimension_t Box::width() const
{
	return w_;
}


Box::dimension_t Box::height() const
{
	return h_;
}


Box::dimension_t Box::xorigin() const
{
	return x_;
}


Box::dimension_t Box::yorigin() const
{
	return y_;
}


void Box::updateMetrics()
{
	shrinkMetrics();
	expandMetrics(x_, y_, w_, h_);
}


void Box::shrinkMetrics()
{
	dimension_t width = 0;
	dimension_t height = 0;

	BoxList::iterator it = children_.begin();
	BoxList::iterator const end = children_.end();
	for (; it != end; ++it) {
		if (!it->visible())
			continue;

		it->shrinkMetrics();
		dimension_t child_width = it->width();
		dimension_t child_height = it->height();

		if (orientation_ == Horizontal) {
			width += child_width;
			height = std::max(height, child_height);
		} else {
			width = std::max(width, child_width);
			height += child_height;
		}
	}

	w_ = visible_ ? std::max(min_w_, width) : 0;
	h_ = visible_ ? std::max(min_h_, height) : 0;
}


void Box::expandMetrics(dimension_t x_in, dimension_t y_in,
			dimension_t w_avail, dimension_t h_avail)
{
	x_ = x_in;
	y_ = y_in;
	w_ = w_avail;
	h_ = h_avail;

	if (orientation_ == Vertical)
		expandVbox(x_in, y_in, w_avail, h_avail);
	else
		expandHbox(x_in, y_in, w_avail, h_avail);
}


void Box::expandHbox(dimension_t x_in, dimension_t y_in,
		     dimension_t w_avail, dimension_t h_avail)
{
	int nvisible_children = 0;
	int nexpanded_children = 0;
	dimension_t w_fixed = 0;

	BoxList::const_iterator cit = children_.begin();
	BoxList::const_iterator const cend = children_.end();
	for (; cit != cend; ++cit) {
		if (cit->visible()) {
			nvisible_children += 1;
			if (cit->expandable())
				nexpanded_children += 1;
			else
				w_fixed += cit->width();
		}
	}

	if (nvisible_children == 0)
		return;

	dimension_t width = 0;
	dimension_t extra = 0;
	if (nexpanded_children > 0) {
		width = w_avail - w_fixed;
		extra = width / nexpanded_children;
	}

	dimension_t x_child = x_in;
	dimension_t y_child = y_in;
	dimension_t h_child = h_avail;

	BoxList::iterator it = children_.begin();
	BoxList::iterator const end = children_.end();
	for (; it != end; ++it) {
		if (!it->visible())
			continue;

		dimension_t w_child = it->width();
		if (it->expandable()) {
			if (nexpanded_children == 1)
				w_child = std::max(w_child, width);
			else
				w_child = std::max(w_child, extra);

			nexpanded_children -= 1;
			width -= w_child;
		}

		it->expandMetrics(x_child, y_child, w_child, h_child);
		x_child += w_child;
	}
}


void Box::expandVbox(dimension_t x_in, dimension_t y_in,
		     dimension_t w_avail, dimension_t h_avail)
{
	int nvisible_children = 0;
	int nexpanded_children = 0;
	dimension_t h_fixed = 0;

	BoxList::const_iterator cit = children_.begin();
	BoxList::const_iterator const cend = children_.end();
	for (; cit != cend; ++cit) {
		if (cit->visible()) {
			nvisible_children += 1;
			if (cit->expandable())
				nexpanded_children += 1;
			else
				h_fixed += cit->height();
		}
	}

	if (nvisible_children == 0)
		return;

	dimension_t height = 0;
	dimension_t extra = 0;
	if (nexpanded_children > 0) {
		height = h_avail - h_fixed;
		extra = height / nexpanded_children;
	}

	dimension_t x_child = x_in;
	dimension_t y_child = y_in;
	dimension_t w_child = w_avail;

	BoxList::iterator it = children_.begin();
	BoxList::iterator const end = children_.end();
	for (; it != end; ++it) {
		if (!it->visible())
			continue;

		dimension_t h_child = it->height();
		if (it->expandable()) {
			if (nexpanded_children == 1)
				h_child = std::max(h_child, height);
			else
				h_child = std::max(h_child, extra);
			nexpanded_children -= 1;
			height -= h_child;
		}

		it->expandMetrics(x_child, y_child, w_child, h_child);
		y_child += h_child;
	}
}


Box & WidgetMap::add(FL_OBJECT * ob, BoxList & container,
		     dimension_t min_w, dimension_t min_h)
{
	Box & box = container.push_back(Box(min_w, min_h));
	widgets_[ob] = &box;
	return box;
}


void WidgetMap::updateMetrics() const
{
	DataMap::const_iterator it = widgets_.begin();
	DataMap::const_iterator const end = widgets_.end();
	for (; it != end; ++it) {
		FL_OBJECT * ob = it->first;
		Box & box = *it->second;

		if (box.visible()) {
			fl_set_object_geometry(ob,
					       box.xorigin(), box.yorigin(),
					       box.width(), box.height());
			if (!ob->visible)
				fl_show_object(ob);
		} else {
			if (ob->visible)
				fl_hide_object(ob);
		}
	}
}


Box & embed(FL_OBJECT * ob, BoxList & container, WidgetMap & widgets, int bw)
{
	container.push_back(Box(0, bw));
	Box & middle = container.push_back(Box(0, 0));
	middle.set(Box::Horizontal);
	container.push_back(Box(0, bw));

	middle.children().push_back(Box(bw, 0));
	Box & center = widgets.add(ob, middle.children(), 0, 0);
	middle.children().push_back(Box(bw, 0));

	return center;
}

} // namespace frontend
} // namespace lyx
