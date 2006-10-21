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
class RefInset : public CommandInset {
public:
	///
	RefInset();
	///
	explicit RefInset(std::string const & data);
	///
	//void write(WriteStream & os) const;
	///
	void infoize(std::ostream & os) const;
	///
	docstring const screenLabel() const;
	///
	void validate(LaTeXFeatures & features) const;
	///
	virtual RefInset * asRefInset() { return this; }

	/// plain text output in ucs4 encoding
	int plaintext(odocstream &, OutputParams const &) const;
	/// docbook output
	int docbook(Buffer const & buf, odocstream & os, OutputParams const &) const;

	struct ref_type_info {
		///
		std::string latex_name;
		///
		std::string gui_name;
		///
		std::string short_gui_name;
	};
	static ref_type_info types[];
	///
	static int getType(std::string const & name);
	///
	static std::string const & getName(int type);
protected:
	///
	virtual void doDispatch(LCursor & cur, FuncRequest & cmd);
	///
	bool getStatus(LCursor &, FuncRequest const &, FuncStatus &) const;
private:
	///
	virtual std::auto_ptr<InsetBase> doClone() const;
};



} // namespace lyx
#endif
