
// This file contains most of the magic that extracts "context
// information" from the unstructered layout-oriented stuff in an
// MathArray.


#include "math_charinset.h"
#include "math_deliminset.h"
#include "math_exfuncinset.h"
#include "math_funcinset.h"
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
		lyxerr << "\nMatrices to: " << ar << "\n";
	}
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


// define a function for tests
typedef bool TestItemFunc(MathInset *);

// define a function for replacing pa
typedef MathInset * ReplaceArgumentFunc(const MathArray & ar);

// search end of nested sequence
MathArray::iterator searchNestedEnd(
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
		MathArray::iterator jt = searchNestedEnd(it, ar.end(), testOpen, testClose);
		if (jt == ar.end())
			continue;

		// create a proper inset as replacement
		MathInset * p = replaceArg(MathArray(it + 1, jt));

		// replace the original stuff by the new inset
		ar.erase(it + 1, jt + 1);
		(*it).reset(p);
	}
} 


bool testParanOpen(MathInset * p)
{
	return extractString(p) == "(";
}


bool testParanClose(MathInset * p)
{
	return extractString(p) == ")";
}


MathInset * replaceByDelimInset(const MathArray & ar)
{
	MathDelimInset * del = new MathDelimInset("(", ")");
	del->cell(0) = ar;
	return del;
}


// replace '('...')' sequences by a real MathDelimInset
void extractDelims(MathArray & ar) {
	lyxerr << "\nDelims from: " << ar << "\n";
	replaceNested(ar, testParanOpen, testParanClose, replaceByDelimInset);
	lyxerr << "\nDelims to: " << ar << "\n";
}


// replace 'f' '(...)' and 'f' '^n' '(...)' sequences by a real MathExFuncInset
// assume 'extractDelims' ran before
void extractFunctions(MathArray & ar)
{
	// we need at least two items...
	if (ar.size() <= 1)
		return;

	lyxerr << "\nFunctions from: " << ar << "\n";
	for (MathArray::size_type i = 0; i < ar.size() - 1; ++i) {
		MathArray::iterator it = ar.begin() + i;

		// is this a function name?
		MathFuncInset * func = (*it)->asFuncInset();
		if (!func)
			continue;

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

		// jt points now to the "argument". Since we had run "extractDelims"
		// before, this could be a single argument only. Get hold of this.
		MathArray arg;
		MathDelimInset * del = (*jt)->asDelimInset();
		if (del && del->isParanthesis()) 
			arg = del->cell(0);
		else
			arg.push_back(*jt);

		// replace the function name by a real function inset
		(*it).reset(new MathExFuncInset(func->name(), arg));
		
		// remove the source of the argument from the array
		ar.erase(jt);
		lyxerr << "\nFunctions to: " << ar << "\n";
	}
} 


bool testIntSymbol(MathInset * p)
{
	return p->asSymbolInset() && p->asSymbolInset()->name() == "int";
}


bool testSmallD(MathInset * p)
{
	string s = extractString(p);
	return s.size() && s[0] == 'd';
}


// replace '\int' ['_^'] x 'd''x'(...)' sequences by a real MathExIntInset
// assume 'extractDelims' ran before
void extractIntegrals(MathArray & ar)
{
	// we need at least three items...
	if (ar.size() <= 2)
		return;

	lyxerr << "\nIntegrals from: " << ar << "\n";
	for (MathArray::size_type i = 0; i < ar.size() - 1; ++i) {
		MathArray::iterator it = ar.begin() + i;

		// is this a integral name?
		if (!testIntSymbol(it->nucleus()))
			continue;

		// search 'd'
		MathArray::iterator jt =
			searchNestedEnd(it, ar.end(), testIntSymbol, testSmallD);

		// create a proper inset as replacement
		//MathInset * p = replaceArg(MathArray(it + 1, jt));

		// replace the original stuff by the new inset
		//ar.erase(it + 1, jt + 1);
		//(*it).reset(p);
	}
}


void extractStructure(MathArray & ar)
{
	extractStrings(ar);
	extractMatrices(ar);
	extractDelims(ar);
	extractFunctions(ar);
	extractIntegrals(ar);
}


void write(MathArray const & dat, WriteStream & wi)
{
	MathArray ar = dat;
	extractStrings(ar);
	for (MathArray::const_iterator it = ar.begin(); it != ar.end(); ++it) {
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

