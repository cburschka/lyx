/**
 * \file counters.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Martin Vermeer
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "counters.h"
#include "debug.h"
#include "gettext.h"

#include "support/lstrings.h"
#include "support/convert.h"

#include <boost/assert.hpp>

#include <sstream>

using std::endl;
using std::ostringstream;
using std::string;


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


void Counters::newCounter(string const & newc)
{
	// First check if newc already exist
	CounterList::iterator const cit = counterList.find(newc);
	// if already exist give warning and return
	if (cit != counterList.end()) {
		lyxerr << "The new counter already exists." << endl;
		return;
	}
	counterList[newc];
}


void Counters::newCounter(string const & newc, string const & masterc)
{
	// First check if newc already exists
	CounterList::iterator const cit = counterList.find(newc);
	// if already existant give warning and return
	if (cit != counterList.end()) {
		lyxerr << "The new counter already exists." << endl;
		return;
	}
	// then check if masterc exists
	CounterList::iterator const it = counterList.find(masterc);
	// if not give warning and return
	if (it == counterList.end()) {
		lyxerr << "The master counter does not exist." << endl;
		return;
	}

	counterList[newc].setMaster(masterc);
}


void Counters::set(string const & ctr, int const val)
{
	CounterList::iterator const it = counterList.find(ctr);
	if (it == counterList.end()) {
		lyxerr << "set: Counter does not exist: " << ctr << endl;
		return;
	}
	it->second.set(val);
}


void Counters::addto(string const & ctr, int const val)
{
	CounterList::iterator const it = counterList.find(ctr);
	if (it == counterList.end()) {
		lyxerr << "addto: Counter does not exist: " << ctr << endl;
		return;
	}
	it->second.addto(val);
}


int Counters::value(string const & ctr) const
{
	CounterList::const_iterator const cit = counterList.find(ctr);
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
	CounterList::iterator const end = counterList.end();
	for (; it != end; ++it) {
		if (it->second.master() == ctr) {
			it->second.reset();
		}
	}
}


void Counters::reset()
{
	CounterList::iterator it = counterList.begin();
	CounterList::iterator const end = counterList.end();
	for (; it != end; ++it) {
		it->second.reset();
	}
}


void Counters::reset(string const & match)
{
	BOOST_ASSERT(!match.empty());

	CounterList::iterator it = counterList.begin();
	CounterList::iterator end = counterList.end();
	for (; it != end; ++it) {
		if (it->first.find(match) != string::npos)
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
		'à', 'á', 'â', 'ã', 'ä', 'å', 'æ', 'ç', 'è',
		'é', 'ë', 'ì', 'î', 'ð', 'ñ', 'ò', 'ô', 'ö',
		'÷', 'ø', 'ù', 'ú'
	};

	if (n < 1 || n > 22)
		return '?';
	return hebrew[n - 1];
}


string const lowerromanCounter(int const n)
{
	static char const * const roman[20] = {
		"i",   "ii",  "iii", "iv", "v",
		"vi",  "vii", "viii", "ix", "x",
		"xi",  "xii", "xiii", "xiv", "xv",
		"xvi", "xvii", "xviii", "xix", "xx"
	};

	if (n < 1 || n > 20)
		return "??";
	return roman[n - 1];
}


string const romanCounter(int const n)
{
	static char const * const roman[20] = {
		"I",   "II",  "III", "IV", "V",
		"VI",  "VII", "VIII", "IX", "X",
		"XI",  "XII", "XIII", "XIV", "XV",
		"XVI", "XVII", "XVIII", "XIX", "XX"
	};

	if (n < 1 || n > 20)
		return "??";
	return roman[n - 1];
}

} // namespace anon


string Counters::labelItem(string const & ctr, string const & numbertype)
{
	if (counterList.find(ctr) == counterList.end()) {
		lyxerr << "Counter " << ctr << " does not exist." << endl;
		return string();
	}

	if (numbertype == "hebrew")
		return string(1, hebrewCounter(value(ctr)));

	if (numbertype == "alph")
		return string(1, loweralphaCounter(value(ctr)));

	if (numbertype == "Alph")
		return string(1, alphaCounter(value(ctr)));

	if (numbertype == "roman")
		return lowerromanCounter(value(ctr));

	if (numbertype == "Roman")
		return romanCounter(value(ctr));

	return convert<string>(value(ctr));
}


string Counters::counterLabel(string const & format)
{
	string label = format;
	while (true) {
#ifdef WITH_WARNINGS
#warning Using boost::regex or boost::spirit would make this code a lot simpler... (Lgb)
#endif

		size_t const i = label.find('\\', 0);
		if (i == string::npos)
			break;
		size_t const j = label.find('{', i + 1);
		if (j == string::npos)
			break;
		size_t const k = label.find('}', j + 1);
		if (k == string::npos)
			break;
		string const numbertype(label, i + 1, j - i - 1);
		string const counter(label, j + 1, k - j - 1);
		string const rep = labelItem(counter, numbertype);
		label = string(label, 0, i) + rep + string(label, k + 1, string::npos);
		//lyxerr << "  : " << " (" << counter  << ","
		//	<< numbertype << ") -> " << label << endl;
	}
	//lyxerr << "counterLabel: " << format  << " -> "	<< label << endl;
	return label;
}


string Counters::enumLabel(string const & ctr, Buffer const & buf)
{
	string format;

	if (ctr == "enumi") {
		format = N_("\\arabic{enumi}.");
	}
 	else if (ctr == "enumii") {
		format = N_("(\\alph{enumii})");
	}
 	else if (ctr == "enumiii") { 
		format = N_("\\roman{enumiii}.");
	}
 	else if (ctr == "enumiv") {
		format = N_("\\Alph{enumiv}.");
	}
		
	return counterLabel(buf.B_(format));
}
