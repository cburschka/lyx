/**
 * \file MathExtern.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

// This file contains most of the magic that extracts "context
// information" from the unstructered layout-oriented stuff in
// MathData.

#include <config.h>

#include "MathExtern.h"

#include "InsetMathAMSArray.h"
#include "InsetMathArray.h"
#include "InsetMathChar.h"
#include "InsetMathDelim.h"
#include "InsetMathDiff.h"
#include "InsetMathExFunc.h"
#include "InsetMathExInt.h"
#include "InsetMathFont.h"
#include "InsetMathFrac.h"
#include "InsetMathLim.h"
#include "InsetMathMatrix.h"
#include "InsetMathNumber.h"
#include "InsetMathScript.h"
#include "InsetMathString.h"
#include "InsetMathSymbol.h"
#include "MathData.h"
#include "MathParser.h"
#include "MathStream.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/FileName.h"
#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/TempFile.h"

#include <algorithm>
#include <sstream>
#include <fstream>
#include <memory>

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace {

enum ExternalMath {
	HTML,
	MAPLE,
	MAXIMA,
	MATHEMATICA,
	MATHML,
	OCTAVE
};


static char const * function_names[] = {
	"arccos", "arcsin", "arctan", "arg", "bmod",
	"cos", "cosh", "cot", "coth", "csc", "deg",
	"det", "dim", "exp", "gcd", "hom", "inf", "ker",
	"lg", "lim", "liminf", "limsup", "ln", "log",
	"max", "min", "sec", "sin", "sinh", "sup",
	"tan", "tanh", "Pr", 0
};

static size_t const npos = lyx::docstring::npos;

// define a function for tests
typedef bool TestItemFunc(MathAtom const &);

// define a function for replacing subexpressions
typedef MathAtom ReplaceArgumentFunc(const MathData & ar);



// try to extract a super/subscript
// modify iterator position to point behind the thing
bool extractScript(MathData & ar,
	MathData::iterator & pos, MathData::iterator last, bool superscript)
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
MathData::iterator extractArgument(MathData & ar,
	MathData::iterator pos, MathData::iterator last, 
	ExternalMath kind, bool function = false)
{
	// nothing to get here
	if (pos == last)
		return pos;

	// something delimited _is_ an argument
	if ((*pos)->asDelimInset()) {
		// leave out delimiters if this is a function argument
		// unless we are doing MathML, in which case we do want
		// the delimiters
		if (function && kind != MATHML && kind != HTML) {
			MathData const & arg = (*pos)->asDelimInset()->cell(0);
			MathData::const_iterator cur = arg.begin();
			MathData::const_iterator end = arg.end();
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
	//for (MathData::iterator it = pos + 1; it != last; ++it) {
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
docstring charSequence
	(MathData::const_iterator it, MathData::const_iterator end)
{
	docstring s;
	for (; it != end && (*it)->asCharInset(); ++it)
		s += (*it)->getChar();
	return s;
}


void extractStrings(MathData & ar)
{
	//lyxerr << "\nStrings from: " << ar << endl;
	for (size_t i = 0; i < ar.size(); ++i) {
		if (!ar[i]->asCharInset())
			continue;
		docstring s = charSequence(ar.begin() + i, ar.end());
		ar[i] = MathAtom(new InsetMathString(s));
		ar.erase(i + 1, i + s.size());
	}
	//lyxerr << "\nStrings to: " << ar << endl;
}


void extractMatrices(MathData & ar)
{
	//lyxerr << "\nMatrices from: " << ar << endl;
	// first pass for explicitly delimited stuff
	for (size_t i = 0; i < ar.size(); ++i) {
		InsetMathDelim const * const inset = ar[i]->asDelimInset();
		if (!inset)
			continue;
		MathData const & arr = inset->cell(0);
		if (arr.size() != 1)
			continue;
		if (!arr.front()->asGridInset())
			continue;
		ar[i] = MathAtom(new InsetMathMatrix(*(arr.front()->asGridInset()), 
		                 inset->left_, inset->right_));
	}

	// second pass for AMS "pmatrix" etc
	for (size_t i = 0; i < ar.size(); ++i) {
		InsetMathAMSArray const * const inset = ar[i]->asAMSArrayInset();
		if (inset) {
			string left = inset->name_left();
			if (left == "Vert")
				left = "[";
			string right = inset->name_right();
			if (right == "Vert")
				right = "]";
			ar[i] = MathAtom(new InsetMathMatrix(*inset, from_ascii(left), from_ascii(right)));
		}
	}
	//lyxerr << "\nMatrices to: " << ar << endl;
}


// convert this inset somehow to a string
bool extractString(MathAtom const & at, docstring & str)
{
	if (at->getChar()) {
		str = docstring(1, at->getChar());
		return true;
	}
	if (at->asStringInset()) {
		str = at->asStringInset()->str();
		return true;
	}
	return false;
}


// is this a known function?
bool isKnownFunction(docstring const & str)
{
	for (int i = 0; function_names[i]; ++i) {
		if (str == function_names[i])
			return true;
	}
	return false;
}


// extract a function name from this inset
bool extractFunctionName(MathAtom const & at, docstring & str)
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
		MathData const & ar = at->asFontInset()->cell(0);
		str = charSequence(ar.begin(), ar.end());
		return ar.size() == str.size();
	}
	return false;
}


bool testString(MathAtom const & at, docstring const & str)
{
	docstring s;
	return extractString(at, s) && str == s;
}


bool testString(MathAtom const & at, char const * const str)
{
	return testString(at, from_ascii(str));
}

// search end of nested sequence
MathData::iterator endNestSearch(
	MathData::iterator it,
	MathData::iterator last,
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
	MathData & ar,
	TestItemFunc testOpen,
	TestItemFunc testClose,
	ReplaceArgumentFunc replaceArg)
{
	Buffer * buf = ar.buffer();
	// use indices rather than iterators for the loop  because we are going
	// to modify the array.
	for (size_t i = 0; i < ar.size(); ++i) {
		// check whether this is the begin of the sequence
		if (!testOpen(ar[i]))
			continue;

		// search end of sequence
		MathData::iterator it = ar.begin() + i;
		MathData::iterator jt = endNestSearch(it, ar.end(), testOpen, testClose);
		if (jt == ar.end())
			continue;

		// replace the original stuff by the new inset
		ar[i] = replaceArg(MathData(buf, it + 1, jt));
		ar.erase(it + 1, jt + 1);
	}
}



//
// split scripts into seperate super- and subscript insets. sub goes in
// front of super...
//

void splitScripts(MathData & ar)
{
	Buffer * buf = ar.buffer();
	//lyxerr << "\nScripts from: " << ar << endl;
	for (size_t i = 0; i < ar.size(); ++i) {
		InsetMathScript const * script = ar[i]->asScriptInset();

		// is this a script inset and do we also have a superscript?
		if (!script || !script->hasUp())
			continue;

		// we must have a nucleus if we only have a superscript
		if (!script->hasDown() && script->nuc().empty())
			continue;

		if (script->nuc().size() == 1) {
			// leave alone sums and integrals
			InsetMathSymbol const * sym =
				script->nuc().front()->asSymbolInset();
			if (sym && (sym->name() == "sum" || sym->name() == "int"))
				continue;
		}

		// create extra script inset and move superscript over
		InsetMathScript * p = ar[i].nucleus()->asScriptInset();
		auto_ptr<InsetMathScript> q(new InsetMathScript(buf, true));
		swap(q->up(), p->up());
		p->removeScript(true);

		// if we don't have a subscript, get rid of the ScriptInset
		if (!script->hasDown()) {
			MathData arg(p->nuc());
			MathData::const_iterator it = arg.begin();
			MathData::const_iterator et = arg.end();
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

void extractExps(MathData & ar)
{
	Buffer * buf = ar.buffer();
	//lyxerr << "\nExps from: " << ar << endl;
	for (size_t i = 0; i + 1 < ar.size(); ++i) {
		// is this 'e'?
		if (ar[i]->getChar() != 'e')
			continue;

		// we need an exponent but no subscript
		InsetMathScript const * sup = ar[i + 1]->asScriptInset();
		if (!sup || sup->hasDown())
			continue;

		// create a proper exp-inset as replacement
		ar[i] = MathAtom(new InsetMathExFunc(buf, from_ascii("exp"), sup->cell(1)));
		ar.erase(i + 1);
	}
	//lyxerr << "\nExps to: " << ar << endl;
}


//
// extract det(...)  from |matrix|
//
void extractDets(MathData & ar)
{
	Buffer * buf = ar.buffer();
	//lyxerr << "\ndet from: " << ar << endl;
	for (MathData::iterator it = ar.begin(); it != ar.end(); ++it) {
		InsetMathDelim const * del = (*it)->asDelimInset();
		if (!del)
			continue;
		if (!del->isAbs())
			continue;
		*it = MathAtom(new InsetMathExFunc(buf, from_ascii("det"), del->cell(0)));
	}
	//lyxerr << "\ndet to: " << ar << endl;
}


//
// search numbers
//

bool isDigitOrSimilar(char_type c)
{
	return ('0' <= c && c <= '9') || c == '.';
}


// returns sequence of digits
docstring digitSequence
	(MathData::const_iterator it, MathData::const_iterator end)
{
	docstring s;
	for (; it != end && (*it)->asCharInset(); ++it) {
		if (!isDigitOrSimilar((*it)->getChar()))
			break;
		s += (*it)->getChar();
	}
	return s;
}


void extractNumbers(MathData & ar)
{
	//lyxerr << "\nNumbers from: " << ar << endl;
	for (size_t i = 0; i < ar.size(); ++i) {
		if (!ar[i]->asCharInset())
			continue;
		if (!isDigitOrSimilar(ar[i]->asCharInset()->getChar()))
			continue;

		docstring s = digitSequence(ar.begin() + i, ar.end());

		ar[i] = MathAtom(new InsetMathNumber(s));
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


MathAtom replaceParenDelims(const MathData & ar)
{
	return MathAtom(new InsetMathDelim(const_cast<Buffer *>(ar.buffer()),
		from_ascii("("), from_ascii(")"), ar));
}


bool testOpenBracket(MathAtom const & at)
{
	return testString(at, "[");
}


bool testCloseBracket(MathAtom const & at)
{
	return testString(at, "]");
}


MathAtom replaceBracketDelims(const MathData & ar)
{
	return MathAtom(new InsetMathDelim(const_cast<Buffer *>(ar.buffer()),
		from_ascii("["), from_ascii("]"), ar));
}


// replace '('...')' and '['...']' sequences by a real InsetMathDelim
void extractDelims(MathData & ar)
{
	//lyxerr << "\nDelims from: " << ar << endl;
	replaceNested(ar, testOpenParen, testCloseParen, replaceParenDelims);
	replaceNested(ar, testOpenBracket, testCloseBracket, replaceBracketDelims);
	//lyxerr << "\nDelims to: " << ar << endl;
}



//
// search well-known functions
//


// replace 'f' '(...)' and 'f' '^n' '(...)' sequences by a real InsetMathExFunc
// assume 'extractDelims' ran before
void extractFunctions(MathData & ar, ExternalMath kind)
{
	// FIXME From what I can see, this is quite broken right now, for reasons
	// I will note below. (RGH)

	// we need at least two items...
	if (ar.size() < 2)
		return;

	Buffer * buf = ar.buffer();

	//lyxerr << "\nFunctions from: " << ar << endl;
	for (size_t i = 0; i + 1 < ar.size(); ++i) {
		MathData::iterator it = ar.begin() + i;
		MathData::iterator jt = it + 1;

		docstring name;
		// is it a function?
		// it certainly is if it is well known...

		// FIXME This will never give us anything. When we get here, *it will
		// never point at a string, but only at a character. I.e., if we are
		// working on "sin(x)", then we are seeing:
		// [char s mathalpha][char i mathalpha][char n mathalpha][delim ( ) [char x mathalpha]]
		// and of course we will not find the function name "sin" in there, but
		// rather "n(x)".
		//
		// It appears that we original ran extractStrings() before we ran
		// extractFunctions(), but Andre changed this at f200be55, I think
		// because this messed up what he was trying to do with "dx" in the
		// context of integrals.
		//
		// This could be fixed by looking at a charSequence instead of just at
		// the various characters, one by one. But I am not sure I understand
		// exactly what we are trying to do here. And it involves a lot of
		// guessing.
		if (!extractFunctionName(*it, name)) {
			// is this a user defined function?
			// probably not, if it doesn't have a name.
			if (!extractString(*it, name))
				continue;
			// it is not if it has no argument
			if (jt == ar.end())
				continue;
			// guess so, if this is followed by
			// a DelimInset with a single item in the cell
			InsetMathDelim const * del = (*jt)->asDelimInset();
			if (!del || del->cell(0).size() != 1)
				continue;
			// fall through into main branch
		}

		// do we have an exponent like in
		// 'sin' '^2' 'x' -> 'sin(x)' '^2'
		MathData exp;
		extractScript(exp, jt, ar.end(), true);

		// create a proper inset as replacement
		auto_ptr<InsetMathExFunc> p(new InsetMathExFunc(buf, name));

		// jt points to the "argument". Get hold of this.
		MathData::iterator st = 
				extractArgument(p->cell(0), jt, ar.end(), kind, true);

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

bool testSymbol(MathAtom const & at, docstring const & name)
{
	return at->asSymbolInset() && at->asSymbolInset()->name() == name;
}


bool testSymbol(MathAtom const & at, char const * const name)
{
	return at->asSymbolInset() && at->asSymbolInset()->name() == from_ascii(name);
}


bool testIntSymbol(MathAtom const & at)
{
	return testSymbol(at, from_ascii("int"));
}


bool testIntegral(MathAtom const & at)
{
	return
	 testIntSymbol(at) ||
		( at->asScriptInset()
		  && !at->asScriptInset()->nuc().empty()
			&& testIntSymbol(at->asScriptInset()->nuc().back()) );
}



bool testIntDiff(MathAtom const & at)
{
	return testString(at, "d");
}


// replace '\int' ['_^'] x 'd''x'(...)' sequences by a real InsetMathExInt
// assume 'extractDelims' ran before
void extractIntegrals(MathData & ar, ExternalMath kind)
{
	// we need at least three items...
	if (ar.size() < 3)
		return;

	Buffer * buf = ar.buffer();

	//lyxerr << "\nIntegrals from: " << ar << endl;
	for (size_t i = 0; i + 1 < ar.size(); ++i) {
		MathData::iterator it = ar.begin() + i;

		// search 'd'
		MathData::iterator jt =
			endNestSearch(it, ar.end(), testIntegral, testIntDiff);

		// something sensible found?
		if (jt == ar.end())
			continue;

		// is this a integral name?
		if (!testIntegral(*it))
			continue;

		// core ist part from behind the scripts to the 'd'
		auto_ptr<InsetMathExInt> p(new InsetMathExInt(buf, from_ascii("int")));

		// handle scripts if available
		if (!testIntSymbol(*it)) {
			p->cell(2) = (*it)->asScriptInset()->down();
			p->cell(3) = (*it)->asScriptInset()->up();
		}
		p->cell(0) = MathData(buf, it + 1, jt);

		// use the "thing" behind the 'd' as differential
		MathData::iterator tt = extractArgument(p->cell(1), jt + 1, ar.end(), kind);

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
MathData::iterator extractTerm(MathData & ar,
	MathData::iterator pos, MathData::iterator last)
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
	return testSymbol(p, from_ascii("sum"));
}


bool testSum(MathAtom const & at)
{
	return
	 testSumSymbol(at) ||
		( at->asScriptInset()
		  && !at->asScriptInset()->nuc().empty()
			&& testSumSymbol(at->asScriptInset()->nuc().back()) );
}


// replace '\sum' ['_^'] f(x) sequences by a real InsetMathExInt
// assume 'extractDelims' ran before
void extractSums(MathData & ar)
{
	// we need at least two items...
	if (ar.size() < 2)
		return;

	Buffer * buf = ar.buffer();

	//lyxerr << "\nSums from: " << ar << endl;
	for (size_t i = 0; i + 1 < ar.size(); ++i) {
		MathData::iterator it = ar.begin() + i;

		// is this a sum name?
		if (!testSum(ar[i]))
			continue;

		// create a proper inset as replacement
		auto_ptr<InsetMathExInt> p(new InsetMathExInt(buf, from_ascii("sum")));

		// collect lower bound and summation index
		InsetMathScript const * sub = ar[i]->asScriptInset();
		if (sub && sub->hasDown()) {
			// try to figure out the summation index from the subscript
			MathData const & ar = sub->down();
			MathData::const_iterator xt =
				find_if(ar.begin(), ar.end(), &testEqualSign);
			if (xt != ar.end()) {
				// we found a '=', use everything in front of that as index,
				// and everything behind as lower index
				p->cell(1) = MathData(buf, ar.begin(), xt);
				p->cell(2) = MathData(buf, xt + 1, ar.end());
			} else {
				// use everything as summation index, don't use scripts.
				p->cell(1) = ar;
			}
		}

		// collect upper bound
		if (sub && sub->hasUp())
			p->cell(3) = sub->up();

		// use something  behind the script as core
		MathData::iterator tt = extractTerm(p->cell(0), it + 1, ar.end());

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
	InsetMathScript const * sup = at->asScriptInset();
	if (sup && !sup->hasDown() && sup->hasUp() && sup->nuc().size() == 1) {
		MathAtom const & ma = sup->nuc().front();
		return testString(ma, "d") || testSymbol(ma, "partial");
	}
	return false;
}


bool testDiffArray(MathData const & ar)
{
	return !ar.empty() && testDiffItem(ar.front());
}


bool testDiffFrac(MathAtom const & at)
{
	return
		at->asFracInset()
			&& testDiffArray(at->asFracInset()->cell(0))
			&& testDiffArray(at->asFracInset()->cell(1));
}


void extractDiff(MathData & ar)
{
	Buffer * buf = ar.buffer();
	//lyxerr << "\nDiffs from: " << ar << endl;
	for (size_t i = 0; i < ar.size(); ++i) {
		MathData::iterator it = ar.begin() + i;

		// is this a "differential fraction"?
		if (!testDiffFrac(*it))
			continue;

		InsetMathFrac const * f = (*it)->asFracInset();
		if (!f) {
			lyxerr << "should not happen" << endl;
			continue;
		}

		// create a proper diff inset
		auto_ptr<InsetMathDiff> diff(new InsetMathDiff(buf));

		// collect function, let jt point behind last used item
		MathData::iterator jt = it + 1;
		//int n = 1;
		MathData numer(f->cell(0));
		splitScripts(numer);
		if (numer.size() > 1 && numer[1]->asScriptInset()) {
			// this is something like  d^n f(x) / d... or  d^n / d...
			// FIXME
			//n = 1;
			if (numer.size() > 2)
				diff->cell(0) = MathData(buf, numer.begin() + 2, numer.end());
			else
				jt = extractTerm(diff->cell(0), jt, ar.end());
		} else {
			// simply d f(x) / d... or  d/d...
			if (numer.size() > 1)
				diff->cell(0) = MathData(buf, numer.begin() + 1, numer.end());
			else
				jt = extractTerm(diff->cell(0), jt, ar.end());
		}

		// collect denominator parts
		MathData denom(f->cell(1));
		splitScripts(denom);
		for (MathData::iterator dt = denom.begin(); dt != denom.end();) {
			// find the next 'd'
			MathData::iterator et
				= find_if(dt + 1, denom.end(), &testDiffItem);

			// point before this
			MathData::iterator st = et - 1;
			InsetMathScript const * script = (*st)->asScriptInset();
			if (script && script->hasUp()) {
				// things like   d.../dx^n
				int mult = 1;
				if (extractNumber(script->up(), mult)) {
					//lyxerr << "mult: " << mult << endl;
					for (int i = 0; i < mult; ++i)
						diff->addDer(MathData(buf, dt + 1, st));
				}
			} else {
				// just  d.../dx
				diff->addDer(MathData(buf, dt + 1, et));
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



// replace '\lim_{x->x0} f(x)' sequences by a real InsetMathLim
// assume 'extractDelims' ran before
void extractLims(MathData & ar)
{
	Buffer * buf = ar.buffer();
	//lyxerr << "\nLimits from: " << ar << endl;
	for (size_t i = 0; i < ar.size(); ++i) {
		MathData::iterator it = ar.begin() + i;

		// must be a script inset with a subscript (without superscript)
		InsetMathScript const * sub = (*it)->asScriptInset();
		if (!sub || !sub->hasDown() || sub->hasUp() || sub->nuc().size() != 1)
			continue;

		// is this a limit function?
		if (!testSymbol(sub->nuc().front(), "lim"))
			continue;

		// subscript must contain a -> symbol
		MathData const & s = sub->down();
		MathData::const_iterator st = find_if(s.begin(), s.end(), &testRightArrow);
		if (st == s.end())
			continue;

		// the -> splits the subscript int x and x0
		MathData x  = MathData(buf, s.begin(), st);
		MathData x0 = MathData(buf, st + 1, s.end());

		// use something behind the script as core
		MathData f;
		MathData::iterator tt = extractTerm(f, it + 1, ar.end());

		// cleanup
		ar.erase(it + 1, tt);

		// create a proper inset as replacement
		*it = MathAtom(new InsetMathLim(buf, f, x, x0));
	}
	//lyxerr << "\nLimits to: " << ar << endl;
}


//
// combine searches
//

void extractStructure(MathData & ar, ExternalMath kind)
{
	//lyxerr << "\nStructure from: " << ar << endl;
	if (kind != MATHML && kind != HTML)
		splitScripts(ar);
	extractDelims(ar);
	extractIntegrals(ar, kind);
	if (kind != MATHML && kind != HTML)
		extractSums(ar);
	extractNumbers(ar);
	extractMatrices(ar);
	if (kind != MATHML && kind != HTML) {
		extractFunctions(ar, kind);
		extractDets(ar);
		extractDiff(ar);
		extractExps(ar);
		extractLims(ar);
		extractStrings(ar);
	}
	//lyxerr << "\nStructure to: " << ar << endl;
}


namespace {

	string captureOutput(string const & cmd, string const & data)
	{
		// In order to avoid parsing problems with command interpreters
		// we pass input data through a file
		TempFile tempfile("casinput");
		FileName const cas_tmpfile = tempfile.name();
		if (cas_tmpfile.empty()) {
			lyxerr << "Warning: cannot create temporary file."
			       << endl;
			return string();
		}
		ofstream os(cas_tmpfile.toFilesystemEncoding().c_str());
		os << data << endl;
		os.close();
		string command =  cmd + " < "
			+ quoteName(cas_tmpfile.toFilesystemEncoding());
		lyxerr << "calling: " << cmd
		       << "\ninput: '" << data << "'" << endl;
		cmd_ret const ret = runCommand(command);
		return ret.second;
	}

	size_t get_matching_brace(string const & str, size_t i)
	{
		int count = 1;
		size_t n = str.size();
		while (i < n) {
			i = str.find_first_of("{}", i+1);
			if (i == npos)
				return i;
			if (str[i] == '{')
				++count;
			else
				--count;
			if (count == 0)
				return i;
		}
		return npos;
	}

	size_t get_matching_brace_back(string const & str, size_t i)
	{
		int count = 1;
		while (i > 0) {
			i = str.find_last_of("{}", i-1);
			if (i == npos)
				return i;
			if (str[i] == '}')
				++count;
			else
				--count;
			if (count == 0)
				return i;
		}
		return npos;
	}

	MathData pipeThroughMaxima(docstring const &, MathData const & ar)
	{
		odocstringstream os;
		MaximaStream ms(os);
		ms << ar;
		docstring expr = os.str();
		docstring const header = from_ascii("simpsum:true;");

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
			lyxerr << "checking expr: '" << to_utf8(expr) << "'" << endl;
			docstring full = header + "tex(" + expr + ");";
			out = captureOutput("maxima", to_utf8(full));

			// leave loop if expression syntax is probably ok
			if (out.find("Incorrect syntax") == npos)
				break;

			// search line with "Incorrect syntax"
			istringstream is(out);
			string line;
			while (is) {
				getline(is, line);
				if (line.find("Incorrect syntax") != npos)
					break;
			}

			// 2nd next line is the one with caret
			getline(is, line);
			getline(is, line);
			size_t pos = line.find('^');
			lyxerr << "found caret at pos: '" << pos << "'" << endl;
			if (pos == npos || pos < 4)
				break; // caret position not found
			pos -= 4; // skip the "tex(" part
			if (expr[pos] == '*')
				break; // two '*' in a row are definitely bad
			expr.insert(pos, from_ascii("*"));
		}

		vector<string> tmp = getVectorFromString(out, "$$");
		if (tmp.size() < 2)
			return MathData();

		out = subst(tmp[1], "\\>", string());
		lyxerr << "output: '" << out << "'" << endl;

		// Ugly code that tries to make the result prettier
		size_t i = out.find("\\mathchoice");
		while (i != npos) {
			size_t j = get_matching_brace(out, i + 12);
			size_t k = get_matching_brace(out, j + 1);
			k = get_matching_brace(out, k + 1);
			k = get_matching_brace(out, k + 1);
			string mid = out.substr(i + 13, j - i - 13);
			if (mid.find("\\over") != npos)
				mid = '{' + mid + '}';
			out = out.substr(0, i)
				+ mid
				+ out.substr(k + 1);
			//lyxerr << "output: " << out << endl;
			i = out.find("\\mathchoice", i);
			break;
		}

		i = out.find("\\over");
		while (i != npos) {
			size_t j = get_matching_brace_back(out, i - 1);
			if (j == npos || j == 0)
				break;
			size_t k = get_matching_brace(out, i + 5);
			if (k == npos || k + 1 == out.size())
				break;
			out = out.substr(0, j - 1)
				+ "\\frac"
				+ out.substr(j, i - j)
				+ out.substr(i + 5, k - i - 4)
				+ out.substr(k + 2);
			//lyxerr << "output: " << out << endl;
			i = out.find("\\over", i + 4);
		}
		MathData res;
		mathed_parse_cell(res, from_utf8(out));
		return res;
	}


	MathData pipeThroughMaple(docstring const & extra, MathData const & ar)
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
		odocstringstream os;
		MapleStream ms(os);
		ms << ar;
		string expr = to_utf8(os.str());
		lyxerr << "ar: '" << ar << "'\n"
		       << "ms: '" << expr << "'" << endl;

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
			size_t pos = line.find('^');
			if (pos == string::npos || pos < 15)
				break; // caret position not found
			pos -= 15; // skip the "on line ..." part
			if (expr[pos] == '*' || (pos > 0 && expr[pos - 1] == '*'))
				break; // two '*' in a row are definitely bad
			expr.insert(pos, 1, '*');
		}

		// FIXME UNICODE Is utf8 encoding correct?
		string full = "latex(" + to_utf8(extra) + '(' + expr + "));";
		string out = captureOutput("maple -q", header + full + trailer);

		// change \_ into _

		//
		MathData res;
		mathed_parse_cell(res, from_utf8(out));
		return res;
	}


	MathData pipeThroughOctave(docstring const &, MathData const & ar)
	{
		odocstringstream os;
		OctaveStream vs(os);
		vs << ar;
		string expr = to_utf8(os.str());
		string out;
		// FIXME const cast
		Buffer * buf = const_cast<Buffer *>(ar.buffer());
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
			lyxerr << "output: '" << out << "'" << endl;

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
			size_t pos = line.find('^');
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
			return MathData();
		out = out.substr(i + 6);

		// parse output as matrix or single number
		MathAtom at(new InsetMathArray(buf, from_ascii("array"), from_utf8(out)));
		InsetMathArray const * mat = at->asArrayInset();
		MathData res(buf);
		if (mat->ncols() == 1 && mat->nrows() == 1)
			res.append(mat->cell(0));
		else {
			res.push_back(MathAtom(
				new InsetMathDelim(buf, from_ascii("("), from_ascii(")"))));
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
		size_t const len = macro.length();
		size_t i = out.find(macro);

		while (i != npos) {
			size_t const j = get_matching_brace(out, i + len);
			string const name = out.substr(i + len, j - i - len);
			out = out.substr(0, i)
				+ (roman ? "\\mathrm{" : "")
				+ (translate ? fromMathematicaName(name) : name)
				+ out.substr(roman ? j : j + 1);
			//lyxerr << "output: " << out << endl;
			i = out.find(macro, i);
		}
	}


	MathData pipeThroughMathematica(docstring const &, MathData const & ar)
	{
		odocstringstream os;
		MathematicaStream ms(os);
		ms << ar;
		// FIXME UNICODE Is utf8 encoding correct?
		string const expr = to_utf8(os.str());
		string out;

		lyxerr << "expr: '" << expr << "'" << endl;

		string const full = "TeXForm[" + expr + "]";
		out = captureOutput("math", full);
		lyxerr << "output: '" << out << "'" << endl;

		size_t pos1 = out.find("Out[1]//TeXForm= ");
		size_t pos2 = out.find("In[2]:=");

		if (pos1 == string::npos || pos2 == string::npos)
			return MathData();

		// get everything from pos1+17 to pos2
		out = out.substr(pos1 + 17, pos2 - pos1 - 17);
		out = subst(subst(out, '\r', ' '), '\n', ' ');

		// tries to make the result prettier
		prettifyMathematicaOutput(out, "Mfunction", true, true);
		prettifyMathematicaOutput(out, "Muserfunction", true, false);
		prettifyMathematicaOutput(out, "Mvariable", false, false);

		MathData res;
		mathed_parse_cell(res, from_utf8(out));
		return res;
	}

}

} // anon namespace

void write(MathData const & dat, WriteStream & wi)
{
	MathData ar = dat;
	extractStrings(ar);
	wi.firstitem() = true;
	for (MathData::const_iterator it = ar.begin(); it != ar.end(); ++it) {
		(*it)->write(wi);
		wi.firstitem() = false;
	}
}


void normalize(MathData const & ar, NormalStream & os)
{
	for (MathData::const_iterator it = ar.begin(); it != ar.end(); ++it)
		(*it)->normalize(os);
}


void octave(MathData const & dat, OctaveStream & os)
{
	MathData ar = dat;
	extractStructure(ar, OCTAVE);
	for (MathData::const_iterator it = ar.begin(); it != ar.end(); ++it)
		(*it)->octave(os);
}


void maple(MathData const & dat, MapleStream & os)
{
	MathData ar = dat;
	extractStructure(ar, MAPLE);
	for (MathData::const_iterator it = ar.begin(); it != ar.end(); ++it)
		(*it)->maple(os);
}


void maxima(MathData const & dat, MaximaStream & os)
{
	MathData ar = dat;
	extractStructure(ar, MAXIMA);
	for (MathData::const_iterator it = ar.begin(); it != ar.end(); ++it)
		(*it)->maxima(os);
}


void mathematica(MathData const & dat, MathematicaStream & os)
{
	MathData ar = dat;
	extractStructure(ar, MATHEMATICA);
	for (MathData::const_iterator it = ar.begin(); it != ar.end(); ++it)
		(*it)->mathematica(os);
}


void mathmlize(MathData const & dat, MathStream & os)
{
	MathData ar = dat;
	extractStructure(ar, MATHML);
	if (ar.empty())
		os << "<mrow/>";
	else if (ar.size() == 1)
		os << ar.front();
	else {
		os << MTag("mrow");
		for (MathData::const_iterator it = ar.begin(); it != ar.end(); ++it)
			(*it)->mathmlize(os);
		os << ETag("mrow");
	}
}


void htmlize(MathData const & dat, HtmlStream & os)
{
	MathData ar = dat;
	extractStructure(ar, HTML);
	if (ar.empty())
		return;
	if (ar.size() == 1) {
		os << ar.front();
		return;
	}
	for (MathData::const_iterator it = ar.begin(); it != ar.end(); ++it)
		(*it)->htmlize(os);
}


// convert this inset somehow to a number
bool extractNumber(MathData const & ar, int & i)
{
	idocstringstream is(charSequence(ar.begin(), ar.end()));
	is >> i;
	// Do not convert is implicitly to bool, since that is forbidden in C++11.
	return !is.fail();
}


bool extractNumber(MathData const & ar, double & d)
{
	idocstringstream is(charSequence(ar.begin(), ar.end()));
	is >> d;
	// Do not convert is implicitly to bool, since that is forbidden in C++11.
	return !is.fail();
}


MathData pipeThroughExtern(string const & lang, docstring const & extra,
	MathData const & ar)
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
	odocstringstream os;
	NormalStream ns(os);
	os << '[' << extra << ' ';
	ns << ar;
	os << ']';
	// FIXME UNICODE Is utf8 encoding correct?
	string data = to_utf8(os.str());

	// search external script
	FileName const file = libFileSearch("mathed", "extern_" + lang);
	if (file.empty()) {
		lyxerr << "converter to '" << lang << "' not found" << endl;
		return MathData();
	}

	// run external sript
	string out = captureOutput(file.absFileName(), data);
	MathData res;
	mathed_parse_cell(res, from_utf8(out));
	return res;
}


} // namespace lyx
