// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#ifndef CONVERTER_H
#define CONVERTER_H

#ifdef __GNUG__
#pragma interface
#endif

#include <map>
#include <vector>
#include "LString.h"

class Buffer;

///
struct Command {
	///
	Command(string const & f, string const & t, string const & c)
		: from(f), to(t), command(c),
		  original_dir(false), need_aux(false) {}
	///
	string from;
	///
	string to;
	///
	string command;
	/// Do we need to run the converter in the original directory?
	bool original_dir;
	/// This converter needs the .aux files
	bool need_aux;
	/// If the converter put the result in a directory, then result_dir
	/// is the name of the directory
	string result_dir;
	/// If the converter put the result in a directory, then result_file
	/// is the name of the main file in that directory
	string result_file;
	///
	bool visited;
	///
	std::vector<Command>::iterator previous;
};

///
class Format {
public:
	///
	Format() : in_degree(0) {}
	///
	Format(string const & n);
	///
	string name;
	///
	string prettyname;
	///
	string viewer;
	///
	int in_degree;
};

///
class Formats {
public:
	///
	static
	void Add(string const & name);
	///
	static
	void SetViewer(string const & name, string const & command);
	///
	static
	bool View(Buffer * buffer, string const & filename);
	///
	static
	Format * GetFormat(string const & name);
	///
	static
	string const PrettyName(string const & name);
private:
	///
	static
	std::map<string, Format> formats;
};

///
class Converter {
public:
	///
	static
	void Add(string const & from, string const & to,
		 string const & command, string const & flags);
	///
	static
	std::vector<std::pair<string, string> > const
	GetReachable(string const & from,
		     bool only_viewable = false);
	///
	static
	bool Convert(Buffer * buffer, string const & from_file,
		     string const & to_file, string const & using_format,
		     string * view_file = 0);
	static
	string const SplitFormat(string const & str, string & format);
	///
	static
	string dvi_papersize(Buffer * buffer);
	///
	static
	string dvips_options(Buffer * buffer);
private:
	///
	static
	bool runLaTeX(Buffer * buffer, string const & command);
	///
	static
	std::vector<Command> commands;
	///
	static
	string latex_command;
};

#endif
