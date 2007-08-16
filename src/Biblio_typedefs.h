// -*- C++ -*-
/**
 * \file Biblio_typedef.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Richard Heck
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef BIBLIO_TYPEDEFS_H
#define BIBLIO_TYPEDEFS_H

#include "support/docstring.h"
#include <map>

namespace lyx {
namespace biblio {

/// Class to represent information about a BibTeX or
/// bibliography entry.
/// The keys are BibTeX fields, and the values are the
/// associated field values.
/// \param isBibTex false if this is from an InsetBibitem
/// \param allData the entire BibTeX entry, more or less
/// \param entryType the BibTeX entry type
class BibTeXInfo : public std::map<docstring, docstring> {
	public:
		BibTeXInfo();
		BibTeXInfo(bool isBibTeX);
		bool hasKey(docstring const & key);
		bool isBibTeX;
		docstring allData;
		docstring entryType;
};

/*
class BibKeyList : public std::set<std::string, BibTeXInfo> {
 public:
 	std::set<string> keys;
}

*/

/// First entry is the bibliography key, second the data
typedef std::map<std::string, BibTeXInfo> BibKeyList;
	
}
}
#endif
