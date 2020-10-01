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
	bool isLabeled() const override { return true; }
	///
	docstring toolTip(BufferView const &, int, int) const override
		{ return tooltip_; }
	///
	bool hasSettings() const override { return true; }
	///
	InsetCode lyxCode() const override { return COUNTER_CODE; }
	///
	void latex(otexstream &, OutputParams const &) const override;
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const override;
	///
	void docbook(XMLStream &, OutputParams const &) const override;
	///
	docstring xhtml(XMLStream &, OutputParams const &) const override;
	///
	void toString(odocstream &) const override;
	///
	void validate(LaTeXFeatures & features) const override;
	///
	void updateBuffer(ParIterator const & it, UpdateType, bool const) override;
	///
	std::string contextMenuName() const override;
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
	static const std::vector<std::pair<std::string, std::string>> counterTable;
	static const std::map<std::string, std::string> valueTable;

protected:
	///
	InsetCounter(InsetCounter const &);

private:
	/// \name Private functions inherited from Inset class
	//@{
	///
	Inset * clone() const override { return new InsetCounter(*this); }
	//@}

	/// \name Private functions inherited from InsetCommand class
	//@{
	///
	docstring screenLabel() const override { return screen_label_; }
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
