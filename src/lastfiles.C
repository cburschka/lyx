/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#include <config.h>


#ifdef __GNUG__
#pragma implementation
#endif

#include "lastfiles.h"
#include "debug.h"

#include "support/FileInfo.h"

#include <fstream>
#include <algorithm>
#include <iterator>


using std::ifstream;
using std::ofstream;
using std::getline;
using std::endl;
using std::find;
using std::copy;
using std::ostream_iterator;


LastFiles::LastFiles(string const & filename, bool st, unsigned int num)
	: dostat(st)
{
	setNumberOfFiles(num);
	readFile(filename);
}


void LastFiles::setNumberOfFiles(unsigned int no)
{
	if (0 < no && no <= ABSOLUTEMAXLASTFILES)
		num_files = no;
	else {
		lyxerr << "LyX: lastfiles: too many files\n"
			"\tdefault (=" << int(DEFAULTFILES)
		       << ") used." << endl;
		num_files = DEFAULTFILES;
	}
}


void LastFiles::readFile(string const & filename)
{
	// we will not complain if we can't find filename nor will
	// we issue a warning. (Lgb)
	ifstream ifs(filename.c_str());
	string tmp;
	FileInfo fileInfo;

	while (getline(ifs, tmp) && files.size() < num_files) {
		if (dostat) {
			if (!(fileInfo.newFile(tmp).exist() &&
			      fileInfo.isRegular()))
				continue;
		}
		files.push_back(tmp);
	}
}


void LastFiles::writeFile(string const & filename) const
{
	ofstream ofs(filename.c_str());
	if (ofs) {
		copy(files.begin(), files.end(),
		     ostream_iterator<string>(ofs, "\n"));
	} else
		lyxerr << "LyX: Warning: unable to save LastFiles: "
		       << filename << endl;
}


void LastFiles::newFile(string const & file)
{
	// If file already exist, delete it and reinsert at front.
	Files::iterator it = find(files.begin(), files.end(), file);
	if (it != files.end())
		files.erase(it);
	files.push_front(file);
	if (files.size() > num_files)
		files.pop_back();
}


string const LastFiles::operator[](unsigned int i) const
{
	if (i < files.size())
		return files[i];
	return string();
}
