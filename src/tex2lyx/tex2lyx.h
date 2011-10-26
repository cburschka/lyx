// -*- C++ -*-
/**
 * \file tex2lyx.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef TEX2LYX_H
#define TEX2LYX_H

#include "Parser.h"
#include "TextClass.h"

#include <iosfwd>
#include <string>
#include <vector>
#include <map>


namespace lyx {

namespace support { class FileName; }

/// Simple support for frontend::Alert::warning().
namespace frontend { 
namespace Alert {
	void warning(docstring const & title, docstring const & message,
				 bool const &);
}
}

class Context;

/// A trivial subclass, just to give us a public default constructor
class TeX2LyXDocClass : public DocumentClass
{
public:
	void setName(std::string const & name) { name_ = name; }
};

/// in preamble.cpp
void parse_preamble(Parser & p, std::ostream & os, 
	std::string const & forceclass, TeX2LyXDocClass & tc);
/// Translate babel language name to LyX language name
extern std::string babel2lyx(std::string const & language);
/// translate color name to LyX color code
extern std::string color2code(std::string const & name);

/// used packages with options
extern std::map<std::string, std::vector<std::string> > used_packages;
extern const char * const modules_placeholder;
extern std::string h_inputencoding;
extern std::string h_paragraph_separation;

/// in text.cpp
std::string translate_len(std::string const &);

void parse_text(Parser & p, std::ostream & os, unsigned flags, bool outer,
		Context & context);

/*!
 * Parses a subdocument, usually useful in insets (whence the name).
 *
 * It ignores \c context.need_layout and \c context.need_end_layout and
 * starts and ends always a new layout.
 * Therefore this may only be used to parse text in insets or table cells.
 */
void parse_text_in_inset(Parser & p, std::ostream & os, unsigned flags,
                         bool outer, Context const & context,
                         InsetLayout const * layout = 0);


/// in math.cpp
void parse_math(Parser & p, std::ostream & os, unsigned flags, mode_type mode);


/// in table.cpp
void handle_tabular(Parser & p, std::ostream & os, bool is_long_tabular,
		    Context & context);


/// in tex2lyx.cpp
std::string const trim(std::string const & a, char const * p = " \t\n\r");

void split(std::string const & s, std::vector<std::string> & result,
	char delim = ',');
std::string join(std::vector<std::string> const & input,
	char const * delim);

bool is_math_env(std::string const & name);
char const * const * is_known(std::string const &, char const * const *);

/*!
 * Adds the command \p command to the list of known commands.
 * \param o1 first optional parameter to the latex command \\newcommand
 * (with brackets), or the empty string if there were no optional arguments.
 * \param o2 wether \\newcommand had a second optional parameter.
 * If \p definition is not empty the command is assumed to be from the LyX
 * preamble and added to possible_textclass_commands.
 */
void add_known_command(std::string const & command, std::string const & o1,
	bool o2, docstring const & definition = docstring());
extern void add_known_environment(std::string const & environment,
	std::string const & o1, bool o2, docstring const & beg,
	docstring const & end);
extern Layout const * findLayoutWithoutModule(TextClass const & textclass,
	std::string const & name, bool command);
extern InsetLayout const * findInsetLayoutWithoutModule(
	TextClass const & textclass, std::string const & name, bool command);
/*!
 * Check whether a module provides command (if \p command is true) or
 * environment (if \p command is false) \p name, and add the module to the
 * list of used modules if yes.
 */
extern bool checkModule(std::string const & name, bool command);
// Access to environment stack
extern std::vector<std::string> active_environments;
std::string active_environment();

enum ArgumentType {
	required,
	verbatim,
	item,
	optional
};

class FullCommand {
public:
	FullCommand() {}
	FullCommand(std::vector<ArgumentType> const & a, docstring const & d)
		: args(a), def(d) {}
	std::vector<ArgumentType> args;
	docstring def;
};

class FullEnvironment {
public:
	FullEnvironment() {}
	FullEnvironment(std::vector<ArgumentType> const & a,
	                docstring const & b, docstring const & e)
		: args(a), beg(b), end(e) {}
	std::vector<ArgumentType> args;
	docstring beg;
	docstring end;
};

typedef std::map<std::string, std::vector<ArgumentType> > CommandMap;
typedef std::map<std::string, FullCommand> FullCommandMap;
typedef std::map<std::string, FullEnvironment> FullEnvironmentMap;

/// Known TeX commands with arguments that get parsed into ERT.
extern CommandMap known_commands;
/// Known TeX environments with arguments that get parsed into ERT.
extern CommandMap known_environments;
/// Known TeX math environments with arguments that get parsed into LyX mathed.
extern CommandMap known_math_environments;
/// Commands that might be defined by the document class or modules
extern FullCommandMap possible_textclass_commands;
/// Environments that might be defined by the document class or modules
extern FullEnvironmentMap possible_textclass_environments;
///
extern bool noweb_mode;
/// Did we recognize any pdflatex-only construct?
extern bool pdflatex;
/// LyX format that is created by tex2lyx
int const LYX_FORMAT = 413;

/// path of the master .tex file
extern std::string getMasterFilePath();
/// path of the currently processed .tex file
extern std::string getParentFilePath();


/*!
 *  Reads tex input from \a infilename and writes lyx output to \a outfilename.
 *  The (latex) encoding can be provided as \a encoding.
 *  Uses some common settings for the preamble, so this should only
 *  be used more than once for included documents.
 *  Caution: Overwrites the existing preamble settings if the new document
 *  contains a preamble.
 *  \return true if the conversion was successful, else false.
 */
bool tex2lyx(std::string const & infilename, 
	     support::FileName const & outfilename, 
	     std::string const & encoding);


} // namespace lyx

#endif
