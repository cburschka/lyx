// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file ControlCitation.h
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#ifndef CONTROLCITATION_H
#define CONTROLCITATION_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlCommand.h"
#include "biblio.h" // biblio::InfoMap

/** A controller for Citation dialogs.
 */
class ControlCitation : public ControlCommand
{
public:
	///
	ControlCitation(LyXView &, Dialogs &);

	/// Returns a reference to the map of stored keys
	biblio::InfoMap const & bibkeysInfo() const;

private:
	/// create the InfoMap of keys and data
	virtual void setDaughterParams();
	/// 
	virtual void clearDaughterParams();

	/// The info associated with each key
	biblio::InfoMap bibkeysInfo_;
};

#endif // CONTROLCITATION_H
