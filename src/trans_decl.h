// -*- C++ -*-
/**
 * \file trans_decl.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Matthias Ettrich
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef TRANS_DECL_H
#define TRANS_DECL_H

#include "tex-accent.h"

#include <list>
#include <string>


///
struct Keyexc {
	/// character to make exception
	char c;
	/// exception data
	std::string data;
	/// Combination with another deadkey
	bool combined;
	/// The accent comined with
	tex_accent accent;
};

///
typedef std::list<Keyexc> KmodException;

///
struct KmodInfo {
	///
	std::string data;
	///
	tex_accent accent;
	///
	KmodException exception_list;
	///
	KmodInfo();
};

#endif
