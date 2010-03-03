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

class InsetLabel : public InsetCommand {
public:
	///
	InsetLabel(Buffer * buf, InsetCommandParams const &);

	/// verify label and update references.
	/**
	  * Overloaded from Inset::initView.
	  **/
	void initView();

	///
	bool isLabeled() const { return true; }

	///
	docstring screenLabel() const;
	///
	bool hasSettings() const { return true; }
	///
	InsetCode lyxCode() const { return LABEL_CODE; }
	///
	int plaintext(odocstream &, OutputParams const &) const;
	///
	int docbook(odocstream &, OutputParams const &) const;
	///
	docstring xhtml(XHTMLStream &, OutputParams const &) const;
	///
	static ParamInfo const & findInfo(std::string const &);
	///
	static std::string defaultCommand() { return "label"; }
	///
	static bool isCompatibleCommand(std::string const & s) 
		{ return s == "label"; }
	///
	void updateBuffer(ParIterator const & it, UpdateType);
	///
	void addToToc(DocIterator const &);
	///
	void updateCommand(docstring const & new_label, bool updaterefs = true);
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd, FuncStatus & status) const;
	///
	docstring const & activeCounter() const { return active_counter_; }
	///
	docstring const & counterValue() const { return counter_value_; }
	///
	docstring const & prettyCounter() const { return pretty_counter_; }
protected:
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
private:
	///
	Inset * clone() const { return new InsetLabel(*this); }
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
