// -*- C++ -*-
/**
 * \file texstream.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Enrico Forestieri
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_TEXSTREAM_H
#define LYX_TEXSTREAM_H

#include "support/docstream.h"
#include "support/unique_ptr.h"

namespace lyx {

class TexRow;
struct TexString;


/** Wrapper class for odocstream.
    This class is used to automatically count the lines of the exported latex
    code.
  */

class otexrowstream {
public:
	///
	explicit otexrowstream(odocstream & os);
	/// defaulted
	~otexrowstream();
	///
	odocstream & os() { return os_; }
	///
	TexRow & texrow() { return *texrow_; }
	///
	unique_ptr<TexRow> releaseTexRow();
	///
	void put(char_type const & c);
	///
	void append(TexString ts);
private:
	///
	odocstream & os_;
	///
	unique_ptr<TexRow> texrow_;
};

///
otexrowstream & operator<<(otexrowstream &, odocstream_manip);
///
otexrowstream & operator<<(otexrowstream &, TexString);
///
otexrowstream & operator<<(otexrowstream &, docstring const &);
///
otexrowstream & operator<<(otexrowstream &, std::string const &);
///
otexrowstream & operator<<(otexrowstream &, char const *);
///
otexrowstream & operator<<(otexrowstream &, char);
///
template <typename Type>
otexrowstream & operator<<(otexrowstream & ots, Type value);


/** Subclass for otexrowstream.
    This class is used to ensure that no blank lines may be inadvertently output.
    To this end, use the special variables "breakln" and "safebreakln" as if
    they were iomanip's to ensure that the next output will start at the
    beginning of a line. Using "breakln", a '\n' char will be output if needed,
    while using "safebreakln", "%\n" will be output if needed.
    The class also records the last output character and can tell whether
    a paragraph break was just output.
  */

class otexstream : public otexrowstream {
public:
	///
	explicit otexstream(odocstream & os)
		: otexrowstream(os), canbreakline_(false),
		  protectspace_(false), terminate_command_(false),
		  parbreak_(true), blankline_(true), lastchar_(0) {}
	///
	void put(char_type const & c);
	///
	void append(TexString ts);
	///
	void canBreakLine(bool breakline) { canbreakline_ = breakline; }
	///
	bool canBreakLine() const { return canbreakline_; }
	///
	void protectSpace(bool protectspace) { protectspace_ = protectspace; }
	///
	bool protectSpace() const { return protectspace_; }
	///
	void terminateCommand(bool terminate) { terminate_command_ = terminate; }
	///
	bool terminateCommand() const { return terminate_command_; }
	///
	void lastChar(char_type const & c)
	{
		parbreak_ = (!canbreakline_ && c == '\n');
		blankline_ = ((!canbreakline_ && c == ' ') || c == '\n');
		canbreakline_ = (c != '\n');
		lastchar_ = c;
	}
	///
	char_type lastChar() const { return lastchar_; }
	///
	bool afterParbreak() const { return parbreak_; }
	///
	bool blankLine() const { return blankline_; }
private:
	///
	bool canbreakline_;
	///
	bool protectspace_;
	///
	bool terminate_command_;
	///
	bool parbreak_;
	///
	bool blankline_;
	///
	char_type lastchar_;
};


/// because we need to pass ods_ to the base class
struct otexstringstream_helper { odocstringstream ods_; };

/// otexstringstream : a odocstringstream with tex/row correspondence
class otexstringstream : otexstringstream_helper, public otexstream {
public:
	otexstringstream() : otexstringstream_helper(), otexstream(ods_) {}
	///
	docstring str() const { return ods_.str(); }
	///
	size_t length();
	///
	bool empty() { return 0 == length(); }
	/// move-returns the contents and reset the texstream
	TexString release();
};


/// Helper structs for breaking a line
struct BreakLine {
	char n;
};

struct SafeBreakLine {
	char n;
};

/// Helper structs for terminating a command
struct TerminateCommand {
	char n;
};

extern BreakLine breakln;
extern SafeBreakLine safebreakln;
extern TerminateCommand termcmd;

///
otexstream & operator<<(otexstream &, BreakLine);
///
otexstream & operator<<(otexstream &, SafeBreakLine);
///
otexstream & operator<<(otexstream &, TerminateCommand);
///
otexstream & operator<<(otexstream &, odocstream_manip);
///
otexstream & operator<<(otexstream &, TexString);
///
otexstream & operator<<(otexstream &, docstring const &);
///
otexstream & operator<<(otexstream &, std::string const &);
///
otexstream & operator<<(otexstream &, char const *);
///
otexstream & operator<<(otexstream &, char);
///
template <typename Type>
otexstream & operator<<(otexstream & ots, Type value);


} // namespace lyx

#endif
