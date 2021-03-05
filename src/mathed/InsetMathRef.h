// -*- C++ -*-
/**
 * \file InsetMathRef.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef REF_INSET_H
#define REF_INSET_H


#include "InsetMathCommand.h"


namespace lyx {
class Buffer;

// for \ref
class InsetMathRef : public InsetMathCommand {
public:
	///
	explicit InsetMathRef(Buffer * buf);
	///
	explicit InsetMathRef(Buffer * buf, docstring const & data);
	///
	void updateBuffer(ParIterator const &, UpdateType, bool const deleted = false) override;
	///
	void write(TeXMathStream & os) const override;
	///
	void infoize(odocstream & os) const override;
	///
	bool hasSettings() const override { return true; }
	///
	bool clickable(BufferView const &, int, int) const override { return true; }
	///
	std::string contextMenuName() const override { return "context-ref"; }
	///
	mode_type currentMode() const override { return TEXT_MODE; }
	///
	bool lockedMode() const override { return true; }
	///
	bool asciiOnly() const override { return true; }
	///
	docstring const screenLabel() const override;
	///
	void validate(LaTeXFeatures & features) const override;
	///
	void changeTarget(docstring const & target);
	///
	InsetMathRef * asRefInset() override { return this; }

	/// docbook output
	void docbook(XMLStream &, OutputParams const &) const override;
	/// generate something that will be understood by the Dialogs.
	std::string const createDialogStr() const;

	struct ref_type_info {
		///
		docstring latex_name;
		///
		docstring gui_name;
		///
		docstring short_gui_name;
	};
	static ref_type_info types[];
	///
	docstring const getTarget() const;
	///
	InsetCode lyxCode() const override { return MATH_REF_CODE; }

protected:
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd) override;
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const override;
private:
	///
	Inset * clone() const override;
};



} // namespace lyx
#endif
