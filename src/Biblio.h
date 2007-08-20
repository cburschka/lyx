// -*- C++ -*-
/**
 * \file Biblio.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Herbert Voﬂ
 * \author Richard Heck
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef BIBLIO_H
#define BIBLIO_H

#include "support/docstring.h"

#include <vector>
#include <map>
#include <set>


namespace lyx {
	
class Buffer;

namespace biblio {

enum CiteEngine {
	ENGINE_BASIC,
	ENGINE_NATBIB_AUTHORYEAR,
	ENGINE_NATBIB_NUMERICAL,
	ENGINE_JURABIB
};

enum CiteStyle {
	CITE,
	CITET,
	CITEP,
	CITEALT,
	CITEALP,
	CITEAUTHOR,
	CITEYEAR,
	CITEYEARPAR
};


class CitationStyle {
public:
	///
	CitationStyle(CiteStyle s = CITE, bool f = false, bool force = false)
		: style(s), full(f), forceUCase(force) {}
	/// \param latex_str a LaTeX command, "cite", "Citep*", etc
	CitationStyle(std::string const & latex_str);
	///
	std::string const asLatexStr() const;
	///
	CiteStyle style;
	///
	bool full;
	///
	bool forceUCase;
};

/// Returns a vector of available Citation styles.
std::vector<CiteStyle> const getCiteStyles(CiteEngine const );

} // namespace biblio

/// Class to represent information about a BibTeX or
/// bibliography entry.
/// The keys are BibTeX fields (e.g., author, title, etc), 
/// and the values are the associated field values.
class BibTeXInfo : public std::map<docstring, docstring> {
public:
	///
	BibTeXInfo();
	///Search for the given field and return the associated info.
	///The point of this is that BibTeXInfo::operator[] has no const
	///form.
	docstring const & getValueForField(docstring const & field) const;
	///
	docstring const & getValueForField(std::string const & field) const;
	///
	bool hasField(docstring const & field) const;
	/// return the short form of an authorlist
	docstring const getAbbreviatedAuthor() const;
	/// 
	docstring const getYear() const;
	/// Returns formatted BibTeX data suitable for framing.
	docstring const getInfo() const;
	/// the BibTeX key for this entry
	docstring bibKey;
	/// a single string containing all BibTeX data associated with this key
	docstring allData;
	/// the BibTeX entry type (article, book, incollection, ...)
	docstring entryType;
	/// true if from BibTeX; false if from bibliography environment
	bool isBibTeX;
};


/// Class to represent a collection of bibliographical data, whether
/// from BibTeX or from bibliography environments.
/// BiblioInfo.first is the bibliography key
/// BiblioInfo.second is the data for that key
class BiblioInfo : public std::map<docstring, BibTeXInfo> {
public:
	/// Returns a sorted vector of bibliography keys
	std::vector<docstring> const getKeys() const;
	/// Returns a sorted vector of present BibTeX fields
	std::vector<docstring> const getFields() const;
	/// Returns a sorted vector of BibTeX entry types in use
	std::vector<docstring> const getEntries() const;
	/// Fills keys with BibTeX information derived from the various insets
	/// in a given buffer, in its master document.
	void fillWithBibKeys(Buffer const * const buf);
	/// return the short form of an authorlist
	docstring const getAbbreviatedAuthor(docstring const & key) const;
	/// return the year from the bibtex data record
	docstring const getYear(docstring const & key) const;
	/// Returns formatted BibTeX data associated with a given key.
	/// Empty if no info exists. 
	docstring const getInfo(docstring const & key) const;
	
	/**
	  * "Translates the available Citation Styles into strings for a given key,
	  * either numerical or author-year depending upon the active engine. (See
	  * below for those methods.)
	  */
	std::vector<docstring> const
			getCiteStrings(docstring const & key, Buffer const & buf) const;
	/**
		* "Translates" the available Citation Styles into strings for a given key.
		* The returned string is displayed by the GUI.
		* [XX] is used in place of the actual reference
		* Eg, the vector will contain: [XX], Jones et al. [XX], ...
		* User supplies :
		*  the key,
		*  the buffer
		*/
	std::vector<docstring> const
			getNumericalStrings(docstring const & key, Buffer const & buf) const;
	/**
		* "Translates" the available Citation Styles into strings for a given key.
		* The returned string is displayed by the GUI.
		* Eg, the vector will contain:
		*  Jones et al. (1990), (Jones et al. 1990), Jones et al. 1990, ...
		* User supplies :
		*  the key,
		*  the buffer
		*/
	std::vector<docstring> const
			getAuthorYearStrings(docstring const & key, Buffer const & buf) const;

	std::set<docstring> fieldNames;
	std::set<docstring> entryTypes;
};

} // namespace lyx
#endif
