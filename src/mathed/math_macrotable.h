// -*- C++ -*-
#ifndef MATHMACROTABLE
#define MATHMACROTABLE

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
	static MathMacroTemplate & provideTemplate(string const &, int);
	///
	static MathMacroTemplate & provideTemplate(string const &);
	///
	static bool hasTemplate(string const &);
	///
	static MathMacro * cloneTemplate(string const &);
private:
	///
	static void builtinMacros();
	///
	static bool built;
	///
	typedef std::map<string, MathMacroTemplate *> table_type;
	//
	static table_type macro_table;
	///
	static void dump();
};
#endif
