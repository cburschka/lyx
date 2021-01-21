/**
 * \file BranchList.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Martin Vermeer
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "BranchList.h"
#include "Color.h"
#include "ColorSet.h"

#include "frontends/Application.h"

#include "support/convert.h"
#include "support/lstrings.h"

#include <algorithm>

using namespace std;


namespace lyx {

Branch::Branch()
	: selected_(false), filenameSuffix_(false)
{
	lmcolor_ = "background";
	dmcolor_ = "background";
}


docstring const & Branch::branch() const
{
	return branch_;
}


void Branch::setBranch(docstring const & s)
{
	branch_ = s;
}


bool Branch::isSelected() const
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


bool Branch::hasFileNameSuffix() const
{
	return filenameSuffix_;
}


void Branch::setFileNameSuffix(bool b)
{
	filenameSuffix_ = b;
}


string const & Branch::color() const
{
	return (theApp() && theApp()->isInDarkMode())
			? dmcolor_ : lmcolor_;
}


string const & Branch::lightModeColor() const
{
	return lmcolor_;
}


string const & Branch::darkModeColor() const
{
	return dmcolor_;
}


void Branch::setColor(string const & col)
{
	if (theApp() && theApp()->isInDarkMode())
		setColors(string(), col);
	else
		setColors(col);
}


void Branch::setColors(string const & lmcol, string const & dmcol)
{
	if (lmcol.empty() && lmcolor_ == "background" && support::prefixIs(dmcol, "#"))
		lmcolor_ = X11hexname(inverseRGBColor(rgbFromHexName(dmcol)));
	else if (!lmcol.empty())
		lmcolor_ = lmcol;
	if (dmcol.empty() && dmcolor_ == "background" && support::prefixIs(lmcol, "#"))
		dmcolor_ = X11hexname(inverseRGBColor(rgbFromHexName(lmcol)));
	else if (!dmcol.empty())
		dmcolor_ = dmcol;

	// Update the Color table
	string lmcolor = lmcolor_;
	string dmcolor = dmcolor_;
	if (lmcolor == "none")
		lmcolor = lcolor.getX11HexName(Color_background);
	else if (lmcolor.size() != 7 || lmcolor[0] != '#')
		lmcolor = lcolor.getX11HexName(lcolor.getFromLyXName(lmcolor));
	if (dmcolor == "none")
		lmcolor = lcolor.getX11HexName(Color_background, true);
	else if (dmcolor.size() != 7 || dmcolor[0] != '#')
		dmcolor = lcolor.getX11HexName(lcolor.getFromLyXName(dmcolor), true);

	// FIXME UNICODE
	lcolor.setColor("branch" + convert<string>(branch_list_id_)
			+ to_utf8(branch_), lmcolor, dmcolor);
}


namespace {

std::function<bool (Branch const &)> BranchNameIs(docstring const & d)
{
	return [d](Branch const & b){ return b.branch() == d; };
}

} // namespace


Branch * BranchList::find(docstring const & name)
{
	List::iterator it =
		find_if(list_.begin(), list_.end(), BranchNameIs(name));
	return it == list_.end() ? nullptr : &*it;
}


Branch const * BranchList::find(docstring const & name) const
{
	List::const_iterator it =
		find_if(list_.begin(), list_.end(), BranchNameIs(name));
	return it == list_.end() ? nullptr : &*it;
}


bool BranchList::add(docstring const & s)
{
	bool added = false;
	size_t i = 0;
	while (true) {
		size_t const j = s.find_first_of(separator_, i);
		docstring name;
		if (j == docstring::npos)
			name = s.substr(i);
		else
			name = s.substr(i, j - i);
		// Is this name already in the list?
		bool const already = find(name);
		if (!already) {
			added = true;
			Branch br;
			br.setBranch(name);
			br.setSelected(false);
			br.setFileNameSuffix(false);
			br.setListID(id_);
			list_.push_back(br);
		}
		if (j == docstring::npos)
			break;
		i = j + 1;
	}
	return added;
}


bool BranchList::remove(docstring const & s)
{
	size_t const size = list_.size();
	list_.remove_if(BranchNameIs(s));
	return size != list_.size();
}


bool BranchList::rename(docstring const & oldname,
	docstring const & newname, bool const merge)
{
	if (newname.empty())
		return false;
	if (find(newname)) {
		// new name already taken
		if (merge)
			return remove(oldname);
		return false;
	}

	Branch * branch = find(oldname);
	if (!branch)
		return false;
	branch->setBranch(newname);
	return true;
}


docstring BranchList::getFileNameSuffix() const
{
	docstring result;
	for (auto const & br : list_) {
		if (br.isSelected() && br.hasFileNameSuffix())
			result += "-" + br.branch();
	}
	return support::subst(result, from_ascii("/"), from_ascii("_"));
}

} // namespace lyx
