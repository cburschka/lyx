// -*- C++ -*-
/* This file is part of*
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2002-2001 The LyX Team.
 *
 * ====================================================== */

#ifndef INSETCOMMANDPARAMS_H
#define INSETCOMMANDPARAMS_H

#ifdef __GNUG__
#pragma interface
#endif

#include <config.h>

#include "LString.h"

#include <iosfwd>

class LyXLex;

class InsetCommandParams {
public:
	///
	InsetCommandParams();
	///
	explicit
	InsetCommandParams(string const & n,
			    string const & c = string(),
			    string const & o = string());
	///
	void read(LyXLex &);
	/// Parse the command
	void scanCommand(string const &);
	///
	void write(std::ostream &) const;
	/// Build the complete LaTeX command
	string const getCommand() const;
	///
	string const & getCmdName() const { return cmdname; }
	///
	string const & getOptions() const { return options; }
	///
	string const & getContents() const { return contents; }
	///
	void setCmdName(string const & n) { cmdname = n; }
	///
	void setOptions(string const & o) { options = o; }
	///
	void setContents(string const & c) { contents = c; }
	///
	string const getAsString() const;
	///
	void setFromString(string const &);
	///
	bool preview() const { return preview_; }
	///
	void preview(bool p) { preview_ = p; }

private:
	///
	string cmdname;
	///
	string contents;
	///
	string options;
	///
	bool preview_;
};


///
bool operator==(InsetCommandParams const &,
		InsetCommandParams const &);

///
bool operator!=(InsetCommandParams const &,
		InsetCommandParams const &);

#endif
