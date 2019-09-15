// -*- C++ -*-
/**
 * \file MathCompletionList.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Stefan Schimanski
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_COMPLETIONLIST_H
#define MATH_COMPLETIONLIST_H

#include "CompletionList.h"

#include "support/docstring.h"

#include <vector>


namespace lyx {

class MathCompletionList : public CompletionList {
public:
	///
	explicit MathCompletionList(Cursor const & cur);
	///
	virtual ~MathCompletionList();

	///
	virtual bool sorted() const { return false; }
	///
	virtual size_t size() const;
	///
	virtual docstring const & data(size_t idx) const;
	///
	virtual std::string icon(size_t idx) const;

	///
	static void addToFavorites(docstring const & completion);

private:
	///
	static std::vector<docstring> globals;
	///
	std::vector<docstring> locals;
};

} // namespace lyx

#endif // MATH_COMPLETIONLIST_H
