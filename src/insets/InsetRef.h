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
	static int getType(std::string const & name);
	///
	static std::string const & getName(int type);

	///
	InsetRef(Buffer * buffer, InsetCommandParams const &);

	/// \name Public functions inherited from Inset class
	//@{
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
	DisplayType display() const { return Inline; }
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
	void forOutliner(docstring &, size_t) const;
	///
	void validate(LaTeXFeatures & features) const;
	///
	void updateBuffer(ParIterator const & it, UpdateType);
	///
	void addToToc(DocIterator const & di, bool output_active) const;
	///
	bool forceLTR() const { return true; }
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

	//FIXME: private
	/// \name Private functions inherited from InsetCommand class
	//@{
	///
	docstring screenLabel() const { return screen_label_; }
	//@}

protected:
	///
	InsetRef(InsetRef const &);

private:
	/// \name Private functions inherited from Inset class
	//@{
	///
	Inset * clone() const { return new InsetRef(*this); }
	//@}
	
	/// \return the label with things that need to be escaped escaped
	docstring getEscapedLabel(OutputParams const &) const;
	/// \return the command for a formatted reference to ref
	/// \param label we're cross-referencing
	/// \param argument for reference command
	/// \param prefix of the label (before :)
	docstring getFormattedCmd(docstring const & ref, docstring & label,
			docstring & prefix) const;

	///
	mutable docstring screen_label_;
	///
	mutable docstring tooltip_;
};


} // namespace lyx

#endif // INSET_REF_H
