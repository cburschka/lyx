// -*- C++ -*-
/**
 * \file command_inset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */


#ifndef COMMAND_INSET_H
#define COMMAND_INSET_H

#include "math_nestinset.h"

#include "insets/render_button.h"


/// Inset for things like \name[options]{contents}
class CommandInset : public MathNestInset {
public:
	///
	explicit CommandInset(std::string const & name);
	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void write(WriteStream & os) const;
	//
	// void infoize(std::ostream & os) const;
	///
	virtual std::string const screenLabel() const;
	/// generate something that will be understood by the Dialogs.
	std::string const createDialogStr(std::string const & name) const;
	///
	std::string const & commandname() const { return name_; }
private:
	///
	std::string name_;
	///
	mutable bool set_label_;
	///
	mutable RenderButton button_;
};

#endif
