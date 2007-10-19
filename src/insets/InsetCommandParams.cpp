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


InsetCommandParams::InsetCommandParams(InsetCode code)
	: insetCode_(code), preview_(false)
{
	cmdName_ = getDefaultCmd(code);
	info_ = findInfo(code, cmdName_);
	BOOST_ASSERT(info_);
	params_.resize(info_->n);
}


InsetCommandParams::InsetCommandParams(InsetCode code,
	string const & cmdName)
	: insetCode_(code), cmdName_(cmdName), preview_(false)
{
	info_ = findInfo(code, cmdName);
	BOOST_ASSERT(info_);
	params_.resize(info_->n);
}


//FIXME This should go into the Insets themselves...so they will tell
//us what parameters they want.
//Should this just vanish in favor of the two arg version, or is there
//a reason to use it in some cases? What should happen in the single
//arg case, then? Maybe use the default? or leave that up to the inset?
InsetCommandParams::CommandInfo const *
	InsetCommandParams::findInfo(InsetCode code)
{
	// No parameter may be named "preview", because that is a required
	// flag for all commands.

	switch (code) {
	case BIBITEM_CODE: {
		static const char * const paramnames[] = {"label", "key", ""};
		static const bool isoptional[] = {true, false};
		static const CommandInfo info = {2, paramnames, isoptional};
		return &info;
	}
	case BIBTEX_CODE: {
		static const char * const paramnames[] =
				{"options", "btprint", "bibfiles", ""};
		static const bool isoptional[] = {true, true, false};
		static const CommandInfo info = {3, paramnames, isoptional};
		return &info;
	}
	case CITE_CODE: {
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
	case FLOAT_LIST_CODE: {
		static const char * const paramnames[] = {"type", ""};
		static const bool isoptional[] = {false};
		static const CommandInfo info = {1, paramnames, isoptional};
		return &info;
	}
	case HFILL_CODE: {
		static const char * const paramnames[] = {""};
		static const CommandInfo info = {0, paramnames, 0};
		return &info;
	}
	case HYPERLINK_CODE: {
		static const char * const paramnames[] =
				{"name", "target", ""};
		static const bool isoptional[] = {true, false};
		static const CommandInfo info = {2, paramnames, isoptional};
		return &info;
	}
	case INCLUDE_CODE: {
		//This is only correct for the case of listings, but it'll do for now.
		//In the other cases, this second parameter should just be empty.
		static const char * const paramnames[] = {"filename", "lstparams", ""};
		static const bool isoptional[] = {false, true};
		static const CommandInfo info = {2, paramnames, isoptional};
		return &info;
	}
	case INDEX_CODE: 
	case INDEX_PRINT_CODE:
	case LABEL_CODE: {
		static const char * const paramnames[] = {"name", ""};
		static const bool isoptional[] = {false};
		static const CommandInfo info = {1, paramnames, isoptional};
		return &info;
	}
	case NOMENCL_CODE: {
		static const char * const paramnames[] = {"prefix", "symbol", "description", ""};
		static const bool isoptional[] = {true, false, false};
		static const CommandInfo info = {3, paramnames, isoptional};
		return &info;
	}
	case NOMENCL_PRINT_CODE: {
		static const char * const paramnames[] = {"labelwidth", ""};
		static const bool isoptional[] = {true};
		static const CommandInfo info = {1, paramnames, isoptional};
		return &info;
	}
	case REF_CODE: {
		static const char * const paramnames[] =
				{"name", "reference", ""};
		static const bool isoptional[] = {true, false};
		static const CommandInfo info = {2, paramnames, isoptional};
		return &info;
	}
	case TOC_CODE: {
		static const char * const paramnames[] = {"type", ""};
		static const bool isoptional[] = {false};
		static const CommandInfo info = {1, paramnames, isoptional};
		return &info;
	}
	default:
		BOOST_ASSERT(false);
	}
	return 0;
}


//FIXME Will eventually call a static method, etc.
InsetCommandParams::CommandInfo const *
		InsetCommandParams::findInfo(InsetCode code,
		                             std::string const &/* cmdName*/)
{
	return findInfo(code);
}


//FIXME Should call InsetBibitem::getDefaultCmd(), eg
std::string InsetCommandParams::getDefaultCmd(InsetCode code) {
	switch (code) {
		case BIBITEM_CODE: 
			return "bibitem";
		case BIBTEX_CODE:
			return "bibtex"; //this is an unused dummy
		case CITE_CODE:
			return "cite";
		case FLOAT_LIST_CODE:
			return "listoftables";
		case HFILL_CODE:
			return "hfill";
		case HYPERLINK_CODE:
			return "href";
		case INCLUDE_CODE:
			return "include";
		case INDEX_CODE: 
			return "index";
		case INDEX_PRINT_CODE:
			return "printindex";
		case LABEL_CODE:
			return "label";
		case NOMENCL_CODE:
			return "nomenclature";
		case NOMENCL_PRINT_CODE:
			return "printnomenclature";
		case REF_CODE:
			return "ref";
		case TOC_CODE:
			return "tableofcontents";
		default:
			BOOST_ASSERT(false);
	}
	return "";
}


void InsetCommandParams::setCmdName(string const & name)
{
	//FIXME Check command compatibility
	cmdName_ = name;
	CommandInfo const * const info = findInfo(insetCode_, cmdName_);
	if (!info) {
		lyxerr << "Command '" << name << "' is not compatible with a '" <<
			insetType() << "' inset." << std::endl;
		return;
	}
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
	//FIXME
	if (lex.isOK()) {
		lex.next();
		string const insetType = lex.getString();
		InsetCode const code = insetCode(insetType);
		if (code != insetCode_) {
			lex.printError("InsetCommand: Attempt to change type of parameters.");
			throw ExceptionMessage(WarningException, _("InsetCommand Error: "),
				from_utf8("Attempt to change type of parameters."));
		}
	}

	if (lex.isOK()) {
		lex.next();
		string const test = lex.getString();
		if (test != "LatexCommand") {
			lex.printError("InsetCommand: no LatexCommand line found.");
			throw ExceptionMessage(WarningException, _("InsetCommand error:"),
				from_utf8("Can't find LatexCommand line."));
		}
	}
	lex.next();
	cmdName_ = lex.getString();
	//FIXME
	//check that this command is ok with the inset...
	//so that'll be some kind of static call, depending 
	//upon what insetType_ is.
	//it's possible that should go into InsetCommand.cpp,
	//or maybe it's a standalone function.
	info_ = findInfo(insetCode_, cmdName_);
	if (!info_) {
		lex.printError("InsetCommand: Unknown inset name `$$Token'");
		throw ExceptionMessage(WarningException,
			_("Unknown inset name: "), from_utf8(insetType()));
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
			lex.printError("Unknown parameter name `$$Token' for command " + cmdName_);
			throw ExceptionMessage(WarningException,
				_("Inset Command: ") + from_ascii(cmdName_),
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
	os << "CommandInset " << insetType() << '\n';
	os << "LatexCommand " << cmdName_ << '\n';
	for (size_t i = 0; i < info_->n; ++i)
		if (!params_[i].empty())
			// FIXME UNICODE
			os << info_->paramnames[i] << ' '
			   << Lexer::quoteString(to_utf8(params_[i]))
			   << '\n';
}


docstring const InsetCommandParams::getCommand() const
{
	docstring s = '\\' + from_ascii(cmdName_);
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
	       << insetType() << " inset." << endl;
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
	       << insetType() << " inset." << endl;
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
	       << insetType() << " inset." << endl;
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
	       << insetType() << " inset." << endl;
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
	return o1.insetCode_ == o2.insetCode_ &&
	       o1.cmdName_ == o2.cmdName_ &&
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
