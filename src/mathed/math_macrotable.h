// -*- C++ -*-
#ifndef MATH_MACROTABLE_H
#define MATH_MACROTABLE_H

#include <map>
#include "LString.h"
#include "math_atom.h"

#ifdef __GNUG__
#pragma interface
#endif

class MathArray;

///
struct MathMacroTable {
public:
	///
	static void create(string const &, int);
	///
	static void create(string const &, int, MathArray const &, MathArray const &);
	///
	static MathAtom & provide(string const &);
	///
	static bool has(string const &);
	///
	static void builtinMacros();
	///
	static void dump();
private:
	/// create internal macros (like \longrightarrow...)
	static void define(string const & display);

	///
	typedef std::map<string, MathAtom> table_type;
	//
	static table_type macro_table;
};
#endif
