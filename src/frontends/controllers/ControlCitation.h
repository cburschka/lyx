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
	std::vector<std::string> const availableKeys() const;
	///
	biblio::CiteEngine const getEngine() const;

	/// \return information for this key.
	docstring const getInfo(std::string const & key) const;

	/// Search a given string within the passed keys.
	/// \return the vector of matched keys.
	std::vector<std::string> searchKeys(
		std::vector<std::string> const & keys_to_search, //< Keys to search.
		docstring const & search_expression, //< Search expression (regex possible)
		bool case_sensitive = false, // set to true is the search should be case sensitive
		bool regex = false /// \set to true if \c search_expression is a regex
		); //

	/// \return possible citations based on this key.
	std::vector<docstring> const getCiteStrings(std::string const & key) const;

	/// available CiteStyle-s (depends on availability of Natbib/Jurabib)
	static std::vector<biblio::CiteStyle> const & getCiteStyles() {
		return citeStyles_;
	}
private:
	/// The info associated with each key
	biblio::InfoMap bibkeysInfo_;

	///
	static std::vector<biblio::CiteStyle> citeStyles_;
};

} // namespace frontend
} // namespace lyx

#endif // CONTROLCITATION_H
