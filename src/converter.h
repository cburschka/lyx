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
	bool isChildFormat() const;
	///
	string const parentFormat() const;
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
	return compare_no_case(a.prettyname(), b.prettyname()) < 0;
}


///
class Formats {
public:
	///
	typedef std::vector<Format> FormatList;
	///
	typedef FormatList::const_iterator const_iterator;
	///
	Format const & get(FormatList::size_type i) const {
		return formatlist[i];
	}
	///
	Format const * getFormat(string const & name) const;
	///
	int getNumber(string const & name) const;
	///
	void add(string const & name);
	///
	void add(string const & name, string const & extension,
		 string const & prettyname, string const & shortcut);
	///
	void erase(string const & name);
	///
	void sort();
	///
	void setViewer(string const & name, string const & command);
	///
	bool view(Buffer const * buffer, string const & filename,
		  string const & format_name) const;
	///
	string const prettyName(string const & name) const;
	///
	string const extension(string const & name) const;
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
	void readFlags();
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
	Converter const & get(int i) const {
		return converterlist_[i];
	}
	///
	Converter const * getConverter(string const & from, string const & to);
	///
	int getNumber(string const & from, string const & to);
	///
	void add(string const & from, string const & to,
		 string const & command, string const & flags);
	//
	void erase(string const & from, string const & to);
	///
	void sort();
	///
	std::vector<Format const *> const
	getReachableTo(string const & target, bool clear_visited);
	///
	std::vector<Format const *> const
	getReachable(string const & from, bool only_viewable,
		     bool clear_visited);
	///
	bool isReachable(string const & from, string const & to);
	///
	EdgePath const getPath(string const & from, string const & to);
	///
	bool usePdflatex(EdgePath const & path);
	///
	bool convert(Buffer const * buffer,
		     string const & from_file, string const & to_file_base,
		     string const & from_format, string const & to_format,
		     string & to_file);
	///
	bool convert(Buffer const * buffer,
		     string const & from_file, string const & to_file_base,
		     string const & from_format, string const & to_format);
	///
	string const papersize(Buffer const * buffer);
	///
	string const dvips_options(Buffer const * buffer);
	///
	string const dvipdfm_options(Buffer const * buffer);
	///
	void update(Formats const & formats);
	///
	void updateLast(Formats const & formats);
	///
	void buildGraph();
	///
	bool formatIsUsed(string const & format);
	///
	const_iterator begin() const {
		return converterlist_.begin();
	}
	const_iterator end() const {
		return converterlist_.end();
	}
private:
	///
	bool scanLog(Buffer const * buffer, string const & command,
		     string const & filename);
	///
	bool runLaTeX(Buffer const * buffer, string const & command);
	///
	ConverterList converterlist_;
	///
	string latex_command_;
	///
	struct Vertex {
		std::vector<int> in_vertices;
		std::vector<int> out_vertices;
		std::vector<int> out_edges;
	};
	///
	static
	std::vector<Vertex> vertices_;
	///
	std::vector<bool> visited_;
	///
	std::queue<int> Q_;
	///
	int bfs_init(string const & start, bool clear_visited = true);
	///
	bool move(string const & from, string const & to, bool copy);
};

extern Formats formats;
extern Converters converters;

extern Formats system_formats;
extern Converters system_converters;

#endif
