#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "FloatList.h"
#include "gettext.h"

// This class is now mostly finished, except one thing, it is a global
// object. This will not do. The user (and layout files) are free to
// create floats and modify them to fit into a certain document. So it is
// pretty clear that each layout needs its own list, as do documents.
// However this is also not enough since we really want the user to be
// able to create "presistent" floats, in the sense that a user created
// float can be used across sessions and across documents. So we need a
// global¹ floatlist as well. The interaction between these are not quite
// clear, but it seems natural that the definition found in the document
// takes precedence.
// We also have the issue about what get stored _in_ the lyx file.
//
// ¹ not absolutely global but somewhere where documents,layouts and
// the bufferview can have access to it.
//
// Lgb

FloatList::FloatList()
{
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
		return cit->second.placement();
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
		return cit->second;
#ifdef HAVE_EXCEPTIONS
	throw UnknownFloatType(t);
#else
	static Floating empty_float;
	return empty_float;
#endif
}


void FloatList::erase(string const & t)
{
	list.erase(t);
}


FloatList::const_iterator FloatList::operator[](string const & t) const
{
	return list.find(t);
}
