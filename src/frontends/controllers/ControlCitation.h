// -*- C++ -*-
/**
 * \file ControlCitation.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLCITATION_H
#define CONTROLCITATION_H

#include "Biblio.h"
#include "ControlCommand.h"

namespace lyx {
namespace frontend {

/** A controller for Citation dialogs.
 */
class ControlCitation : public ControlCommand {
public:
	///
	ControlCitation(Dialog &);
	virtual ~ControlCitation() {}
	virtual bool initialiseParams(std::string const & data);

	/// clean-up on hide.
	virtual void clearParams();

	/** Disconnect from the inset when the Apply button is pressed.
	 *  Allows easy insertion of multiple citations.
	 */
	virtual bool disconnectOnApply() const { return true; }

	/// \return the list of all available bibliography keys.
	std::vector<docstring> const availableKeys() const;
	/// \return the list of all used BibTeX fields
	std::vector<docstring> const availableFields() const;
	/// \return the list of all used BibTeX entry types
	std::vector<docstring> const availableEntries() const;
	///
	void filterByEntryType(
		std::vector<docstring> & keyVector, docstring entryType);
	///
	biblio::CiteEngine const getEngine() const;

	/// \return information for this key.
	docstring const getInfo(docstring const & key) const;

	/// Search a given string within the passed keys.
	/// \return the vector of matched keys.
	std::vector<docstring> searchKeys(
		std::vector<docstring> const & keys_to_search, //< Keys to search.
		bool only_keys, //< whether to search only the keys
		docstring const & search_expression, //< Search expression (regex possible)
		docstring field, //< field to search, empty for all fields
		bool case_sensitive = false, //< set to true is the search should be case sensitive
		bool regex = false //< \set to true if \c search_expression is a regex
		); //

	/// \return possible citations based on this key.
	std::vector<docstring> const getCiteStrings(docstring const & key) const;

	/// available CiteStyle-s (depends on availability of Natbib/Jurabib)
	static std::vector<biblio::CiteStyle> const & getCiteStyles() {
		return citeStyles_;
	}
private:
	/// The BibTeX information available to the dialog
	BiblioInfo bibkeysInfo_;

	///
	static std::vector<biblio::CiteStyle> citeStyles_;
};

} // namespace frontend
} // namespace lyx

#endif // CONTROLCITATION_H
