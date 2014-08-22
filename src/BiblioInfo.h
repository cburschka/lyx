// -*- C++ -*-
/**
 * \file BiblioInfo.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Herbert Voß
 * \author Richard Heck
 * \author Julien Rioux
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef BIBLIOINFO_H
#define BIBLIOINFO_H

#include "support/docstring.h"

#include "Citation.h"

#include <map>
#include <set>
#include <vector>


namespace lyx {

class Buffer;

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
	BibTeXInfo() : is_bibtex_(true), modifier_(0) {}
	/// argument sets isBibTeX_, so should be false only if it's coming
	/// from a bibliography environment
	BibTeXInfo(bool ib) : is_bibtex_(ib), modifier_(0) {}
	/// constructor that sets the entryType
	BibTeXInfo(docstring const & key, docstring const & type);
	/// \return the short form of an authorlist, used for sorting
	docstring const getAbbreviatedAuthor(bool jurabib_style = false) const;
	/// \return the short form of an authorlist, translated to the
	/// buffer language.
	docstring const getAbbreviatedAuthor(Buffer const & buf, bool jurabib_style = false) const;
	///
	docstring const getYear() const;
	///
	docstring const getXRef() const;
	/// \return formatted BibTeX data suitable for framing.
	/// \param pointer to crossref information
	docstring const & getInfo(BibTeXInfo const * const xref,
			Buffer const & buf, bool richtext) const;
	/// \return formatted BibTeX data for a citation label
	docstring const getLabel(BibTeXInfo const * const xref,
		Buffer const & buf, docstring const & format, bool richtext,
		const docstring & before, const docstring & after,
		const docstring & dialog, bool next = false) const;
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
	void label(docstring const & d) { label_= d; }
	///
	void key(docstring const & d) { bib_key_= d; }
	///
	docstring const & label() const { return label_; }
	///
	docstring const & key() const { return bib_key_; }
	/// numerical key for citing this entry. currently used only
	/// by XHTML output routines.
	docstring citeNumber() const { return cite_number_; }
	///
	void setCiteNumber(docstring const & num) { cite_number_ = num; }
	/// a,b,c, etc, for author-year. currently used only by XHTML
	/// output routines.
	char modifier() const { return modifier_; }
	///
	void setModifier(char c) { modifier_ = c; }
	///
	docstring entryType() const { return entry_type_; }
	///
	bool isBibTeX() const { return is_bibtex_; }
private:
	/// like operator[], except, if the field is empty, it will attempt
	/// to get the data from xref BibTeXInfo object, which would normally
	/// be the one referenced in the crossref field.
	docstring getValueForKey(std::string const & key, Buffer const & buf,
		docstring const & before, docstring const & after, docstring const & dialog,
		BibTeXInfo const * const xref, size_t maxsize = 4096) const;
	/// replace %keys% in a format string with their values
	/// called from getInfo()
	/// format strings may contain:
	///   %key%, which represents a key
	///   {%key%[[format]]}, which prints format if key is non-empty
	/// the latter may optionally contain an `else' clause as well:
	///   {%key%[[if format]][[else format]]}
	/// Material intended only for rich text (HTML) output should be
	/// wrapped in "{!" and "!}". These markers are to be postprocessed
	/// by processRichtext(); this step is left as a separate routine since
	/// expandFormat() is recursive while postprocessing should be done
	/// only once on the final string (just like convertLaTeXCommands).
	/// a simple macro facility is also available. keys that look like
	/// "%!key%" are substituted with their definition.
	/// moreover, keys that look like "%_key%" are treated as translatable
	/// so that things like "pp." and "vol." can be translated.
	docstring expandFormat(docstring const & fmt,
		BibTeXInfo const * const xref, int & counter,
		Buffer const & buf, docstring before = docstring(),
		docstring after = docstring(), docstring dialog = docstring(),
		bool next = false) const;
	/// true if from BibTeX; false if from bibliography environment
	bool is_bibtex_;
	/// the BibTeX key for this entry
	docstring bib_key_;
	/// the label that will appear in citations
	/// this is easily set from bibliography environments, but has
	/// to be calculated for entries we get from BibTeX
	docstring label_;
	/// a single string containing all BibTeX data associated with this key
	docstring all_data_;
	/// the BibTeX entry type (article, book, incollection, ...)
	docstring entry_type_;
	/// a cache for getInfo()
	mutable docstring info_;
	/// a cache for getInfo(richtext = true)
	mutable docstring info_richtext_;
	///
	docstring cite_number_;
	///
	char modifier_;
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
	docstring const getAbbreviatedAuthor(docstring const & key, Buffer const & buf) const;
	/// \return the year from the bibtex data record for \param key
	/// if \param use_modifier is true, then we will also append any
	/// modifier for this entry (e.g., 1998b).
	/// Note that this will get the year from the crossref if it's
	/// not present in the record itself.
	docstring const getYear(docstring const & key,
			bool use_modifier = false) const;
	/// \return the year from the bibtex data record for \param key
	/// if \param use_modifier is true, then we will also append any
	/// modifier for this entry (e.g., 1998b).
	/// Note that this will get the year from the crossref if it's
	/// not present in the record itself.
	/// If no year is found, \return "No year" translated to the buffer
	/// language.
	docstring const getYear(docstring const & key, Buffer const & buf,
			bool use_modifier = false) const;
	///
	docstring const getCiteNumber(docstring const & key) const;
	/// \return formatted BibTeX data associated with a given key.
	/// Empty if no info exists.
	/// Note that this will retrieve data from the crossref as needed.
	/// If \param richtext is true, then it will output any richtext tags
	/// marked in the citation format and escape < and > elsewhere.
	docstring const getInfo(docstring const & key, Buffer const & buf,
			bool richtext = false) const;
	/// \return formatted BibTeX data for citation labels.
	/// Citation labels can have more than one key.
	docstring const getLabel(std::vector<docstring> keys,
		Buffer const & buf, std::string const & style, bool for_xhtml,
		size_t max_size, docstring const & before, docstring const & after,
		docstring const & dialog = docstring()) const;
	/// Is this a reference from a bibtex database
	/// or from a bibliography environment?
	bool isBibtex(docstring const & key) const;
	/// Translates the available citation styles into strings for a given
	/// list of keys, using either numerical or author-year style depending
	/// upon the active engine.
	std::vector<docstring> const getCiteStrings(std::vector<docstring> const & keys,
		std::vector<CitationStyle> const & styles, Buffer const & buf,
		docstring const & before, docstring const & after, docstring const & dialog,
	  size_t max_size) const;
	/// A list of BibTeX keys cited in the current document, sorted by
	/// the last name of the author.
	/// Make sure you have called collectCitedEntries() before you try to
	/// use this. You should probably call it just before you use this.
	std::vector<docstring> const & citedEntries() const
		{ return cited_entries_; }
	///
	void makeCitationLabels(Buffer const & buf);
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
	/// Collects the cited entries from buf.
	void collectCitedEntries(Buffer const & buf);
	///
	std::set<docstring> field_names_;
	///
	std::set<docstring> entry_types_;
	/// our map: keys --> BibTeXInfo
	std::map<docstring, BibTeXInfo> bimap_;
	/// a possibly sorted list of entries cited in our Buffer.
	/// do not try to make this a vector<BibTeXInfo *> or anything of
	/// the sort, because reloads will invalidate those pointers.
	std::vector<docstring> cited_entries_;
};

} // namespace lyx

#endif // BIBLIOINFO_H
