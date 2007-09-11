/**
 * \file InsetCommandParams.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Georg Baum
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetCommandParams.h"

#include "debug.h"
#include "gettext.h"
#include "Lexer.h"

#include "support/ExceptionMessage.h"
#include "support/lstrings.h"

#include <boost/assert.hpp>


namespace lyx {

using support::findToken;

using std::string;
using std::endl;
using std::ostream;

using support::ExceptionMessage;
using support::WarningException;

InsetCommandParams::InsetCommandParams(string const & name)
	: name_(name), preview_(false)
{
	info_ = findInfo(name);
	BOOST_ASSERT(info_);
	params_.resize(info_->n);
}


InsetCommandParams::CommandInfo const *
InsetCommandParams::findInfo(std::string const & name)
{
	// No parameter may be named "preview", because that is a required
	// flag for all commands.

	// InsetBibitem
	if (name == "bibitem") {
		static const char * const paramnames[] = {"label", "key", ""};
		static const bool isoptional[] = {true, false};
		static const CommandInfo info = {2, paramnames, isoptional};
		return &info;
	}

	// InsetBibtex
	if (name == "bibtex") {
		static const char * const paramnames[] =
				{"options", "btprint", "bibfiles", ""};
		static const bool isoptional[] = {true, true, false};
		static const CommandInfo info = {3, paramnames, isoptional};
		return &info;
	}

	// InsetCitation
	// FIXME: Use is_possible_cite_command() in
	// InsetCitation, see comment in src/factory.cpp.
	if (name == "cite" || name == "citet" || name == "citep" || name == "citealt" ||
	    name == "citealp" || name == "citeauthor" || name == "citeyear" ||
	    name == "citeyearpar" || name == "citet*" || name == "citep*" ||
	    name == "citealt*" || name == "citealp*" ||
	    name == "citeauthor*" || name == "Citet" || name == "Citep" ||
	    name == "Citealt" || name == "Citealp" || name == "Citeauthor" ||
	    name == "Citet*" || name == "Citep*" || name == "Citealt*" ||
	    name == "Citealp*" || name == "Citeauthor*" ||
	    name == "citefield" || name == "citetitle" || name == "cite*") {
		// standard cite does only take one argument if jurabib is
		// not used, but jurabib extends this to two arguments, so
		// we have to allow both here. InsetCitation takes care that
		// LaTeX output is nevertheless correct.
		static const char * const paramnames[] =
				{"after", "before", "key", ""};
		static const bool isoptional[] = {true, true, false};
		static const CommandInfo info = {3, paramnames, isoptional};
		return &info;
	}

	// InsetFloatlist
	if (name == "floatlist") {
		static const char * const paramnames[] = {"type", ""};
		static const bool isoptional[] = {false};
		static const CommandInfo info = {1, paramnames, isoptional};
		return &info;
	}

	// InsetHfill
	if (name == "hfill") {
		static const char * const paramnames[] = {""};
		static const CommandInfo info = {0, paramnames, 0};
		return &info;
	}

	// InsetInclude
	if (name == "include" || name == "input" || name == "verbatiminput" ||
	    name == "verbatiminput*") {
		static const char * const paramnames[] = {"filename", ""};
		static const bool isoptional[] = {false};
		static const CommandInfo info = {1, paramnames, isoptional};
		return &info;
	}

	if (name == "lstinputlisting") {
		static const char * const paramnames[] = {"filename", "lstparams", ""};
		static const bool isoptional[] = {false, true};
		static const CommandInfo info = {2, paramnames, isoptional};
		return &info;
	}

	// InsetIndex, InsetPrintIndex, InsetLabel
	if (name == "index" || name == "printindex" || name == "label") {
		static const char * const paramnames[] = {"name", ""};
		static const bool isoptional[] = {false};
		static const CommandInfo info = {1, paramnames, isoptional};
		return &info;
	}

	// InsetNomencl
	if (name == "nomenclature") {
		static const char * const paramnames[] = {"prefix", "symbol", "description", ""};
		static const bool isoptional[] = {true, false, false};
		static const CommandInfo info = {3, paramnames, isoptional};
		return &info;
	}

	// InsetPrintNomencl
	if (name == "printnomenclature") {
		static const char * const paramnames[] = {"labelwidth", ""};
		static const bool isoptional[] = {true};
		static const CommandInfo info = {1, paramnames, isoptional};
		return &info;
	}

	// InsetRef
	if (name == "eqref" || name == "pageref" || name == "vpageref" ||
	    name == "vref" || name == "prettyref" || name == "ref") {
		static const char * const paramnames[] =
				{"name", "reference", ""};
		static const bool isoptional[] = {true, false};
		static const CommandInfo info = {2, paramnames, isoptional};
		return &info;
	}

	// InsetTOC
	if (name == "tableofcontents") {
		static const char * const paramnames[] = {"type", ""};
		static const bool isoptional[] = {false};
		static const CommandInfo info = {1, paramnames, isoptional};
		return &info;
	}

	// InsetUrl
	if (name == "htmlurl" || name == "url") {
		static const char * const paramnames[] =
				{"name", "target", ""};
		static const bool isoptional[] = {true, false};
		static const CommandInfo info = {2, paramnames, isoptional};
		return &info;
	}

	return 0;
}


void InsetCommandParams::setCmdName(string const & name)
{
	name_ = name;
	CommandInfo const * const info = findInfo(name);
	BOOST_ASSERT(info);
	ParamVector params(info->n);
	// Overtake parameters with the same name
	for (size_t i = 0; i < info_->n; ++i) {
		int j = findToken(info->paramnames, info_->paramnames[i]);
		if (j >= 0)
			params[j] = params_[i];
	}
	info_ = info;
	std::swap(params, params_);
}


void InsetCommandParams::scanCommand(string const & cmd)
{
	string tcmdname, toptions, tsecoptions, tcontents;

	if (cmd.empty()) return;

	enum { WS, CMDNAME, OPTION, SECOPTION, CONTENT } state = WS;

	// Used to handle things like \command[foo[bar]]{foo{bar}}
	int nestdepth = 0;

	for (string::size_type i = 0; i < cmd.length(); ++i) {
		char const c = cmd[i];
		if ((state == CMDNAME && c == ' ') ||
		    (state == CMDNAME && c == '[') ||
		    (state == CMDNAME && c == '{')) {
			state = WS;
		}
		if ((state == OPTION  && c == ']') ||
		    (state == SECOPTION  && c == ']') ||
		    (state == CONTENT && c == '}')) {
			if (nestdepth == 0) {
				state = WS;
			} else {
				--nestdepth;
			}
		}
		if ((state == OPTION  && c == '[') ||
		    (state == SECOPTION  && c == '[') ||
		    (state == CONTENT && c == '{')) {
			++nestdepth;
		}
		switch (state) {
		case CMDNAME:	tcmdname += c; break;
		case OPTION:	toptions += c; break;
		case SECOPTION:	tsecoptions += c; break;
		case CONTENT:	tcontents += c; break;
		case WS: {
			char const b = i? cmd[i-1]: 0;
			if (c == '\\') {
				state = CMDNAME;
			} else if (c == '[' && b != ']') {
				state = OPTION;
				nestdepth = 0; // Just to be sure
			} else if (c == '[' && b == ']') {
				state = SECOPTION;
				nestdepth = 0; // Just to be sure
			} else if (c == '{') {
				state = CONTENT;
				nestdepth = 0; // Just to be sure
			}
			break;
		}
		}
	}

	// Don't mess with this.
	if (!tcmdname.empty())  setCmdName(tcmdname);
	if (!toptions.empty())  setOptions(toptions);
	if (!tsecoptions.empty())  setSecOptions(tsecoptions);
	if (!tcontents.empty()) setContents(tcontents);

	LYXERR(Debug::PARSER) << "Command <" <<  cmd
		<< "> == <" << to_utf8(getCommand())
		<< "> == <" << getCmdName()
		<< '|' << getContents()
		<< '|' << getOptions()
		<< '|' << getSecOptions() << '>' << endl;
}


void InsetCommandParams::read(Lexer & lex)
{
	if (lex.isOK()) {
		lex.next();
		name_ = lex.getString();
		info_ = findInfo(name_);
		if (!info_) {
			lex.printError("InsetCommand: Unknown inset name `$$Token'");
			throw ExceptionMessage(WarningException,
				_("Unknown inset name: "),
				from_utf8(name_));
		}
	}

	string token;
	while (lex.isOK()) {
		lex.next();
		token = lex.getString();
		if (token == "\\end_inset")
			break;
		// FIXME Why is preview_ read but not written?
		if (token == "preview") {
			lex.next();
			preview_ = lex.getBool();
			continue;
		}
		int const i = findToken(info_->paramnames, token);
		if (i >= 0) {
			lex.next(true);
			params_[i] = lex.getDocString();
		} else {
			lex.printError("Unknown parameter name `$$Token' for command " + name_);
			throw ExceptionMessage(WarningException,
				_("Inset Command: ") + from_ascii(name_),
				_("Unknown parameter name: ") + from_utf8(token));
		}
	}
	if (token != "\\end_inset") {
		lex.printError("Missing \\end_inset at this point. "
			       "Read: `$$Token'");
		throw ExceptionMessage(WarningException,
			_("Missing \\end_inset at this point."),
			from_utf8(token));
	}
}


void InsetCommandParams::write(ostream & os) const
{
	os << "LatexCommand " << name_ << '\n';
	for (size_t i = 0; i < info_->n; ++i)
		if (!params_[i].empty())
			// FIXME UNICODE
			os << info_->paramnames[i] << ' '
			   << Lexer::quoteString(to_utf8(params_[i]))
			   << '\n';
}


docstring const InsetCommandParams::getCommand() const
{
	docstring s = '\\' + from_ascii(name_);
	bool noparam = true;
	for (size_t i = 0; i < info_->n; ++i) {
		if (info_->optional[i]) {
			if (params_[i].empty()) {
				// We need to write this parameter even if
				// it is empty if nonempty optional parameters
				// follow before the next required parameter.
				for (size_t j = i + 1; j < info_->n; ++j) {
					if (!info_->optional[j])
						break;
					if (!params_[j].empty()) {
						s += "[]";
						noparam = false;
						break;
					}
				}
			} else {
				s += '[' + params_[i] + ']';
				noparam = false;
			}
		} else {
			s += '{' + params_[i] + '}';
			noparam = false;
		}
	}
	if (noparam)
		// Make sure that following stuff does not change the
		// command name.
		s += "{}";
	return s;
}


std::string const InsetCommandParams::getOptions() const
{
	for (size_t i = 0; i < info_->n; ++i)
		if (info_->optional[i])
			return to_utf8(params_[i]);
	lyxerr << "Programming error: get nonexisting option in "
	       << name_ << " inset." << endl;;
	return string();
}


std::string const InsetCommandParams::getSecOptions() const
{
	bool first = true;
	for (size_t i = 0; i < info_->n; ++i)
		if (info_->optional[i]) {
			if (first)
				first = false;
			else
				return to_utf8(params_[i]);
		}
	// Happens in InsetCitation
	lyxerr << "Programming error: get nonexisting second option in "
	       << name_ << " inset." << endl;;
	return string();
}


std::string const InsetCommandParams::getContents() const
{
	for (size_t i = 0; i < info_->n; ++i)
		if (!info_->optional[i])
			return to_utf8(params_[i]);
	BOOST_ASSERT(false);
	return string();
}


void InsetCommandParams::setOptions(std::string const & o)
{
	for (size_t i = 0; i < info_->n; ++i)
		if (info_->optional[i]) {
			params_[i] = from_utf8(o);
			return;
		}
	lyxerr << "Programming error: set nonexisting option in "
	       << name_ << " inset." << endl;;
}


void InsetCommandParams::setSecOptions(std::string const & s)
{
	bool first = true;
	for (size_t i = 0; i < info_->n; ++i)
		if (info_->optional[i]) {
			if (first)
				first = false;
			else {
				params_[i] = from_utf8(s);
				return;
			}
		}
	// Happens in InsetCitation
	lyxerr << "Programming error: set nonexisting second option in "
	       << name_ << " inset." << endl;;
}


void InsetCommandParams::setContents(std::string const & c)
{
	for (size_t i = 0; i < info_->n; ++i)
		if (!info_->optional[i]) {
			params_[i] = from_utf8(c);
			return;
		}
	BOOST_ASSERT(false);
}


docstring const & InsetCommandParams::operator[](string const & name) const
{
	int const i = findToken(info_->paramnames, name);
	BOOST_ASSERT(i >= 0);
	return params_[i];
}


docstring & InsetCommandParams::operator[](string const & name)
{
	int const i = findToken(info_->paramnames, name);
	BOOST_ASSERT(i >= 0);
	return params_[i];
}


void InsetCommandParams::clear()
{
	for (size_t i = 0; i < info_->n; ++i)
		params_[i].clear();
}


bool operator==(InsetCommandParams const & o1,
		InsetCommandParams const & o2)
{
	return o1.name_ == o2.name_ &&
	       o1.info_ == o2.info_ &&
	       o1.params_ == o2.params_ &&
	       o1.preview_ == o2.preview_;
}


bool operator!=(InsetCommandParams const & o1,
		InsetCommandParams const & o2)
{
	return !(o1 == o2);
}


} // namespace lyx
