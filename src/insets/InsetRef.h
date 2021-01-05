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
	docstring layoutName() const override;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd) override;
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd, FuncStatus & status) const override;
	///
	bool isLabeled() const override { return true; }
	///
	docstring toolTip(BufferView const &, int, int) const override
		{ return tooltip_; }
	///
	docstring getTOCString() const;
	///
	bool hasSettings() const override { return true; }
	///
	InsetCode lyxCode() const override { return REF_CODE; }
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
	void forOutliner(docstring &, size_t const, bool const) const override;
	///
	void validate(LaTeXFeatures & features) const override;
	///
	void updateBuffer(ParIterator const & it, UpdateType, bool const deleted = false) override;
	///
	void addToToc(DocIterator const & di, bool output_active,
				  UpdateType utype, TocBackend & backend) const override;
	///
	bool forceLTR(OutputParams const &) const override;
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
	/// \return the command for a formatted reference to ref
	/// \param label we're cross-referencing
	/// \param argument for reference command
	/// \param prefix of the label (before :)
	/// Also used by InsetMathRef
	static docstring getFormattedCmd(docstring const & ref, docstring & label,
			docstring & prefix, bool use_refstyle, bool use_caps = false);

protected:
	///
	InsetRef(InsetRef const &);

private:
	/// \name Private functions inherited from Inset class
	//@{
	///
	Inset * clone() const override { return new InsetRef(*this); }
	//@}

	/// \name Private functions inherited from InsetCommand class
	//@{
	///
	docstring screenLabel() const override;
	//@}

	/// \return the label with things that need to be escaped escaped
	docstring getEscapedLabel(OutputParams const &) const;

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
