// -*- C++ -*-
#ifndef MATHMACROTABLE
#define MATHMACROTABLE

#include <map>
#include "LString.h"

#include <boost/utility.hpp>
#include <boost/smart_ptr.hpp>

#ifdef __GNUG__
#pragma interface
#endif

class MathMacroTemplate;
class MathMacro;

///
class MathMacroTable : noncopyable {
public:
	///
	void addTemplate(boost::shared_ptr<MathMacroTemplate> const &);
	///
	MathMacro * createMacro(string const &) const;
	///
	boost::shared_ptr<MathMacroTemplate> const
	getTemplate(string const &) const;
	///
	void builtinMacros();
	///
	static MathMacroTable mathMTable;
	///
	static bool built;
private:
	///
	typedef std::map<string, boost::shared_ptr<MathMacroTemplate> > table_type;
	///
	table_type macro_table;
};
#endif
