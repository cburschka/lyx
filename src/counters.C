/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 *
 * ====================================================== */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>

#include "counters.h"
#include "debug.h"
#include "support/lstrings.h"

using std::endl;
using std::vector;


Counter::Counter()
{
	reset();
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
	value_ = 0;
}

string Counter::master() const
{
	return master_;
}

void Counter::setMaster(string const & m)
{
	master_ = m;
}


Counters::Counters()
{
	// Ehh, should this take a textclass arg?

	// Sectioning counters:
	newCounter("part");
	newCounter("chapter");
	newCounter("section", "chapter");
	newCounter("subsection", "section");
	newCounter("subsubsection", "subsection");
	newCounter("paragraph", "subsubsection");
	newCounter("subparagraph", "paragraph");

	sects.push_back("chapter");
	sects.push_back("section");
	sects.push_back("subsection");
	sects.push_back("subsubsection");
	sects.push_back("paragraph");
	sects.push_back("subparagraph");

	// Enumeration counters:
	newCounter("enumi");
	newCounter("enumii", "enumi");
	newCounter("enumiii", "enumii");
	newCounter("enumiv", "enumiii");

	enums.push_back("enumi");
	enums.push_back("enumii");
	enums.push_back("enumiii");
	enums.push_back("enumiv");

	// Biblio:
	newCounter("bibitem");

	// Float counters:
	newCounter("Figure");
	newCounter("Table");
}


void Counters::newCounter(string const & newc)
{
	// First check if newc already exist
	CounterList::iterator cit = counterList.find(newc);
	// if already exist give warning and return
	if (cit != counterList.end()) {
		lyxerr << "The new counter already exists." << endl;
		return;
	}
	counterList[newc];
	cit = counterList.find(newc);
	cit->second.setMaster("");
}


void Counters::newCounter(string const & newc, string const & masterc)
{
	// First check if newc already exists
	CounterList::iterator cit = counterList.find(newc);
	// if already existant give warning and return
	if (cit != counterList.end()) {
		lyxerr << "The new counter already exists." << endl;
		return;
	}
	// then check if masterc exists
	CounterList::iterator it = counterList.find(masterc);
	// if not give warning and return
	if (it == counterList.end()) {
		lyxerr << "The master counter does not exist." << endl;
		return;
	}

	counterList[newc];
	cit = counterList.find(newc);
    cit->second.setMaster(masterc);
}


void Counters::set(string const & ctr, int val)
{
	CounterList::iterator it = counterList.find(ctr);
	if (it == counterList.end()) {
		lyxerr << "set: Counter does not exist: " << ctr << endl;
		return;
	}
	it->second.set(val);
}


void Counters::addto(string const & ctr, int val)
{
	CounterList::iterator it = counterList.find(ctr);
	if (it == counterList.end()) {
		lyxerr << "addto: Counter does not exist: " << ctr << endl;
		return;
	}
	it->second.addto(val);
}


int Counters::value(string const & ctr) const
{
	CounterList::const_iterator cit = counterList.find(ctr);
	if (cit == counterList.end()) {
		lyxerr << "value: Counter does not exist: " << ctr << endl;
		return 0;
	}
	return cit->second.value();
}


void Counters::step(string const & ctr)
{
	CounterList::iterator it = counterList.find(ctr);
	if (it == counterList.end()) {
		lyxerr << "step: Counter does not exist: " << ctr << endl;
		return;
	}

	it->second.step();
	it = counterList.begin();
	CounterList::iterator end = counterList.end();
	for (; it != end; ++it) {
		if (it->second.master() == ctr) {
			it->second.reset();
		}
	}
}

void Counters::reset(string const & match)
{
	CounterList::iterator it = counterList.begin();
	CounterList::iterator end = counterList.end();
	for (; it != end; ++it) {
		if (it->first.find(match) != string::npos || match == "")
			it->second.reset();
	}
}

void Counters::copy(Counters & from, Counters & to, string const & match)
{
	CounterList::iterator it = counterList.begin();
	CounterList::iterator end = counterList.end();
	for (; it != end; ++it) {
		if (it->first.find(match) != string::npos || match == "") {
			to.set(it->first, from.value(it->first));
		}
	}
}


namespace {

inline
char loweralphaCounter(int n)
{
	if (n < 1 || n > 26)
		return '?';
	else
		return 'a' + n - 1;
}

inline
char alphaCounter(int n)
{
	if (n < 1 || n > 26)
		return '?';
	else
		return 'A' + n - 1;
}

inline
char hebrewCounter(int n)
{
	static const char hebrew[22] = {
		'à', 'á', 'â', 'ã', 'ä', 'å', 'æ', 'ç', 'è',
		'é', 'ë', 'ì', 'î', 'ð', 'ñ', 'ò', 'ô', 'ö',
		'÷', 'ø', 'ù', 'ú'
	};
	if (n < 1 || n > 22)
		return '?';
	else
		return hebrew[n-1];
}

inline
string const romanCounter(int n)
{
	static char const * roman[20] = {
		"i",   "ii",  "iii", "iv", "v",
		"vi",  "vii", "viii", "ix", "x",
		"xi",  "xii", "xiii", "xiv", "xv",
		"xvi", "xvii", "xviii", "xix", "xx"
	};
	if (n < 1 || n > 20)
		return "??";
	else
		return roman[n-1];
}

} // namespace anon

string Counters::labelItem(string const & ctr,
		string const & numbertype,
		string const & langtype,
		bool first)
{
	ostringstream s, o;
	CounterList::iterator it = counterList.find(ctr);
	if (it == counterList.end()) {
		lyxerr << "Counter does not exist." << endl;
		return "";
	}
	string mstr = it->second.master();
	if (!first) {
		s << "." << value(ctr);
	} else {
		if (numbertype == "sectioning" || numbertype == "appendix") {
			if (numbertype == "appendix") {
				if (langtype == "hebrew") {
					o << hebrewCounter(value(ctr));
				} else {
					o << alphaCounter(value(ctr));
				}
			} else o << value(ctr);
		}
		s << o.str();
	}
	return s.str();
}

string Counters::numberLabel(string const & ctr,
		string const & numbertype,
		string const & langtype,
		int head)
{
	ostringstream s, o;
	if (numbertype == "sectioning" || numbertype == "appendix") {
		if (ctr == "chapter" && head == 0) {
			s << labelItem("chapter", numbertype, langtype, true);
		} else if (ctr == "section" && head <= 1) {
			s << numberLabel("chapter", numbertype, langtype, head)
			  << labelItem("section", numbertype, langtype, head == 1);
		} else if (ctr == "subsection" && head <= 2) {
			s << numberLabel("section", numbertype, langtype, head)
			  << labelItem("subsection", numbertype, langtype, head == 2);
		} else if (ctr == "subsubsection" && head <= 3) {
			s << numberLabel("subsection", numbertype, langtype, head)
			  << labelItem("subsubsection", numbertype, langtype, head == 3);
		} else if (ctr == "paragraph" && head <= 4) {
			s << numberLabel("subsubsection", numbertype, langtype, head)
			  << labelItem("paragraph", numbertype, langtype, head == 4);
		} else if (ctr == "subparagraph" && head <= 5) {
			s << numberLabel("paragraph", numbertype, langtype, head)
			  << labelItem("subparagraph", numbertype, langtype, head == 5);
		} else if (ctr == "Figure" || ctr == "Table") {
			// figure, table, ...
			lyxerr << "Counter:" << ctr << endl;
			s << numberLabel("chapter", numbertype, langtype, head)
			  << labelItem(ctr, numbertype, langtype, head == 1);
		}

	} else if (numbertype == "enumeration") {
		ostringstream ei, eii, eiii, eiv;
		//string ei, eiii, eiv;
		//char eii;
		if (langtype == "hebrew") {
			ei << '.' << value("enumi");
			eii << '(' << hebrewCounter(value("enumii")) << ')';
			eiii << '.' << romanCounter(value("enumiii"));
			eiv << '.' << alphaCounter(value("enumiv"));
		} else {
			ei << value("enumi") << '.';
			eii << '(' << loweralphaCounter(value("enumii")) << ')';
			eiii << romanCounter(value("enumiii")) << '.';
			eiv << alphaCounter(value("enumiv")) << '.';
		}
		if (ctr == "enumii") {
			s << eii.str();
		} else if (ctr == "enumi") {
			s << ei.str();
		} else if (ctr == "enumiii") {
			s << eiii.str();
		} else if (ctr == "enumiv") {
			s << eiv.str();
		}
	}
	return s.str();
}
