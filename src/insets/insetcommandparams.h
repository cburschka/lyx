// -*- C++ -*-
/**
 * \file insetcommandparams.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETCOMMANDPARAMS_H
#define INSETCOMMANDPARAMS_H

#include <string>
#include <iosfwd>


class LyXLex;


class InsetCommandParams {
public:
	///
	InsetCommandParams();
	///
	explicit InsetCommandParams(std::string const & n,
			    std::string const & c = std::string(),
			    std::string const & o = std::string());
	///
	void read(LyXLex &);
	/// Parse the command
	void scanCommand(std::string const &);
	///
	void write(std::ostream &) const;
	/// Build the complete LaTeX command
	std::string const getCommand() const;
	///
	std::string const & getCmdName() const { return cmdname; }
	///
	std::string const & getOptions() const { return options; }
	///
	std::string const & getContents() const { return contents; }
	///
	void setCmdName(std::string const & n) { cmdname = n; }
	///
	void setOptions(std::string const & o) { options = o; }
	///
	void setContents(std::string const & c) { contents = c; }
	///
	bool preview() const { return preview_; }
	///
	void preview(bool p) { preview_ = p; }

private:
	///
	std::string cmdname;
	///
	std::string contents;
	///
	std::string options;
	///
	bool preview_;
};


///
bool operator==(InsetCommandParams const &, InsetCommandParams const &);

///
bool operator!=(InsetCommandParams const &, InsetCommandParams const &);

#endif
