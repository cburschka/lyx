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
 * \file ControlToc.h
 * \author Angus Leeming <leeming@lyx.org>
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
