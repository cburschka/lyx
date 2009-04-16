/**
 * \file IndicesList.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "IndicesList.h"
#include "Color.h"

#include "frontends/Application.h"

#include "support/convert.h"
#include "support/lstrings.h"

#include <algorithm>

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace {

class IndexNamesEqual : public std::unary_function<Index, bool>
{
public:
	IndexNamesEqual(docstring const & name) : name_(name) {}

	bool operator()(Index const & index) const
	{
		return index.index() == name_;
	}
private:
	docstring name_;
};


class IndexHasShortcut : public std::unary_function<Index, bool>
{
public:
	IndexHasShortcut(docstring const & shortcut) : shortc_(shortcut) {}

	bool operator()(Index const & index) const
	{
		return index.shortcut() == shortc_;
	}
private:
	docstring shortc_;
};

}


/////////////////////////////////////////////////////////////////////
//
// Index
//
/////////////////////////////////////////////////////////////////////


Index::Index()
{
	// no theApp() with command line export
	if (theApp())
		theApp()->getRgbColor(Color_indexlabel, color_);
}


docstring const & Index::index() const
{
	return index_;
}


void Index::setIndex(docstring const & s)
{
	index_ = s;
}


docstring const & Index::shortcut() const
{
	return shortcut_;
}


void Index::setShortcut(docstring const & s)
{
	shortcut_ = s;
}


RGBColor const & Index::color() const
{
	return color_;
}


void Index::setColor(RGBColor const & c)
{
	color_ = c;
}


void Index::setColor(string const & str)
{
	if (str.size() == 7 && str[0] == '#')
		color_ = rgbFromHexName(str);
	else
		// no color set or invalid color -- use predefined color
		theApp()->getRgbColor(Color_indexlabel, color_);
}


/////////////////////////////////////////////////////////////////////
//
// IndicesList
//
/////////////////////////////////////////////////////////////////////


Index * IndicesList::find(docstring const & name)
{
	List::iterator it =
		find_if(list.begin(), list.end(), IndexNamesEqual(name));
	return it == list.end() ? 0 : &*it;
}


Index const * IndicesList::find(docstring const & name) const
{
	List::const_iterator it =
		find_if(list.begin(), list.end(), IndexNamesEqual(name));
	return it == list.end() ? 0 : &*it;
}


Index * IndicesList::findShortcut(docstring const & shortcut)
{
	List::iterator it =
		find_if(list.begin(), list.end(), IndexHasShortcut(shortcut));
	return it == list.end() ? 0 : &*it;
}


Index const * IndicesList::findShortcut(docstring const & shortcut) const
{
	List::const_iterator it =
		find_if(list.begin(), list.end(), IndexHasShortcut(shortcut));
	return it == list.end() ? 0 : &*it;
}


bool IndicesList::add(docstring const & n, docstring const & s)
{
	bool added = false;
	size_t i = 0;
	while (true) {
		size_t const j = n.find_first_of(separator_, i);
		docstring name;
		if (j == docstring::npos)
			name = n.substr(i);
		else
			name = n.substr(i, j - i);
		// Is this name already in the list?
		bool const already =
			find_if(list.begin(), list.end(),
				     IndexNamesEqual(name)) != list.end();
		if (!already) {
			added = true;
			Index in;
			in.setIndex(name);
			docstring sc = s.empty() ?
				trim(lowercase(name.substr(0, 3))) : s;
			if (findShortcut(sc) != 0) {
				int i = 1;
				docstring scn = sc + convert<docstring>(i);
				while (findShortcut(scn) != 0) {
					++i;
					scn = sc + convert<docstring>(i);
				}
				in.setShortcut(scn);
			} else
				in.setShortcut(sc);
			list.push_back(in);
		}
		if (j == docstring::npos)
			break;
		i = j + 1;
	}
	return added;
}


bool IndicesList::addDefault(docstring const & n)
{
	if (findShortcut(from_ascii("idx")) != 0)
		// we already have a default
		return false;
	return add(n, from_ascii("idx"));
}

bool IndicesList::remove(docstring const & s)
{
	size_t const size = list.size();
	list.remove_if(IndexNamesEqual(s));
	return size != list.size();
}


bool IndicesList::rename(docstring const & oldname,
	docstring const & newname)
{
	if (newname.empty())
		return false;
	if (find_if(list.begin(), list.end(),
		    IndexNamesEqual(newname)) != list.end())
		// new name already taken
		return false;

	Index * index = find(oldname);
	if (!index)
		return false;
	index->setIndex(newname);
	return true;
}


} // namespace lyx
