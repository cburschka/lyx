// -*- C++ -*-
/**
 * \file ControlToc.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLTOC_H
#define CONTROLTOC_H


#include "ControlCommand.h"
#include "toc.h"
#include <vector>

namespace lyx {
namespace frontend {

/** A controller for TOC dialogs.
 */
class ControlToc : public ControlCommand {
public:
	///
	ControlToc(Dialog &);

	/// Goto this paragraph id
	void goTo(toc::TocItem const &);

	/// Return the list of types available
	std::vector<std::string> const getTypes() const;

	/// Return the guiname from a given cmdName of the TOC param
	std::string const getGuiName() const;

	/// Given a type, returns the contents
	toc::Toc const getContents(std::string const & type) const;
};

} // namespace frontend
} // namespace lyx

#endif // CONTROLTOC_H
