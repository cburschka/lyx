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

#include <boost/signals/signal0.hpp>


class RenderGraphic : public RenderBase
{
public:
	RenderGraphic();
	RenderGraphic(RenderGraphic const &);
	std::auto_ptr<RenderBase> clone() const;

	/// compute the size of the object returned in dim
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	/// draw inset
	void draw(PainterInfo & pi, int x, int y) const;

	/// Refresh the info about which file to display and how to display it.
	void update(lyx::graphics::Params const & params);

	/// Is the stored checksum different to that of the graphics loader?
	bool hasFileChanged() const;

	/** Connect and you'll be informed when the loading status of the image
	 *  changes.
	 */
	typedef boost::signal0<void>::slot_type slot_type;
	boost::signals::connection connect(slot_type const &) const;

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
