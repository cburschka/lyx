/**
 * \file ControlCommandBuffer.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars
 * \author Asger and Jürgen
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlCommandBuffer.h"

#include "BufferView.h"
#include "Cursor.h"
#include "LyXFunc.h"
#include "LyXAction.h"
#include "FuncRequest.h"

#include "frontends/LyXView.h"

#include "support/lyxalgo.h"
#include "support/lstrings.h"

using std::back_inserter;
using std::transform;
using std::string;
using std::vector;

namespace lyx {

using support::prefixIs;

namespace frontend {

namespace {

class prefix_p {
public:
	string p;
	prefix_p(string const & s)
		: p(s) {}
	bool operator()(string const & s) const {
		return prefixIs(s, p);
	}
};

} // end of anon namespace


ControlCommandBuffer::ControlCommandBuffer(LyXView & lv)
	: lv_(lv), history_pos_(history_.end())
{
	transform(lyxaction.func_begin(), lyxaction.func_end(),
		back_inserter(commands_), firster());
}


string const ControlCommandBuffer::historyUp()
{
	if (history_pos_ == history_.begin())
		return string();

	return *(--history_pos_);
}


string const ControlCommandBuffer::historyDown()
{
	if (history_pos_ == history_.end())
		return string();
	if (history_pos_ + 1 == history_.end())
		return string();

	return *(++history_pos_);
}


docstring const ControlCommandBuffer::getCurrentState() const
{
	return lv_.view()->cursor().currentState();
}


void ControlCommandBuffer::hide() const
{
	lv_.showMiniBuffer(false);
}


vector<string> const
ControlCommandBuffer::completions(string const & prefix, string & new_prefix)
{
	vector<string> comp;

	copy_if(commands_.begin(), commands_.end(),
		back_inserter(comp), prefix_p(prefix));

	if (comp.empty()) {
		new_prefix = prefix;
		return comp;
	}

	if (comp.size() == 1) {
		new_prefix = comp[0];
		return vector<string>();
	}

	// find maximal available prefix
	string const tmp = comp[0];
	string test = prefix;
	if (tmp.length() > test.length())
		test += tmp[test.length()];
	while (test.length() < tmp.length()) {
		vector<string> vtmp;
		copy_if(comp.begin(), comp.end(),
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
	FuncRequest func = lyxaction.lookupFunc(str);
	func.origin = FuncRequest::COMMANDBUFFER;
	lv_.dispatch(func);
}

} // namespace frontend
} // namespace lyx
