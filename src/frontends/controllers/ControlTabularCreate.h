// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 2001 The LyX Team.
 *
 *======================================================
 *
 * \file ControlTabularCreate.h
 */

#ifndef CONTROLTABULARCREATE_H
#define CONTROLTABULARCREATE_H

#include <utility>
 
#ifdef __GNUG__
#pragma interface
#endif

#include "ControlDialog_impl.h"

/** A controller for the TabularCreate Dialog.
 */
class ControlTabularCreate : public ControlDialogBD {
public:
	/// 
	ControlTabularCreate(LyXView &, Dialogs &);
 
	typedef std::pair<unsigned int, unsigned int> rowsCols;

	///
	rowsCols & params();

private:
	/// Apply from dialog
	virtual void apply();

	/// set the params before show or update
	virtual void setParams();
    
	/// rows, cols params
	rowsCols params_;
};

#endif // CONTROLTABULARCREATE_H
