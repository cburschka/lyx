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
class Format {
public:
	///
	Format() {}
	///
	Format(string const & n, string const & e, string const & p,
	       string const & s, string const & v) :
		name(n), extension(e), prettyname(p), shortcut(s),
		viewer(v) {};
	///
	string name;
	///
	string extension;
	///
	string prettyname;
	///
	string shortcut;
	///
	string viewer;
	///
	bool dummy() const;
	///
	string const getname() const {
		return name;
	}
};

///
struct Command {
	///
	Command(Format const * f, Format const * t, string const & c)
		: from(f), to(t), command(c), importer(false), 
		  latex(false), original_dir(false), need_aux(false) {}
	///
	Format const * from;
	///
	Format const * to;
	///
	string command;
	/// The converter is used for importing
	bool importer;
	/// The converter is latex or its derivatives
	bool latex;
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
	/// Command to convert the program output to a LaTeX log file format
	string parselog;
	/// Backends in which the converter is not used
	std::vector<string> disable;

	/// Used by the BFS algorithm
	bool visited;
	/// Used by the BFS algorithm
	std::vector<Command>::iterator previous;
};

class FormatPair {
public:
	///
	Format const * format;
	///
	Format const * from;
	///
	string command;
	///
	FormatPair(Format const * f1, Format const * f2, string c)
		: format(f1), from(f2), command(c) {}
};

///
class Formats {
public:
        ///
        typedef std::map<string, Format> FormatList;
	///
	void Add(string const & name);
	///
	void Add(string const & name, string const & extension, 
		 string const & prettyname, string const & shortcut);
	///
	void SetViewer(string const & name, string const & command);
	///
	bool View(Buffer const * buffer, string const & filename,
		  string const & format_name);
	///
	Format * GetFormat(string const & name);
	///
	string const PrettyName(string const & name);
	///
	string const Extension(string const & name);
	///
	std::vector<Format> const GetAllFormats() const;
private:
	///
	FormatList formats;
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
	std::vector<FormatPair> const GetReachableTo(string const & target);
	///
	static
	std::vector<FormatPair> const
	GetReachable(string const & from, bool only_viewable);
	///
	static
	bool IsReachable(string const & from, string const & to);
	///
	static
	bool Convert(Buffer const * buffer,
		     string const & from_file, string const & to_file_base,
		     string const & from_format, string const & to_format,
		     string const & using_format, string & to_file);
	///
	static
	bool Convert(Buffer const * buffer,
		     string const & from_file, string const & to_file_base,
		     string const & from_format, string const & to_format,
		     string const & using_format = string());
	///
	static
	string const SplitFormat(string const & str, string & format);
	///
	static
	string const dvi_papersize(Buffer const * buffer);
	///
	static
	string const dvips_options(Buffer const * buffer);
	///
	static
	void init();
	///
	static
	std::vector<Command> const GetAllCommands();
private:
	///
	static
	bool scanLog(Buffer const * buffer, string const & command, 
		     string const & filename);
	///
	static
	bool runLaTeX(Buffer const * buffer, string const & command);
	///
	static
	std::vector<Command> commands;
	///
	static
	string latex_command;
};

extern Formats formats;
extern Formats system_formats;


#endif
