// -*- C++ -*-
/**
 * \file render_graphic.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef RENDER_GRAPHIC_H
#define RENDER_GRAPHIC_H

#include "render_base.h"

#include "graphics/GraphicsLoader.h"
#include "graphics/GraphicsParams.h"


class RenderGraphic : public RenderBase
{
public:
	RenderGraphic(InsetBase const *);
	RenderGraphic(RenderGraphic const &, InsetBase const *);
	std::auto_ptr<RenderBase> clone(InsetBase const *) const;

	/// compute the size of the object returned in dim
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	/// draw inset
	void draw(PainterInfo & pi, int x, int y) const;

	/// Refresh the info about which file to display and how to display it.
	void update(lyx::graphics::Params const & params);

	/// Is the stored checksum different to that of the graphics loader?
	bool hasFileChanged() const;

	/// equivalent to dynamic_cast
	virtual RenderGraphic * asGraphic() { return this; }

private:
	/// Not implemented.
	RenderGraphic & operator=(RenderGraphic const &);

	/// The stored data.
	lyx::graphics::Loader loader_;
	lyx::graphics::Params params_;

	/// Cached variable (not copied).
	mutable unsigned long checksum_;
};


#endif // NOT RENDER_GRAPHIC_H
