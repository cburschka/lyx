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

struct Command {
	Command(string const & f, string const & t, string const & c,
		bool o)
		: from(f), to(t), command(c), original_dir(o) {}
	///
	string from;
	///
	string to;
	///
	string command;
	///
	bool original_dir;
	///
	bool visited;
	///
	std::vector<Command>::iterator previous;
};

class Format {
public:
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
	bool View(string const & filename);
	///
	static
	Format * GetFormat(string const & name);
	///
	static
	string PrettyName(string const & name);
private:
	///
	static
	std::map<string, Format> formats;
};

class Converter {
public:
	///
	static
	void Add(string const & from, string const & to,
		 string const & command, string const & flags);
	///
	static
	std::vector<std::pair<string, string> > GetReachable(string const & from,
							     bool only_viewable = false);
	///
	static
	bool convert(Buffer * buffer, string const & from_file,
		     string const & to_format);
	///
	static
	string SplitFormat(string const & str, string & format); 
private:
	///
	static
	bool runLaTeX(Buffer * buffer, string const & command);
	///
	static
	std::vector<Command> commands;
};

#endif
