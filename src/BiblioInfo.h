// -*- C++ -*-
/**
 * \file BiblioInfo.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Herbert Voß
 * \author Richard Heck
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef BIBLIOINFO_H
#define BIBLIOINFO_H

#include "support/docstring.h"

#include "Citation.h"

#include <vector>
#include <map>
#include <set>


namespace lyx {
	
class Buffer;

/// FIXME: To Citation.cpp?
/// Returns a vector of available Citation styles.
std::vector<CiteStyle> citeStyles(CiteEngine);
/// \param latex_str a LaTeX command, "cite", "Citep*", etc
CitationStyle citationStyleFromString(std::string const & latex_str);
/// the other way round
std::string citationStyleToString(CitationStyle const &);


/// Class to represent information about a BibTeX or
/// bibliography entry.
/// This class basically wraps a std::map, and many of its
/// methods simply delegate to the corresponding methods of
/// std::map.
class BibTeXInfo {
public:
	/// The keys are BibTeX fields (e.g., author, title, etc), 
	/// and the values are the associated field values.
	typedef std::map<docstring, docstring>::const_iterator const_iterator;
	///
	BibTeXInfo() : is_bibtex_(true) {}
	/// argument sets isBibTeX_, so should be false only if it's coming
	/// from a bibliography environment
	BibTeXInfo(bool ib) : is_bibtex_(ib) {}
	/// constructor that sets the entryType
	BibTeXInfo(docstring const & key, docstring const & type);
	///
	bool hasField(docstring const & field) const;
	/// \return the short form of an authorlist
	docstring const getAbbreviatedAuthor() const;
	/// 
	docstring const getYear() const;
	///
	docstring const getXRef() const;
	/// \return formatted BibTeX data suitable for framing.
	/// \param pointer to crossref information
	docstring const getInfo(BibTeXInfo const * const xref = 0) const;
	///
	int count(docstring const & f) const { return bimap_.count(f); }
	///
	const_iterator find(docstring const & f) const { return bimap_.find(f); }
	///
	const_iterator end() const { return bimap_.end(); }
	/// \return value for field f
	/// note that this will create an empty field if it does not exist
	docstring & operator[](docstring const & f) 
		{ return bimap_[f]; }
	/// \return value for field f
	/// this one, since it is const, will simply return docstring() if
	/// we don't have the field and will NOT create an empty field
	docstring const & operator[](docstring const & field) const;
	///
	docstring const & operator[](std::string const & field) const;
	///
	docstring const & allData() const { return all_data_; }
	///
	void setAllData(docstring const & d) { all_data_ = d; }
	///
	docstring entryType() const { return entry_type_; }
private:
	/// like operator[], except it will also check the given xref
	docstring getValueForKey(std::string const & key, 
			BibTeXInfo const * const xref = 0) const;
	/// true if from BibTeX; false if from bibliography environment
	bool is_bibtex_;
	/// the BibTeX key for this entry
	docstring bib_key_;
	/// a single string containing all BibTeX data associated with this key
	docstring all_data_;
	/// the BibTeX entry type (article, book, incollection, ...)
	docstring entry_type_;
	/// a cache for getInfo()
	mutable docstring info_;
	/// our map: <field, value>
	std::map <docstring, docstring> bimap_;
};


/// Class to represent a collection of bibliographical data, whether
/// from BibTeX or from bibliography environments.
class BiblioInfo {
public:
	/// bibliography key --> data for that key
	typedef std::map<docstring, BibTeXInfo>::const_iterator const_iterator;
	/// \return a sorted vector of bibliography keys
	std::vector<docstring> const getKeys() const;
	/// \return a sorted vector of present BibTeX fields
	std::vector<docstring> const getFields() const;
	/// \return a sorted vector of BibTeX entry types in use
	std::vector<docstring> const getEntries() const;
	/// \return the short form of an authorlist
	docstring const getAbbreviatedAuthor(docstring const & key) const;
	/// \return the year from the bibtex data record
	/// Note that this will get the year from the crossref if it's
	/// not present in the record itself
	docstring const getYear(docstring const & key) const;
	/// \return formatted BibTeX data associated with a given key.
	/// Empty if no info exists. 
	/// Note that this will retrieve data from the crossref as needed.
	docstring const getInfo(docstring const & key) const;
	/**
	  * "Translates" the available Citation Styles into strings for a given key,
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
	///
	const_iterator begin() const { return bimap_.begin(); }
	///
	void clear() { bimap_.clear(); }
	///
	bool empty() const { return bimap_.empty(); }
	///
	const_iterator end() const { return bimap_.end(); }
	///
	const_iterator find(docstring const & f) const { return bimap_.find(f); }
	///
	void mergeBiblioInfo(BiblioInfo const & info);
	///
	BibTeXInfo & operator[](docstring const & f) { return bimap_[f]; }
	///
	void addFieldName(docstring const & f) { field_names_.insert(f); }
	///
	void addEntryType(docstring const & f) { entry_types_.insert(f); }
private:
	///
	std::set<docstring> field_names_;
	///
	std::set<docstring> entry_types_;
	/// our map: keys --> BibTeXInfo
	std::map<docstring, BibTeXInfo> bimap_;
};

} // namespace lyx

#endif // BIBLIOINFO_H
