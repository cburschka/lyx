#include <config.h>

#include "LString.h"
#include "Lsstream.h"
#include "debug.h"
#include "support/filetools.h" //  LibFileSearch
#include "math_data.h"
#include "math_inset.h"
#include "math_parser.h"

#include <iostream>
#include <fstream>

using std::ifstream;
using std::istream;
using std::ostream;
using std::endl;


namespace {

class Correction {
public:
	///
	Correction() {}
	///
	bool correct(MathAtom & at, char c) const;
	///
	bool read(istream & is);
	///
	void write(ostream & os) const;
private:
	///	
	MathAtom from1_;
	///	
	char from2_;
	///	
	MathAtom to_;
};


bool Correction::read(istream & is)
{
	string s1, s2, s3;
	is >> s1 >> s2 >> s3;
	if (!is)
		return false;
	MathArray ar1, ar3;
	mathed_parse_cell(ar1, s1);
	mathed_parse_cell(ar3, s3);
	if (ar1.size() != 1  ||  s2.size() != 1  ||  ar3.size() !=1)
		return false;
	from1_ = ar1.front();
	from2_ = s2[0];
	to_    = ar3.front();
	return true;
}


void Correction::write(ostream & os) const
{
	os << "from: '" << from1_ << "' and '" << from2_
	   << "' to '" << to_ << "'" << endl;
}


bool Correction::correct(MathAtom & at, char c) const
{
	//lyxerr[Debug::MATHED]
	//	<< "trying to correct ar: " << at << " from: '" << from1_ << "'" << endl;
	if (from2_ != c)
		return false;
	if (!at->match(from1_.nucleus()))
		return false;
	lyxerr[Debug::MATHED]
		<< "match found! subst in " << at
		<< " from: '" << from1_ << "' to '" << to_ << "'" << endl;
	at = to_;
	return true;
}


istream & operator>>(istream & is, Correction & corr)
{
	corr.read(is);
	return is;
}


ostream & operator<<(ostream & os, Correction & corr)
{
	corr.write(os);
	return os;
}




class Corrections {
public:
	///	
	typedef vector<Correction>::const_iterator const_iterator;
	///	
	Corrections() {}
	///	
	void insert(const Correction & corr) { data_.push_back(corr); }
	///
	bool correct(MathAtom & at, char c) const;
private:
	///	
	vector<Correction> data_;	
};


bool Corrections::correct(MathAtom & at, char c) const
{
	for (const_iterator it = data_.begin(); it != data_.end(); ++it)
		if (it->correct(at, c))
			return true;
	return false;
}


Corrections theCorrections;

void initAutoCorrect()
{
	lyxerr[Debug::MATHED] << "reading autocorrect file" << endl;
	string const file = LibFileSearch(string(), "autocorrect");
	if (file.empty()) {
		lyxerr << "Could not find autocorrect file" << endl;
		return;
	}

	string line;
	ifstream is(file.c_str());
	while (getline(is, line)) {
		if (line.size() == 0 || line[0] == '#') {
			//lyxerr[Debug::MATHED] << "ignoring line '" << line << "'" << endl;
			continue;
		}
		istringstream il(line);
		//lyxerr[Debug::MATHED] << "line '" << line << "'" << endl;
		Correction corr;
		if (corr.read(il)) {
			//lyxerr[Debug::MATHED] << "parsed: '" << corr << "'" << endl;
			theCorrections.insert(corr);
		}
	}

	lyxerr[Debug::MATHED] << "done reading autocorrections." << endl;
}


} // namespace anon


bool math_autocorrect(MathAtom & at, char c)
{
	static bool initialized = false;

	if (!initialized) {
		initAutoCorrect();
		initialized = true;
	}

	return theCorrections.correct(at, c);
}
