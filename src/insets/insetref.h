// -*- C++ -*-
/**
 * \file insetref.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author José Matos
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef INSET_REF_H
#define INSET_REF_H


#include "insetcommand.h"

struct LaTeXFeatures;

/** The reference inset
 */
class InsetRef : public InsetCommand {
public:
	struct type_info {
		///
		string latex_name;
		///
		string gui_name;
		///
		string short_gui_name;
	};
	static type_info types[];
	///
	static int getType(string const & name);
	///
	static string const & getName(int type);


	InsetRef(InsetCommandParams const &, Buffer const &);

	InsetRef(InsetRef const &);

	~InsetRef();
	///
	virtual std::auto_ptr<InsetBase> clone() const {
		return std::auto_ptr<InsetBase>(new InsetRef(*this));
	}
	///
	dispatch_result localDispatch(FuncRequest const & cmd);
	///
	string const getScreenLabel(Buffer const *) const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	InsetOld::Code lyxCode() const { return InsetOld::REF_CODE; }
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
	///
	void validate(LaTeXFeatures & features) const;
private:
	///
	bool isLatex;
};
#endif
