
// This file contains most of the magic that extracts "context
// information" from the unstructered layout-oriented stuff in an
// MathArray.

#include <algorithm>

#include "math_charinset.h"
#include "math_deliminset.h"
#include "math_diffinset.h"
#include "math_exfuncinset.h"
#include "math_exintinset.h"
#include "math_funcinset.h"
#include "math_fracinset.h"
#include "math_matrixinset.h"
#include "math_mathmlstream.h"
#include "math_scriptinset.h"
#include "math_stringinset.h"
#include "math_symbolinset.h"
#include "debug.h"


std::ostream & operator<<(std::ostream & os, MathArray const & ar)
{
	NormalStream ns(os);	
	ns << ar;
	return os;
}


// define a function for tests
typedef bool TestItemFunc(MathInset *);

// define a function for replacing subexpressions
typedef MathInset * ReplaceArgumentFunc(const MathArray & ar);



// try to extract an "argument" to some function.
// returns position behind the argument
MathArray::iterator extractArgument(MathArray & ar,
	MathArray::iterator pos, MathArray::iterator last, string const & = "")
{
	// nothing to get here
	if (pos == last)
		return pos;

	// something deliminited _is_ an argument
	if ((*pos)->asDelimInset()) {
		ar.push_back(*pos);
		return pos + 1;
	}

	// always take the first thing, no matter what it is
	ar.push_back(*pos);

	// go ahead if possible
	++pos;
	if (pos == last)
		return pos;

	// if the next item is a subscript, it most certainly belongs to the
	// thing we have
	if ((*pos)->asScriptInset()) {
		ar.push_back(*pos);
		// go ahead if possible
		++pos;
		if (pos == last)
			return pos;
	}

	// but it might be more than that.
	// FIXME: not implemented
	//for (MathArray::iterator it = pos + 1; it != last; ++it) {
	//	// always take the first thing, no matter
	//	if (it == pos) {
	//		ar.push_back(*it);
	//		continue;
	//	}
	//}
	return pos;
}


MathScriptInset const * asScript(MathArray::const_iterator it)
{
	if (it->nucleus()->asScriptInset())
		return 0;
	++it;
	if (!it->nucleus())
		return 0;
	return it->nucleus()->asScriptInset();
}



// returns sequence of char with same code starting at it up to end
// it might be less, though...
string charSequence(MathArray::const_iterator it, MathArray::const_iterator end)
{
	string s;
	MathCharInset const * p = it->nucleus()->asCharInset();
	if (p) {
		for (MathTextCodes c = p->code(); it != end; ++it) {
			p = it->nucleus()->asCharInset();
			if (!p || p->code() != c)
				break;
			s += p->getChar();
		}
	}
	return s;
}


void extractStrings(MathArray & dat)
{
	//lyxerr << "\nStrings from: " << ar << "\n";
	MathArray ar;
	MathArray::const_iterator it = dat.begin();
	while (it != dat.end()) {
		if (it->nucleus() && it->nucleus()->asCharInset()) {
			string s = charSequence(it, dat.end());
			MathTextCodes c = it->nucleus()->asCharInset()->code();
			ar.push_back(MathAtom(new MathStringInset(s, c)));
			it += s.size();
		} else {
			ar.push_back(*it);
			++it;
		}
	}
	ar.swap(dat);
	//lyxerr << "\nStrings to: " << ar << "\n";
}


MathInset * singleItem(MathArray & ar)
{
	return ar.size() == 1 ? ar.begin()->nucleus() : 0;
}


void extractMatrices(MathArray & ar)
{
	lyxerr << "\nMatrices from: " << ar << "\n";
	for (MathArray::iterator it = ar.begin(); it != ar.end(); ++it) {
		MathDelimInset * del = (*it)->asDelimInset();
		if (!del)
			continue;
		MathInset * arr = singleItem(del->cell(0));
		if (!arr || !arr->asArrayInset())
			continue;
		*it = MathAtom(new MathMatrixInset(*(arr->asArrayInset())));
	}
	lyxerr << "\nMatrices to: " << ar << "\n";
}


// convert this inset somehow to a string
string extractString(MathInset * p)
{
	if (p && p->getChar())
		return string(1, p->getChar());
	if (p && p->asStringInset())
		return p->asStringInset()->str();
	return string();
}


bool stringTest(MathInset * p, const string & str)
{
	return extractString(p) == str;
}


// search end of nested sequence
MathArray::iterator endNestSearch(
	MathArray::iterator it,
	MathArray::iterator last,
	TestItemFunc testOpen,
	TestItemFunc testClose
)
{
	for (int level = 0; it != last; ++it) {
		if (testOpen(it->nucleus()))
			++level;
		if (testClose(it->nucleus()))
			--level;
		if (level == 0)
			break;
	}
	return it;
}


// replace nested sequences by a real Insets
void replaceNested(
	MathArray & ar,
	TestItemFunc testOpen,
	TestItemFunc testClose,
	ReplaceArgumentFunc replaceArg
)
{
	// use indices rather than iterators for the loop  because we are going
	// to modify the array.
	for (MathArray::size_type i = 0; i < ar.size(); ++i) {
		// check whether this is the begin of the sequence
		MathArray::iterator it = ar.begin() + i;
		if (!testOpen(it->nucleus()))
			continue;

		// search end of sequence
		MathArray::iterator jt = endNestSearch(it, ar.end(), testOpen, testClose);
		if (jt == ar.end())
			continue;

		// create a proper inset as replacement
		MathInset * p = replaceArg(MathArray(it + 1, jt));

		// replace the original stuff by the new inset
		ar.erase(it + 1, jt + 1);
		(*it).reset(p);
	}
} 


//
// search deliminiters
//

bool openParanTest(MathInset * p)
{
	return stringTest(p, "(");
}


bool closeParanTest(MathInset * p)
{
	return stringTest(p, ")");
}


MathInset * delimReplacement(const MathArray & ar)
{
	MathDelimInset * del = new MathDelimInset("(", ")");
	del->cell(0) = ar;
	return del;
}


// replace '('...')' sequences by a real MathDelimInset
void extractDelims(MathArray & ar)
{
	lyxerr << "\nDelims from: " << ar << "\n";
	replaceNested(ar, openParanTest, closeParanTest, delimReplacement);
	lyxerr << "\nDelims to: " << ar << "\n";
}



//
// search well-known functions
//


// replace 'f' '(...)' and 'f' '^n' '(...)' sequences by a real MathExFuncInset
// assume 'extractDelims' ran before
void extractFunctions(MathArray & ar)
{
	// we need at least two items...
	if (ar.size() <= 1)
		return;

	lyxerr << "\nFunctions from: " << ar << "\n";
	for (MathArray::size_type i = 0; i + 1 < ar.size(); ++i) {
		MathArray::iterator it = ar.begin() + i;

		// is this a well known function name?
		MathFuncInset * func = (*it)->asFuncInset();
		string name;
		if (func) 
			name = func->name();
		else {
			// is this a user defined function?
			// guess so, if this is a "string" and it is followed by
			// a DelimInset
			//name = extractString((*it)->nucleus());
			//if (name.size() && it + 1
			//if ((*it
			// FIXME
			continue;
		}	

		// do we have an exponent?
		// simply skippping the postion does the right thing:
		// 'sin' '^2' 'x' -> 'sin(x)' '^2'
		MathArray::iterator jt = it + 1;
		if (MathScriptInset * script = (*jt)->asScriptInset()) {
			// allow superscripts only
			if (script->hasDown())
				continue;
			++jt;
			if (jt == ar.end())
				continue;
		}
	
		// create a proper inset as replacement
		MathExFuncInset * p = new MathExFuncInset(name);

		// jt points to the "argument". Get hold of this.
		MathArray::iterator st = extractArgument(p->cell(0), jt, ar.end());

		// replace the function name by a real function inset
		(*it).reset(p);
		
		// remove the source of the argument from the array
		ar.erase(jt, st);
		lyxerr << "\nFunctions to: " << ar << "\n";
	}
} 


//
// search integrals
//

bool symbolTest(MathInset * p, string const & name)
{
	return p->asSymbolInset() && p->asSymbolInset()->name() == name;
}


bool intSymbolTest(MathInset * p)
{
	return symbolTest(p, "int");
}


bool intDiffTest(MathInset * p)
{
	return stringTest(p, "d");
}


// replace '\int' ['_^'] x 'd''x'(...)' sequences by a real MathExIntInset
// assume 'extractDelims' ran before
void extractIntegrals(MathArray & ar)
{
	// we need at least three items...
	if (ar.size() <= 2)
		return;

	lyxerr << "\nIntegrals from: " << ar << "\n";
	for (MathArray::size_type i = 0; i + 1< ar.size(); ++i) {
		MathArray::iterator it = ar.begin() + i;

		// is this a integral name?
		if (!intSymbolTest(it->nucleus()))
			continue;

		// search 'd'
		MathArray::iterator jt =
			endNestSearch(it, ar.end(), intSymbolTest, intDiffTest);

		// something sensible found?
		if (jt == ar.end())
			continue;

		// create a proper inset as replacement
		MathExIntInset * p = new MathExIntInset("int");

		// collect scripts
		MathArray::iterator st = it + 1;
		if ((*st)->asScriptInset()) {
			p->scripts(*st);
			p->cell(0) = MathArray(st + 1, jt);
		} else {
			p->cell(0) = MathArray(st, jt);
		}

		// use the atom behind the 'd' as differential
		MathArray::iterator tt = extractArgument(p->cell(1), jt + 1, ar.end());
		
		// remove used parts
		ar.erase(it + 1, tt);
		(*it).reset(p);
	}
	lyxerr << "\nIntegrals to: " << ar << "\n";
}


//
// search sums
//

bool sumSymbolTest(MathInset * p)
{
	return p->asSymbolInset() && p->asSymbolInset()->name() == "sum";
}


bool equalSign(MathInset * p)
{
	return stringTest(p, "=");
}


bool equalSign1(MathAtom const & at)
{
	return equalSign(at.nucleus());
}



// replace '\sum' ['_^'] f(x) sequences by a real MathExIntInset
// assume 'extractDelims' ran before
void extractSums(MathArray & ar)
{
	// we need at least two items...
	if (ar.size() <= 1)
		return;

	lyxerr << "\nSums from: " << ar << "\n";
	for (MathArray::size_type i = 0; i + 1< ar.size(); ++i) {
		MathArray::iterator it = ar.begin() + i;

		// is this a sum name?
		if (!sumSymbolTest(it->nucleus()))
			continue;

		// create a proper inset as replacement
		MathExIntInset * p = new MathExIntInset("sum");

		// collect scripts
		MathArray::iterator st = it + 1;
		if (st != ar.end() && (*st)->asScriptInset()) {
			p->scripts(*st);
			++st;

			// try to figure out the summation index from the subscript
			MathScriptInset * script = p->scripts()->asScriptInset();
			if (script->hasDown()) {
				MathArray & ar = script->down().data_;
				MathArray::iterator it =
					std::find_if(ar.begin(), ar.end(), &equalSign1);
				if (it != ar.end()) {
					// we found a '=', use everything in front of that as index,
					// and everything behind as start value
					p->cell(1) = MathArray(ar.begin(), it);
					ar.erase(ar.begin(), it + 1);
				} else {
					// use everything as summation index, don't use scripts.
					p->cell(1) = ar;
				}
			}
		}

		// use some  behind the script as core
		MathArray::iterator tt = extractArgument(p->cell(0), st, ar.end());

		// cleanup
		ar.erase(it + 1, tt);
		(*it).reset(p);
	}
	lyxerr << "\nSums to: " << ar << "\n";
}


//
// search differential stuff
//

// tests for 'd' or '\partial'
bool diffItemTest(MathInset * p)
{
	return stringTest(p, "d");
}


bool diffItemTest(MathArray const & ar)
{
	return ar.size() && diffItemTest(ar.front().nucleus());
}


bool diffFracTest(MathInset * p)
{
	return
		p->asFracInset() &&
		diffItemTest(p->asFracInset()->cell(0)) &&
		diffItemTest(p->asFracInset()->cell(1));
}

void extractDiff(MathArray & ar)
{
	lyxerr << "\nDiffs from: " << ar << "\n";
	for (MathArray::size_type i = 0; i < ar.size(); ++i) {
		MathArray::iterator it = ar.begin() + i;

		// is this a "differential fraction"?
		if (!diffFracTest(it->nucleus()))
			continue;
		
		MathFracInset * f = (*it)->asFracInset();
		if (!f) {
			lyxerr << "should not happen\n";
			continue;
		}

		// create a proper diff inset
		MathDiffInset * p = new MathDiffInset;

		// collect function, let jt point behind last used item
		MathArray::iterator jt = it + 1; 
		int n = 1; 
		MathArray & numer = f->cell(0);
		if (numer.size() > 1 && numer.at(1)->asScriptInset()) {
			// this is something like  d^n f(x) / d... or  d^n / d...
			n = 1; // FIXME
			if (numer.size() > 2) 
				p->cell(0) = MathArray(numer.begin() + 2, numer.end());
			else
				jt = extractArgument(p->cell(0), jt, ar.end());
		} else {
			// simply d f(x) / d... or  d/d...
			if (numer.size() > 1) 
				p->cell(0) = MathArray(numer.begin() + 1, numer.end());
			else
				jt = extractArgument(p->cell(0), jt, ar.end());
		}

		// collect denominator
		MathArray & denom = f->cell(1);
		for (MathArray::iterator dt = denom.begin(); dt + 1 != denom.end(); ) {
			if (!diffItemTest((*dt).nucleus())) {
				lyxerr << "extractDiff: should not happen 2\n";
				return;
			}
			MathArray diff;
			dt = extractArgument(diff, dt + 1, denom.end());
			p->addDer(diff);
			// safeguard
			if (dt == denom.end()) 
				break;
		}

		// cleanup
		ar.erase(it + 1, jt);
		(*it).reset(p);
	}
	lyxerr << "\nDiffs to: " << ar << "\n";
}

//
// combine searches
//

void extractStructure(MathArray & ar)
{
	extractMatrices(ar);
	extractDelims(ar);
	extractFunctions(ar);
	extractIntegrals(ar);
	extractSums(ar);
	extractDiff(ar);
	extractStrings(ar);
}


void write(MathArray const & dat, WriteStream & wi)
{
	MathArray ar = dat;
	extractStrings(ar);
	for (MathArray::const_iterator it = ar.begin(); it != ar.end(); ++it) {
		wi.firstitem = (it == ar.begin());
		MathInset const * p = it->nucleus();
		if (it + 1 != ar.end()) {
			if (MathScriptInset const * q = asScript(it)) {
				q->write(p, wi);
				++it;
				continue;
			} 
		}
		p->write(wi);
	}
}


void normalize(MathArray const & ar, NormalStream & os)
{
	for (MathArray::const_iterator it = ar.begin(); it != ar.end(); ++it)
		(*it)->normalize(os);
}


void octavize(MathArray const & dat, OctaveStream & os)
{
	MathArray ar = dat;
	extractStructure(ar);
	for (MathArray::const_iterator it = ar.begin(); it != ar.end(); ++it) {
		MathInset const * p = it->nucleus();
		if (it + 1 != ar.end()) {
			if (MathScriptInset const * q = asScript(it)) {
				q->octavize(p, os);
				++it;	
				continue;
			}
		}
		p->octavize(os);
	}
}


void maplize(MathArray const & dat, MapleStream & os)
{
	MathArray ar = dat;
	extractStructure(ar);
	for (MathArray::const_iterator it = ar.begin(); it != ar.end(); ++it) {
		MathInset const * p = it->nucleus();
		if (it + 1 != ar.end()) {
			if (MathScriptInset const * q = asScript(it)) {
				q->maplize(p, os);
				++it;	
				continue;
			}
		}
		p->maplize(os);
	}
}


void mathmlize(MathArray const & dat, MathMLStream & os)
{
	MathArray ar = dat;
	extractStructure(ar);
	if (ar.size() == 0)
		os << "<mrow/>";
	else if (ar.size() == 1)
		os << ar.begin()->nucleus();
	else {
		os << MTag("mrow");
		for (MathArray::const_iterator it = ar.begin(); it != ar.end(); ++it) {
			MathInset const * p = it->nucleus();
			if (it + 1 != ar.end()) {
				if (MathScriptInset const * q = asScript(it)) {
					q->mathmlize(p, os);
					++it;	
					continue;
				}
			}
			p->mathmlize(os);
		}
		os << ETag("mrow");
	}
}

