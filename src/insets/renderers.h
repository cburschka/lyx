// -*- C++ -*-
/**
 * \file renderers.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef RENDERERS_H
#define RENDERERS_H

#include "box.h"
#include "dimension.h"

#include "graphics/GraphicsLoader.h"
#include "graphics/GraphicsParams.h"

#include <boost/weak_ptr.hpp>
#include <boost/signals/signal0.hpp>


class BufferView;
class MetricsInfo;
class PainterInfo;


class RenderInset
{
public:
	virtual ~RenderInset();

	virtual RenderInset * clone() const = 0;

	/// compute the size of the object returned in dim
	virtual void metrics(MetricsInfo & mi, Dimension & dim) const = 0;
	/// draw inset and update (xo, yo)-cache
	virtual void draw(PainterInfo & pi, int x, int y) const = 0;

	/// An accessor function to the cached store.
	BufferView * view() const;

protected:
	RenderInset();
	RenderInset(RenderInset const &);
	RenderInset & operator=(RenderInset const &);

	/// These are cached variables (are not copied).
	mutable boost::weak_ptr<BufferView> view_;
	mutable Dimension dim_;
};


class ButtonRenderer : public RenderInset
{
public:
	ButtonRenderer();

	virtual RenderInset * clone() const;

	/// This should provide the text for the button
	void update(string const &, bool editable);

	/// compute the size of the object returned in dim
	virtual void metrics(MetricsInfo & mi, Dimension & dim) const;
	/// draw inset and update (xo, yo)-cache
	virtual void draw(PainterInfo & pi, int x, int y) const;

	/// The "sensitive area" box, i.e., the button area
	Box box() const { return button_box_; }
	///
	void setBox(Box b) { button_box_ = b; }
	
private:
	/// The stored data.
	string text_;
	bool editable_;
	Box button_box_;
};


class GraphicRenderer : public RenderInset
{
public:
	GraphicRenderer();
	GraphicRenderer(GraphicRenderer const &);

	virtual RenderInset * clone() const;

	/// Refresh the info about which file to display and how to display it.
	void update(lyx::graphics::Params const & params);

	/// compute the size of the object returned in dim
	virtual void metrics(MetricsInfo & mi, Dimension & dim) const;
	/// draw inset and update (xo, yo)-cache
	virtual void draw(PainterInfo & pi, int x, int y) const;

	/// Is the stored checksum different to that of the graphics loader?
	bool hasFileChanged() const;

	/** Connect and you'll be informed when the loading status of the image
	 *  changes.
	 */
	typedef boost::signal0<void>::slot_type slot_type;
	virtual boost::signals::connection connect(slot_type const &) const;

private:
	/// Not implemented.
	GraphicRenderer & operator=(GraphicRenderer const &);

	/// The message to display instead of the graphic itself.
	string const statusMessage() const;

	/// Is the image ready to draw, or should we display a message instead?
	bool readyToDisplay() const;

	/// The stored data.
	lyx::graphics::Loader loader_;
	lyx::graphics::Params params_;

	/// Cached variable (not copied).
	mutable unsigned long checksum_;
};


#endif // NOT RENDERERS_H
