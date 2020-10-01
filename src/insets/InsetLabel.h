// -*- C++ -*-
/**
 * \file InsetLabel.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_LABEL_H
#define INSET_LABEL_H

#include "InsetCommand.h"


namespace lyx {

class Counter;

class InsetLabel : public InsetCommand
{
public:
	///
	InsetLabel(Buffer * buf, InsetCommandParams const &);

	///
	docstring const & activeCounter() const { return active_counter_; }
	///
	docstring const & counterValue() const { return counter_value_; }
	///
	docstring const & prettyCounter() const { return pretty_counter_; }
	/// Updates only the label string, doesn't handle undo nor references.
	void updateLabel(docstring const & new_label, bool const active = true);
	/// Updates the label and the references to it.
	/// Will also handle undo/redo if \p cursor is passed.
	void updateLabelAndRefs(docstring const & new_label, Cursor * cursor = 0);

	/// \name Public functions inherited from Inset class
	//@{
	/// verify label and update references.
	void initView() override;
	///
	bool isLabeled() const override { return true; }
	///
	bool hasSettings() const override { return true; }
	///
	InsetCode lyxCode() const override { return LABEL_CODE; }
	///
	void latex(otexstream & os, OutputParams const & runparams_in) const override;
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const override;
	///
	void docbook(XMLStream &, OutputParams const &) const override;
	///
	docstring xhtml(XMLStream &, OutputParams const &) const override;
	///
	void updateBuffer(ParIterator const & it, UpdateType, bool const deleted = false) override;
	///
	void addToToc(DocIterator const & di, bool output_active,
				  UpdateType utype, TocBackend & backend) const override;
	/// Is the content of this inset part of the immediate (visible) text sequence?
	bool isPartOfTextSequence() const override { return false; }
	//@}

	/// \name Static public methods obligated for InsetCommand derived classes
	//@{
	///
	static ParamInfo const & findInfo(std::string const &);
	///
	static std::string defaultCommand() { return "label"; }
	///
	static bool isCompatibleCommand(std::string const & s)
		{ return s == "label"; }
	//@}

	//FIXME: private
	/// \name Private functions inherited from InsetCommand class
	//@{
	///
	docstring screenLabel() const override;
	//@}

private:
	/// \name Private functions inherited from Inset class
	//@{
	///
	Inset * clone() const override { return new InsetLabel(*this); }
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd, FuncStatus & status) const override;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd) override;
	//@}

	///
	void uniqueLabel(docstring & label) const;
	///
	void updateReferences(docstring const & old_label,
		docstring const & new_label);
	///
	docstring screen_label_;
	///
	docstring active_counter_;
	///
	docstring counter_value_;
	///
	docstring pretty_counter_;
};


} // namespace lyx

#endif
