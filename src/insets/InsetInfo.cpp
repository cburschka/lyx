/**
 * \file InsetInfo.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Bo Peng
 *
 * Full author contact details are available in file CREDITS.
 */
#include <config.h>

#include "InsetInfo.h"

#include <sstream>
#include <stack>

#include "BufferParams.h"
#include "BufferView.h"
#include "debug.h"
#include "FuncRequest.h"
#include "gettext.h"
#include "InsetSpecialChar.h"
#include "KeyMap.h"
#include "LaTeXFeatures.h"
#include "LyXAction.h"
#include "LyXRC.h"
#include "Lexer.h"
#include "MenuBackend.h"
#include "MetricsInfo.h"
#include "ParagraphParameters.h"
#include "TextClassList.h"
#include "support/lstrings.h"
#include "support/ExceptionMessage.h"

using std::pair;
using std::string;
using std::ostream;
using std::ostringstream;
using std::stack;

namespace lyx {

using support::prefixIs;
using support::trim;
using support::split;
using support::rtrim;
using support::ExceptionMessage;
using support::WarningException;

InsetInfo::InsetInfo(BufferParams const & bp, string const & name) 
	: InsetText(bp), type_(UNKNOWN_INFO), name_(), bp_(bp),
	  mouse_hover_(false)
{
	setAutoBreakRows(true);
	setDrawFrame(true);
	setInfo(name);
}


Inset * InsetInfo::editXY(Cursor &, int, int)
{
	return this;
}


void InsetInfo::draw(PainterInfo & pi, int x, int y) const
{
	InsetText::draw(pi, x, y); 
	if (mouse_hover_) {
		odocstringstream os;
		os << _("Information regarding ")
		   <<_(nameTranslator().find(type_))
		   << _(" ") << from_utf8(name_);
		pi.base.bv->message(os.str());
	}
}


namespace {

Translator<InsetInfo::info_type, string> const initTranslator()
{	
	Translator<InsetInfo::info_type, string> translator(InsetInfo::UNKNOWN_INFO, "unknown");

	translator.addPair(InsetInfo::SHORTCUT_INFO, "shortcut");
	translator.addPair(InsetInfo::LYXRC_INFO, "lyxrc");
	translator.addPair(InsetInfo::PACKAGE_INFO, "package");
	translator.addPair(InsetInfo::TEXTCLASS_INFO, "textclass");
	translator.addPair(InsetInfo::MENU_INFO, "menu");

	return translator;
}

} // namespace anon

Translator<InsetInfo::info_type, std::string> const & InsetInfo::nameTranslator() const
{
	static Translator<info_type, string> const translator =
		initTranslator();
	return translator;
}

	

void InsetInfo::read(Buffer const &, Lexer & lex)
{
	string token;
	while (lex.isOK()) {
		lex.next();
		token = lex.getString();
		if (token == "type") {
			lex.next();
			token = lex.getString();
			type_ = nameTranslator().find(token);
		} else if (token == "arg") {
			lex.next();
			name_ = lex.getString();
		} else if (token == "\\end_inset")
			break;
	}	
	if (token != "\\end_inset") {
		lex.printError("Missing \\end_inset at this point");
		throw ExceptionMessage(WarningException,
			_("Missing \\end_inset at this point."),
			from_utf8(token));
	}
	updateInfo();
}


void InsetInfo::write(Buffer const &, std::ostream & os) const
{
	os << "Info\ntype  \""
	   << nameTranslator().find(type_)
	   << "\"\narg   \"" << name_ << '\"';
}


void InsetInfo::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	// FIXME: we should allow selection, copy etc...
	switch (cmd.action) {
	case LFUN_MOUSE_PRESS:
	case LFUN_MOUSE_RELEASE:
	case LFUN_MOUSE_MOTION:
	case LFUN_MOUSE_DOUBLE:
	case LFUN_MOUSE_TRIPLE:
		// do not dispatch to InsetText
		cur.dispatched();
		break;

	default:
		InsetText::doDispatch(cur, cmd);
		break;
	}
}


void InsetInfo::setInfo(string const & name)
{
	if (name.empty())
		return;
	// info_type name
	string type;
	name_ = trim(split(name, type, ' '));
	type_ = nameTranslator().find(type);
	updateInfo();
}


void InsetInfo::updateInfo()
{
	InsetText::clear();

	switch (type_) {
	case UNKNOWN_INFO:
		setText(_("Unknown Info: ") + from_utf8(name_),
			bp_.getFont(), false);
		break;
	case SHORTCUT_INFO: {
		FuncRequest func = lyxaction.lookupFunc(name_);
		if (func.action != LFUN_UNKNOWN_ACTION)
			setText(theTopLevelKeymap().printbindings(func),
				bp_.getFont(), false);
		break;
	}
	case LYXRC_INFO: {
		ostringstream oss;
		lyxrc.write(oss, true, name_);
		string result = oss.str();
		// remove leading \\name
		result = result.substr(name_.size() + 2);
		// remove \n and ""
		result = rtrim(result, "\n");
		result = trim(result, "\"");
		setText(from_utf8(result), bp_.getFont(), false);
		break;
	}
	case PACKAGE_INFO:
		// check in packages.lst
		setText(LaTeXFeatures::isAvailable(name_) ? _("yes") : _("no"),
			bp_.getFont(), false);
		break;
	case TEXTCLASS_INFO: {
		// name_ is the class name
		pair<bool, lyx::textclass_type> pp =
			textclasslist.numberOfClass(name_);
		setText(pp.first ? _("yes") : _("no"),
			bp_.getFont(), false);
		break;
	}
	case MENU_INFO: {
		stack<docstring> names;
		FuncRequest func = lyxaction.lookupFunc(name_);
		if (func.action == LFUN_UNKNOWN_ACTION) {
			setText(_("No menu entry for "), bp_.getFont(), false);
			break;
		}
		// iterate through the menubackend to find it
		Menu menu = menubackend.getMenubar();
		if (!menu.searchFunc(func, names)) {
			setText(_("No menu entry for "), bp_.getFont(), false);
			break;
		}
		// if find, return its path.
		InsetText::clear();
		Paragraph & info = paragraphs().front();
		unsigned int i = 0;
		while (!names.empty()) {
			// do not insert > for the top level menu item
			if (i != 0)
				info.insertInset(0, new InsetSpecialChar(InsetSpecialChar::MENU_SEPARATOR),
					Change(Change::UNCHANGED));
			for (i = 0; i < names.top().length(); ++i)
				info.insertChar(i, names.top()[i], bp_.getFont(), false);
			names.pop();
		}
		break;
	}
	}
	// remove indent
	paragraphs().begin()->params().noindent(true);
}


bool InsetInfo::setMouseHover(bool mouse_hover)
{
	mouse_hover_ = mouse_hover;
	return true;
}

} // namespace lyx
