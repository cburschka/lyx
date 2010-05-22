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
	static type_info types[];
	///
	static int getType(std::string const & name);
	///
	static std::string const & getName(int type);


	InsetRef(Buffer * buffer, InsetCommandParams const &);

	///
	bool isLabeled() const { return true; }
	///
	docstring screenLabel() const { return screen_label_; }
	///
	docstring toolTip(BufferView const &, int, int) const
		{ return tooltip_; }
	///
	bool hasSettings() const { return true; }
	///
	InsetCode lyxCode() const { return REF_CODE; }
	///
	DisplayType display() const { return Inline; }
	///
	int latex(odocstream &, OutputParams const &) const;
	///
	int plaintext(odocstream &, OutputParams const &) const;
	///
	int docbook(odocstream &, OutputParams const &) const;
	///
	docstring xhtml(XHTMLStream &, OutputParams const &) const;
	/// the string that is passed to the TOC
	void tocString(odocstream &) const;
	///
	void validate(LaTeXFeatures & features) const;
	///
	static ParamInfo const & findInfo(std::string const &);
	///
	static std::string defaultCommand() { return "ref"; }
	///
	static bool isCompatibleCommand(std::string const & s);
	///
	void updateBuffer(ParIterator const & it, UpdateType);
	///
	void addToToc(DocIterator const &);
protected:
	///
	InsetRef(InsetRef const &);
private:
	///
	Inset * clone() const { return new InsetRef(*this); }
	///
	bool isLatex;
	/// Force inset into LTR environment if surroundings are RTL
	bool forceLTR() const { return true; }
	///
	mutable docstring screen_label_;
	///
	mutable docstring tooltip_;
};

} // namespace lyx

#endif
