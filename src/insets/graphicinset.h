// -*- C++ -*-
/**
 * \file graphicinset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef GRAPHICINSET_H
#define GRAPHICINSET_H

#include "dimension.h"

#include "graphics/GraphicsLoader.h"
#include "graphics/GraphicsParams.h"
#include "graphics/GraphicsTypes.h"

#include <boost/weak_ptr.hpp>
#include <boost/signals/signal0.hpp>


class BufferView;
class MetricsInfo;
class PainterInfo;


class GraphicInset
{
public:
	GraphicInset();
	GraphicInset(GraphicInset const &);

	/** Set the message that the inset will show when the
	 *  display of the graphic is deactivated.
	 *  The default is nothing, meaning that the inset will
	 *  show a message descibing the state of the image
	 *  loading process.
	 */
	void setNoDisplayMessage(string const & msg);
	
	/// Refresh the info about which file to display and how to display it.
	void update(grfx::Params const & params);
	/// File name, image size, rotation angle etc.
	grfx::Params const & params() const;

	/// compute the size of the object returned in dim
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	/// draw inset and update (xo, yo)-cache
	void draw(PainterInfo & pi, int x, int y) const;

	/// Is the stored checksum different to that of the graphics loader?
	bool hasFileChanged() const;
	/// Set the cached store.
	void view(BufferView *) const;
	/// An accessor function to the cached store.
	BufferView * view() const;

	/** Connect and you'll be informed when the loading status of the image
	 *  changes.
	 */
	typedef boost::signal0<void>::slot_type slot_type;
	boost::signals::connection connect(slot_type const &) const;

	/// The message to display instead of the graphic itself.
	string const statusMessage() const;

private:
	enum DisplayType {
		IMAGE,
		STATUS_MESSAGE,
		NODISPLAY_MESSAGE
	};

	/// Is the image ready to draw, or should we display a message instead?
	DisplayType displayType() const;
	
	/// The stored data.
	grfx::Loader loader_;
	grfx::Params params_;
	string nodisplay_message_;

	/// These are all cached variables.
	mutable unsigned long checksum_;
	mutable boost::weak_ptr<BufferView> view_;
	mutable Dimension dim_;
};


#endif // NOT GRAPHICINSET_H
