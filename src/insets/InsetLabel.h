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

class InsetLabel : public InsetCommand {
public:
	///
	InsetLabel(InsetCommandParams const &);
	///
	docstring const getScreenLabel(Buffer const &) const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	Inset::Code lyxCode() const { return Inset::LABEL_CODE; }
	/// Appends \c list with this label
	void getLabelList(Buffer const &, std::vector<docstring> & list) const;
	///
	int latex(Buffer const &, odocstream &, OutputParams const &) const;
	///
	int plaintext(Buffer const &, odocstream &, OutputParams const &) const;
	///
	int docbook(Buffer const &, odocstream &, OutputParams const &) const;
protected:
	virtual void doDispatch(Cursor & cur, FuncRequest & cmd);
private:
	virtual Inset * clone() const;
};


} // namespace lyx

#endif
