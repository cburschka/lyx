// -*- C++ -*-
/**
 * \file insetref.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author José Matos
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_REF_H
#define INSET_REF_H

#include "insetcommand.h"


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


	InsetRef(InsetCommandParams const &, Buffer const &);

	InsetRef(InsetRef const &);

	///
	virtual std::auto_ptr<InsetBase> clone() const {
		return std::auto_ptr<InsetBase>(new InsetRef(*this));
	}
	///
	std::string const getScreenLabel(Buffer const &) const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	InsetOld::Code lyxCode() const { return InsetOld::REF_CODE; }
	///
	bool display() const { return false; }
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
	///
	void validate(LaTeXFeatures & features) const;
protected:
	///
	void priv_dispatch(LCursor & cur, FuncRequest & cmd);
private:
	///
	bool isLatex;
};
#endif
