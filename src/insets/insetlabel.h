// -*- C++ -*-
/**
 * \file insetlabel.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef INSET_LABEL_H
#define INSET_LABEL_H


#include "insetcommand.h"

class InsetLabel : public InsetCommand {
public:
	///
	InsetLabel(InsetCommandParams const &, bool same_id = false);
	///
	~InsetLabel();
	///
	virtual Inset * clone(Buffer const &, bool same_id = false) const {
		return new InsetLabel(params(), same_id);
	}
	///
	virtual dispatch_result localDispatch(FuncRequest const & cmd);
	///
	string const getScreenLabel(Buffer const *) const { return getContents(); }
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	Inset::Code lyxCode() const { return Inset::LABEL_CODE; }
	///
	void edit(BufferView *, int, int, mouse_button::state);
	///
	void edit(BufferView * bv, bool front = true);
	///
	std::vector<string> const getLabelList() const;
	///
	int latex(Buffer const *, std::ostream &,
		  bool fragile, bool free_spc) const;
	///
	int ascii(Buffer const *, std::ostream &, int linelen) const;
	///
	int linuxdoc(Buffer const *, std::ostream &) const;
	///
	int docbook(Buffer const *, std::ostream &, bool mixcont) const;
};

#endif
