/**
 * \file BranchList.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "BranchList.h"

#include <boost/assert.hpp>

#include <functional>


using std::string;
using std::bind2nd;
using std::binary_function;


string const & Branch::getBranch() const
{
	return branch_;
}


void Branch::setBranch(string const & s)
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


string const & Branch::getColor() const
{
	return color_;
}


void Branch::setColor(string const & c)
{
	color_ = c;
}


namespace {

struct SameName {
	SameName(string const & name) : name_(name) {}
	bool operator()(Branch const & branch) const
		{ return branch.getBranch() == name_; }
private:
	string name_;
};

}// namespace anon


Branch * BranchList::find(std::string const & name)
{
	List::iterator it =
		std::find_if(list.begin(), list.end(), SameName(name));
	return it == list.end() ? 0 : &*it;
}

	
Branch const * BranchList::find(std::string const & name) const
{
	List::const_iterator it =
		std::find_if(list.begin(), list.end(), SameName(name));
	return it == list.end() ? 0 : &*it;
}

	
bool BranchList::add(string const & s)
{
	bool added = false;
	string::size_type i = 0;
	while (true) {
		string::size_type const j = s.find_first_of(separator_, i);
		string name;
		if (j == string::npos)
			name = s.substr(i);
		else
			name = s.substr(i, j - i);
		// Is this name already in the list?
		List::const_iterator it = list.begin();
		List::const_iterator end = list.end();
		bool already = false;
		for (; it != end; ++it) {
			if (it->getBranch() == name) {
				already = true;
				break;
			}
		}
		if (!already) {
			added = true;
			Branch br;
			br.setBranch(name);
			br.setSelected(false);
			br.setColor("none");
			list.push_back(br);
		}
		if (j == string::npos)
			break;
		i = j + 1;
	}
	return added;
}


namespace {

struct match : public binary_function<Branch, string, bool> {
	bool operator()(Branch const & br, string const & s) const {
	return (br.getBranch() == s);
	}
};

} // namespace anon.


bool BranchList::remove(string const & s)
{
	List::size_type const size = list.size();
	list.remove_if(bind2nd(match(), s));
	return size != list.size();
}
