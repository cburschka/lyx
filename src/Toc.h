// -*- C++ -*-
/**
 * \file TocBackend.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 * \author Abdelrazak Younes
 * \author Guillaume Munch
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef TOC_H
#define TOC_H

#include "support/shared_ptr.h"

#include <map>
#include <vector>
#include <string>


namespace lyx {

// TocItem is defined in TocBackend.h
class TocItem;

typedef std::vector<TocItem> Toc;

class TocList : public std::map<std::string, shared_ptr<Toc> >
{
private:
	// TocList should never map to null pointers.
	// We forbid the following method which creates null pointers.
	using std::map<std::string, shared_ptr<Toc> >::operator[];
};


} // namespace lyx

#endif // TOC_H
