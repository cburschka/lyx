// -*- C++ -*-
/**
 * \file WorkArea.h
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author unknown
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef WORKAREA_H
#define WORKAREA_H

#ifdef __GNUG__
#pragma interface
#endif

#include "frontends/Painter.h"
#include "frontends/mouse_state.h"
#include "frontends/key_state.h"
#include "frontends/LyXKeySym.h"

#include <boost/signals/signal0.hpp>
#include <boost/signals/signal1.hpp>
#include <boost/signals/signal2.hpp>
#include <boost/signals/signal3.hpp>

#include <utility>

///
class WorkArea {
public:
	///
	WorkArea() {}
	///
	virtual ~WorkArea() {}
	///
	virtual Painter & getPainter() = 0;
	///
	virtual int workWidth() const = 0;
	///
	virtual int workHeight() const = 0;
  
	///
	virtual int xpos() const = 0;
	///
	virtual int ypos() const = 0;
	///
	virtual void resize(int xpos, int ypos, int width, int height) = 0;
	///
	virtual void redraw() const = 0;
  	///
	virtual void setFocus() const = 0;
	///
	virtual bool hasFocus() const = 0;
	///
	virtual bool visible() const = 0;
	///
	virtual void greyOut() const = 0;
        ///
        virtual void setScrollbarParams(int height, int pos, int line_height) = 0;
 
	/// a selection exists
	virtual void haveSelection(bool) const = 0;
	///
	virtual string const getClipboard() const = 0;
	///
	virtual void putClipboard(string const &) const = 0;
	// Signals
	///
	boost::signal0<void> workAreaExpose;
	///
	boost::signal1<void, int> scrollDocView;
	///
	boost::signal2<void, LyXKeySymPtr, key_modifier::state> workAreaKeyPress;
	///
	boost::signal3<void, int, int, mouse_button::state> workAreaButtonPress;
	///
	boost::signal3<void, int, int, mouse_button::state> workAreaButtonRelease;
	///
	boost::signal3<void, int, int, mouse_button::state> workAreaMotionNotify;
	///
	boost::signal0<void> workAreaFocus;
	///
	boost::signal0<void> workAreaUnfocus;
	///
	boost::signal3<void, int, int, mouse_button::state> workAreaDoubleClick;
	///
	boost::signal3<void, int, int, mouse_button::state> workAreaTripleClick;
	/// emitted when an X client has requested our selection
	boost::signal0<void> selectionRequested;
	/// emitted when another X client has stolen our selection
	boost::signal0<void> selectionLost;
};
 
#endif // WORKAREA_H
