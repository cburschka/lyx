// -*- C++ -*-
/**
 * \file ControlPrefs.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef CONTROLPREFS_H
#define CONTROLPREFS_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlDialog_impl.h"
#include "LString.h"
#include "funcrequest.h"
#include "lyxrc.h"

 
class ControlPrefs : public ControlDialogBI {
public:
	ControlPrefs(LyXView &, Dialogs &);

	// FIXME: we should probably devolve the individual
	// settings to methods here. But for now, this will
	// do
 
	LyXRC & rc() { return rc_; } 

	LyXRC const & rc() const { return rc_; }
 
	/// save the rc
	void save();
 
private:
	/// apply current lyxrc
	virtual void apply();
 
	/// temporary lyxrc
	LyXRC rc_;
};

#endif // CONTROLPREFS_H
