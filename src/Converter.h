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
#include "support/trivstring.h"

#include <vector>
#include <set>
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
	std::string const from() const { return from_; }
	///
	std::string const to() const { return to_; }
	///
	std::string const command() const { return command_; }
	///
	void setCommand(std::string const & command) { command_ = command; }
	///
	std::string const flags() const { return flags_; }
	///
	void setFlags(std::string const & flags) { flags_ = flags; }
	///
	Format const * From() const { return From_; }
	///
	void setFrom(Format const * From) { From_ = From; }
	///
	void setTo(Format const * To) { To_ = To; }
	///
	Format const * To() const { return To_; }
	///
	bool latex() const { return latex_; }
	///
	std::string const latex_flavor() const { return latex_flavor_; }
	///
	bool xml() const { return xml_; }
	///
	bool need_aux() const { return need_aux_; }
	///
	bool nice() const { return nice_; }
	///
	std::string const result_dir() const { return result_dir_; }
	///
	std::string const result_file() const { return result_file_; }
	///
	std::string const parselog() const { return parselog_; }
private:
	///
	trivstring from_;
	///
	trivstring to_;
	///
	trivstring command_;
	///
	trivstring flags_;
	///
	Format const * From_;
	///
	Format const * To_;

	/// The converter is latex or its derivatives
	bool latex_;
	/// The latex derivate
	trivstring latex_flavor_;
	/// The converter is xml
	bool xml_;
	/// This converter needs the .aux files
	bool need_aux_;
	/// we need a "nice" file from the backend, c.f. OutputParams.nice.
	bool nice_;
	/// If the converter put the result in a directory, then result_dir
	/// is the name of the directory
	trivstring result_dir_;
	/// If the converter put the result in a directory, then result_file
	/// is the name of the main file in that directory
	trivstring result_file_;
	/// Command to convert the program output to a LaTeX log file format
	trivstring parselog_;
};


///
class Converters {
public:
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
	std::vector<Format const *> const
	getReachableTo(std::string const & target, bool clear_visited);
	///
	std::vector<Format const *> const
	getReachable(std::string const & from, bool only_viewable,
	       bool clear_visited,
	       std::set<std::string> const & excludes = std::set<std::string>());

	std::vector<Format const *> importableFormats();
	std::vector<Format const *> exportableFormats(bool only_viewable);

	std::vector<std::string> loaders() const;
	std::vector<std::string> savers() const;

	/// Does a conversion path from format \p from to format \p to exist?
	bool isReachable(std::string const & from, std::string const & to);
	///
	Graph::EdgePath getPath(std::string const & from, std::string const & to);
	///
	OutputParams::FLAVOR getFlavor(Graph::EdgePath const & path,
				       Buffer const * buffer = 0);
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
	trivstring latex_command_;
	///
	trivstring dvilualatex_command_;
	///
	trivstring lualatex_command_;
	///
	trivstring pdflatex_command_;
	///
	trivstring xelatex_command_;
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
