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
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#ifndef CONTROLTOC_H
#define CONTROLTOC_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlCommand.h"
#include "buffer.h" // Buffer::SingleList

/** A controller for TOC dialogs.
 */
class ControlToc : public ControlCommand
{
public:
	///
	ControlToc(LyXView &, Dialogs &);

	/// Goto this paragraph id
	void Goto(int const & id) const;

	/// Returns a vector of list types in the document
	std::vector<string> const getTypes() const;

	/// Given a type, returns the contents
	Buffer::SingleList const getContents(string const & type) const;
};

namespace toc
{
    /** Given the cmdName of the TOC param, returns the type used
	by ControlToc::getContents() */
	string const getType(string const & cmdName);

} // namespace toc

#endif // CONTROLTOC_H
