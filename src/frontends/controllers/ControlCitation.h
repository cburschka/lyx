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

	///
	bool usingNatbib() const;
	/// Possible citations based on this key
	std::vector<string> const getCiteStrings(string const & key) const;

	/// available CiteStyle-s (depends on availability of Natbib
	static std::vector<biblio::CiteStyle> const & getCiteStyles()
		{ return citeStyles_; }

private:
	/// create the InfoMap of keys and data
	virtual void setDaughterParams();
	/// 
	virtual void clearDaughterParams();

	/** disconnect from the inset when the Apply button is pressed.
	 Allows easy insertion of multiple citations. */
	virtual bool disconnectOnApply() { return true; }

	/// The info associated with each key
	biblio::InfoMap bibkeysInfo_;

	///
	static std::vector<biblio::CiteStyle> citeStyles_;
};


#endif // CONTROLCITATION_H
