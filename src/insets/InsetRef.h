// -*- C++ -*-
/**
 * \file InsetRef.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author José Matos
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_REF_H
#define INSET_REF_H

#include "InsetCommand.h"


namespace lyx {

/// The reference inset
class InsetRef : public InsetCommand {
public:
	struct type_info {
		///
		std::string latex_name;
		///
		std::string gui_name;
		///
		std::string short_gui_name;
	};
	static const type_info types[];

	///
	InsetRef(Buffer * buffer, InsetCommandParams const &);

	///
	void changeTarget(docstring const & new_label);

	/// \name Public functions inherited from Inset class
	//@{
	///
	docstring layoutName() const;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd, FuncStatus & status) const;
	///
	bool isLabeled() const { return true; }
	///
	docstring toolTip(BufferView const &, int, int) const
		{ return tooltip_; }
	///
	docstring getTOCString() const;
	///
	bool hasSettings() const { return true; }
	///
	InsetCode lyxCode() const { return REF_CODE; }
	///
	void latex(otexstream &, OutputParams const &) const;
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const;
	///
	int docbook(odocstream &, OutputParams const &) const;
	///
	docstring xhtml(XMLStream &, OutputParams const &) const;
	///
	void toString(odocstream &) const;
	///
	void forOutliner(docstring &, size_t const, bool const) const;
	///
	void validate(LaTeXFeatures & features) const;
	///
	void updateBuffer(ParIterator const & it, UpdateType, bool const deleted = false);
	///
	void addToToc(DocIterator const & di, bool output_active,
				  UpdateType utype, TocBackend & backend) const;
	///
	bool forceLTR(OutputParams const &) const;
	//@}

	/// \name Static public methods obligated for InsetCommand derived classes
	//@{
	///
	static ParamInfo const & findInfo(std::string const &);
	///
	static std::string defaultCommand() { return "ref"; }
	///
	static bool isCompatibleCommand(std::string const & s);
	//@}
	///
	bool outputActive() const { return active_; }

protected:
	///
	InsetRef(InsetRef const &);

private:
	/// \name Private functions inherited from Inset class
	//@{
	///
	Inset * clone() const { return new InsetRef(*this); }
	//@}

	/// \name Private functions inherited from InsetCommand class
	//@{
	///
	docstring screenLabel() const;
	//@}

	/// \return the label with things that need to be escaped escaped
	docstring getEscapedLabel(OutputParams const &) const;
	/// \return the command for a formatted reference to ref
	/// \param label we're cross-referencing
	/// \param argument for reference command
	/// \param prefix of the label (before :)
	docstring getFormattedCmd(docstring const & ref, docstring & label,
			docstring & prefix, docstring const & caps) const;

	///
	mutable docstring screen_label_;
	///
	mutable bool broken_;
	///
	mutable bool active_;
	///
	mutable docstring tooltip_;
};


} // namespace lyx

#endif // INSET_REF_H
