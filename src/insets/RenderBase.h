// -*- C++ -*-
/**
 * \file RenderBase.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef RENDERBASE_H
#define RENDERBASE_H

#include "Dimension.h"


namespace lyx {

class Inset;
class MetricsInfo;
class PainterInfo;

class RenderButton;
class RenderGraphic;
class RenderPreview;
class RenderMonitoredPreview;

class RenderBase {
public:
	virtual ~RenderBase() {}

	virtual RenderBase * clone(Inset const *) const = 0;

	/// compute the size of the object returned in dim.
	/// \retval true if the metrics has changed.
	virtual void metrics(MetricsInfo & mi, Dimension & dim) const = 0;
	/// draw inset and update (xo, yo)-cache
	virtual void draw(PainterInfo & pi, int x, int y) const = 0;

	/// render state, exact meaning of state is render-specific
	void setRenderState(bool state) { state_ = state; }
	/// get render state
	bool renderState() const { return state_; }

	/// equivalent to dynamic_cast
	virtual RenderButton * asButton() { return 0; }
	virtual RenderGraphic * asGraphic() { return 0; }
	virtual RenderPreview * asPreview() { return 0; }
	virtual RenderMonitoredPreview * asMonitoredPreview() { return 0; }

protected:
	RenderBase() : state_(false) {}
	RenderBase(RenderBase const & x) : state_(x.state_) {}
	RenderBase & operator=(RenderBase const &) { return *this; }

	/// render state. currently, render_button uses this to store mouse_hover_
	bool state_;
	/// Cached
	mutable Dimension dim_;
};


} // namespace lyx

#endif // NOT RENDERBASE_H
