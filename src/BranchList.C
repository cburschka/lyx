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


string const Branch::getBranch() const
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


void Branch::setSelected(bool b)
{
	selected_ = b;
}


string const Branch::getColor() const
{
	return color_;
}


void Branch::setColor(string const & c)
{
	color_ = c;
}


void BranchList::clear()
{
	list.clear();
}


string BranchList::getColor(string const & s) const
{
	List::const_iterator it = list.begin();
	List::const_iterator end = list.end();
	for (; it != end; ++it) {
		if (s == it->getBranch()) {
			return it->getColor();
		}
	}
	BOOST_ASSERT(false); // Always
	return string(); // never gets here
}



void BranchList::setColor(string const & s, string const & val)
{
	List::iterator it = list.begin();
	List::iterator end = list.end();
	for (; it != end; ++it) {
		if (s == it->getBranch()) {
			it->setColor(val);
			return;
		}
	}
	BOOST_ASSERT(false);
}


void BranchList::setSelected(string const & s, bool val)
{
	List::iterator it = list.begin();
	List::iterator end = list.end();
	for (; it != end; ++it) {
		if (s == it->getBranch()) {
			it->setSelected(val);
			return;
		}
	}
	BOOST_ASSERT(false);
}


void BranchList::add(string const & s)
{
	string::size_type i = 0;
	while (true) {
		string::size_type const j = s.find_first_of(separator(), i);
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
}


namespace {

struct match : public binary_function<Branch, string, bool> {
	bool operator()(Branch const & br, string const & s) const {
	return (br.getBranch() == s);
	}
};

} // namespace anon.


void BranchList::remove(string const & s)
{
	list.remove_if(bind2nd(match(), s));
}


bool BranchList::selected(string const & s) const
{
	List::const_iterator it = list.begin();
	List::const_iterator end = list.end();
	for (; it != end; ++it) {
		if (s == it->getBranch())
			return it->getSelected();
	}
	return false;
}


string BranchList::allBranches() const
{
	List::const_iterator it = list.begin();
	List::const_iterator end = list.end();
	string ret;
	for (; it != end; ++it) {
		ret += it->getBranch() + separator();
	}
	// remove final '|':
	string::size_type i = ret.find_last_of(separator());
	if (i != string::npos)
		ret.erase(i);
	return ret;
}


string BranchList::allSelected() const
{
	List::const_iterator it = list.begin();
	List::const_iterator end = list.end();
	string ret;
	for (; it != end; ++it) {
		if (it->getSelected())
			ret += it->getBranch() + separator();
	}
	// remove final '|':
	string::size_type i = ret.find_last_of(separator());
	if (i != string::npos)
		ret.erase(i);
	return ret;
}


string const BranchList::separator() const
{
	return separator_;
}
