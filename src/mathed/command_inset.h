// -*- C++ -*-

/**
 *  \file command_inset.h
 *
 *  This file is part of LyX, the document processor.
 *  Licence details can be found in the file COPYING.
 *
 *  \author André Pönitz
 *
 *  Full author contact details are available in file CREDITS.
 */


#ifndef COMMAND_INSET_H
#define COMMAND_INSET_H

#include "math_nestinset.h"
#include "insets/renderers.h"


/// Inset for things like \name[options]{contents}
class CommandInset : public MathNestInset {
public:
	///
	explicit CommandInset(string const & name);
	///
	InsetBase * clone() const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void write(WriteStream & os) const;
	///
	//void infoize(std::ostream & os) const;
	///
	dispatch_result dispatch(FuncRequest const & cmd, idx_type & idx, pos_type & pos);
	///
	virtual string const screenLabel() const;
	/// generate something that will be understood by the Dialogs.
	string const createDialogStr(string const & name) const;

	string const & commandname() const { return name_; }

private:
	string name_;
	mutable bool set_label_;
	mutable ButtonRenderer button_;
};

#endif
