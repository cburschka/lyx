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

#include "InsetCommandParams.h"

#include "InsetBibitem.h"
#include "InsetBibtex.h"
#include "InsetCitation.h"
#include "InsetFloatList.h"
#include "InsetHyperlink.h"
#include "InsetInclude.h"
#include "InsetIndex.h"
#include "InsetLabel.h"
#include "InsetNomencl.h"
#include "InsetRef.h"
#include "InsetTOC.h"

#include "Lexer.h"

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
/// Returns 0 if the combination is not known.  [FIXME: 0?]
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
	case NOMENCL_CODE:
		return InsetNomencl::findInfo(cmdName);
	case NOMENCL_PRINT_CODE:
		return InsetPrintNomencl::findInfo(cmdName);
	case REF_CODE:
		return InsetRef::findInfo(cmdName);
	case TOC_CODE:
		return InsetTOC::findInfo(cmdName);
	default:
		LASSERT(false, /**/);
	}
	static const ParamInfo pi;
	return pi; // to silence the warning
}


/////////////////////////////////////////////////////////////////////
//
// ParamInfo::ParamData
//
/////////////////////////////////////////////////////////////////////

ParamInfo::ParamData::ParamData(std::string const & s, ParamType t)
	: name_(s), type_(t)
{}


bool ParamInfo::ParamData::isOptional() const
{
	return type_ == ParamInfo::LATEX_OPTIONAL;
}


bool ParamInfo::ParamData::operator==(ParamInfo::ParamData const & rhs) const
{
	return name() == rhs.name() && type() == rhs.type();
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


void ParamInfo::add(std::string const & name, ParamType type)
{ 
	info_.push_back(ParamData(name, type)); 
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
	LASSERT(hasParam(name), /**/);
	const_iterator it = begin();
	const_iterator last = end();
	for (; it != last; ++it) {
		if (it->name() == name)
			return *it;
	}
	return *it; // silence warning
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
		case NOMENCL_CODE:
			return InsetNomencl::defaultCommand();
		case NOMENCL_PRINT_CODE:
			return InsetPrintNomencl::defaultCommand();
		case REF_CODE:
			return InsetRef::defaultCommand();
		case TOC_CODE:
			return InsetTOC::defaultCommand();
		default:
			LASSERT(false, /**/);
	}
	return string(); // silence the warning
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
		case NOMENCL_CODE:
			return InsetNomencl::isCompatibleCommand(s);
		case NOMENCL_PRINT_CODE:
			return InsetPrintNomencl::isCompatibleCommand(s);
		case REF_CODE:
			return InsetRef::isCompatibleCommand(s);
		case TOC_CODE:
			return InsetTOC::isCompatibleCommand(s);
		default:
			LASSERT(false, /**/);
	}
	return false; // silence the warning
}


void InsetCommandParams::setCmdName(string const & name)
{
	if (!isCompatibleCommand(insetCode_, cmdName_)) {
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


bool InsetCommandParams::writeEmptyOptional(ParamInfo::const_iterator ci) const
{
	if (!ci->isOptional()) {
		LASSERT(false, /**/);
	}
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


docstring InsetCommandParams::getCommand() const
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
			docstring const & data = (*this)[name];
			s += '{' + data + '}';
			noparam = false;
			break;
		}
		case ParamInfo::LATEX_OPTIONAL: {
			docstring const & data = (*this)[name];
			if (!data.empty()) {
				s += '[' + data + ']';
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
	if (it == info_.end()) {
		LASSERT(false, return docstring());
	}
	return (*this)[it->name()];
}


docstring const & InsetCommandParams::operator[](string const & name) const
{
	static const docstring dummy; //so we don't return a ref to temporary
	LASSERT(info_.hasParam(name), return dummy);
	ParamMap::const_iterator data = params_.find(name);
	if (data == params_.end() || data->second.empty())
		return dummy;
	return data->second;
}


docstring & InsetCommandParams::operator[](string const & name)
{
	LASSERT(info_.hasParam(name), /**/);
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
