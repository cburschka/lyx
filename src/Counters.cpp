/**
 * \file Counters.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Martin Vermeer
 * \author André Pönitz
 * \author Richard Heck (roman numerals)
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Counters.h"
#include "Layout.h"
#include "Lexer.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h"

#include <algorithm>
#include <sstream>

using namespace std;
using namespace lyx::support;

namespace lyx {


Counter::Counter()
	: initial_value_(0)
{
	reset();
}


Counter::Counter(docstring const & mc, docstring const & ls,
                docstring const & lsa)
	: initial_value_(0), master_(mc), labelstring_(ls), labelstringappendix_(lsa)
{
	reset();
}


bool Counter::read(Lexer & lex)
{
	enum {
		CT_WITHIN = 1,
		CT_LABELSTRING,
		CT_LABELSTRING_APPENDIX,
		CT_PRETTYFORMAT,
		CT_INITIALVALUE,
		CT_END
	};

	LexerKeyword counterTags[] = {
		{ "end", CT_END },
	  { "initialvalue", CT_INITIALVALUE},
		{ "labelstring", CT_LABELSTRING },
		{ "labelstringappendix", CT_LABELSTRING_APPENDIX },
		{ "prettyformat", CT_PRETTYFORMAT },
		{ "within", CT_WITHIN }
	};

	lex.pushTable(counterTags);

	bool getout = false;
	while (!getout && lex.isOK()) {
		int le = lex.lex();
		switch (le) {
			case Lexer::LEX_UNDEF:
				lex.printError("Unknown counter tag `$$Token'");
				continue;
			default:
				break;
		}
		switch (le) {
			case CT_WITHIN:
				lex.next();
				master_ = lex.getDocString();
				if (master_ == "none")
					master_.erase();
				break;
			case CT_INITIALVALUE:
				lex.next();
				initial_value_ = lex.getInteger();
				// getInteger() returns -1 on error, and larger
				// negative values do not make much sense.
				// In the other case, we subtract one, since the
				// counter will be incremented before its first use.
				if (initial_value_ <= -1)
					initial_value_ = 0;
				else
					initial_value_ -= 1;
				break;
			case CT_PRETTYFORMAT:
				lex.next();
				prettyformat_ = lex.getDocString();
				break;
			case CT_LABELSTRING:
				lex.next();
				labelstring_ = lex.getDocString();
				labelstringappendix_ = labelstring_;
				break;
			case CT_LABELSTRING_APPENDIX:
				lex.next();
				labelstringappendix_ = lex.getDocString();
				break;
			case CT_END:
				getout = true;
				break;
		}
	}

	// Here if have a full counter if getout == true
	if (!getout)
		LYXERR0("No End tag found for counter!");
	lex.popTable();
	return getout;
}


void Counter::set(int v)
{
	value_ = v;
}


void Counter::addto(int v)
{
	value_ += v;
}


int Counter::value() const
{
	return value_;
}


void Counter::step()
{
	++value_;
}


void Counter::reset()
{
	value_ = initial_value_;
}


docstring const & Counter::master() const
{
	return master_;
}


bool Counter::checkAndRemoveMaster(docstring const & cnt)
{
	if (master_ != cnt)
		return false;
	master_ = docstring();
	return true;
}


docstring const & Counter::labelString(bool in_appendix) const
{
	return in_appendix ? labelstringappendix_ : labelstring_;
}


Counter::StringMap & Counter::flatLabelStrings(bool in_appendix) const
{
	return in_appendix ? flatlabelstringappendix_ : flatlabelstring_;
}


Counters::Counters() : appendix_(false), subfloat_(false), longtable_(false)
{
	layout_stack_.push_back(0);
	counter_stack_.push_back(from_ascii(""));
}


void Counters::newCounter(docstring const & newc,
			  docstring const & masterc,
			  docstring const & ls,
			  docstring const & lsa)
{
	if (!masterc.empty() && !hasCounter(masterc)) {
		lyxerr << "Master counter does not exist: "
		       << to_utf8(masterc)
		       << endl;
		return;
	}
	counterList_[newc] = Counter(masterc, ls, lsa);
}


bool Counters::hasCounter(docstring const & c) const
{
	return counterList_.find(c) != counterList_.end();
}


bool Counters::read(Lexer & lex, docstring const & name, bool makenew)
{
	if (hasCounter(name)) {
		LYXERR(Debug::TCLASS, "Reading existing counter " << to_utf8(name));
		return counterList_[name].read(lex);
	}

	LYXERR(Debug::TCLASS, "Reading new counter " << to_utf8(name));
	Counter cnt;
	bool success = cnt.read(lex);
	// if makenew is false, we will just discard what we read
	if (success && makenew)
		counterList_[name] = cnt;
	else if (!success)
		LYXERR0("Error reading counter `" << name << "'!");
	return success;
}


void Counters::set(docstring const & ctr, int const val)
{
	CounterList::iterator const it = counterList_.find(ctr);
	if (it == counterList_.end()) {
		lyxerr << "set: Counter does not exist: "
		       << to_utf8(ctr) << endl;
		return;
	}
	it->second.set(val);
}


void Counters::addto(docstring const & ctr, int const val)
{
	CounterList::iterator const it = counterList_.find(ctr);
	if (it == counterList_.end()) {
		lyxerr << "addto: Counter does not exist: "
		       << to_utf8(ctr) << endl;
		return;
	}
	it->second.addto(val);
}


int Counters::value(docstring const & ctr) const
{
	CounterList::const_iterator const cit = counterList_.find(ctr);
	if (cit == counterList_.end()) {
		lyxerr << "value: Counter does not exist: "
		       << to_utf8(ctr) << endl;
		return 0;
	}
	return cit->second.value();
}


void Counters::resetSlaves(docstring const & ctr)
{
	CounterList::iterator it = counterList_.begin();
	CounterList::iterator const end = counterList_.end();
	for (; it != end; ++it) {
		if (it->second.master() == ctr) {
			it->second.reset();
			resetSlaves(it->first);
		}
	}
}


void Counters::step(docstring const & ctr, UpdateType utype)
{
	CounterList::iterator it = counterList_.find(ctr);
	if (it == counterList_.end()) {
		lyxerr << "step: Counter does not exist: "
		       << to_utf8(ctr) << endl;
		return;
	}

	it->second.step();
	if (utype == OutputUpdate) {
		LBUFERR(!counter_stack_.empty());
		counter_stack_.pop_back();
		counter_stack_.push_back(ctr);
	}

	resetSlaves(ctr);
}


void Counters::reset()
{
	appendix_ = false;
	subfloat_ = false;
	current_float_.erase();
	CounterList::iterator it = counterList_.begin();
	CounterList::iterator const end = counterList_.end();
	for (; it != end; ++it)
		it->second.reset();
	counter_stack_.clear();
	counter_stack_.push_back(from_ascii(""));
	layout_stack_.clear();
	layout_stack_.push_back(0);
}


void Counters::reset(docstring const & match)
{
	LASSERT(!match.empty(), return);

	CounterList::iterator it = counterList_.begin();
	CounterList::iterator end = counterList_.end();
	for (; it != end; ++it) {
		if (it->first.find(match) != string::npos)
			it->second.reset();
	}
}


bool Counters::remove(docstring const & cnt)
{
	bool retval = counterList_.erase(cnt);
	if (!retval)
		return false;
	CounterList::iterator it = counterList_.begin();
	CounterList::iterator end = counterList_.end();
	for (; it != end; ++it) {
		if (it->second.checkAndRemoveMaster(cnt))
			LYXERR(Debug::TCLASS, "Removed master counter `" +
					to_utf8(cnt) + "' from counter: " + to_utf8(it->first));
	}
	return retval;
}


void Counters::copy(Counters & from, Counters & to, docstring const & match)
{
	CounterList::iterator it = counterList_.begin();
	CounterList::iterator end = counterList_.end();
	for (; it != end; ++it) {
		if (it->first.find(match) != string::npos || match == "") {
			to.set(it->first, from.value(it->first));
		}
	}
}


namespace {

char loweralphaCounter(int const n)
{
	if (n < 1 || n > 26)
		return '?';
	return 'a' + n - 1;
}


char alphaCounter(int const n)
{
	if (n < 1 || n > 26)
		return '?';
	return 'A' + n - 1;
}


char hebrewCounter(int const n)
{
	static const char hebrew[22] = {
		'\xe0', '\xe1', '\xe2', '\xe3', '\xe4', '\xe5', '\xe6', '\xe7', '\xe8',
		'\xe9', '\xeb', '\xec', '\xee', '\xf0', '\xf1', '\xf2', '\xf4', '\xf6',
		'\xf7', '\xf8', '\xf9', '\xfa'
	};

	if (n < 1 || n > 22)
		return '?';
	return hebrew[n - 1];
}


// On the special cases, see http://mathworld.wolfram.com/RomanNumerals.html
// and for a list of roman numerals up to and including 3999, see
// http://www.research.att.com/~njas/sequences/a006968.txt. (Thanks to Joost
// for this info.)
docstring const romanCounter(int const n)
{
	static char const * const ones[9] = {
		"I",   "II",  "III", "IV", "V",
		"VI",  "VII", "VIII", "IX"
	};

	static char const * const tens[9] = {
		"X", "XX", "XXX", "XL", "L",
		"LX", "LXX", "LXXX", "XC"
	};

	static char const * const hunds[9] = {
		"C", "CC", "CCC", "CD", "D",
		"DC", "DCC", "DCCC", "CM"
	};

	if (n >= 1000 || n < 1)
		return from_ascii("??");

	int val = n;
	string roman;
	switch (n) {
	//special cases
	case 900:
		roman = "CM";
		break;
	case 400:
		roman = "CD";
		break;
	default:
		if (val >= 100) {
			int hundreds = val / 100;
			roman = hunds[hundreds - 1];
			val = val % 100;
		}
		if (val >= 10) {
			switch (val) {
			//special case
			case 90:
				roman = roman + "XC";
				val = 0; //skip next
				break;
			default:
				int tensnum = val / 10;
				roman = roman + tens[tensnum - 1];
				val = val % 10;
			} // end switch
		} // end tens
		if (val > 0)
			roman = roman + ones[val -1];
	}
	return from_ascii(roman);
}


docstring const lowerromanCounter(int const n)
{
	return lowercase(romanCounter(n));
}


docstring const fnsymbolCounter(int const n)
{
	switch(n) {
	case 1: return docstring(1, 0x002a); //*
	case 2: return docstring(1, 0x2020); // dagger
	case 3: return docstring(1, 0x2021); // double dagger
	case 4: return docstring(1, 0x00A7); // section sign
	case 5: return docstring(1, 0x00B6); // pilcrow sign
	case 6: return docstring(1, 0x2016); // vertical bar
	case 7: return docstring(2, 0x002a); // two *
	case 8: return docstring(2, 0x2020); // two daggers
	case 9: return docstring(2, 0x2021); // two double daggers
	default:
		return from_ascii("?");
	};
}

} // namespace anon


docstring Counters::labelItem(docstring const & ctr,
			      docstring const & numbertype) const
{
	CounterList::const_iterator const cit = counterList_.find(ctr);
	if (cit == counterList_.end()) {
		lyxerr << "Counter "
		       << to_utf8(ctr)
		       << " does not exist." << endl;
		return docstring();
	}

	int val = cit->second.value();

	if (numbertype == "hebrew")
		return docstring(1, hebrewCounter(val));

	if (numbertype == "alph")
		return docstring(1, loweralphaCounter(val));

	if (numbertype == "Alph")
		return docstring(1, alphaCounter(val));

	if (numbertype == "roman")
		return lowerromanCounter(val);

	if (numbertype == "Roman")
		return romanCounter(val);

	if (numbertype == "fnsymbol")
		return fnsymbolCounter(val);

	return convert<docstring>(val);
}


docstring Counters::theCounter(docstring const & counter,
			       string const & lang) const
{
	CounterList::const_iterator it = counterList_.find(counter);
	if (it == counterList_.end())
		return from_ascii("#");
	Counter const & ctr = it->second;
	Counter::StringMap & sm = ctr.flatLabelStrings(appendix());
	Counter::StringMap::iterator smit = sm.find(lang);
	if (smit != sm.end())
		return counterLabel(smit->second, lang);

	vector<docstring> callers;
	docstring const & fls = flattenLabelString(counter, appendix(),
						   lang, callers);
	sm[lang] = fls;
	return counterLabel(fls, lang);
}


docstring Counters::flattenLabelString(docstring const & counter,
				       bool in_appendix,
				       string const & lang,
				       vector<docstring> & callers) const
{
	if (find(callers.begin(), callers.end(), counter) != callers.end()) {
		// recursion detected
		lyxerr << "Warning: Recursion in label for counter `"
		       << counter << "' detected"
		       << endl;
		return from_ascii("??");
	}

	CounterList::const_iterator it = counterList_.find(counter);
	if (it == counterList_.end())
		return from_ascii("#");
	Counter const & c = it->second;

	docstring ls = translateIfPossible(c.labelString(in_appendix), lang);

	callers.push_back(counter);
	if (ls.empty()) {
		if (!c.master().empty())
			ls = flattenLabelString(c.master(), in_appendix, lang, callers)
				+ from_ascii(".");
		callers.pop_back();
		return ls + from_ascii("\\arabic{") + counter + "}";
	}

	while (true) {
		//lyxerr << "ls=" << to_utf8(ls) << endl;
		size_t const i = ls.find(from_ascii("\\the"), 0);
		if (i == docstring::npos)
			break;
		size_t const j = i + 4;
		size_t k = j;
		while (k < ls.size() && lowercase(ls[k]) >= 'a'
		       && lowercase(ls[k]) <= 'z')
			++k;
		docstring const newc = ls.substr(j, k - j);
		docstring const repl = flattenLabelString(newc, in_appendix,
							  lang, callers);
		ls.replace(i, k - j + 4, repl);
	}
	callers.pop_back();

	return ls;
}


docstring Counters::counterLabel(docstring const & format,
				 string const & lang) const
{
	docstring label = format;

	// FIXME: Using regexps would be better, but we compile boost without
	// wide regexps currently.
	docstring const the = from_ascii("\\the");
	while (true) {
		//lyxerr << "label=" << label << endl;
		size_t const i = label.find(the, 0);
		if (i == docstring::npos)
			break;
		size_t const j = i + 4;
		size_t k = j;
		while (k < label.size() && lowercase(label[k]) >= 'a'
		       && lowercase(label[k]) <= 'z')
			++k;
		docstring const newc(label, j, k - j);
		label.replace(i, k - i, theCounter(newc, lang));
	}
	while (true) {
		//lyxerr << "label=" << label << endl;

		size_t const i = label.find('\\', 0);
		if (i == docstring::npos)
			break;
		size_t const j = label.find('{', i + 1);
		if (j == docstring::npos)
			break;
		size_t const k = label.find('}', j + 1);
		if (k == docstring::npos)
			break;
		docstring const numbertype(label, i + 1, j - i - 1);
		docstring const counter(label, j + 1, k - j - 1);
		label.replace(i, k + 1 - i, labelItem(counter, numbertype));
	}
	//lyxerr << "DONE! label=" << label << endl;
	return label;
}


docstring Counters::prettyCounter(docstring const & name,
			       string const & lang) const
{
	CounterList::const_iterator it = counterList_.find(name);
	if (it == counterList_.end())
		return from_ascii("#");
	Counter const & ctr = it->second;

	docstring const value = theCounter(name, lang);
	docstring const & format =
	    translateIfPossible(ctr.prettyFormat(), lang);
	if (format.empty())
		return value;
	return subst(format, from_ascii("##"), value);
}


docstring Counters::currentCounter() const
{
	LBUFERR(!counter_stack_.empty());
	return counter_stack_.back();
}


void Counters::setActiveLayout(Layout const & lay)
{
	LASSERT(!layout_stack_.empty(), return);
	Layout const * const lastlay = layout_stack_.back();
	// we want to check whether the layout has changed and, if so,
	// whether we are coming out of or going into an environment.
	if (!lastlay) {
		layout_stack_.pop_back();
		layout_stack_.push_back(&lay);
		if (lay.isEnvironment())
			beginEnvironment();
	} else if (lastlay->name() != lay.name()) {
		layout_stack_.pop_back();
		layout_stack_.push_back(&lay);
		if (lastlay->isEnvironment()) {
			// we are coming out of an environment
			// LYXERR0("Out: " << lastlay->name());
			endEnvironment();
		}
		if (lay.isEnvironment()) {
			// we are going into a new environment
			// LYXERR0("In: " << lay.name());
			beginEnvironment();
		}
	}
}


void Counters::beginEnvironment()
{
	counter_stack_.push_back(counter_stack_.back());
}


void Counters::endEnvironment()
{
	LASSERT(!counter_stack_.empty(), return);
	counter_stack_.pop_back();
}


} // namespace lyx
