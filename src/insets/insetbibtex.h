// -*- C++ -*-
/**
 * \file insetbibtex.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef INSET_BIBTEX_H
#define INSET_BIBTEX_H


#include <vector>
#include "insetcommand.h"

class Buffer;

/** Used to insert BibTeX's information
  */
class InsetBibtex : public InsetCommand {
public:
	///
	InsetBibtex(InsetCommandParams const &, bool same_id = false);
	///
	~InsetBibtex();
	///
	Inset * clone(Buffer const &, bool same_id = false) const {
		return new InsetBibtex(params(), same_id);
	}
	/// small wrapper for the time being
	virtual dispatch_result localDispatch(FuncRequest const & cmd);
	///
	string const getScreenLabel(Buffer const *) const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	Inset::Code lyxCode() const { return Inset::BIBTEX_CODE; }
	///
	void edit(BufferView *, int x, int y, mouse_button::state button);
	///
	void edit(BufferView * bv, bool front = true);
	///
	int latex(Buffer const *, std::ostream &,
		  bool fragile, bool freespace) const;
	///
	void fillWithBibKeys(Buffer const *,
		std::vector<std::pair<string,string> > &) const;
	///
	std::vector<string> const getFiles(Buffer const &) const;
	///
	bool addDatabase(string const &);
	///
	bool delDatabase(string const &);
	///
	bool display() const { return true; }
	///
	struct Holder {
		InsetBibtex * inset;
		BufferView * view;
	};

private:
	///
	Holder holder;
};

#endif // INSET_BIBTEX_H
