// -*- C++ -*-
/**
 * \file ControlToc.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef CONTROLTOC_H
#define CONTROLTOC_H

#ifdef __GNUG__
#pragma interface
#endif

#include <vector>
#include "ControlCommand.h"
#include "toc.h"

/** A controller for TOC dialogs.
 */
class ControlToc : public ControlCommand
{
public:
	///
	ControlToc(LyXView &, Dialogs &);

	/// Goto this paragraph id
	void goTo(toc::TocItem const &) const;

	/// Return the list of types available
	std::vector<string> const getTypes() const;

	/// Given a type, returns the contents
	toc::Toc const getContents(string const & type) const;
};

#endif // CONTROLTOC_H
