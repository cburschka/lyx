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
		  OutputParams const &) const;
	///
	int plaintext(Buffer const &, std::ostream &,
		  OutputParams const &) const;
	///
	int linuxdoc(Buffer const &, std::ostream &,
		     OutputParams const &) const;
	///
	int docbook(Buffer const &, std::ostream &,
		    OutputParams const &) const;
protected:
	///
	virtual
	DispatchResult
	priv_dispatch(FuncRequest const & cmd, idx_type &, pos_type &);
};

#endif
