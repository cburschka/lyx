/**
 * \file math_extern.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

// This file contains most of the magic that extracts "context
// information" from the unstructered layout-oriented stuff in an
// MathArray.

#include <config.h>

#include "math_extern.h"
#include "math_arrayinset.h"
#include "math_charinset.h"
#include "math_deliminset.h"
#include "math_data.h"
#include "math_diffinset.h"
#include "math_exfuncinset.h"
#include "math_exintinset.h"
#include "math_fontinset.h"
#include "math_fracinset.h"
#include "math_liminset.h"
#include "math_matrixinset.h"
#include "math_mathmlstream.h"
#include "math_numberinset.h"
#include "math_scriptinset.h"
#include "math_stringinset.h"
#include "math_symbolinset.h"
#include "math_parser.h"
#include "debug.h"
#include "support/filetools.h"
#include "support/lstrings.h"
#include "frontends/controllers/ControlMath.h"

#include <algorithm>
#include <sstream>

using lyx::support::cmd_ret;
using lyx::support::getVectorFromString;
using lyx::support::LibFileSearch;
using lyx::support::RunCommand;
using lyx::support::subst;

using lyx::frontend::function_names;

using std::string;
using std::endl;
using std::find_if;
using std::auto_ptr;
using std::istringstream;
using std::ostream;
using std::ostringstream;
using std::swap;
using std::vector;


ostream & operator<<(ostream & os, MathArray const & ar)
{
	NormalStream ns(os);
	ns << ar;
	return os;
}


// define a function for tests
typedef bool TestItemFunc(MathAtom const &);

// define a function for replacing subexpressions
typedef MathAtom ReplaceArgumentFunc(const MathArray & ar);



// try to extract a super/subscript
// modify iterator position to point behind the thing
bool extractScript(MathArray & ar,
	MathArray::iterator & pos, MathArray::iterator last, bool superscript)
{
	// nothing to get here
	if (pos == last)
		return false;

	// is this a scriptinset?
	if (!(*pos)->asScriptInset())
		return false;

	// do we want superscripts only?
	if (superscript && !(*pos)->asScriptInset()->hasUp())
		return false;

	// it is a scriptinset, use it.
	ar.push_back(*pos);
	++pos;
	return true;
}


// try to extract an "argument" to some function.
// returns position behind the argument
MathArray::iterator extractArgument(MathArray & ar,
	MathArray::iterator pos, MathArray::iterator last, bool function = false)
{
	// nothing to get here
	if (pos == last)
		return pos;

	// something delimited _is_ an argument
	if ((*pos)->asDelimInset()) {
		// leave out delimiters if this is a function argument
		if (function) {
			MathArray const & arg = (*pos)->asDelimInset()->cell(0);
			MathArray::const_iterator cur = arg.begin();
			MathArray::const_iterator end = arg.end();
			while (cur != end)
				ar.push_back(*cur++);
		} else
			ar.push_back(*pos);
		++pos;
		if (pos == last)
			return pos;
		// if there's one, get following superscript only if this
		// isn't a function argument
		if (!function)
			extractScript(ar, pos, last, true);
		return pos;
	}

	// always take the first thing, no matter what it is
	ar.push_back(*pos);

	// go ahead if possible
	++pos;
	if (pos == last)
		return pos;

	// if the next item is a super/subscript, it most certainly belongs
	// to the thing we have
	extractScript(ar, pos, last, false);
	if (pos == last)
		return pos;

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


// returns sequence of char with same code starting at it up to end
// it might be less, though...
string charSequence
	(MathArray::const_iterator it, MathArray::const_iterator end)
{
	string s;
	for (; it != end && (*it)->asCharInset(); ++it)
		s += (*it)->getChar();
	return s;
}


void extractStrings(MathArray & ar)
{
	//lyxerr << "\nStrings from: " << ar << endl;
	for (size_t i = 0; i < ar.size(); ++i) {
		if (!ar[i]->asCharInset())
			continue;
		string s = charSequence(ar.begin() + i, ar.end());
		ar[i] = MathAtom(new MathStringInset(s));
		ar.erase(i + 1, i + s.size());
	}
	//lyxerr << "\nStrings to: " << ar << endl;
}


void extractMatrices(MathArray & ar)
{
	//lyxerr << "\nMatrices from: " << ar << endl;
	// first pass for explicitly delimited stuff
	for (size_t i = 0; i < ar.size(); ++i) {
		if (!ar[i]->asDelimInset())
			continue;
		MathArray const & arr = ar[i]->asDelimInset()->cell(0);
		if (arr.size() != 1)
			continue;
		if (!arr.front()->asGridInset())
			continue;
		ar[i] = MathAtom(new MathMatrixInset(*(arr.front()->asGridInset())));
	}

	// second pass for AMS "pmatrix" etc
	for (size_t i = 0; i < ar.size(); ++i)
		if (ar[i]->asAMSArrayInset())
			ar[i] = MathAtom(new MathMatrixInset(*(ar[i]->asGridInset())));
	//lyxerr << "\nMatrices to: " << ar << endl;
}


// convert this inset somehow to a string
bool extractString(MathAtom const & at, string & str)
{
	if (at->getChar()) {
		str = string(1, at->getChar());
		return true;
	}
	if (at->asStringInset()) {
		str = at->asStringInset()->str();
		return true;
	}
	return false;
}


// is this a known function?
bool isKnownFunction(string const & str)
{
	for (int i = 0; *function_names[i]; ++i) {
		if (str == function_names[i])
			return true;
	}
	return false;
}


// extract a function name from this inset
bool extractFunctionName(MathAtom const & at, string & str)
{
	if (at->asSymbolInset()) {
		str = at->asSymbolInset()->name();
		return isKnownFunction(str);
	}
	if (at->asUnknownInset()) {
		// assume it is well known...
		str = at->name();
		return true;
	}
	if (at->asFontInset() && at->name() == "mathrm") {
		// assume it is well known...
		MathArray const & ar = at->asFontInset()->cell(0);
		str = charSequence(ar.begin(), ar.end());
		return ar.size() == str.size();
	}
	return false;
}


// convert this inset somehow to a number
bool extractNumber(MathArray const & ar, int & i)
{
	istringstream is(charSequence(ar.begin(), ar.end()));
	is >> i;
	return is;
}


bool extractNumber(MathArray const & ar, double & d)
{
	istringstream is(charSequence(ar.begin(), ar.end()));
	is >> d;
	return is;
}


bool testString(MathAtom const & at, string const & str)
{
	string s;
	return extractString(at, s) && str == s;
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
		if (testOpen(*it))
			++level;
		if (testClose(*it))
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
	for (size_t i = 0; i < ar.size(); ++i) {
		// check whether this is the begin of the sequence
		if (!testOpen(ar[i]))
			continue;

		// search end of sequence
		MathArray::iterator it = ar.begin() + i;
		MathArray::iterator jt = endNestSearch(it, ar.end(), testOpen, testClose);
		if (jt == ar.end())
			continue;

		// replace the original stuff by the new inset
		ar[i] = replaceArg(MathArray(it + 1, jt));
		ar.erase(it + 1, jt + 1);
	}
}



//
// split scripts into seperate super- and subscript insets. sub goes in
// front of super...
//

void splitScripts(MathArray & ar)
{
	//lyxerr << "\nScripts from: " << ar << endl;
	for (size_t i = 0; i < ar.size(); ++i) {
		MathScriptInset const * script = ar[i]->asScriptInset();

		// is this a script inset and do we also have a superscript?
		if (!script || !script->hasUp())
			continue;

		// we must have a nucleus if we only have a superscript
		if (!script->hasDown() && script->nuc().size() == 0)
			continue;

		if (script->nuc().size() == 1) {
			// leave alone sums and integrals
			MathSymbolInset const * sym =
				script->nuc().front()->asSymbolInset();
			if (sym && (sym->name() == "sum" || sym->name() == "int"))
				continue;
		}

		// create extra script inset and move superscript over
		MathScriptInset * p = ar[i].nucleus()->asScriptInset();
		auto_ptr<MathScriptInset> q(new MathScriptInset(true));
		swap(q->up(), p->up());
		p->removeScript(true);

		// if we don't have a subscript, get rid of the ScriptInset
		if (!script->hasDown()) {
			MathArray arg(p->nuc());
			MathArray::const_iterator it = arg.begin();
			MathArray::const_iterator et = arg.end();
			ar.erase(i);
			while (it != et)
				ar.insert(i++, *it++);
		} else
			++i;

		// insert new inset behind
		ar.insert(i, MathAtom(q.release()));
	}
	//lyxerr << "\nScripts to: " << ar << endl;
}


//
// extract exp(...)
//

void extractExps(MathArray & ar)
{
	//lyxerr << "\nExps from: " << ar << endl;
	for (size_t i = 0; i + 1 < ar.size(); ++i) {
		// is this 'e'?
		if (ar[i]->getChar() != 'e')
			continue;

		// we need an exponent but no subscript
		MathScriptInset const * sup = ar[i + 1]->asScriptInset();
		if (!sup || sup->hasDown())
			continue;

		// create a proper exp-inset as replacement
		ar[i] = MathAtom(new MathExFuncInset("exp", sup->cell(1)));
		ar.erase(i + 1);
	}
	//lyxerr << "\nExps to: " << ar << endl;
}


//
// extract det(...)  from |matrix|
//
void extractDets(MathArray & ar)
{
	//lyxerr << "\ndet from: " << ar << endl;
	for (MathArray::iterator it = ar.begin(); it != ar.end(); ++it) {
		MathDelimInset const * del = (*it)->asDelimInset();
		if (!del)
			continue;
		if (!del->isAbs())
			continue;
		*it = MathAtom(new MathExFuncInset("det", del->cell(0)));
	}
	//lyxerr << "\ndet to: " << ar << endl;
}


//
// search numbers
//

bool isDigitOrSimilar(char c)
{
	return ('0' <= c && c <= '9') || c == '.';
}


// returns sequence of digits
string digitSequence
	(MathArray::const_iterator it, MathArray::const_iterator end)
{
	string s;
	for (; it != end && (*it)->asCharInset(); ++it) {
		if (!isDigitOrSimilar((*it)->getChar()))
			break;
		s += (*it)->getChar();
	}
	return s;
}


void extractNumbers(MathArray & ar)
{
	//lyxerr << "\nNumbers from: " << ar << endl;
	for (size_t i = 0; i < ar.size(); ++i) {
		if (!ar[i]->asCharInset())
			continue;
		if (!isDigitOrSimilar(ar[i]->asCharInset()->getChar()))
			continue;

		string s = digitSequence(ar.begin() + i, ar.end());

		ar[i] = MathAtom(new MathNumberInset(s));
		ar.erase(i + 1, i + s.size());
	}
	//lyxerr << "\nNumbers to: " << ar << endl;
}



//
// search delimiters
//

bool testOpenParen(MathAtom const & at)
{
	return testString(at, "(");
}


bool testCloseParen(MathAtom const & at)
{
	return testString(at, ")");
}


MathAtom replaceParenDelims(const MathArray & ar)
{
	return MathAtom(new MathDelimInset("(", ")", ar));
}


bool testOpenBracket(MathAtom const & at)
{
	return testString(at, "[");
}


bool testCloseBracket(MathAtom const & at)
{
	return testString(at, "]");
}


MathAtom replaceBracketDelims(const MathArray & ar)
{
	return MathAtom(new MathDelimInset("[", "]", ar));
}


// replace '('...')' and '['...']' sequences by a real MathDelimInset
void extractDelims(MathArray & ar)
{
	//lyxerr << "\nDelims from: " << ar << endl;
	replaceNested(ar, testOpenParen, testCloseParen, replaceParenDelims);
	replaceNested(ar, testOpenBracket, testCloseBracket, replaceBracketDelims);
	//lyxerr << "\nDelims to: " << ar << endl;
}



//
// search well-known functions
//


// replace 'f' '(...)' and 'f' '^n' '(...)' sequences by a real MathExFuncInset
// assume 'extractDelims' ran before
void extractFunctions(MathArray & ar)
{
	// we need at least two items...
	if (ar.size() < 2)
		return;

	//lyxerr << "\nFunctions from: " << ar << endl;
	for (size_t i = 0; i + 1 < ar.size(); ++i) {
		MathArray::iterator it = ar.begin() + i;
		MathArray::iterator jt = it + 1;

		string name;
		// is it a function?
		// it certainly is if it is well known...
		if (!extractFunctionName(*it, name)) {
			// is this a user defined function?
			// it it probably not, if it doesn't have a name.
			if (!extractString(*it, name))
				continue;
			// it is not if it has no argument
			if (jt == ar.end())
				continue;
			// guess so, if this is followed by
			// a DelimInset with a single item in the cell
			MathDelimInset const * del = (*jt)->asDelimInset();
			if (!del || del->cell(0).size() != 1)
				continue;
			// fall trough into main branch
		}

		// do we have an exponent like in
		// 'sin' '^2' 'x' -> 'sin(x)' '^2'
		MathArray exp;
		extractScript(exp, jt, ar.end(), true);

		// create a proper inset as replacement
		auto_ptr<MathExFuncInset> p(new MathExFuncInset(name));

		// jt points to the "argument". Get hold of this.
		MathArray::iterator st = extractArgument(p->cell(0), jt, ar.end(), true);

		// replace the function name by a real function inset
		*it = MathAtom(p.release());

		// remove the source of the argument from the array
		ar.erase(it + 1, st);

		// re-insert exponent
		ar.insert(i + 1, exp);
		//lyxerr << "\nFunctions to: " << ar << endl;
	}
}


//
// search integrals
//

bool testSymbol(MathAtom const & at, string const & name)
{
	return at->asSymbolInset() && at->asSymbolInset()->name() == name;
}


bool testIntSymbol(MathAtom const & at)
{
	return testSymbol(at, "int");
}


bool testIntegral(MathAtom const & at)
{
	return
	 testIntSymbol(at) ||
		( at->asScriptInset()
		  && at->asScriptInset()->nuc().size()
			&& testIntSymbol(at->asScriptInset()->nuc().back()) );
}



bool testIntDiff(MathAtom const & at)
{
	return testString(at, "d");
}


// replace '\int' ['_^'] x 'd''x'(...)' sequences by a real MathExIntInset
// assume 'extractDelims' ran before
void extractIntegrals(MathArray & ar)
{
	// we need at least three items...
	if (ar.size() < 3)
		return;

	//lyxerr << "\nIntegrals from: " << ar << endl;
	for (size_t i = 0; i + 1 < ar.size(); ++i) {
		MathArray::iterator it = ar.begin() + i;

		// search 'd'
		MathArray::iterator jt =
			endNestSearch(it, ar.end(), testIntegral, testIntDiff);

		// something sensible found?
		if (jt == ar.end())
			continue;

		// is this a integral name?
		if (!testIntegral(*it))
			continue;

		// core ist part from behind the scripts to the 'd'
		auto_ptr<MathExIntInset> p(new MathExIntInset("int"));

		// handle scripts if available
		if (!testIntSymbol(*it)) {
			p->cell(2) = (*it)->asScriptInset()->down();
			p->cell(3) = (*it)->asScriptInset()->up();
		}
		p->cell(0) = MathArray(it + 1, jt);

		// use the "thing" behind the 'd' as differential
		MathArray::iterator tt = extractArgument(p->cell(1), jt + 1, ar.end());

		// remove used parts
		ar.erase(it + 1, tt);
		*it = MathAtom(p.release());
	}
	//lyxerr << "\nIntegrals to: " << ar << endl;
}


bool testTermDelimiter(MathAtom const & at)
{
	return testString(at, "+") || testString(at, "-");
}


// try to extract a "term", i.e., something delimited by '+' or '-'.
// returns position behind the term
MathArray::iterator extractTerm(MathArray & ar,
	MathArray::iterator pos, MathArray::iterator last)
{
	while (pos != last && !testTermDelimiter(*pos)) {
		ar.push_back(*pos);
		++pos;
	}
	return pos;
}


//
// search sums
//


bool testEqualSign(MathAtom const & at)
{
	return testString(at, "=");
}


bool testSumSymbol(MathAtom const & p)
{
	return testSymbol(p, "sum");
}


bool testSum(MathAtom const & at)
{
	return
	 testSumSymbol(at) ||
		( at->asScriptInset()
		  && at->asScriptInset()->nuc().size()
			&& testSumSymbol(at->asScriptInset()->nuc().back()) );
}


// replace '\sum' ['_^'] f(x) sequences by a real MathExIntInset
// assume 'extractDelims' ran before
void extractSums(MathArray & ar)
{
	// we need at least two items...
	if (ar.size() < 2)
		return;

	//lyxerr << "\nSums from: " << ar << endl;
	for (size_t i = 0; i + 1 < ar.size(); ++i) {
		MathArray::iterator it = ar.begin() + i;

		// is this a sum name?
		if (!testSum(ar[i]))
			continue;

		// create a proper inset as replacement
		auto_ptr<MathExIntInset> p(new MathExIntInset("sum"));

		// collect lower bound and summation index
		MathScriptInset const * sub = ar[i]->asScriptInset();
		if (sub && sub->hasDown()) {
			// try to figure out the summation index from the subscript
			MathArray const & ar = sub->down();
			MathArray::const_iterator xt =
				find_if(ar.begin(), ar.end(), &testEqualSign);
			if (xt != ar.end()) {
				// we found a '=', use everything in front of that as index,
				// and everything behind as lower index
				p->cell(1) = MathArray(ar.begin(), xt);
				p->cell(2) = MathArray(xt + 1, ar.end());
			} else {
				// use everything as summation index, don't use scripts.
				p->cell(1) = ar;
			}
		}

		// collect upper bound
		if (sub && sub->hasUp())
			p->cell(3) = sub->up();

		// use something  behind the script as core
		MathArray::iterator tt = extractTerm(p->cell(0), it + 1, ar.end());

		// cleanup
		ar.erase(it + 1, tt);
		*it = MathAtom(p.release());
	}
	//lyxerr << "\nSums to: " << ar << endl;
}


//
// search differential stuff
//

// tests for 'd' or '\partial'
bool testDiffItem(MathAtom const & at)
{
	if (testString(at, "d") || testSymbol(at, "partial"))
		return true;

	// we may have d^n .../d and splitScripts() has not yet seen it
	MathScriptInset const * sup = at->asScriptInset();
	if (sup && !sup->hasDown() && sup->hasUp() && sup->nuc().size() == 1) {
		MathAtom const & ma = sup->nuc().front();
		return testString(ma, "d") || testSymbol(ma, "partial");
	}
	return false;
}


bool testDiffArray(MathArray const & ar)
{
	return ar.size() && testDiffItem(ar.front());
}


bool testDiffFrac(MathAtom const & at)
{
	return
		at->asFracInset()
			&& testDiffArray(at->asFracInset()->cell(0))
			&& testDiffArray(at->asFracInset()->cell(1));
}


void extractDiff(MathArray & ar)
{
	//lyxerr << "\nDiffs from: " << ar << endl;
	for (size_t i = 0; i < ar.size(); ++i) {
		MathArray::iterator it = ar.begin() + i;

		// is this a "differential fraction"?
		if (!testDiffFrac(*it))
			continue;

		MathFracInset const * f = (*it)->asFracInset();
		if (!f) {
			lyxerr << "should not happen" << endl;
			continue;
		}

		// create a proper diff inset
		auto_ptr<MathDiffInset> diff(new MathDiffInset);

		// collect function, let jt point behind last used item
		MathArray::iterator jt = it + 1;
		//int n = 1;
		MathArray numer(f->cell(0));
		splitScripts(numer);
		if (numer.size() > 1 && numer[1]->asScriptInset()) {
			// this is something like  d^n f(x) / d... or  d^n / d...
			// FIXME
			//n = 1;
			if (numer.size() > 2)
				diff->cell(0) = MathArray(numer.begin() + 2, numer.end());
			else
				jt = extractTerm(diff->cell(0), jt, ar.end());
		} else {
			// simply d f(x) / d... or  d/d...
			if (numer.size() > 1)
				diff->cell(0) = MathArray(numer.begin() + 1, numer.end());
			else
				jt = extractTerm(diff->cell(0), jt, ar.end());
		}

		// collect denominator parts
		MathArray denom(f->cell(1));
		splitScripts(denom);
		for (MathArray::iterator dt = denom.begin(); dt != denom.end();) {
			// find the next 'd'
			MathArray::iterator et
				= find_if(dt + 1, denom.end(), &testDiffItem);

			// point before this
			MathArray::iterator st = et - 1;
			MathScriptInset const * script = (*st)->asScriptInset();
			if (script && script->hasUp()) {
				// things like   d.../dx^n
				int mult = 1;
				if (extractNumber(script->up(), mult)) {
					//lyxerr << "mult: " << mult << endl;
					for (int i = 0; i < mult; ++i)
						diff->addDer(MathArray(dt + 1, st));
				}
			} else {
				// just  d.../dx
				diff->addDer(MathArray(dt + 1, et));
			}
			dt = et;
		}

		// cleanup
		ar.erase(it + 1, jt);
		*it = MathAtom(diff.release());
	}
	//lyxerr << "\nDiffs to: " << ar << endl;
}


//
// search limits
//


bool testRightArrow(MathAtom const & at)
{
	return testSymbol(at, "to") || testSymbol(at, "rightarrow");
}



// replace '\lim_{x->x0} f(x)' sequences by a real MathLimInset
// assume 'extractDelims' ran before
void extractLims(MathArray & ar)
{
	//lyxerr << "\nLimits from: " << ar << endl;
	for (size_t i = 0; i < ar.size(); ++i) {
		MathArray::iterator it = ar.begin() + i;

		// must be a script inset with a subscript (without superscript)
		MathScriptInset const * sub = (*it)->asScriptInset();
		if (!sub || !sub->hasDown() || sub->hasUp() || sub->nuc().size() != 1)
			continue;

		// is this a limit function?
		if (!testSymbol(sub->nuc().front(), "lim"))
			continue;

		// subscript must contain a -> symbol
		MathArray const & s = sub->down();
		MathArray::const_iterator st = find_if(s.begin(), s.end(), &testRightArrow);
		if (st == s.end())
			continue;

		// the -> splits the subscript int x and x0
		MathArray x  = MathArray(s.begin(), st);
		MathArray x0 = MathArray(st + 1, s.end());

		// use something behind the script as core
		MathArray f;
		MathArray::iterator tt = extractTerm(f, it + 1, ar.end());

		// cleanup
		ar.erase(it + 1, tt);

		// create a proper inset as replacement
		*it = MathAtom(new MathLimInset(f, x, x0));
	}
	//lyxerr << "\nLimits to: " << ar << endl;
}


//
// combine searches
//

void extractStructure(MathArray & ar)
{
	//lyxerr << "\nStructure from: " << ar << endl;
	splitScripts(ar);
	extractDelims(ar);
	extractIntegrals(ar);
	extractSums(ar);
	extractNumbers(ar);
	extractMatrices(ar);
	extractFunctions(ar);
	extractDets(ar);
	extractDiff(ar);
	extractExps(ar);
	extractLims(ar);
	extractStrings(ar);
	//lyxerr << "\nStructure to: " << ar << endl;
}


void write(MathArray const & dat, WriteStream & wi)
{
	MathArray ar = dat;
	extractStrings(ar);
	wi.firstitem() = true;
	for (MathArray::const_iterator it = ar.begin(); it != ar.end(); ++it) {
		(*it)->write(wi);
		wi.firstitem() = false;
	}
}


void normalize(MathArray const & ar, NormalStream & os)
{
	for (MathArray::const_iterator it = ar.begin(); it != ar.end(); ++it)
		(*it)->normalize(os);
}


void octave(MathArray const & dat, OctaveStream & os)
{
	MathArray ar = dat;
	extractStructure(ar);
	for (MathArray::const_iterator it = ar.begin(); it != ar.end(); ++it)
		(*it)->octave(os);
}


void maple(MathArray const & dat, MapleStream & os)
{
	MathArray ar = dat;
	extractStructure(ar);
	for (MathArray::const_iterator it = ar.begin(); it != ar.end(); ++it)
		(*it)->maple(os);
}


void maxima(MathArray const & dat, MaximaStream & os)
{
	MathArray ar = dat;
	extractStructure(ar);
	for (MathArray::const_iterator it = ar.begin(); it != ar.end(); ++it)
		(*it)->maxima(os);
}


void mathematica(MathArray const & dat, MathematicaStream & os)
{
	MathArray ar = dat;
	extractStructure(ar);
	for (MathArray::const_iterator it = ar.begin(); it != ar.end(); ++it)
		(*it)->mathematica(os);
}


void mathmlize(MathArray const & dat, MathMLStream & os)
{
	MathArray ar = dat;
	extractStructure(ar);
	if (ar.size() == 0)
		os << "<mrow/>";
	else if (ar.size() == 1)
		os << ar.front();
	else {
		os << MTag("mrow");
		for (MathArray::const_iterator it = ar.begin(); it != ar.end(); ++it)
			(*it)->mathmlize(os);
		os << ETag("mrow");
	}
}




namespace {

	string captureOutput(string const & cmd, string const & data)
	{
		string command =  "echo '" + data + "' | " + cmd;
		lyxerr << "calling: " << command << endl;
		cmd_ret const ret = RunCommand(command);
		return ret.second;
	}

	string::size_type get_matching_brace(string const & str, string::size_type i)
	{
		int count = 1;
		string::size_type n = str.size();
		while (i < n) {
			i = str.find_first_of("{}", i+1);
			if (i == string::npos) return i;
			if (str[i] == '{')
				++count;
			else
				--count;
			if (count == 0)
				return i;
		}
		return string::npos;
	}

	string::size_type get_matching_brace_back(string const & str, string::size_type i)
	{
		int count = 1;
		while (i > 0) {
			i = str.find_last_of("{}", i-1);
			if (i == string::npos) return i;
			if (str[i] == '}')
				++count;
			else
				--count;
			if (count == 0)
				return i;
		}
		return string::npos;
	}

	MathArray pipeThroughMaxima(string const &, MathArray const & ar)
	{
		ostringstream os;
		MaximaStream ms(os);
		ms << ar;
		string expr = os.str();
		string const header = "simpsum:true;";

		string out;
		for (int i = 0; i < 100; ++i) { // at most 100 attempts
			// try to fix missing '*' the hard way
			//
			// > echo "2x;" | maxima
			// ...
			// (C1) Incorrect syntax: x is not an infix operator
			// 2x;
			//  ^
			//
			lyxerr << "checking expr: '" << expr << "'" << endl;
			string full = header + "tex(" + expr + ");";
			out = captureOutput("maxima", full);

			// leave loop if expression syntax is probably ok
			if (out.find("Incorrect syntax") == string::npos)
				break;

			// search line with "Incorrect syntax"
			istringstream is(out);
			string line;
			while (is) {
				getline(is, line);
				if (line.find("Incorrect syntax") != string::npos)
					break;
			}

			// 2nd next line is the one with caret
			getline(is, line);
			getline(is, line);
			string::size_type pos = line.find('^');
			lyxerr << "found caret at pos: '" << pos << "'" << endl;
			if (pos == string::npos || pos < 4)
				break; // caret position not found
			pos -= 4; // skip the "tex(" part
			if (expr[pos] == '*')
				break; // two '*' in a row are definitely bad
			expr.insert(pos,  "*");
		}

		vector<string> tmp = getVectorFromString(out, "$$");
		if (tmp.size() < 2)
			return MathArray();

		out = subst(tmp[1],"\\>", "");
		lyxerr << "out: '" << out << "'" << endl;

		// Ugly code that tries to make the result prettier

		string::size_type i = out.find("\\mathchoice");
		while (i != string::npos) {
			string::size_type j = get_matching_brace(out, i + 12);
			string::size_type k = get_matching_brace(out, j + 1);
			k = get_matching_brace(out, k + 1);
			k = get_matching_brace(out, k + 1);
			string mid = out.substr(i + 13,j - i - 13);
			if (mid.find("\\over") != string::npos)
				mid = '{' + mid + '}';
			out = out.substr(0,i)
				+ mid
				+ out.substr(k + 1);
			//lyxerr << "out: " << out << endl;
			i = out.find("\\mathchoice", i);
			break;
		}

		i = out.find("\\over");
		while (i != string::npos) {
			string::size_type j = get_matching_brace_back(out, i - 1);
			if (j == string::npos || j == 0) break;
			string::size_type k = get_matching_brace(out, i + 5);
			if (k == string::npos || k + 1 == out.size()) break;
			out = out.substr(0,j - 1)
				+ "\\frac"
				+ out.substr(j,i - j)
				+ out.substr(i + 5,k - i - 4)
				+ out.substr(k + 2);
			//lyxerr << "out: " << out << endl;
			i = out.find("\\over", i + 4);
		}
		MathArray res;
		mathed_parse_cell(res, out);
		return res;
	}


	MathArray pipeThroughMaple(string const & extra, MathArray const & ar)
	{
		string header = "readlib(latex):\n";

		// remove the \\it for variable names
		//"#`latex/csname_font` := `\\it `:"
		header +=
			"`latex/csname_font` := ``:\n";

		// export matrices in (...) instead of [...]
		header +=
			"`latex/latex/matrix` := "
				"subs(`[`=`(`, `]`=`)`,"
					"eval(`latex/latex/matrix`)):\n";

		// replace \\cdots with proper '*'
		header +=
			"`latex/latex/*` := "
				"subs(`\\,`=`\\cdot `,"
					"eval(`latex/latex/*`)):\n";

		// remove spurious \\noalign{\\medskip} in matrix output
		header +=
			"`latex/latex/matrix`:= "
				"subs(`\\\\\\\\\\\\noalign{\\\\medskip}` = `\\\\\\\\`,"
					"eval(`latex/latex/matrix`)):\n";

		//"#`latex/latex/symbol` "
		//	" := subs((\\'_\\' = \\'`\\_`\\',eval(`latex/latex/symbol`)): ";

		string trailer = "quit;";
		ostringstream os;
		MapleStream ms(os);
		ms << ar;
		string expr = os.str();
		lyxerr << "ar: '" << ar << "'\n"
		       << "ms: '" << os.str() << "'" << endl;

		for (int i = 0; i < 100; ++i) { // at most 100 attempts
			// try to fix missing '*' the hard way by using mint
			//
			// ... > echo "1A;" | mint -i 1 -S -s -q
			// on line     1: 1A;
			//                 ^ syntax error -
			//                   Probably missing an operator such as * p
			//
			lyxerr << "checking expr: '" << expr << "'" << endl;
			string out = captureOutput("mint -i 1 -S -s -q -q", expr + ';');
			if (out.empty())
				break; // expression syntax is ok
			istringstream is(out);
			string line;
			getline(is, line);
			if (line.find("on line") != 0)
				break; // error message not identified
			getline(is, line);
			string::size_type pos = line.find('^');
			if (pos == string::npos || pos < 15)
				break; // caret position not found
			pos -= 15; // skip the "on line ..." part
			if (expr[pos] == '*' || (pos > 0 && expr[pos - 1] == '*'))
				break; // two '*' in a row are definitely bad
			expr.insert(pos, 1, '*');
		}

		string full = "latex(" +  extra + '(' + expr + "));";
		string out = captureOutput("maple -q", header + full + trailer);

		// change \_ into _

		//
		MathArray res;
		mathed_parse_cell(res, out);
		return res;
	}


	MathArray pipeThroughOctave(string const &, MathArray const & ar)
	{
		ostringstream os;
		OctaveStream vs(os);
		vs << ar;
		string expr = os.str();
		string out;

		lyxerr << "pipe: ar: '" << ar << "'\n"
		       << "pipe: expr: '" << expr << "'" << endl;

		for (int i = 0; i < 100; ++i) { // at most 100 attempts
			//
			// try to fix missing '*' the hard way
			// parse error:
			// >>> ([[1 2 3 ];[2 3 1 ];[3 1 2 ]])([[1 2 3 ];[2 3 1 ];[3 1 2 ]])
			//                                   ^
			//
			lyxerr << "checking expr: '" << expr << "'" << endl;
			out = captureOutput("octave -q 2>&1", expr);
			lyxerr << "checking out: '" << out << "'" << endl;

			// leave loop if expression syntax is probably ok
			if (out.find("parse error:") == string::npos)
				break;

			// search line with single caret
			istringstream is(out);
			string line;
			while (is) {
				getline(is, line);
				lyxerr << "skipping line: '" << line << "'" << endl;
				if (line.find(">>> ") != string::npos)
					break;
			}

			// found line with error, next line is the one with caret
			getline(is, line);
			string::size_type pos = line.find('^');
			lyxerr << "caret line: '" << line << "'" << endl;
			lyxerr << "found caret at pos: '" << pos << "'" << endl;
			if (pos == string::npos || pos < 4)
				break; // caret position not found
			pos -= 4; // skip the ">>> " part
			if (expr[pos] == '*')
				break; // two '*' in a row are definitely bad
			expr.insert(pos, 1, '*');
		}

		// remove 'ans = ' taking into account that there may be an
		// ansi control sequence before, such as '\033[?1034hans = '
		size_t i = out.find("ans = ");
		if (i == string::npos)
			return MathArray();
		out = out.substr(i + 6);

		// parse output as matrix or single number
		MathAtom at(new MathArrayInset("array", out));
		MathArrayInset const * mat = at->asArrayInset();
		MathArray res;
		if (mat->ncols() == 1 && mat->nrows() == 1)
			res.append(mat->cell(0));
		else {
			res.push_back(MathAtom(new MathDelimInset("(", ")")));
			res.back().nucleus()->cell(0).push_back(at);
		}
		return res;
	}


	string fromMathematicaName(string const & name)
	{
		if (name == "Sin")    return "sin";
		if (name == "Sinh")   return "sinh";
		if (name == "ArcSin") return "arcsin";
		if (name == "Cos")    return "cos";
		if (name == "Cosh")   return "cosh";
		if (name == "ArcCos") return "arccos";
		if (name == "Tan")    return "tan";
		if (name == "Tanh")   return "tanh";
		if (name == "ArcTan") return "arctan";
		if (name == "Cot")    return "cot";
		if (name == "Coth")   return "coth";
		if (name == "Csc")    return "csc";
		if (name == "Sec")    return "sec";
		if (name == "Exp")    return "exp";
		if (name == "Log")    return "log";
		if (name == "Arg" )   return "arg";
		if (name == "Det" )   return "det";
		if (name == "GCD" )   return "gcd";
		if (name == "Max" )   return "max";
		if (name == "Min" )   return "min";
		if (name == "Erf" )   return "erf";
		if (name == "Erfc" )  return "erfc";
		return name;
	}


	void prettifyMathematicaOutput(string & out, string const & macroName,
			bool roman, bool translate)
	{
		string const macro = "\\" + macroName + "{";
		string::size_type const len = macro.length();
		string::size_type i = out.find(macro);

		while (i != string::npos) {
			string::size_type const j = get_matching_brace(out, i + len);
			string const name = out.substr(i + len, j - i - len);
			out = out.substr(0, i)
				+ (roman ? "\\mathrm{" : "")
				+ (translate ? fromMathematicaName(name) : name)
				+ out.substr(roman ? j : j + 1);
			//lyxerr << "out: " << out << endl;
			i = out.find(macro, i);
		}
	}


	MathArray pipeThroughMathematica(string const &, MathArray const & ar)
	{
		ostringstream os;
		MathematicaStream ms(os);
		ms << ar;
		string const expr = os.str();
		string out;

		lyxerr << "expr: '" << expr << "'" << endl;

		string const full = "TeXForm[" + expr + "]";
		out = captureOutput("math", full);
		lyxerr << "out: '" << out << "'" << endl;

		string::size_type pos1 = out.find("Out[1]//TeXForm= ");
		string::size_type pos2 = out.find("In[2]:=");

		if (pos1 == string::npos || pos2 == string::npos)
			return MathArray();

		// get everything from pos1+17 to pos2
		out = out.substr(pos1 + 17, pos2 - pos1 - 17);
		out = subst(subst(out, '\r', ' '), '\n', ' ');

		// tries to make the result prettier
		prettifyMathematicaOutput(out, "Mfunction", true, true);
		prettifyMathematicaOutput(out, "Muserfunction", true, false);
		prettifyMathematicaOutput(out, "Mvariable", false, false);

		MathArray res;
		mathed_parse_cell(res, out);
		return res;
	}

}


MathArray pipeThroughExtern(string const & lang, string const & extra,
	MathArray const & ar)
{
	if (lang == "octave")
		return pipeThroughOctave(extra, ar);

	if (lang == "maxima")
		return pipeThroughMaxima(extra, ar);

	if (lang == "maple")
		return pipeThroughMaple(extra, ar);

	if (lang == "mathematica")
		return pipeThroughMathematica(extra, ar);

	// create normalized expression
	ostringstream os;
	NormalStream ns(os);
	os << '[' << extra << ' ';
	ns << ar;
	os << ']';
	string data = os.str();

	// search external script
	string file = LibFileSearch("mathed", "extern_" + lang);
	if (file.empty()) {
		lyxerr << "converter to '" << lang << "' not found" << endl;
		return MathArray();
	}

	// run external sript
	string out = captureOutput(file, data);
	MathArray res;
	mathed_parse_cell(res, out);
	return res;
}
