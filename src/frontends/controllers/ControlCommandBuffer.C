/**
 * \file ControlCommandBuffer.C
 * Read the file COPYING
 *
 * \author Lars
 * \author Asger and Juergen
 * \author John Levon 
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlCommandBuffer.h"
#include "support/lyxalgo.h"
#include "support/lstrings.h"
#include "LyXAction.h"
#include "lyxfunc.h"
#include "debug.h"

using std::vector;
using std::back_inserter;
using std::transform;
using std::endl;
 
namespace {
 
struct prefix_p {
	string p;
	prefix_p(string const & s) 
		: p(s) {}
	bool operator()(string const & s) const {
		return prefixIs(s, p);
	}
};
 
} // end of anon namespace

 
ControlCommandBuffer::ControlCommandBuffer(LyXFunc & lf)
	: lyxfunc_(lf), history_pos_(history_.end())
{
	transform(lyxaction.func_begin(), lyxaction.func_end(),
		back_inserter(commands_), lyx::firster()); 
}
 
	
string const ControlCommandBuffer::historyUp()
{
	if (history_pos_ == history_.begin())
		return "";

	return *(--history_pos_);
}
 

string const ControlCommandBuffer::historyDown()
{
	if (history_pos_ == history_.end())
		return "";
	if (history_pos_ + 1 == history_.end())
		return "";

	return *(++history_pos_);
}


vector<string> const ControlCommandBuffer::completions(string const & prefix, string & new_prefix)
{
	vector<string> comp;

	lyx::copy_if(commands_.begin(), commands_.end(),
		back_inserter(comp), prefix_p(prefix));

	if (comp.empty()) {
		new_prefix = prefix;
		return comp;
	}

	if (comp.size() == 1) {
		new_prefix = comp[0];
		return vector<string>();
	}

	// find maximal avaliable prefix
	string const tmp = comp[0];
	string test(prefix);
	if (tmp.length() > test.length())
		test += tmp[test.length()];
	while (test.length() < tmp.length()) {
		vector<string> vtmp;
		lyx::copy_if(comp.begin(), comp.end(),
			back_inserter(vtmp), prefix_p(test));
		if (vtmp.size() != comp.size()) {
			test.erase(test.length() - 1);
			break;
		}
		test += tmp[test.length()];
	}
 
	new_prefix = test;
	return comp;
}
 

void ControlCommandBuffer::dispatch(string const & str)
{
	if (str.empty())
		return;
 
	history_.push_back(str);
	history_pos_ = history_.end();
	lyxfunc_.dispatch(str, true);
}
