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


//FIXME There is no reason now for this to take a string argument.
//It'd be much more robust if it took an InsetCode, since then
//the compiler would do some checking for us.
InsetCommandParams::InsetCommandParams(string const & insetType)
	: insetType_(insetType), preview_(false)
{
	cmdName_ = getDefaultCmd(insetType);
	info_ = findInfo(insetType, cmdName_);
	BOOST_ASSERT(info_);
	params_.resize(info_->n);
}


InsetCommandParams::InsetCommandParams(string const & insetType,
	string const & cmdName)
	: insetType_(insetType), cmdName_(cmdName), preview_(false)
{
	info_ = findInfo(insetType, cmdName);
	BOOST_ASSERT(info_);
	params_.resize(info_->n);
}


//FIXME This should go into the Insets themselves...so they will tell
//us what parameters they want.
//Should this just vanish in favor of the two arg version, or is there
//a reason to use it in some cases? What should happen in the single
//arg case, then? Maybe use the default? or leave that up to the inset?
InsetCommandParams::CommandInfo const *
InsetCommandParams::findInfo(std::string const & insetType)
{
	// No parameter may be named "preview", because that is a required
	// flag for all commands.

	// InsetBibitem
	if (insetType == "bibitem") {
		static const char * const paramnames[] = {"label", "key", ""};
		static const bool isoptional[] = {true, false};
		static const CommandInfo info = {2, paramnames, isoptional};
		return &info;
	}

	// InsetBibtex
	if (insetType == "bibtex") {
		static const char * const paramnames[] =
				{"options", "btprint", "bibfiles", ""};
		static const bool isoptional[] = {true, true, false};
		static const CommandInfo info = {3, paramnames, isoptional};
		return &info;
	}

	// InsetCitation
	if (insetType == "citation") {
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
	if (insetType == "floatlist") {
		static const char * const paramnames[] = {"type", ""};
		static const bool isoptional[] = {false};
		static const CommandInfo info = {1, paramnames, isoptional};
		return &info;
	}

	// InsetHfill
	if (insetType == "hfill") {
		static const char * const paramnames[] = {""};
		static const CommandInfo info = {0, paramnames, 0};
		return &info;
	}

	// InsetInclude
	//FIXME This is really correct only for lstinputlistings, but it shouldn't
	//cause a problem before we get things sorted out. Eventually, this calls
	//InsetInclude::getParams(cmdName_), or something of the sort.
	if (insetType == "include") {
		static const char * const paramnames[] = {"filename", "lstparams", ""};
		static const bool isoptional[] = {false, true};
		static const CommandInfo info = {2, paramnames, isoptional};
		return &info;
	}

	// InsetIndex, InsetPrintIndex, InsetLabel
	if (insetType == "index" || insetType == "index_print" || insetType == "label") {
		static const char * const paramnames[] = {"name", ""};
		static const bool isoptional[] = {false};
		static const CommandInfo info = {1, paramnames, isoptional};
		return &info;
	}

	// InsetNomencl
	if (insetType == "nomenclature") {
		static const char * const paramnames[] = {"prefix", "symbol", "description", ""};
		static const bool isoptional[] = {true, false, false};
		static const CommandInfo info = {3, paramnames, isoptional};
		return &info;
	}

	// InsetPrintNomencl
	if (insetType == "nomencl_print") {
		static const char * const paramnames[] = {"labelwidth", ""};
		static const bool isoptional[] = {true};
		static const CommandInfo info = {1, paramnames, isoptional};
		return &info;
	}

	// InsetRef
	if (insetType == "ref") {
		static const char * const paramnames[] =
				{"name", "reference", ""};
		static const bool isoptional[] = {true, false};
		static const CommandInfo info = {2, paramnames, isoptional};
		return &info;
	}

	// InsetTOC
	if (insetType == "toc") {
		static const char * const paramnames[] = {"type", ""};
		static const bool isoptional[] = {false};
		static const CommandInfo info = {1, paramnames, isoptional};
		return &info;
	}

	// InsetUrl
	if (insetType == "url") {
		static const char * const paramnames[] =
				{"name", "target", ""};
		static const bool isoptional[] = {true, false};
		static const CommandInfo info = {2, paramnames, isoptional};
		return &info;
	}

	return 0;
}


//FIXME Will eventually call a static method, etc.
InsetCommandParams::CommandInfo const *
		InsetCommandParams::findInfo(std::string const & insetType,
		                             std::string const & cmdName)
{
	return findInfo(insetType);
}


//FIXME Should call InsetBibitem::getDefaultCmd(), eg
std::string InsetCommandParams::getDefaultCmd(std::string insetType) {
	if (insetType == "bibitem")
		return "bibitem";
	if (insetType == "bibtex") 
		return "";
	if (insetType == "citation")
		return "cite";
	if (insetType == "floatlist")
		return "";
	if (insetType == "hfill")
		return "hfill";
	if (insetType == "include")
		return "include";
	if (insetType == "index")
		 return "index";
	if (insetType == "index_print")
		return "print_index";
	if (insetType == "label")
		return "label";
	if (insetType == "nomenclature")
		return "nomenclature";
	if (insetType == "nomencl_print")
		return "printnomenclature";
	if (insetType == "ref")
		return "ref";
	if (insetType == "toc")
		return "tableofcontents";
	if (insetType == "url")
		return "url";
	return "";	
}


void InsetCommandParams::setCmdName(string const & name)
{
	//FIXME Check command compatibility
	cmdName_ = name;
	BOOST_ASSERT(!insetType_.empty());
	CommandInfo const * const info = findInfo(insetType_, cmdName_);
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
	//FIXME
	if (lex.isOK()) {
		lex.next();
		string insetType = lex.getString();
		if (!insetType_.empty() && insetType != insetType_) {
			lex.printError("InsetCommand: Attempt to change type of parameters.");
			throw ExceptionMessage(WarningException, _("InsetCommand Error: "),
				from_utf8("Attempt to change type of parameters."));
		}
		// OK, we survived...
		insetType_ = insetType;
	}

	if (lex.isOK()) {
		lex.next();
		string test = lex.getString();
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
	info_ = findInfo(insetType_, cmdName_);
	if (!info_) {
		lex.printError("InsetCommand: Unknown inset name `$$Token'");
		throw ExceptionMessage(WarningException,
			_("Unknown inset name: "), from_utf8(insetType_));
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
	os << "CommandInset " << insetType_ << '\n';
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
	       << insetType_ << " inset." << endl;
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
	       << insetType_ << " inset." << endl;
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
	       << insetType_ << " inset." << endl;
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
	       << insetType_ << " inset." << endl;
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
	return o1.insetType_ == o2.insetType_ &&
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
