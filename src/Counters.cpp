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
#include "support/counter_reps.h"
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
	: initial_value_(0), saved_value_(0)
{
	reset();
}


Counter::Counter(docstring const & mc, docstring const & ls,
		docstring const & lsa, docstring const & guiname)
	: initial_value_(0), saved_value_(0), master_(mc), labelstring_(ls),
	  labelstringappendix_(lsa), guiname_(guiname)
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
		CT_GUINAME,
		CT_END
	};

	LexerKeyword counterTags[] = {
		{ "end", CT_END },
		{ "guiname", CT_GUINAME },
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
			case CT_GUINAME:
				lex.next();
				guiname_ = lex.getDocString();
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


void Counter::saveValue()
{
	saved_value_ = value_;
}


void Counter::restoreValue()
{
	value_ = saved_value_;
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
	layout_stack_.push_back(nullptr);
	counter_stack_.push_back(from_ascii(""));
}


void Counters::newCounter(docstring const & newc,
			  docstring const & masterc,
			  docstring const & ls,
			  docstring const & lsa,
			  docstring const & guiname)
{
	if (!masterc.empty() && !hasCounter(masterc)) {
		lyxerr << "Master counter does not exist: "
		       << to_utf8(masterc)
		       << endl;
		return;
	}
	counterList_[newc] = Counter(masterc, ls, lsa, guiname);
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


void Counters::saveValue(docstring const & ctr) const
{
	CounterList::const_iterator const cit = counterList_.find(ctr);
	if (cit == counterList_.end()) {
		lyxerr << "value: Counter does not exist: "
		       << to_utf8(ctr) << endl;
		return;
	}
	Counter const & cnt = cit->second;
	Counter & ccnt = const_cast<Counter &>(cnt);
	ccnt.saveValue();
}


void Counters::restoreValue(docstring const & ctr) const
{
	CounterList::const_iterator const cit = counterList_.find(ctr);
	if (cit == counterList_.end()) {
		lyxerr << "value: Counter does not exist: "
		       << to_utf8(ctr) << endl;
		return;
	}
	Counter const & cnt = cit->second;
	Counter & ccnt = const_cast<Counter &>(cnt);
	ccnt.restoreValue();
}


void Counters::resetSlaves(docstring const & count)
{
	for (auto & ctr : counterList_) {
		if (ctr.second.master() == count) {
			ctr.second.reset();
			resetSlaves(ctr.first);
		}
	}
}


void Counters::stepMaster(docstring const & ctr, UpdateType utype)
{
	CounterList::iterator it = counterList_.find(ctr);
	if (it == counterList_.end()) {
		lyxerr << "step: Counter does not exist: "
		       << to_utf8(ctr) << endl;
		return;
	}
	step(it->second.master(), utype);
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


docstring const & Counters::guiName(docstring const & cntr) const
{
	CounterList::const_iterator it = counterList_.find(cntr);
	if (it == counterList_.end()) {
		lyxerr << "step: Counter does not exist: "
			   << to_utf8(cntr) << endl;
		return empty_docstring();
	}

	docstring const & guiname = it->second.guiName();
	if (guiname.empty())
		return cntr;
	return guiname;
}


void Counters::reset()
{
	appendix_ = false;
	subfloat_ = false;
	current_float_.erase();
	for (auto & ctr : counterList_)
		ctr.second.reset();
	counter_stack_.clear();
	counter_stack_.push_back(from_ascii(""));
	layout_stack_.clear();
	layout_stack_.push_back(nullptr);
}


void Counters::reset(docstring const & match)
{
	LASSERT(!match.empty(), return);

	for (auto & ctr : counterList_) {
		if (ctr.first.find(match) != string::npos)
			ctr.second.reset();
	}
}


bool Counters::remove(docstring const & cnt)
{
	bool retval = counterList_.erase(cnt);
	if (!retval)
		return false;
	for (auto & ctr : counterList_) {
		if (ctr.second.checkAndRemoveMaster(cnt))
			LYXERR(Debug::TCLASS, "Removed master counter `" +
					to_utf8(cnt) + "' from counter: " + to_utf8(ctr.first));
	}
	return retval;
}


void Counters::copy(Counters & from, Counters & to, docstring const & match)
{
	for (auto const & ctr : counterList_) {
		if (ctr.first.find(match) != string::npos || match == "") {
			to.set(ctr.first, from.value(ctr.first));
		}
	}
}


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


vector<docstring> Counters::listOfCounters() const {
	vector<docstring> ret;
	for(auto const & k : counterList_)
		ret.emplace_back(k.first);
	return ret;
}


} // namespace lyx
