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

#include "LayoutFile.h"
#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "InsetGraphics.h"
#include "InsetSpecialChar.h"
#include "KeyMap.h"
#include "LaTeXFeatures.h"
#include "LyXAction.h"
#include "LyXRC.h"
#include "Lexer.h"
#include "MetricsInfo.h"
#include "ParagraphParameters.h"

#include "frontends/Application.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/docstring_list.h"
#include "support/FileName.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/ExceptionMessage.h"

#include <sstream>

using namespace std;
using namespace lyx::support;

namespace lyx {


InsetInfo::InsetInfo(Buffer const & buf, string const & name) 
	: InsetText(buf), type_(UNKNOWN_INFO), name_(),
	  mouse_hover_(false)
{
	setAutoBreakRows(true);
	setDrawFrame(true);
	setInfo(name);
}


Inset * InsetInfo::editXY(Cursor & cur, int x, int y)
{
	cur.push(*this);
	return InsetText::editXY(cur, x, y);
}


void InsetInfo::draw(PainterInfo & pi, int x, int y) const
{
	InsetText::draw(pi, x, y); 
}


string InsetInfo::infoType() const
{
	return nameTranslator().find(type_);
}


docstring InsetInfo::toolTip(BufferView const &, int, int) const
{
	odocstringstream os;
	os << _("Information regarding ")
	   << _(nameTranslator().find(type_))
	   << " " << from_utf8(name_);
	return os.str();
}

namespace {

Translator<InsetInfo::info_type, string> const initTranslator()
{	
	Translator<InsetInfo::info_type, string> 
		translator(InsetInfo::UNKNOWN_INFO, "unknown");

	translator.addPair(InsetInfo::SHORTCUTS_INFO, "shortcuts");
	translator.addPair(InsetInfo::SHORTCUT_INFO, "shortcut");
	translator.addPair(InsetInfo::LYXRC_INFO, "lyxrc");
	translator.addPair(InsetInfo::PACKAGE_INFO, "package");
	translator.addPair(InsetInfo::TEXTCLASS_INFO, "textclass");
	translator.addPair(InsetInfo::MENU_INFO, "menu");
	translator.addPair(InsetInfo::ICON_INFO, "icon");
	translator.addPair(InsetInfo::BUFFER_INFO, "buffer");

	return translator;
}

} // namespace anon

Translator<InsetInfo::info_type, string> 
	const & InsetInfo::nameTranslator() const
{
	static Translator<info_type, string> const translator = initTranslator();
	return translator;
}

	

void InsetInfo::read(Lexer & lex)
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


void InsetInfo::write(ostream & os) const
{
	os << "Info\ntype  \"" << nameTranslator().find(type_)
	   << "\"\narg   \"" << name_ << '\"';
}


bool InsetInfo::showInsetDialog(BufferView * bv) const
{
	bv->showDialog("info");
	return true;
}


bool InsetInfo::getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const
{
	switch (cmd.action) {

	case LFUN_INSET_MODIFY:
		flag.setEnabled(true);
		break;
	//FIXME: do something.
	/*
	*/

	default:
		return false;
	}
	return true;
}


void InsetInfo::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	// allow selection, copy but not cut, delete etc
	switch (cmd.action) {
	case LFUN_MOUSE_PRESS:
	case LFUN_MOUSE_RELEASE:
	case LFUN_MOUSE_MOTION:
	case LFUN_MOUSE_DOUBLE:
	case LFUN_MOUSE_TRIPLE:
	case LFUN_COPY:
	case LFUN_INSET_SETTINGS:
		InsetText::doDispatch(cur, cmd);
		break;

	default:
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
}


void InsetInfo::updateInfo()
{
	InsetText::clear();

	BufferParams const & bp = buffer().params();	

	switch (type_) {
	case UNKNOWN_INFO:
		setText(_("Unknown Info: ") + from_utf8(name_),
			bp.getFont(), false);
		break;
	case SHORTCUTS_INFO: {
		FuncRequest func = lyxaction.lookupFunc(name_);
		if (func.action != LFUN_UNKNOWN_ACTION)
			setText(theTopLevelKeymap().printBindings(func),
				bp.getFont(), false);
		break;
	}
	case SHORTCUT_INFO: {
		FuncRequest func = lyxaction.lookupFunc(name_);
		if (func.action != LFUN_UNKNOWN_ACTION) {
			KeyMap::Bindings bindings = theTopLevelKeymap().findBindings(func);
			if (!bindings.empty())
				setText(bindings.rbegin()->print(KeySequence::ForGui),
					bp.getFont(), false);
		}
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
		setText(from_utf8(result), bp.getFont(), false);
		break;
	}
	case PACKAGE_INFO:
		// check in packages.lst
		setText(LaTeXFeatures::isAvailable(name_) ? _("yes") : _("no"),
			bp.getFont(), false);
		break;
	case TEXTCLASS_INFO: {
		// name_ is the class name
		setText(LayoutFileList::get().haveClass(name_) ? _("yes") : _("no"),
		bp.getFont(), false);
		break;
	}
	case MENU_INFO: {
		docstring_list names;
		FuncRequest func = lyxaction.lookupFunc(name_);
		if (func.action == LFUN_UNKNOWN_ACTION) {
			setText(bformat(_("Unknown action %1$s"), from_utf8(name_)), bp.getFont(), false);
			break;
		}
		// iterate through the menubackend to find it
		if (!theApp()->searchMenu(func, names)) {
			setText(bformat(_("No menu entry for action %1$s"), from_utf8(name_)),
				bp.getFont(), false);
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
			for (i = 0; i != names.back().length(); ++i)
				info.insertChar(i, names.back()[i], bp.getFont(), false);
			names.pop_back();
		}
		break;
	}
	case ICON_INFO: {
		FuncRequest func = lyxaction.lookupFunc(name_);
		docstring icon_name = theApp()->iconName(func, true);
		//FIXME: We should use the icon directly instead of going through
		// FileName. The code below won't work if the icon is embedded in the
		// executable through the Qt resource system.
		FileName file(to_utf8(icon_name));
		if (!file.exists())
			break;
		InsetGraphicsParams igp;
		igp.filename = file;
		InsetGraphics * inset = new InsetGraphics(buffer());
		inset->setParams(igp);
		paragraphs().front().insertInset(0, inset, Change(Change::UNCHANGED));
		break;
	}
	case BUFFER_INFO: {
		if (name_ == "name")
			setText(from_utf8(buffer().fileName().onlyFileName()),
				bp.getFont(), false);
		else if (name_ == "path")
			setText(from_utf8(buffer().filePath()), bp.getFont(), false);
		else if (name_ == "class")
			setText(from_utf8(bp.documentClass().name()), bp.getFont(), false);
		else
			setText(_("Unknown buffer info"), bp.getFont(), false);
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


docstring InsetInfo::contextMenu(BufferView const &, int, int) const
{
	return from_ascii("context-info");
}

} // namespace lyx
