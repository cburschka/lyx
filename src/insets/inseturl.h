// -*- C++ -*-
/**
 * \file inseturl.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author José Matos
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef INSET_URL_H
#define INSET_URL_H


#include "insetcommand.h"

struct LaTeXFeatures;

/** The url inset
 */
class InsetUrl : public InsetCommand {
public:
	///
	explicit
	InsetUrl(InsetCommandParams const &);
	///
	//explicit
	//InsetUrl(InsetCommandParams const &, bool same_id);
	///
	~InsetUrl();
	///
	virtual Inset * clone(Buffer const &) const {
		return new InsetUrl(params());
	}
	///
	//virtual Inset * clone(Buffer const &, bool same_id) const {
	//	return new InsetUrl(params(), same_id);
	//}
	///
	dispatch_result localDispatch(FuncRequest const & cmd);
	///
	Inset::Code lyxCode() const { return Inset::URL_CODE; }
	///
	void validate(LaTeXFeatures &) const;
	///
	string const getScreenLabel(Buffer const *) const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	bool display() const { return false; }
	///
	int latex(Buffer const *, std::ostream &,
		  LatexRunParams const &) const;
	///
	int ascii(Buffer const *, std::ostream &, int linelen) const;
	///
	int linuxdoc(Buffer const *, std::ostream &) const;
	///
	int docbook(Buffer const *, std::ostream &, bool mixcont) const;
};

#endif
