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

#include "Lexer.h"

#include "support/debug.h"
#include "support/ExceptionMessage.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/docstream.h"

#include <boost/assert.hpp>

using namespace std;
using namespace lyx::support;

namespace lyx {

ParamInfo::ParamData::ParamData(std::string const & s, bool b) :
	name_(s), optional_(b)
{}


bool ParamInfo::ParamData::operator==(ParamInfo::ParamData const & rhs) const
{
	return name() == rhs.name() && isOptional() == rhs.isOptional();
}


bool ParamInfo::hasParam(std::string const & name) const
{
	const_iterator it = begin();
	for (; it != end(); ++it) {
		if (it->name() == name)
			return true;
	}
	return false;
}


void ParamInfo::add(std::string const & name, bool opt)
{ 
	info_.push_back(ParamData(name, opt)); 
}


bool ParamInfo::operator==(ParamInfo const & rhs) const
{
	// the idea here is to check each ParamData for equality
	const_iterator itL  = begin();
	const_iterator itR  = rhs.begin();
	const_iterator endL = end();
	const_iterator endR = rhs.end();
	while (true) {
		// if they both end together, return true
		if (itL == endL && itR == endR)
				return true;
		// but if one ends before the other, return false
		if (itL == endL || itR == endR)
			return false;
		//check this one for equality
		if (*itL != *itR)
			return false;
		// equal, so check the next one
		++itL;
		++itR;
	}
}


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


ParamInfo const & InsetCommandParams::findInfo(
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
	static const ParamInfo pi;
	return pi; // to silence the warning
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
	return string(); // silence the warning
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
	return false; // silence the warning
}


void InsetCommandParams::setCmdName(string const & name)
{
	if (!isCompatibleCommand(insetCode_, cmdName_)){
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
	if (lex.isOK()) {
		lex.next();
		string const insetType = lex.getString();
		InsetCode const code = insetCode(insetType);
		if (code != insetCode_) {
			lex.printError("InsetCommandParams: Attempt to change type of inset.");
			throw ExceptionMessage(WarningException, _("InsetCommandParams Error: "),
		                         _("Attempt to change type of parameters."));
		}
	}

	if (lex.isOK()) {
		lex.next();
		string const test = lex.getString();
		if (test != "LatexCommand") {
			lex.printError("InsetCommandParams: No LatexCommand line found.");
			throw ExceptionMessage(WarningException, _("InsetCommandParams error:"),
			                       _("Can't find LatexCommand line."));
		}
	}
	lex.next();
	cmdName_ = lex.getString();
	if (!isCompatibleCommand(insetCode_, cmdName_)){
		lex.printError("InsetCommandParams: Incompatible command name " + cmdName_ + ".");
		throw ExceptionMessage(WarningException, _("InsetCommandParams Error: "),
		                       _("Incompatible command name."));
	}

	info_ = findInfo(insetCode_, cmdName_);
	
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
			params_[token] = lex.getDocString();
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
	ParamInfo::const_iterator it  = info_.begin();
	ParamInfo::const_iterator end = info_.end();
	for (; it != end; ++it) {
		std::string const & name = it->name();
		docstring const & data = (*this)[name];
		if (!data.empty()) {
			// FIXME UNICODE
			os << name << ' '
			   << Lexer::quoteString(to_utf8(data))
			   << '\n';
		}
	}
}


docstring const InsetCommandParams::getCommand() const
{
	docstring s = '\\' + from_ascii(cmdName_);
	bool noparam = true;
	ParamInfo::const_iterator it  = info_.begin();
	ParamInfo::const_iterator end = info_.end();
	for (; it != end; ++it) {
		std::string const & name = it->name();
		docstring const & data = (*this)[name];
		if (!it->isOptional()) {
			s += '{' + data + '}';
			noparam = false;
			continue;
		}
		if (!data.empty()) {
			s += '[' + data + ']';
			noparam = false;
			continue;
		}
		// This param is therefore optional but empty.
		// But we need to write it anyway if nonempty
		// optional parameters follow before the next
		// required parameter.
		ParamInfo::const_iterator it2 = it;
		for (++it2; it2 != end; ++it2) {
			if (!it2->isOptional())
				break;
			std::string const & name2 = it2->name();
			docstring const & data2 = (*this)[name2];
			if (!data2.empty()) {
				s += "[]";
				noparam = false;
				break;
			}
		}
	}
	if (noparam)
		// Make sure that following stuff does not change the
		// command name.
		s += "{}";
	return s;
}


namespace {
	//predicate for what follows
	bool paramIsNonOptional(ParamInfo::ParamData pi)
	{
		return !pi.isOptional();
	}
}

docstring const InsetCommandParams::getFirstNonOptParam() const
{
	ParamInfo::const_iterator it = 
		find_if(info_.begin(), info_.end(), paramIsNonOptional);
	if (it == info_.end())
		BOOST_ASSERT(false);
	return (*this)[it->name()];
}


docstring const & InsetCommandParams::operator[](string const & name) const
{
	static const docstring dummy; //so we don't return a ref to temporary
	if (!info_.hasParam(name))
		BOOST_ASSERT(false);
	ParamMap::const_iterator data = params_.find(name);
	if (data == params_.end() || data->second.empty())
		return dummy;
	return data->second;
}


docstring & InsetCommandParams::operator[](string const & name)
{
	if (!info_.hasParam(name))
		BOOST_ASSERT(false);
	return params_[name];
}


void InsetCommandParams::clear()
{
	params_.clear();
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
