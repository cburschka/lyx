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

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlDialogs.h"

/** A controller for the TabularCreate Dialog.
 */
class ControlTabularCreate : public ControlDialog<ControlConnectBD> {
public:
	/// 
	ControlTabularCreate(LyXView &, Dialogs &);
	///
	string & params() const;

private:
	/// Apply from dialog
	virtual void apply();

	/// set the params before show or update
	virtual void setParams();
	/// clean-up on hide.
	virtual void clearParams();
    
	///
	string * params_;
};

#endif // CONTROLTABULARCREATE_H
