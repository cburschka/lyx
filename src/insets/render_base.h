// -*- C++ -*-
/**
 * \file render_base.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef RENDER_BASE_H
#define RENDER_BASE_H

#include "dimension.h"

class MetricsInfo;
class PainterInfo;


class RenderBase {
public:
	virtual ~RenderBase() {}

	virtual RenderBase * clone() const = 0;

	/// compute the size of the object returned in dim
	virtual void metrics(MetricsInfo & mi, Dimension & dim) const = 0;
	/// draw inset and update (xo, yo)-cache
	virtual void draw(PainterInfo & pi, int x, int y) const = 0;

protected:
	RenderBase() {}
	RenderBase(RenderBase const &) {}
	void operator=(RenderBase const &) {}

	/// Cached
	mutable Dimension dim_;
};

#endif // NOT RENDER_BASE_H
