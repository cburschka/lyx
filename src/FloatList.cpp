/**
 * \file FloatList.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "FloatList.h"
#include "Floating.h"

using namespace std;

namespace lyx {

FloatList::FloatList()
{}


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
	list[fl.floattype()] = fl;
}


string const FloatList::defaultPlacement(string const & t) const
{
	List::const_iterator cit = list.find(t);
	if (cit != list.end())
		return cit->second.placement();
	return string();
}


string const FloatList::allowedPlacement(string const & t) const
{
	List::const_iterator cit = list.find(t);
	if (cit != list.end())
		return cit->second.allowedPlacement();
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
	static Floating const empty_float;
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


} // namespace lyx
