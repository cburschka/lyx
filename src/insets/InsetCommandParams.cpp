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

#include "InsetCommandParams.h"

#include "InsetBibitem.h"
#include "InsetBibtex.h"
#include "InsetCitation.h"
#include "InsetFloatList.h"
#include "InsetHFill.h"
#include "InsetHyperlink.h"
#include "InsetInclude.h"
#include "InsetIndex.h"
#include "InsetLabel.h"
#include "InsetNomencl.h"
#include "InsetRef.h"
#include "InsetTOC.h"

#include "support/debug.h"
#include "support/gettext.h"
#include "Lexer.h"

#include "support/ExceptionMessage.h"
#include "support/lstrings.h"
#include "support/docstream.h"

#include <boost/assert.hpp>

using namespace std;
using namespace lyx::support;

namespace lyx {


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


CommandInfo const * InsetCommandParams::findInfo(
	InsetCode code, string const & cmdName)
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
	case HFILL_CODE:
		return InsetHFill::findInfo(cmdName);
	case HYPERLINK_CODE:
		return InsetHyperlink::findInfo(cmdName);
	case INCLUDE_CODE:
		return InsetInclude::findInfo(cmdName);
	case INDEX_PRINT_CODE:
		return InsetPrintIndex::findInfo(cmdName);
	case LABEL_CODE:
		return InsetLabel::findInfo(cmdName);	
	case NOMENCL_CODE:
		return InsetNomencl::findInfo(cmdName);
	case NOMENCL_PRINT_CODE:
		return InsetPrintNomencl::findInfo(cmdName);
	case REF_CODE:
		return InsetRef::findInfo(cmdName);
	case TOC_CODE:
		return InsetTOC::findInfo(cmdName);
	default:
		BOOST_ASSERT(false);
	}
	return 0;
}


string InsetCommandParams::getDefaultCmd(InsetCode code) {
	switch (code) {
		case BIBITEM_CODE: 
			return InsetBibitem::defaultCommand();
		case BIBTEX_CODE:
			return InsetBibtex::defaultCommand();
		case CITE_CODE:
			return InsetCitation::defaultCommand();
		case FLOAT_LIST_CODE:
			return InsetFloatList::defaultCommand();
		case HFILL_CODE:
			return InsetHFill::defaultCommand();
		case HYPERLINK_CODE:
			return InsetHyperlink::defaultCommand();
		case INCLUDE_CODE:
			return InsetInclude::defaultCommand();
		case INDEX_PRINT_CODE:
			return InsetPrintIndex::defaultCommand();
		case LABEL_CODE:
			return InsetLabel::defaultCommand();
		case NOMENCL_CODE:
			return InsetNomencl::defaultCommand();
		case NOMENCL_PRINT_CODE:
			return InsetPrintNomencl::defaultCommand();
		case REF_CODE:
			return InsetRef::defaultCommand();
		case TOC_CODE:
			return InsetTOC::defaultCommand();
		default:
			BOOST_ASSERT(false);
	}
	return string(); //silence the warning
}


bool InsetCommandParams::isCompatibleCommand(
		InsetCode code, string const & s)
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
		case HFILL_CODE:
			return InsetHFill::isCompatibleCommand(s);
		case HYPERLINK_CODE:
			return InsetHyperlink::isCompatibleCommand(s);
		case INCLUDE_CODE:
			return InsetInclude::isCompatibleCommand(s);
		case INDEX_PRINT_CODE:
			return InsetPrintIndex::isCompatibleCommand(s);
		case LABEL_CODE:
			return InsetLabel::isCompatibleCommand(s);
		case NOMENCL_CODE:
			return InsetNomencl::isCompatibleCommand(s);
		case NOMENCL_PRINT_CODE:
			return InsetPrintNomencl::isCompatibleCommand(s);
		case REF_CODE:
			return InsetRef::isCompatibleCommand(s);
		case TOC_CODE:
			return InsetTOC::isCompatibleCommand(s);
		default:
			BOOST_ASSERT(false);
	}
	return false; //silence the warning
}


void InsetCommandParams::setCmdName(string const & name)
{
	if (!isCompatibleCommand(insetCode_, cmdName_)){
		LYXERR0("InsetCommand: Incompatible command name " << 
				name << ".");
		throw ExceptionMessage(WarningException, _("InsetCommand Error: "),
		                       from_utf8("Incompatible command name."));
	}

	cmdName_ = name;
	CommandInfo const * const info = findInfo(insetCode_, cmdName_);
	if (!info) {
		LYXERR0("Command '" << name << "' is not compatible with a '" <<
			insetType() << "' inset.");
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
	swap(params, params_);
}


void InsetCommandParams::read(Lexer & lex)
{
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
	if (!isCompatibleCommand(insetCode_, cmdName_)){
		lex.printError("InsetCommand: Incompatible command name " + cmdName_ + ".");
		throw ExceptionMessage(WarningException, _("InsetCommand Error: "),
		                       from_utf8("Incompatible command name."));
	}

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
	if (preview_)
		os << "preview true\n";
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


docstring const InsetCommandParams::getFirstNonOptParam() const
{
	for (size_t i = 0; i < info_->n; ++i)
		if (!info_->optional[i])
			return params_[i];
	BOOST_ASSERT(false);
	return docstring();
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
