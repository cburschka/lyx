// -*- C++ -*-
/**
 * \file insetlabel.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_LABEL_H
#define INSET_LABEL_H

#include "insetcommand.h"

class InsetLabel : public InsetCommand {
public:
	///
	InsetLabel(InsetCommandParams const &);
	///
	~InsetLabel();
	///
	std::auto_ptr<InsetBase> clone() const;
	///
	std::string const getScreenLabel(Buffer const &) const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	InsetOld::Code lyxCode() const { return InsetOld::LABEL_CODE; }
	/// Appends \c list with this label
	void getLabelList(Buffer const &, std::vector<std::string> & list) const;
	///
	int latex(Buffer const &, std::ostream &,
		  LatexRunParams const &) const;
	///
	int ascii(Buffer const &, std::ostream &, int linelen) const;
	///
	int linuxdoc(Buffer const &, std::ostream &) const;
	///
	int docbook(Buffer const &, std::ostream &, bool mixcont) const;
protected:
	///
	virtual
	DispatchResult
	priv_dispatch(FuncRequest const & cmd, idx_type &, pos_type &);
};

#endif
