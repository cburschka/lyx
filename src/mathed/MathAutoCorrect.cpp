/**
 * \file MathAutoCorrect.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "MathAutoCorrect.h"
#include "MathData.h"
#include "InsetMath.h"
#include "MathSupport.h"
#include "MathParser.h"

#include "support/debug.h"
#include "support/FileName.h"
#include "support/filetools.h" //  LibFileSearch
#include "support/docstream.h"

#include <fstream>
#include <sstream>

using namespace std;

namespace lyx {

using support::libFileSearch;

namespace {

class Correction {
public:
	///
	/// \brief Correction
	// it is fine not to initialize from2_
	// coverity[UNINIT_CTOR]
	Correction() {}
	///
	bool correct(MathAtom & at, char_type c) const;
	///
	bool read(idocstream & is);
	///
	void write(odocstream & os) const;
private:
	///
	MathAtom from1_;
	///
	char_type from2_;
	///
	MathAtom to_;
};


bool Correction::read(idocstream & is)
{
	docstring s1, s2, s3;
	is >> s1 >> s2 >> s3;
	if (!is)
		return false;
	if (s2.size() != 1)
		return false;
	MathData ar1, ar3;
	mathed_parse_cell(ar1, s1);
	mathed_parse_cell(ar3, s3);
	if (ar1.size() != 1 || ar3.size() != 1)
		return false;
	from1_ = ar1.front();
	from2_ = s2[0];
	to_    = ar3.front();
	return true;
}


void Correction::write(odocstream & os) const
{
	os << "from: '" << from1_ << "' and '" << from2_
	   << "' to '" << to_ << '\'' << endl;
}


bool Correction::correct(MathAtom & at, char_type c) const
{
	//LYXERR(Debug::MATHED,
	//	"trying to correct ar: " << at << " from: '" << from1_ << '\'');
	if (from2_ != c)
		return false;
	if (asString(at) != asString(from1_))
		return false;
	LYXERR(Debug::MATHED, "match found! subst in " << at
		<< " from: '" << from1_ << "' to '" << to_ << '\'');
	at = to_;
	return true;
}


#if 0
idocstream & operator>>(idocstream & is, Correction & corr)
{
	corr.read(is);
	return is;
}


odocstream & operator<<(odocstream & os, Correction & corr)
{
	corr.write(os);
	return os;
}
#endif



class Corrections {
public:
	///
	typedef vector<Correction>::const_iterator const_iterator;
	///
	Corrections() {}
	///
	void insert(const Correction & corr) { data_.push_back(corr); }
	///
	bool correct(MathAtom & at, char_type c) const;
private:
	///
	vector<Correction> data_;
};


bool Corrections::correct(MathAtom & at, char_type c) const
{
	for (const_iterator it = data_.begin(); it != data_.end(); ++it)
		if (it->correct(at, c))
			return true;
	return false;
}


Corrections theCorrections;

void initAutoCorrect()
{
	LYXERR(Debug::MATHED, "reading autocorrect file");
	support::FileName const file = libFileSearch(string(), "autocorrect");
	if (file.empty()) {
		lyxerr << "Could not find autocorrect file" << endl;
		return;
	}

	string line;
	ifstream is(file.toFilesystemEncoding().c_str());
	while (getline(is, line)) {
		if (line.empty() || line[0] == '#') {
			//LYXERR(Debug::MATHED, "ignoring line '" << line << '\'');
			continue;
		}
		idocstringstream il(from_utf8(line));

		//LYXERR(Debug::MATHED, "line '" << line << '\'');
		Correction corr;
		if (corr.read(il)) {
			//LYXERR(Debug::MATHED, "parsed: '" << corr << '\'');
			theCorrections.insert(corr);
		}
	}

	LYXERR(Debug::MATHED, "done reading autocorrections.");
}


} // namespace anon


bool math_autocorrect(MathAtom & at, char_type c)
{
	static bool initialized = false;

	if (!initialized) {
		initAutoCorrect();
		initialized = true;
	}

	return theCorrections.correct(at, c);
}


} // namespace lyx
