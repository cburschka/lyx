/**
 * \file InsetCommandParams.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Georg Baum
 * \author Richard Heck
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>
#include <algorithm>
#include <functional>


#include "InsetCommandParams.h"

#include "InsetBibitem.h"
#include "InsetBibtex.h"
#include "InsetCitation.h"
#include "InsetFloatList.h"
#include "InsetHyperlink.h"
#include "InsetInclude.h"
#include "InsetIndex.h"
#include "InsetLabel.h"
#include "InsetLine.h"
#include "InsetNomencl.h"
#include "InsetRef.h"
#include "InsetTOC.h"

#include "Buffer.h"
#include "Encoding.h"
#include "Lexer.h"
#include "OutputParams.h"

#include "frontends/alert.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/ExceptionMessage.h"
#include "support/gettext.h"
#include "support/lstrings.h"

#include "support/lassert.h"

using namespace std;
using namespace lyx::support;


namespace lyx {

/// Get information for \p code and command \p cmdName.
/// Don't call this without first making sure the command name is
/// acceptable to the inset.
static ParamInfo const & findInfo(InsetCode code, string const & cmdName)
{
	switch (code) {
	case BIBITEM_CODE:
		return InsetBibitem::findInfo(cmdName);
	case BIBTEX_CODE:
		return InsetBibtex::findInfo(cmdName);
	case CITE_CODE:
		return InsetCitation::findInfo(cmdName);
	case FLOAT_LIST_CODE:
		return InsetFloatList::findInfo(cmdName);
	case HYPERLINK_CODE:
		return InsetHyperlink::findInfo(cmdName);
	case INCLUDE_CODE:
		return InsetInclude::findInfo(cmdName);
	case INDEX_PRINT_CODE:
		return InsetPrintIndex::findInfo(cmdName);
	case LABEL_CODE:
		return InsetLabel::findInfo(cmdName);
	case LINE_CODE:
		return InsetLine::findInfo(cmdName);
	case NOMENCL_CODE:
		return InsetNomencl::findInfo(cmdName);
	case NOMENCL_PRINT_CODE:
		return InsetPrintNomencl::findInfo(cmdName);
	case REF_CODE:
		return InsetRef::findInfo(cmdName);
	case TOC_CODE:
		return InsetTOC::findInfo(cmdName);
	default:
		LATTEST(false);
		// fall through in release mode
	}
	static const ParamInfo pi;
	return pi;
}


/////////////////////////////////////////////////////////////////////
//
// ParamInfo::ParamData
//
/////////////////////////////////////////////////////////////////////

ParamInfo::ParamData::ParamData(std::string const & s, ParamType t,
                                ParamHandling h, bool ignore,
                                docstring default_value)
	: name_(s), type_(t), handling_(h), ignore_(ignore),
	  default_value_(default_value)
{}


bool ParamInfo::ParamData::isOptional() const
{
	return type_ == ParamInfo::LATEX_OPTIONAL;
}


bool ParamInfo::ParamData::operator==(ParamInfo::ParamData const & rhs) const
{
	return name() == rhs.name() && type() == rhs.type()
		&& handling() == rhs.handling();
}


bool ParamInfo::hasParam(std::string const & name) const
{
	const_iterator it = begin();
	const_iterator last = end();
	for (; it != last; ++it) {
		if (it->name() == name)
			return true;
	}
	return false;
}


void ParamInfo::add(std::string const & name, ParamType type,
                    ParamHandling handling, bool ignore,
                    docstring default_value)
{
	info_.push_back(ParamData(name, type, handling, ignore, default_value));
}


bool ParamInfo::operator==(ParamInfo const & rhs) const
{
	if (size() != rhs.size())
		return false;
	return equal(begin(), end(), rhs.begin());
}


ParamInfo::ParamData const &
	ParamInfo::operator[](std::string const & name) const
{
	const_iterator it = begin();
	const_iterator last = end();
	for (; it != last; ++it) {
		if (it->name() == name)
			return *it;
	}
	LATTEST(false);
	// we will try to continue in release mode
	static const ParamData pd("asdfghjkl", LYX_INTERNAL);
	return pd;
}


/////////////////////////////////////////////////////////////////////
//
// InsetCommandParams
//
/////////////////////////////////////////////////////////////////////


InsetCommandParams::InsetCommandParams(InsetCode code)
	: insetCode_(code), preview_(false)
{
	cmdName_ = getDefaultCmd(code);
	info_ = findInfo(code, cmdName_);
}


InsetCommandParams::InsetCommandParams(InsetCode code,
	string const & cmdName)
	: insetCode_(code), cmdName_(cmdName), preview_(false)
{
	info_ = findInfo(code, cmdName);
}


std::string InsetCommandParams::insetType() const
{
	return insetName(insetCode_);
}


string InsetCommandParams::getDefaultCmd(InsetCode code)
{
	switch (code) {
		case BIBITEM_CODE:
			return InsetBibitem::defaultCommand();
		case BIBTEX_CODE:
			return InsetBibtex::defaultCommand();
		case CITE_CODE:
			return InsetCitation::defaultCommand();
		case FLOAT_LIST_CODE:
			return InsetFloatList::defaultCommand();
		case HYPERLINK_CODE:
			return InsetHyperlink::defaultCommand();
		case INCLUDE_CODE:
			return InsetInclude::defaultCommand();
		case INDEX_PRINT_CODE:
			return InsetPrintIndex::defaultCommand();
		case LABEL_CODE:
			return InsetLabel::defaultCommand();
		case LINE_CODE:
			return InsetLine::defaultCommand();
		case NOMENCL_CODE:
			return InsetNomencl::defaultCommand();
		case NOMENCL_PRINT_CODE:
			return InsetPrintNomencl::defaultCommand();
		case REF_CODE:
			return InsetRef::defaultCommand();
		case TOC_CODE:
			return InsetTOC::defaultCommand();
		default:
			LATTEST(false);
			// fall through in release mode
	}
	return string();
}


bool InsetCommandParams::isCompatibleCommand(InsetCode code, string const & s)
{
	switch (code) {
		case BIBITEM_CODE:
			return InsetBibitem::isCompatibleCommand(s);
		case BIBTEX_CODE:
			return InsetBibtex::isCompatibleCommand(s);
		case CITE_CODE:
			return InsetCitation::isCompatibleCommand(s);
		case FLOAT_LIST_CODE:
			return InsetFloatList::isCompatibleCommand(s);
		case HYPERLINK_CODE:
			return InsetHyperlink::isCompatibleCommand(s);
		case INCLUDE_CODE:
			return InsetInclude::isCompatibleCommand(s);
		case INDEX_PRINT_CODE:
			return InsetPrintIndex::isCompatibleCommand(s);
		case LABEL_CODE:
			return InsetLabel::isCompatibleCommand(s);
		case LINE_CODE:
			return InsetLine::isCompatibleCommand(s);
		case NOMENCL_CODE:
			return InsetNomencl::isCompatibleCommand(s);
		case NOMENCL_PRINT_CODE:
			return InsetPrintNomencl::isCompatibleCommand(s);
		case REF_CODE:
			return InsetRef::isCompatibleCommand(s);
		case TOC_CODE:
			return InsetTOC::isCompatibleCommand(s);
	default:
		LATTEST(false);
		// fall through in release mode
	}
	return false;
}


void InsetCommandParams::setCmdName(string const & name)
{
	if (!isCompatibleCommand(insetCode_, name)) {
		LYXERR0("InsetCommand: Incompatible command name " <<
				name << ".");
		throw ExceptionMessage(WarningException, _("InsetCommand Error: "),
		                       _("Incompatible command name."));
	}

	cmdName_ = name;
	info_ = findInfo(insetCode_, cmdName_);
}


void InsetCommandParams::read(Lexer & lex)
{
	Read(lex, 0);
}


void InsetCommandParams::Read(Lexer & lex, Buffer const * buffer)
{
	lex.setContext("InsetCommandParams::read");
	lex >> insetName(insetCode_).c_str();
	lex >> "LatexCommand";
	lex >> cmdName_;
	if (!isCompatibleCommand(insetCode_, cmdName_)) {
		lex.printError("Incompatible command name " + cmdName_ + ".");
		throw ExceptionMessage(WarningException, _("InsetCommandParams Error: "),
		                       _("Incompatible command name."));
	}

	info_ = findInfo(insetCode_, cmdName_);

	for (ParamInfo::ParamData const & param : info_)
		if (param.ignore()) {
			params_[param.name()] = param.defaultValue();
		}

	string token;
	while (lex.isOK()) {
		lex.next();
		token = lex.getString();
		if (token == "\\end_inset")
			break;
		if (token == "preview") {
			lex.next();
			preview_ = lex.getBool();
			continue;
		}
		if (info_.hasParam(token)) {
			lex.next(true);
			docstring data = lex.getDocString();
			if (buffer && token == "filename") {
				data = from_utf8(buffer->includedFilePath(to_utf8(data)));
			} else if (buffer && token == "bibfiles") {
				int i = 0;
				docstring newdata;
				docstring bib = support::token(data, ',', i);
				while (!bib.empty()) {
					bib = from_utf8(buffer->includedFilePath(to_utf8(bib), "bib"));
					if (!newdata.empty())
						newdata.append(1, ',');
					newdata.append(bib);
					bib = support::token(data, ',', ++i);
				}
				data = newdata;
			} else if (buffer && token == "options") {
				data = from_utf8(buffer->includedFilePath(to_utf8(data), "bst"));
			}
			params_[token] = data;
		} else {
			lex.printError("Unknown parameter name `$$Token' for command " + cmdName_);
			throw ExceptionMessage(WarningException,
				_("InsetCommandParams: ") + from_ascii(cmdName_),
				_("Unknown parameter name: ") + from_utf8(token));
		}
	}
	if (token != "\\end_inset") {
		lex.printError("Missing \\end_inset at this point. "
			       "Read: `$$Token'");
		throw ExceptionMessage(WarningException,
			_("InsetCommandParams Error: "),
			_("Missing \\end_inset at this point: ") + from_utf8(token));
	}
}


void InsetCommandParams::write(ostream & os) const
{
	Write(os, 0);
}


void InsetCommandParams::Write(ostream & os, Buffer const * buffer) const
{
	os << "CommandInset " << insetType() << '\n';
	os << "LatexCommand " << cmdName_ << '\n';
	if (preview_)
		os << "preview true\n";
	ParamInfo::const_iterator it  = info_.begin();
	ParamInfo::const_iterator end = info_.end();
	for (; it != end; ++it) {
		if (it->ignore())
			continue;
		string const & name = it->name();
		string data = to_utf8((*this)[name]);
		if (!data.empty()) {
			// Adjust path of files if document was moved
			if (buffer && name == "filename") {
				data = buffer->includedFilePath(data);
			} else if (buffer && name == "bibfiles") {
				int i = 0;
				string newdata;
				string bib = token(data, ',', i);
				while (!bib.empty()) {
					bib = buffer->includedFilePath(bib, "bib");
					if (!newdata.empty())
						newdata.append(1, ',');
					newdata.append(bib);
					bib = token(data, ',', ++i);
				}
				data = newdata;
			} else if (buffer && name == "options") {
				data = buffer->includedFilePath(data, "bst");
			}
			os << name << ' '
			   << Lexer::quoteString(data)
			   << '\n';
		}
	}
}


bool InsetCommandParams::writeEmptyOptional(ParamInfo::const_iterator ci) const
{
	LASSERT(ci->isOptional(), return false);

	++ci; // we want to start with the next one
	ParamInfo::const_iterator end = info_.end();
	for (; ci != end; ++ci) {
		switch (ci->type()) {
		case ParamInfo::LYX_INTERNAL:
			break;

		case ParamInfo::LATEX_REQUIRED:
			return false;

		case ParamInfo::LATEX_OPTIONAL: {
			std::string const & name = ci->name();
			docstring const & data = (*this)[name];
			if (!data.empty())
				return true;
			break;
		}

		} //end switch
	}
	return false;
}


docstring InsetCommandParams::prepareCommand(OutputParams const & runparams,
					     docstring const & command,
					     ParamInfo::ParamHandling handling) const
{
	docstring result;
	bool ltrimmed = false;
	// Trimming can be done on top of any of the other handlings
	// We check this here since handling might be changed below.
	if (handling & ParamInfo::HANDLING_LTRIM) {
		// this is used if no other handling is done
		result = command;
		ltrimmed = true;
	}
	if (handling & ParamInfo::HANDLING_LATEXIFY)
		if ((*this)["literal"] == "true")
			handling = ParamInfo::HANDLING_NONE;

	// LATEXIFY, ESCAPE and NONE are mutually exclusive
	if (handling & ParamInfo::HANDLING_LATEXIFY) {
		// First handle backslash
		result = subst(command, from_ascii("\\"), from_ascii("\\textbackslash{}"));
		// Then get LaTeX macros
		pair<docstring, docstring> command_latexed =
			runparams.encoding->latexString(result, runparams.dryrun);
		result = command_latexed.first;
		if (!command_latexed.second.empty()) {
			// Issue a warning about omitted characters
			// FIXME: should be passed to the error dialog
			frontend::Alert::warning(_("Uncodable characters"),
				bformat(_("The following characters that are used in the inset %1$s are not\n"
					  "representable in the current encoding and therefore have been omitted:\n%2$s."),
					from_utf8(insetType()), command_latexed.second));
		}
		// Now escape special commands
		static docstring const backslash = from_ascii("\\");
		static char_type const chars_escape[6] = {
			'&', '_', '$', '%', '#', '^'};

		if (!result.empty()) {
			int previous;
			// The characters in chars_name[] need to be changed to a command when
			// they are LaTeXified.
			for (int k = 0; k < 6; k++)
				for (size_t i = 0, pos;
					(pos = result.find(chars_escape[k], i)) != string::npos;
					i = pos + 2) {
						//(Only) \\^ needs to be terminated
						docstring const term = (k == 5) ? from_ascii("{}") : docstring();
						if (pos == 0)
							previous = 0;
						else
							previous = pos - 1;
						// only if not already escaped
						if (result[previous] != '\\')
							result.replace(pos, 1, backslash + chars_escape[k] + term);
				}
		}
	}
	else if (handling & ParamInfo::HANDLING_ESCAPE)
		result = escape(command);
	else if (handling & ParamInfo::HANDLING_NONE)
		result = command;

	return ltrimmed ? ltrim(result) : result;
}


docstring InsetCommandParams::getCommand(OutputParams const & runparams) const
{
	docstring s = '\\' + from_ascii(cmdName_);
	bool noparam = true;
	ParamInfo::const_iterator it  = info_.begin();
	ParamInfo::const_iterator end = info_.end();
	for (; it != end; ++it) {
		std::string const & name = it->name();
		switch (it->type()) {
		case ParamInfo::LYX_INTERNAL:
			break;

		case ParamInfo::LATEX_REQUIRED: {
			docstring const data =
				prepareCommand(runparams, (*this)[name], it->handling());
			s += '{' + data + '}';
			noparam = false;
			break;
		}
		case ParamInfo::LATEX_OPTIONAL: {
			docstring data =
				prepareCommand(runparams, (*this)[name], it->handling());
			if (!data.empty()) {
				s += '[' + protectArgument(data) + ']';
				noparam = false;
			} else if (writeEmptyOptional(it)) {
					s += "[]";
					noparam = false;
			}
			break;
		}
		} //end switch
	}
	if (noparam)
		// Make sure that following stuff does not change the
		// command name.
		s += "{}";
	return s;
}


docstring InsetCommandParams::getFirstNonOptParam() const
{
	ParamInfo::const_iterator it =
		find_if(info_.begin(), info_.end(),
			not1(mem_fun_ref(&ParamInfo::ParamData::isOptional)));
	LASSERT(it != info_.end(), return docstring());
	return (*this)[it->name()];
}


docstring const & InsetCommandParams::operator[](string const & name) const
{
	static const docstring dummy;
	LASSERT(info_.hasParam(name), return dummy);
	ParamMap::const_iterator data = params_.find(name);
	if (data == params_.end() || data->second.empty())
		return dummy;
	ParamInfo::ParamData const & param = info_[name];
	if (param.ignore())
		return param.defaultValue();
	return data->second;
}


docstring & InsetCommandParams::operator[](string const & name)
{
	LATTEST(info_.hasParam(name));
	// this will add the name in release mode
	ParamInfo::ParamData const & param = info_[name];
	if (param.ignore())
		params_[name] = param.defaultValue();
	return params_[name];
}


void InsetCommandParams::clear()
{
	params_.clear();
}


bool operator==(InsetCommandParams const & o1, InsetCommandParams const & o2)
{
	return o1.insetCode_ == o2.insetCode_
		&& o1.cmdName_ == o2.cmdName_
		&& o1.info_ == o2.info_
		&& o1.params_ == o2.params_
		&& o1.preview_ == o2.preview_;
}


bool operator!=(InsetCommandParams const & o1, InsetCommandParams const & o2)
{
	return !(o1 == o2);
}


} // namespace lyx
