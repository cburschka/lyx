// -*- C++ -*-
#ifndef MATH_MACROTABLE_H
#define MATH_MACROTABLE_H

#include <map>
#include "LString.h"
#include "math_macrotemplate.h"

#ifdef __GNUG__
#pragma interface
#endif


class MathMacro;


///
struct MathMacroTable {
public:
	///
	static void insertTemplate(MathMacroTemplate const &);
	///
	static MathMacroTemplate & provideTemplate(string const &);
	///
	static bool hasTemplate(string const &);
	///
	static MathMacro * cloneTemplate(string const &);
	///
	static void createTemplate(string const &, int, string const &);
	///
	static void builtinMacros();
private:
	///
	typedef std::map<string, MathMacroTemplate> table_type;
	//
	static table_type macro_table;
public:
	///
	static void dump();
};
#endif
