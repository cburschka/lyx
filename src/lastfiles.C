/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-1999 The LyX Team.
 *
 * ======================================================*/

#include <config.h>


#ifdef __GNUG__
#pragma implementation
#endif

#include "lyxlex.h"
#include "support/FileInfo.h"
#include "lastfiles.h"
#include "support/filetools.h"
#include "error.h"

LastFiles::LastFiles(string const & filename, bool st, char num)
	: dostat(st)
{
	setNumberOfFiles(num);
	files = new string[num_files];
	readFile(filename);
}


LastFiles::~LastFiles()
{
	delete[] files;
}


void LastFiles::setNumberOfFiles(char no)
{
	if (1 <= no && no <= ABSOLUTEMAXLASTFILES)
		num_files = no;
	else {
		lyxerr.print(string("LyX: lastfiles: too many files\n"
			"\tdefault (=") + tostr(DEFAULTFILES) + ") used.");
		num_files = DEFAULTFILES;
	}
}


void LastFiles::readFile(string const & filename)
{
	// we will not complain if we can't find filename nor will
	// we issue a warning. Lgb.
	LyXLex lex(0, 0); /* LyXLex should be changed
			      * to allow constructor with
			      * no parameters. */
	bool error = false;

	lex.setFile(filename);

	if (!lex.IsOK()) return;

	string tmp;
	FileInfo fileInfo;
	int i = 0;

	while (lex.IsOK() && !error && i < num_files) {
		switch(lex.lex()) {
		case LyXLex::LEX_FEOF:
			error = true;
			break;
		default:
			tmp = lex.GetString();
			// Check if the file exist
			if (dostat) {
				if (!(fileInfo.newFile(tmp).exist() &&
				      fileInfo.isRegular()))
					break; // the file does not exist
			}
			files[i] = tmp;
			i++;
			break;
		}
	}
}


void LastFiles::writeFile(string const & filename) const
{
	FilePtr fd(filename, FilePtr::write);
	if (fd()) {
 		for (int i = 0; i < num_files; i++) {
 			if (!files[i].empty())
 				fprintf(fd, "\"%s\"\n", files[i].c_str());
 		}
	} else
		lyxerr.print("LyX: Warning: unable to save LastFiles: "
			      +	filename);
}


void LastFiles::newFile(string const & file)
{
	int n;
	// Find this file in list. If not in list, point to last entry
	for(n = 0; n < (num_files - 1); n++)
		if(files[n] == file) break;

	for(int i = n; i >= 1; i--)
		files[i] = files[i - 1];
	files[0] = file;
}
