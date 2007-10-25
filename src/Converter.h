// -*- C++ -*-
/**
 * \file Converter.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONVERTER_H
#define CONVERTER_H

#include "Graph.h"
#include "OutputParams.h"

#include <vector>
#include <string>


namespace lyx {

namespace support { class FileName; }

class Buffer;
class ErrorList;
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
	/// The converter is xml
	bool xml;
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
	Converter const & get(int i) const { return converterlist_[i]; }
	///
	Converter const * getConverter(std::string const & from,
				       std::string const & to) const;
	///
	int getNumber(std::string const & from, std::string const & to) const;
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

	std::vector<Format const *> importableFormats();

	std::vector<std::string> loaders() const;

	/// Does a conversion path from format \p from to format \p to exist?
	bool isReachable(std::string const & from, std::string const & to);
	///
	Graph::EdgePath getPath(std::string const & from, std::string const & to);
	///
	OutputParams::FLAVOR getFlavor(Graph::EdgePath const & path);
	/// Flags for converting files
	enum ConversionFlags {
		/// No special flags
		none = 0,
		/// Use the default converter if no converter is defined
		try_default = 1 << 0,
		/// Get the converted file from cache if possible
		try_cache = 1 << 1
	};
	///
	bool convert(Buffer const * buffer,
		     support::FileName const & from_file, support::FileName const & to_file,
		     support::FileName const & orig_from,
		     std::string const & from_format, std::string const & to_format,
		     ErrorList & errorList, int conversionflags = none);
	///
	void update(Formats const & formats);
	///
	void updateLast(Formats const & formats);
	///
	bool formatIsUsed(std::string const & format);
	///
	const_iterator begin() const { return converterlist_.begin(); }
	///
	const_iterator end() const { return converterlist_.end(); }
	///
	void buildGraph();
private:
	///
	std::vector<Format const *> const
	intToFormat(std::vector<int> const & input);
	///
	bool scanLog(Buffer const & buffer, std::string const & command,
		     support::FileName const & filename, ErrorList & errorList);
	///
	bool runLaTeX(Buffer const & buffer, std::string const & command,
		      OutputParams const &, ErrorList & errorList);
	///
	ConverterList converterlist_;
	///
	std::string latex_command_;
	/// If \p from = /path/file.ext and \p to = /path2/file2.ext2 then
	/// this method moves each /path/file*.ext file to /path2/file2*.ext2
	bool move(std::string const & fmt,
		  support::FileName const & from, support::FileName const & to,
		  bool copy);
	///
	Graph G_;
};

/// The global instance.
/// Implementation is in LyX.cpp.
extern Converters & theConverters();

/// The global copy after reading lyxrc.defaults.
/// Implementation is in LyX.cpp.
extern Converters & theSystemConverters();

} // namespace lyx

#endif //CONVERTER_H
