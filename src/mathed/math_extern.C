// This file contains most of the magic that extracts "context
// information" from the unstructered layout-oriented stuff in an
// MathArray.

#include <config.h>

#include "math_amsarrayinset.h"
#include "math_arrayinset.h"
#include "math_charinset.h"
#include "math_deliminset.h"
#include "math_diffinset.h"
#include "math_exfuncinset.h"
#include "math_exintinset.h"
#include "math_fracinset.h"
#include "math_matrixinset.h"
#include "math_mathmlstream.h"
#include "math_numberinset.h"
#include "math_scriptinset.h"
#include "math_stringinset.h"
#include "math_symbolinset.h"
#include "math_unknowninset.h"
#include "math_parser.h"
#include "Lsstream.h"
#include "debug.h"
#include "support/lyxlib.h"
#include "support/systemcall.h"
#include "support/filetools.h"

#include <algorithm>

using std::ostream;
using std::istringstream;
using std::find_if;
using std::endl;


ostream & operator<<(ostream & os, MathArray const & ar)
{
	NormalStream ns(os);
	ns << ar;
	return os;
}


// define a function for tests
typedef bool TestItemFunc(MathInset *);

// define a function for replacing subexpressions
typedef MathInset * ReplaceArgumentFunc(const MathArray & ar);



// try to extract a super/subscript
// modify iterator position to point behind the thing
bool extractScript(MathArray & ar,
	MathArray::iterator & pos, MathArray::iterator last)
{
	// nothing to get here
	if (pos == last)
		return false;

	// is this a scriptinset?
	if (!(*pos)->asScriptInset())
		return false;

	// it is a scriptinset, use it.
	ar.push_back(*pos);
	++pos;
	return true;
}


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
	extractScript(ar, pos, last);
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


MathScriptInset const * asScript(MathArray::const_iterator it)
{
	if (!it->nucleus())
		return 0;
	if (it->nucleus()->asScriptInset())
		return 0;
	++it;
	if (!it->nucleus())
		return 0;
	return it->nucleus()->asScriptInset();
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
	//lyxerr << "\nStrings from: " << ar << "\n";
	for (MathArray::size_type i = 0; i < ar.size(); ++i) {
		if (!ar[i]->asCharInset())
			continue;
		string s = charSequence(ar.begin() + i, ar.end());
		ar[i].reset(new MathStringInset(s));
		ar.erase(i + 1, i + s.size());
	}
	//lyxerr << "\nStrings to: " << ar << "\n";
}


MathInset * singleItem(MathArray & ar)
{
	return ar.size() == 1 ? ar.begin()->nucleus() : 0;
}


void extractMatrices(MathArray & ar)
{
	//lyxerr << "\nMatrices from: " << ar << "\n";
	// first pass for explicitly delimited stuff
	for (MathArray::iterator it = ar.begin(); it != ar.end(); ++it) {
		MathDelimInset * del = (*it)->asDelimInset();
		if (!del)
			continue;
		MathInset * arr = singleItem(del->cell(0));
		if (!arr || !arr->asGridInset())
			continue;
		*it = MathAtom(new MathMatrixInset(*(arr->asGridInset())));
	}

	// second pass for AMS "pmatrix" etc
	for (MathArray::iterator it = ar.begin(); it != ar.end(); ++it) {
		MathAMSArrayInset * ams = (*it)->asAMSArrayInset();
		if (!ams)
			continue;
		*it = MathAtom(new MathMatrixInset(*ams));
	}
	//lyxerr << "\nMatrices to: " << ar << "\n";
}


// convert this inset somehow to a string
bool extractString(MathInset * p, string & str)
{
	if (!p)
		return false;
	if (p->getChar()) {
		str = string(1, p->getChar());
		return true;
	}
	if (p->asStringInset()) {
		str = p->asStringInset()->str();
		return true;
	}
	return false;
}


// convert this inset somehow to a number
bool extractNumber(MathArray const & ar, int & i)
{
	istringstream is(charSequence(ar.begin(), ar.end()).c_str());
	is >> i;
	return is;
}


bool extractNumber(MathArray const & ar, double & d)
{
	istringstream is(charSequence(ar.begin(), ar.end()).c_str());
	is >> d;
	return is;
}


bool testString(MathInset * p, const string & str)
{
	string s;
	return extractString(p, s) && str == s;
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
// split scripts into seperate super- and subscript insets. sub goes in
// front of super...
//

void splitScripts(MathArray & ar)
{
	//lyxerr << "\nScripts from: " << ar << "\n";
	for (MathArray::size_type i = 0; i < ar.size(); ++i) {
		MathArray::iterator it = ar.begin() + i;

		// is this script inset?
		MathScriptInset * p = (*it)->asScriptInset();
		if (!p)
			continue;

		// no problem if we don't have both...
		if (!p->hasUp() || !p->hasDown())
			continue;

		// create extra script inset and move superscript over
		MathScriptInset * q = new MathScriptInset;
		q->ensure(true);
		q->up().data_.swap(p->up().data_);
		p->removeScript(true);

		// insert new inset behind
		++i;
		ar.insert(i, MathAtom(q));
	}
	//lyxerr << "\nScripts to: " << ar << "\n";
}


//
// extract exp(...)
//

void extractExps(MathArray & ar)
{
	//lyxerr << "\nExps from: " << ar << "\n";

	for (MathArray::size_type i = 0; i + 1 < ar.size(); ++i) {
		MathArray::iterator it = ar.begin() + i;

		// is this 'e'?
		MathCharInset const * p = (*it)->asCharInset();
		if (!p || p->getChar() != 'e')
			continue;

		// we need an exponent but no subscript
		MathScriptInset * sup = (*(it + 1))->asScriptInset();
		if (!sup || sup->hasDown())
			continue;

		// create a proper exp-inset as replacement 
		*it = new MathExFuncInset("exp", sup->cell(1));
		ar.erase(it + 1);
	}
	//lyxerr << "\nExps to: " << ar << "\n";
}


//
// extract det(...)  from |matrix|
//
void extractDets(MathArray & ar)
{
	//lyxerr << "\ndet from: " << ar << "\n";
	for (MathArray::iterator it = ar.begin(); it != ar.end(); ++it) {
		MathDelimInset * del = (*it)->asDelimInset();
		if (!del)
			continue;
		if (!del->isAbs())
			continue;
		*it = new MathExFuncInset("det", del->cell(0));
	}
	//lyxerr << "\ndet to: " << ar << "\n";
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
	//lyxerr << "\nNumbers from: " << ar << "\n";
	for (MathArray::size_type i = 0; i < ar.size(); ++i) {
		if (!ar[i]->asCharInset())
			continue;
		if (!isDigitOrSimilar(ar[i]->asCharInset()->getChar()))
			continue;

		string s = digitSequence(ar.begin() + i, ar.end());

		ar[i].reset(new MathNumberInset(s));
		ar.erase(i + 1, i + s.size());
	}
	//lyxerr << "\nNumbers to: " << ar << "\n";
}



//
// search deliminiters
//

bool testOpenParan(MathInset * p)
{
	return testString(p, "(");
}


bool testCloseParan(MathInset * p)
{
	return testString(p, ")");
}


MathInset * replaceDelims(const MathArray & ar)
{
	return new MathDelimInset("(", ")", ar);
}


// replace '('...')' sequences by a real MathDelimInset
void extractDelims(MathArray & ar)
{
	//lyxerr << "\nDelims from: " << ar << "\n";
	replaceNested(ar, testOpenParan, testCloseParan, replaceDelims);
	//lyxerr << "\nDelims to: " << ar << "\n";
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

	//lyxerr << "\nFunctions from: " << ar << "\n";
	for (MathArray::size_type i = 0; i + 1 < ar.size(); ++i) {
		MathArray::iterator it = ar.begin() + i;
		MathArray::iterator jt = it + 1;

		string name;
		// is it a function?
		if ((*it)->asUnknownInset()) {
			// it certainly is if it is well known...
			name = (*it)->asUnknownInset()->name();
		} else {
			// is this a user defined function?
			// it it probably not, if it doesn't have a name.
			if (!extractString((*it).nucleus(), name))
				continue;
			// it is not if it has no argument
			if (jt == ar.end())
				continue;
			// guess so, if this is followed by
			// a DelimInset with a single item in the cell
			MathDelimInset * del = (*jt)->asDelimInset();
			if (!del || del->cell(0).size() != 1)
				continue;
			// fall trough into main branch
		}

		// do we have an exponent like in
		// 'sin' '^2' 'x' -> 'sin(x)' '^2'
		MathArray exp;
		extractScript(exp, jt, ar.end());

		// create a proper inset as replacement
		MathExFuncInset * p = new MathExFuncInset(name);

		// jt points to the "argument". Get hold of this.
		MathArray::iterator st = extractArgument(p->cell(0), jt, ar.end());

		// replace the function name by a real function inset
		(*it).reset(p);

		// remove the source of the argument from the array
		ar.erase(it + 1, st);

		// re-insert exponent
		ar.insert(i + 1, exp);
		//lyxerr << "\nFunctions to: " << ar << "\n";
	}
}


//
// search integrals
//

bool testSymbol(MathInset * p, string const & name)
{
	return p->asSymbolInset() && p->asSymbolInset()->name() == name;
}


bool testIntSymbol(MathInset * p)
{
	return testSymbol(p, "int");
}


bool testIntDiff(MathInset * p)
{
	return testString(p, "d");
}


// replace '\int' ['_^'] x 'd''x'(...)' sequences by a real MathExIntInset
// assume 'extractDelims' ran before
void extractIntegrals(MathArray & ar)
{
	// we need at least three items...
	if (ar.size() <= 2)
		return;

	//lyxerr << "\nIntegrals from: " << ar << "\n";
	for (MathArray::size_type i = 0; i + 1 < ar.size(); ++i) {
		MathArray::iterator it = ar.begin() + i;

		// is this a integral name?
		if (!testIntSymbol(it->nucleus()))
			continue;

		// search 'd'
		MathArray::iterator jt =
			endNestSearch(it, ar.end(), testIntSymbol, testIntDiff);

		// something sensible found?
		if (jt == ar.end())
			continue;

		// create a proper inset as replacement
		MathExIntInset * p = new MathExIntInset("int");

		// collect subscript if any
		MathArray::iterator st = it + 1;
		if (st != ar.end())
			if (MathScriptInset * sub = (*st)->asScriptInset())
				if (sub->hasDown()) {
					p->cell(2) = sub->down().data_;
					++st;
				}

		// collect superscript if any
		if (st != ar.end())
			if (MathScriptInset * sup = (*st)->asScriptInset())
				if (sup->hasUp()) {
					p->cell(3) = sup->up().data_;
					++st;
				}

		// core ist part from behind the scripts to the 'd'
		p->cell(0) = MathArray(st, jt);

		// use the "thing" behind the 'd' as differential
		MathArray::iterator tt = extractArgument(p->cell(1), jt + 1, ar.end());

		// remove used parts
		ar.erase(it + 1, tt);
		(*it).reset(p);
	}
	//lyxerr << "\nIntegrals to: " << ar << "\n";
}


//
// search sums
//

bool testSumSymbol(MathInset * p)
{
	return testSymbol(p, "sum");
}


bool testEqualSign(MathAtom const & at)
{
	return testString(at.nucleus(), "=");
}



// replace '\sum' ['_^'] f(x) sequences by a real MathExIntInset
// assume 'extractDelims' ran before
void extractSums(MathArray & ar)
{
	// we need at least two items...
	if (ar.size() <= 1)
		return;

	//lyxerr << "\nSums from: " << ar << "\n";
	for (MathArray::size_type i = 0; i + 1< ar.size(); ++i) {
		MathArray::iterator it = ar.begin() + i;

		// is this a sum name?
		if (!testSumSymbol(it->nucleus()))
			continue;

		// create a proper inset as replacement
		MathExIntInset * p = new MathExIntInset("sum");

		// collect lower bound and summation index
		MathArray::iterator st = it + 1;
		if (st != ar.end())
			if (MathScriptInset * sub = (*st)->asScriptInset())
				if (sub->hasDown()) {
					// try to figure out the summation index from the subscript
					MathArray & ar = sub->down().data_;
					MathArray::iterator it =
						find_if(ar.begin(), ar.end(), &testEqualSign);
					if (it != ar.end()) {
						// we found a '=', use everything in front of that as index,
						// and everything behind as lower index
						p->cell(1) = MathArray(ar.begin(), it);
						p->cell(2) = MathArray(it + 1, ar.end());
					} else {
						// use everything as summation index, don't use scripts.
						p->cell(1) = ar;
					}
					++st;
				}

		// collect upper bound
		if (st != ar.end())
			if (MathScriptInset * sup = (*st)->asScriptInset())
				if (sup->hasUp()) {
					p->cell(3) = sup->up().data_;
					++st;
				}

		// use some  behind the script as core
		MathArray::iterator tt = extractArgument(p->cell(0), st, ar.end());

		// cleanup
		ar.erase(it + 1, tt);
		(*it).reset(p);
	}
	//lyxerr << "\nSums to: " << ar << "\n";
}


//
// search differential stuff
//

// tests for 'd' or '\partial'
bool testDiffItem(MathAtom const & at)
{
	return testString(at.nucleus(), "d");
}


bool testDiffArray(MathArray const & ar)
{
	return ar.size() && testDiffItem(ar.front());
}


bool testDiffFrac(MathInset * p)
{
	MathFracInset * f = p->asFracInset();
	return f && testDiffArray(f->cell(0)) && testDiffArray(f->cell(1));
}


// is this something like ^number?
bool extractDiffExponent(MathArray::iterator it, int & i)
{
	if (!(*it)->asScriptInset())
		return false;

	string s;
	if (!extractString((*it).nucleus(), s))
		return false;
	istringstream is(s.c_str());
	is >> i;
	return is;
}


void extractDiff(MathArray & ar)
{
	//lyxerr << "\nDiffs from: " << ar << "\n";
	for (MathArray::size_type i = 0; i < ar.size(); ++i) {
		MathArray::iterator it = ar.begin() + i;

		// is this a "differential fraction"?
		if (!testDiffFrac(it->nucleus()))
			continue;

		MathFracInset * f = (*it)->asFracInset();
		if (!f) {
			lyxerr << "should not happen\n";
			continue;
		}

		// create a proper diff inset
		MathDiffInset * diff = new MathDiffInset;

		// collect function, let jt point behind last used item
		MathArray::iterator jt = it + 1;
		//int n = 1;
		MathArray & numer = f->cell(0);
		if (numer.size() > 1 && numer[1]->asScriptInset()) {
			// this is something like  d^n f(x) / d... or  d^n / d...
			// FIXME
			//n = 1;
			if (numer.size() > 2)
				diff->cell(0) = MathArray(numer.begin() + 2, numer.end());
			else
				jt = extractArgument(diff->cell(0), jt, ar.end());
		} else {
			// simply d f(x) / d... or  d/d...
			if (numer.size() > 1)
				diff->cell(0) = MathArray(numer.begin() + 1, numer.end());
			else
				jt = extractArgument(diff->cell(0), jt, ar.end());
		}

		// collect denominator parts
		MathArray & denom = f->cell(1);
		for (MathArray::iterator dt = denom.begin(); dt != denom.end();) {
			// find the next 'd'
			MathArray::iterator et = find_if(dt + 1, denom.end(), &testDiffItem);

			// point before this
			MathArray::iterator st = et - 1;
			MathScriptInset * script = (*st)->asScriptInset();
			if (script && script->hasUp()) {
				// things like   d.../dx^n
				int mult = 1;
				if (extractNumber(script->up().data_, mult)) {
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
		(*it).reset(diff);
	}
	//lyxerr << "\nDiffs to: " << ar << "\n";
}



//
// combine searches
//

void extractStructure(MathArray & ar)
{
	splitScripts(ar);
	extractNumbers(ar);
	extractMatrices(ar);
	extractDelims(ar);
	extractFunctions(ar);
	extractDets(ar);
	extractIntegrals(ar);
	extractSums(ar);
	extractDiff(ar);
	extractExps(ar);
	extractStrings(ar);
}


void write(MathArray const & dat, WriteStream & wi)
{
	MathArray ar = dat;
	extractStrings(ar);
	for (MathArray::const_iterator it = ar.begin(); it != ar.end(); ++it) {
		wi.firstitem() = (it == ar.begin());
		MathInset const * p = it->nucleus();
		if (it + 1 != ar.end()) {
			if (MathScriptInset const * q = asScript(it)) {
				q->write2(p, wi);
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
				q->octavize2(p, os);
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
				q->maplize2(p, os);
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
					q->mathmlize2(p, os);
					++it;
					continue;
				}
			}
			p->mathmlize(os);
		}
		os << ETag("mrow");
	}
}




namespace {

	string captureOutput(string const & cmd, string const & data)
	{
		string outfile = lyx::tempName(string(), "mathextern");
		string full =  "echo '" + data + "' | (" + cmd + ") > " + outfile;
		lyxerr << "calling: " << full << endl;
		Systemcall dummy;
		dummy.startscript(Systemcall::Wait, full);
		string out = GetFileContents(outfile);
		lyx::unlink(outfile);
		lyxerr << "result: '" << out << "'" << endl;
		return out;
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
		string expr = os.str().c_str();
		lyxerr << "ar: '" << ar << "'\n";
		lyxerr << "ms: '" << os.str() << "'\n";

		for (int i = 0; i < 100; ++i) { // at most 100 attempts
			// try to fix missing '*' the hard way by using mint
			//
			// ... > echo "1A;" | mint -i 1 -S -s -q
			// on line     1: 1A;
			//                 ^ syntax error -
			//                   Probably missing an operator such as * p
			//
			lyxerr << "checking expr: '" << expr << "'\n";
			string out = captureOutput("mint -i 1 -S -s -q -q", expr + ";");
			if (out.empty())
				break; // expression syntax is ok
			istringstream is(out.c_str());
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
			expr.insert(pos,  "*");
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
		string expr = os.str().c_str();
		string out;

		lyxerr << "pipe: ar: '" << ar << "'\n";
		lyxerr << "pipe: expr: '" << expr << "'\n";

		for (int i = 0; i < 100; ++i) { // at most 100 attempts
			//
			// try to fix missing '*' the hard way
			// parse error:
			// >>> ([[1 2 3 ];[2 3 1 ];[3 1 2 ]])([[1 2 3 ];[2 3 1 ];[3 1 2 ]])
			//                                   ^
			//
			lyxerr << "checking expr: '" << expr << "'\n";
			out = captureOutput("octave -q 2>&1", expr);
			lyxerr << "checking out: '" << out << "'\n";

			// leave loop if expression syntax is probably ok
			if (out.find("parse error:") == string::npos)
				break;

			// search line with single caret
			istringstream is(out.c_str());
			string line;
			while (is) {
				getline(is, line);
				lyxerr << "skipping line: '" << line << "'\n";
				if (line.find(">>> ") != string::npos)
					break;
			}

			// found line with error, next line is the one with caret
			getline(is, line);
			string::size_type pos = line.find('^');
			lyxerr << "caret line: '" << line << "'\n";
			lyxerr << "found caret at pos: '" << pos << "'\n";
			if (pos == string::npos || pos < 4)
				break; // caret position not found
			pos -= 4; // skip the ">>> " part
			if (expr[pos] == '*')
				break; // two '*' in a row are definitely bad
			expr.insert(pos,  "*");
		}

		if (out.size() < 6)
			return MathArray();

		// remove 'ans = '
		out = out.substr(6);

		// parse output as matrix or single number
		MathAtom at(new MathArrayInset("array", out));
		MathArrayInset const * mat = at.nucleus()->asArrayInset();
		MathArray res;
		if (mat->ncols() == 1 && mat->nrows() == 1)
			res.push_back(mat->cell(0));
		else {
			res.push_back(MathAtom(new MathDelimInset("(", ")")));
			res.back()->cell(0).push_back(at);
		}
		return res;
	}

}


MathArray pipeThroughExtern(string const & lang, string const & extra,
	MathArray const & ar)
{
	if (lang == "octave")
		return pipeThroughOctave(extra, ar);

	if (lang == "maple")
		return pipeThroughMaple(extra, ar);

	// create normalized expression
	ostringstream os;
	NormalStream ns(os);
	os << "[" << extra << ' ';
	ns << ar;
	os << "]";
	string data = os.str().c_str();

	// search external script
	string file = LibFileSearch("mathed", "extern_" + lang);
	if (file.empty()) {
		lyxerr << "converter to '" << lang << "' not found\n";
		return MathArray();
	}

	// run external sript
	string out = captureOutput(file, data);
	MathArray res;
	mathed_parse_cell(res, out);
	return res;
}
