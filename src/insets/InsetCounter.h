// -*- C++ -*-
/**
 * \file InsetCounter.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Richard Kimberly Heck
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_COUNTER_H
#define INSET_COUNTER_H

#include "InsetCommand.h"


namespace lyx {

///
class InsetCounter : public InsetCommand {
public:
	///
	InsetCounter(Buffer * buffer, InsetCommandParams const &);
	/// \name Public functions inherited from Inset class
	//@{
	///
	bool isLabeled() const { return true; }
	///
	docstring toolTip(BufferView const &, int, int) const
		{ return tooltip_; }
	///
	bool hasSettings() const { return true; }
	///
	InsetCode lyxCode() const { return COUNTER_CODE; }
	///
	void latex(otexstream &, OutputParams const &) const;
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const;
	///
	int docbook(odocstream &, OutputParams const &) const;
	///
	docstring xhtml(XHTMLStream &, OutputParams const &) const;
	///
	void toString(odocstream &) const;
	///
	void validate(LaTeXFeatures & features) const;
	///
	void updateBuffer(ParIterator const & it, UpdateType, bool const);
	///
	std::string contextMenuName() const;
	//@}

	/// \name Static public methods obligated for InsetCommand derived classes
	//@{
	///
	static ParamInfo const & findInfo(std::string const &);
	///
	static std::string defaultCommand() { return "set"; }
	///
	static bool isCompatibleCommand(std::string const & s);
	//@}
	/// keys are commands, values are GUI strings
	static const std::map<std::string, std::string> counterTable;
	static const std::map<std::string, std::string> valueTable;

protected:
	///
	InsetCounter(InsetCounter const &);

private:
	/// \name Private functions inherited from Inset class
	//@{
	///
	Inset * clone() const { return new InsetCounter(*this); }
	//@}

	/// \name Private functions inherited from InsetCommand class
	//@{
	///
	docstring screenLabel() const { return screen_label_; }
	//@}
	///
	docstring lyxSaveCounter() const;
	///
	void trackCounters(std::string const & cmd) const;
	///
	mutable docstring screen_label_;
	///
	mutable docstring tooltip_;
};


} // namespace lyx

#endif // INSET_REF_H
