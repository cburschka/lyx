// -*- C++ -*-
/**
 * \file converter.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONVERTER_H
#define CONVERTER_H

#include "graph.h"

#include <vector>
#include <string>


class Buffer;
class Format;
class Formats;
class OutputParams;


///
class Converter {
public:
	///
	Converter(std::string const & f, std::string const & t, std::string const & c,
		  std::string const & l);
	///
	void readFlags();
	///
	std::string from;
	///
	std::string to;
	///
	std::string command;
	///
	std::string flags;
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
	std::string result_dir;
	/// If the converter put the result in a directory, then result_file
	/// is the name of the main file in that directory
	std::string result_file;
	/// Command to convert the program output to a LaTeX log file format
	std::string parselog;
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
	Converter const * getConverter(std::string const & from, std::string const & to);
	///
	int getNumber(std::string const & from, std::string const & to);
	///
	void add(std::string const & from, std::string const & to,
		 std::string const & command, std::string const & flags);
	//
	void erase(std::string const & from, std::string const & to);
	///
	void sort();
	///
	std::vector<Format const *> const
	getReachableTo(std::string const & target, bool clear_visited);
	///
	std::vector<Format const *> const
	getReachable(std::string const & from, bool only_viewable,
		     bool clear_visited);
	///
	bool isReachable(std::string const & from, std::string const & to);
	///
	Graph::EdgePath const getPath(std::string const & from, std::string const & to);
	///
	bool usePdflatex(Graph::EdgePath const & path);
	///
	bool convert(Buffer const * buffer,
		     std::string const & from_file, std::string const & to_file_base,
		     std::string const & from_format, std::string const & to_format,
		     std::string & to_file);
	///
	bool convert(Buffer const * buffer,
		     std::string const & from_file, std::string const & to_file_base,
		     std::string const & from_format, std::string const & to_format);
	///
	void update(Formats const & formats);
	///
	void updateLast(Formats const & formats);
	///
	bool formatIsUsed(std::string const & format);
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
	intToFormat(std::vector<int> const & input);
	///
	bool scanLog(Buffer const & buffer, std::string const & command,
		     std::string const & filename);
	///
	bool runLaTeX(Buffer const & buffer, std::string const & command,
		      OutputParams const &);
	///
	ConverterList converterlist_;
	///
	std::string latex_command_;
	///
	bool move(std::string const & from, std::string const & to, bool copy);
	///
	Graph G_;
};

extern Converters converters;

extern Converters system_converters;

#endif //CONVERTER_H
