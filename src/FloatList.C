#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "FloatList.h"

FloatList::FloatList()
{
	// Insert the latex builtin float-types
	Floating table;
	table.type = "table";
	table.placement = "htbp";
	table.ext = "lot";
	table.within = "";
	table.style = "plain";
	table.name = "Table";
	table.builtin = true;
	list[table.type] = table;
	Floating figure;
	figure.type = "figure";
	figure.placement = "htbp";
	figure.ext = "lof";
	figure.within = "";
	figure.style = "plain";
	figure.name = "Figure";
	figure.builtin = true;
	list[figure.type] = figure;
	// And we add algorithm too since LyX has
	// supported that for a long time
	Floating algorithm;
	algorithm.type = "algorithm";
	algorithm.placement = "htbp";
	algorithm.ext = "loa";
	algorithm.within = "";
	algorithm.style = "ruled";
	algorithm.name = "Algorithm";
	algorithm.builtin = false;
	list[algorithm.type] = algorithm;
}


void FloatList::newFloat(Floating const & fl)
{
	list[fl.type] = fl;
}


string const FloatList::defaultPlacement(string const & t) const
{
	List::const_iterator cit = list.find(t);
	if (cit != list.end())
		return (*cit).second.placement;
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

// The global floatlist
FloatList floatList;
