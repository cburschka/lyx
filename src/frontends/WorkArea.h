// -*- C++ -*-
/**
 * \file WorkArea.h
 * Read the file COPYING
 *
 * \author unknown
 * \author John Levon 
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef WORKAREA_H
#define WORKAREA_H

#include "frontends/key_state.h"
#include "frontends/LyXKeySym.h"
#include "funcrequest.h"

#include <boost/signals/signal0.hpp>
#include <boost/signals/signal1.hpp>
#include <boost/signals/signal2.hpp>
#include <boost/signals/signal3.hpp>

#include <utility>

class Painter;

/**
 * The work area class represents the widget that provides the
 * view onto a document. It is owned by the BufferView, and
 * is responsible for handing events back to its owning BufferView.
 * It works in concert with the LyXScreen class to update the
 * widget view of a document.
 */
class WorkArea {
public:

	WorkArea() {}

	virtual ~WorkArea() {}

	/// return the painter object for this work area
	virtual Painter & getPainter() = 0;

	/// return the width of the work area in pixels
	virtual int workWidth() const = 0;
	/// return the height of the work area in pixels
	virtual int workHeight() const = 0;

	/**
	 * Update the scrollbar.
	 * @param height the total document height in pixels
	 * @param pos the current position in the document, in pixels
	 * @param line_height the line-scroll amount, in pixels
	 */
	virtual void setScrollbarParams(int height, int pos, int line_height) = 0;

	// FIXME: this is an odd place to have it, but xforms needs it here ...
	/// a selection exists
	virtual void haveSelection(bool) const = 0;
	/// get the X clipboard contents
	virtual string const getClipboard() const = 0;
	/// fill the clipboard
	virtual void putClipboard(string const &) const = 0;

	/// work area dimensions have changed
	boost::signal0<void> workAreaResize;
	/// the scrollbar has changed
	boost::signal1<void, int> scrollDocView;
	/// a key combination has been pressed
	boost::signal2<void, LyXKeySymPtr, key_modifier::state> workAreaKeyPress;
	/// some mouse event
	boost::signal1<void, FuncRequest> dispatch;
	/// emitted when an X client has requested our selection
	boost::signal0<void> selectionRequested;
	/// emitted when another X client has stolen our selection
	boost::signal0<void> selectionLost;
};

#endif // WORKAREA_H
