// -*- C++ -*-
/**
 * \file QMiniBuffer.h
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author Lars
 * \author Asger and Juergen
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QMINIBUFFER_H
#define QMINIBUFFER_H

#include "LString.h"

#include "frontends/MiniBuffer.h"

#ifdef __GNUG__
#pragma interface
#endif

/// FIXME: place holder only 
class QMiniBuffer : public MiniBuffer {
public:
	///
	QMiniBuffer(LyXView *);
	
	/// destructor
	virtual ~QMiniBuffer();
	
protected:
	/// Are we in editing mode?
	virtual bool isEditingMode() const;
	/// enter editing mode
	virtual void editingMode();
	/// enter message display mode
	virtual void messageMode();
	
	/// set the minibuffer content in editing mode
	virtual void set_input(string const &);
	
};
 
#endif // QMINIBUFFER_H
