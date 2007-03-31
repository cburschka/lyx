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


namespace lyx {


///
struct Keyexc {
	/// character to make exception
	char_type c;
	/// exception data
	docstring data;
	/// Combination with another deadkey
	bool combined;
	/// The accent comined with
	tex_accent accent;
};

///
typedef std::list<Keyexc> KmodException;

///
class KmodInfo {
public:
	///
	docstring data;
	///
	tex_accent accent;
	///
	KmodException exception_list;
	///
	KmodInfo();
};


} // namespace lyx

#endif
