// -*- C++ -*-
#ifndef MATH_MACROTABLE_H
#define MATH_MACROTABLE_H

#include <map>
#include "LString.h"

#ifdef __GNUG__
#pragma interface
#endif


class MathMacro;
class MathMacroTemplate;

///
struct MathMacroTable {
public:
	///
	static void updateTemplate(MathMacroTemplate *);
	///
	static void insertTemplate(MathMacroTemplate *);
	///
	static MathMacroTemplate & provideTemplate(string const &);
	///
	static bool hasTemplate(string const &);
	///
	static MathMacro * cloneTemplate(string const &);
	///
	static void createTemplate(string const &, int, string const &);
private:
	///
	static void builtinMacros();
	///
	typedef std::map<string, MathMacroTemplate *> table_type;
	//
	static table_type macro_table;
public:
	///
	static void dump();
};
#endif
