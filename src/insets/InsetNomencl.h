// -*- C++ -*-
/**
 * \file InsetNomencl.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author O. U. Baran
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_NOMENCL_H
#define INSET_NOMENCL_H


#include "InsetCommand.h"


namespace lyx {

class LaTeXFeatures;

/** Used to insert glossary labels
  */
class InsetNomencl : public InsetCommand {
public:
	///
	InsetNomencl(InsetCommandParams const &);
	///
	docstring const getScreenLabel(Buffer const &) const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	/// Updates needed features for this inset.
	void validate(LaTeXFeatures & features) const;
	///
	Inset::Code lyxCode() const;
	///
	int docbook(Buffer const &, odocstream &,
		    OutputParams const &) const;
	///
	int docbookGlossary(odocstream &) const;
private:
	virtual std::auto_ptr<Inset> doClone() const {
		return std::auto_ptr<Inset>(new InsetNomencl(params()));
	}
	/// unique id for this glossary entry for docbook export
	docstring glossary_entry_id;
};


class InsetPrintNomencl : public InsetCommand {
public:
	///
	InsetPrintNomencl(InsetCommandParams const &);
	/// Updates needed features for this inset.
	void validate(LaTeXFeatures & features) const;
	// FIXME: This should be editable to set the label width (stored
	// in params_["labelwidth"]).
	// Currently the width can be read from file and written, but not
	// changed.
	///
	EDITABLE editable() const { return NOT_EDITABLE; }
	///
	int docbook(Buffer const &, odocstream &,
		    OutputParams const &) const;
	///
	Inset::Code lyxCode() const;
	///
	bool display() const { return true; }
	///
	docstring const getScreenLabel(Buffer const &) const;
private:
	virtual std::auto_ptr<Inset> doClone() const {
		return std::auto_ptr<Inset>(new InsetPrintNomencl(params()));
	}
};


} // namespace lyx

#endif
