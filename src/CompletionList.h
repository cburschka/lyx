// -*- C++ -*-
/**
 * \file CompletionList.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Stefan Schimanski
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef COMPLETIONLIST_H
#define COMPLETIONLIST_H

#include "support/docstring.h"

namespace lyx {

class CompletionList {
public:
	///
	virtual ~CompletionList() {}
	///
	virtual bool sorted() const = 0;
	///
	virtual size_t size() const = 0;
	/// returns the string shown in the gui.
	virtual lyx::docstring const & data(size_t idx) const = 0;
	/// returns the resource string used to load an icon.
	virtual std::string icon(size_t /*idx*/) const { return std::string(); }
};

} // namespace lyx

#endif

