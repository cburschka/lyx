// -*- C++ -*-

#ifndef CONVERTER_H
#define CONVERTER_H

/**
 * \file converter.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS
 */

#include "latexrunparams.h"
#include "graph.h"

#include <vector>

class Format;
class Formats;

class Buffer;

///
class Converter {
public:
	///
	Converter(string const & f, string const & t, string const & c,
		  string const & l);
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
	///
	typedef std::vector<int> EdgePath; // to be removed SOON
	///
	typedef std::vector<Converter> ConverterList;
	///
	typedef ConverterList::const_iterator const_iterator;
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
	Graph::EdgePath const getPath(string const & from, string const & to);
	///
	bool usePdflatex(Graph::EdgePath const & path);
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
	void update(Formats const & formats);
	///
	void updateLast(Formats const & formats);
	///
	bool formatIsUsed(string const & format);
	///
	const_iterator begin() const {
		return converterlist_.begin();
	}
	const_iterator end() const {
		return converterlist_.end();
	}
	///
	void buildGraph();
private:
	///
	std::vector<Format const *> const
	Converters::intToFormat(std::vector<int> const & input);
	///
	bool scanLog(Buffer const * buffer, string const & command,
		     string const & filename);
	///
	bool runLaTeX(Buffer const * buffer, string const & command,
		      LatexRunParams const &);
	///
	ConverterList converterlist_;
	///
	string latex_command_;
	///
	bool move(string const & from, string const & to, bool copy);
	///
	Graph G_;
};

extern Converters converters;

extern Converters system_converters;

#endif //CONVERTER_H
