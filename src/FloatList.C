#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "FloatList.h"

FloatList::FloatList()
{
	// Insert the latex builtin float-types

	// table
	Floating table("table", "htbp", "lot", "", "plain", "Table", true);
	list[table.type()] = table;

	// figure
	Floating figure("figure", "htbp", "lof", "", "plain", "Figure", true);
	list[figure.type()] = figure;
	
	// And we add algorithm too since LyX has
	// supported that for a long time,
	// but support for this should probably be moved to a layout file.
	Floating algorithm("algorithm", "htbp", "loa",
			   "", "ruled", "Algorithm");
	list[algorithm.type()] = algorithm;
}


FloatList::const_iterator FloatList::begin() const 
{
	return list.begin();
}


FloatList::const_iterator FloatList::end() const 
{
	return list.end();
}


void FloatList::newFloat(Floating const & fl)
{
	list[fl.type()] = fl;
}


string const FloatList::defaultPlacement(string const & t) const
{
	List::const_iterator cit = list.find(t);
	if (cit != list.end())
		return (*cit).second.placement();
	return string();
}


bool FloatList::typeExist(string const & t) const
{
	List::const_iterator cit = list.find(t);
	return cit != list.end();
}


Floating const & FloatList::getType(string const & t) const
{
	// I wish we could use exceptions
	List::const_iterator cit = list.find(t);
	if (cit != list.end())
		return (*cit).second;
#ifdef HAVE_EXCEPTIONS
	throw UnknownFloatType(t);
#else
	static Floating empty_float;
	return empty_float;
#endif
}


FloatList::const_iterator FloatList::operator[](string const & t) const
{
	return list.find(t);
}


// The global floatlist
FloatList floatList;
