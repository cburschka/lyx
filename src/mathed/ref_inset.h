// -*- C++ -*-
/**
 * \file ref_inset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef REF_INSET_H
#define REF_INSET_H


#include "command_inset.h"
class Buffer;

// for \ref
class RefInset : public CommandInset {
public:
	///
	RefInset();
	///
	explicit RefInset(std::string const & data);
	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	//void write(WriteStream & os) const;
	///
	void infoize(std::ostream & os) const;
	///
	std::string const screenLabel() const;
	///
	void validate(LaTeXFeatures & features) const;
	///
	virtual RefInset * asRefInset() { return this; }

	/// plain ascii output
	int plaintext(std::ostream & os, OutputParams const &) const;
	/// linuxdoc output
	int linuxdoc(std::ostream & os, OutputParams const &) const;
	/// docbook output
	int docbook(Buffer const & buf, std::ostream & os, OutputParams const &) const;

	/// small wrapper for the time being
	DispatchResult localDispatch(FuncRequest & cmd);

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
	virtual void doDispatch(LCursor & cur, FuncRequest & cmd);
};

#endif
