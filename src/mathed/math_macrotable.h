// -*- C++ -*-
/**
 * \file math_macrotable.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_MACROTABLE_H
#define MATH_MACROTABLE_H

#include <map>
#include "LString.h"
#include "math_atom.h"


class MathMacroTable {
public:
	///
	static void create(MathAtom const &);
	///
	static MathAtom & provide(string const & name);
	///
	static bool has(string const & name);
	///
	static void dump();
private:
	///
	typedef std::map<string, MathAtom> table_type;
	//
	static table_type macro_table;
};

#endif
