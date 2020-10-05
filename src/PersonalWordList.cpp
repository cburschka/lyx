/**
 * \file PersonalWordList.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Stephan Witt
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "PersonalWordList.h"

#include "support/debug.h"
#include "support/docstring_list.h"
#include "support/filetools.h"
#include "support/Package.h"
#include "support/FileName.h"
#include "support/lstrings.h"
#include "support/os.h"

#include <string>
#include <fstream>

using namespace std;
using namespace lyx::support;
using namespace lyx::support::os;

namespace lyx {

FileName PersonalWordList::dictfile() const
{
	string fname = "pwl_" + lang_ + ".dict";
	return FileName(addName(package().user_support().absFileName(),fname));
}


docstring_list::const_iterator PersonalWordList::begin() const
{
	return words_.begin();
}


docstring_list::const_iterator PersonalWordList::end() const
{
	return words_.end();
}


void PersonalWordList::load()
{
	FileName fn = dictfile();
	LYXERR(Debug::FILES, "load personal dictionary from: " << fn);
	ifstream ifs(fn.toFilesystemEncoding().c_str());

	dirty(!words_.empty());
	words_.clear();
	string line;
	getline(ifs, line);
	if (line == header()) {
		while (ifs) {
			getline(ifs, line);
			if (!line.empty() && !(line[0] == '#')) {
				docstring const word = from_utf8(line);
				insert(word);
			}
		}
		LYXERR(Debug::FILES, "valid dictionary file found: " << words_.size() << " items.");
	} else {
		LYXERR(Debug::FILES, "invalid dictionary file found: header is \"" << line << "\".");
	}
	ifs.close();
	dirty(false);
}


void PersonalWordList::save()
{
	if (!isDirty())
		return;
	FileName fn = dictfile();
	LYXERR(Debug::FILES, "save personal dictionary at: " << fn);
	ofstream ofs(fn.toFilesystemEncoding().c_str());
	docstring_list::iterator it = words_.begin();
	docstring_list::const_iterator et = words_.end();

	ofs << header() << "\n";
	for (; it != et; ++it) {
		ofs << to_utf8(*it) << "\n";
	}
	LYXERR(Debug::FILES, "count of saved items: " << words_.size());
}


bool PersonalWordList::equalwords(docstring const & w1, docstring const & w2) const
{
	return w1 == w2;
}


bool PersonalWordList::exists(docstring const & word) const
{
	docstring_list::const_iterator it = words_.begin();
	docstring_list::const_iterator et = words_.end();
	for (; it != et; ++it) {
		if (equalwords(word,*it))
			return true;
	}
	return false;
}


void PersonalWordList::insert(docstring const & word)
{
	if (exists(word))
		return;
	words_.push_back(word);
	dirty(true);
}


void PersonalWordList::remove(docstring const & word)
{
	docstring_list::iterator it = words_.begin();
	docstring_list::const_iterator et = words_.end();
	for (; it != et; ++it) {
		if (equalwords(word,*it)) {
			words_.erase(it);
			dirty(true);
			return;
		}
	}
}

} // namespace lyx
