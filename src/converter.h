// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#ifndef CONVERTER_H
#define CONVERTER_H

#ifdef __GNUG__
#pragma interface
#endif

#include <vector>
#include <queue>
#include "LString.h"
#include "support/lstrings.h"

class Buffer;

///
class Format {
public:
	///
	Format(string const & n, string const & e, string const & p,
	       string const & s, string const & v) :
		name_(n), extension_(e), prettyname_(p), shortcut_(s),
		viewer_(v) {};
	///
	bool dummy() const;
	///
	bool IsChildFormat() const;
	///
	string const ParentFormat() const;
	///
	string const & name() const {
		return name_;
	}
	///
	string const & extension() const {
		return extension_;
	}
	///
	string const & prettyname() const {
		return prettyname_;
	}
	///
	string const & shortcut() const {
		return shortcut_;
	}
	///
	string const & viewer() const {
		return viewer_;
	}
	///
	void setViewer(string const & v) {
		viewer_ = v;
	}
private:
	string name_;
	///
	string extension_;
	///
	string prettyname_;
	///
	string shortcut_;
	///
	string viewer_;
};


inline
bool operator<(Format const & a, Format const & b)
{
	return compare_no_case(a.prettyname(),b.prettyname()) < 0;
}


///
class Formats {
public:
        ///
        typedef std::vector<Format> FormatList;
	///
	typedef FormatList::const_iterator const_iterator;
	///
	Format const & Get(FormatList::size_type i) const {
		return formatlist[i];
	}
	///
	Format const * GetFormat(string const & name) const;
	///
	int GetNumber(string const & name) const;
	///
	void Add(string const & name);
	///
	void Add(string const & name, string const & extension, 
		 string const & prettyname, string const & shortcut);
	///
	void Delete(string const & name);
	///
	void Sort();
	///
	void SetViewer(string const & name, string const & command);
	///
	bool View(Buffer const * buffer, string const & filename,
		  string const & format_name) const;
	///
	string const PrettyName(string const & name) const;
	///
	string const Extension(string const & name) const;
	///
	const_iterator begin() const {
		return formatlist.begin();
	}
	///
	const_iterator end() const {
		return formatlist.end();
	}
	///
	FormatList::size_type size() const {
		return formatlist.size();
	}
private:
	///
	FormatList formatlist;
};

///////////////////////////////////////////////////////////////////////

///
class Converter {
public:
	///
	Converter(string const & f, string const & t, string const & c,
		  string const & l)
		: from(f), to(t), command(c), flags(l), From(0), To(0),
		  latex(false), original_dir(false), need_aux(false) {}
	///
	void ReadFlags();
	///
	string from;
	///
	string to;
	///
	string command;
	///
	string flags;
	///
	Format const * From;
	///
	Format const * To;

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
};


///
class Converters {
public:
        typedef std::vector<Converter> ConverterList;
	///
	typedef ConverterList::const_iterator const_iterator;
	///
	typedef std::vector<int> EdgePath;
	///
	Converter const & Get(int i) const {
		return converterlist[i];
	}
	///
	Converter const * GetConverter(string const & from, string const & to);
	///
	int GetNumber(string const & from, string const & to);
	///
	void Add(string const & from, string const & to,
		 string const & command, string const & flags);
	//
	void Delete(string const & from, string const & to);
	///
	void Sort();
	///
	std::vector<Format const *> const
	GetReachableTo(string const & target, bool clear_visited);
	///
	std::vector<Format const *> const
	GetReachable(string const & from, bool only_viewable,
		     bool clear_visited);
	///
	bool IsReachable(string const & from, string const & to);
	///
	EdgePath const GetPath(string const & from, string const & to);
	///
	bool UsePdflatex(EdgePath const & path);
	///
	bool Convert(Buffer const * buffer,
		     string const & from_file, string const & to_file_base,
		     string const & from_format, string const & to_format,
		     string & to_file);
	///
	bool Convert(Buffer const * buffer,
		     string const & from_file, string const & to_file_base,
		     string const & from_format, string const & to_format);
	///
	string const papersize(Buffer const * buffer);
	///
	string const dvips_options(Buffer const * buffer);
	///
	string const dvipdfm_options(Buffer const * buffer);
	///
	void Update(Formats const & formats);
	///
	void UpdateLast(Formats const & formats);
	///
	void BuildGraph();
	///
	bool FormatIsUsed(string const & format);
	///
	const_iterator begin() const {
		return converterlist.begin();
	}
	const_iterator end() const {
		return converterlist.end();
	}
private:
	///
	bool scanLog(Buffer const * buffer, string const & command, 
		     string const & filename);
	///
	bool runLaTeX(Buffer const * buffer, string const & command);
	///
	ConverterList converterlist;
	///
	string latex_command;
	///
	struct Vertex {
		std::vector<int> in_vertices;
		std::vector<int> out_vertices;
		std::vector<int> out_edges;
	};
	///
	static
	std::vector<Vertex> vertices;
	///
	std::vector<bool> visited;
	///
	std::queue<int> Q;
	///
	int BFS_init(string const & start, bool clear_visited = true);
	///
	bool Move(string const & from, string const & to, bool copy);
};

extern Formats formats;
extern Converters converters;

extern Formats system_formats;
extern Converters system_converters;

#endif
