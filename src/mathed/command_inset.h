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

#ifdef __GNUG__
#pragma interface
#endif

#ifndef COMMAND_INSET_H
#define COMMAND_INSET_H

#include "button_inset.h"

/// Inset for things like \name[options]{contents}
class CommandInset : public ButtonInset {
public:
	/// name, contents, options deliminited by '|++|'
	explicit CommandInset(string const & data);
	///
	MathInset * clone() const;
	///
	void write(WriteStream & os) const;
	///
	//void infoize(std::ostream & os) const;
	///
	result_type dispatch(FuncRequest const & cmd, idx_type & idx, pos_type & pos);
	///
	string screenLabel() const;
public:
	string name_;
};

#endif
