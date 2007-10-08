/**
 * \file BranchList.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "BranchList.h"
#include "Color.h"

#include "frontends/Application.h"

#include <algorithm>

using std::string;

namespace lyx {


Branch::Branch() : selected_(false)
{
	// no theApp() with command line export
	if (theApp())
		theApp()->getRgbColor(Color::background, color_);
}


docstring const & Branch::getBranch() const
{
	return branch_;
}


void Branch::setBranch(docstring const & s)
{
	branch_ = s;
}


bool Branch::getSelected() const
{
	return selected_;
}


bool Branch::setSelected(bool b)
{
	if (b == selected_)
		return false;
	selected_ = b;
	return true;
}


RGBColor const & Branch::getColor() const
{
	return color_;
}


void Branch::setColor(RGBColor const & c)
{
	color_ = c;
}


void Branch::setColor(string const & c)
{
	if (c.size() == 7 && c[0] == '#')
		color_ = RGBColor(c);
	else
		// no color set or invalid color - use normal background
		theApp()->getRgbColor(Color::background, color_);
}


Branch * BranchList::find(docstring const & name)
{
	List::iterator it =
		std::find_if(list.begin(), list.end(), BranchNamesEqual(name));
	return it == list.end() ? 0 : &*it;
}


Branch const * BranchList::find(docstring const & name) const
{
	List::const_iterator it =
		std::find_if(list.begin(), list.end(), BranchNamesEqual(name));
	return it == list.end() ? 0 : &*it;
}


bool BranchList::add(docstring const & s)
{
	bool added = false;
	docstring::size_type i = 0;
	while (true) {
		docstring::size_type const j = s.find_first_of(separator_, i);
		docstring name;
		if (j == docstring::npos)
			name = s.substr(i);
		else
			name = s.substr(i, j - i);
		// Is this name already in the list?
		bool const already =
			std::find_if(list.begin(), list.end(),
				     BranchNamesEqual(name)) != list.end();
		if (!already) {
			added = true;
			Branch br;
			br.setBranch(name);
			br.setSelected(false);
			list.push_back(br);
		}
		if (j == docstring::npos)
			break;
		i = j + 1;
	}
	return added;
}


bool BranchList::remove(docstring const & s)
{
	size_t const size = list.size();
	list.remove_if(BranchNamesEqual(s));
	return size != list.size();
}


} // namespace lyx
