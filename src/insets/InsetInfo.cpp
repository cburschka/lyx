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

#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "InsetGraphics.h"
#include "InsetSpecialChar.h"
#include "KeyMap.h"
#include "LaTeXFeatures.h"
#include "LayoutFile.h"
#include "LyXAction.h"
#include "LyXRC.h"
#include "Lexer.h"
#include "MetricsInfo.h"
#include "ParagraphParameters.h"

#include "frontends/Application.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/docstring_list.h"
#include "support/ExceptionMessage.h"
#include "support/FileName.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"

#include <sstream>

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace {

typedef Translator<InsetInfo::info_type, string> NameTranslator;

NameTranslator const initTranslator()
{	
	NameTranslator translator(InsetInfo::UNKNOWN_INFO, "unknown");

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

/// The translator between the information type enum and corresponding string.
NameTranslator const & nameTranslator()
{
	static NameTranslator const translator = initTranslator();
	return translator;
}

} // namespace anon

/////////////////////////////////////////////////////////////////////////
//
// InsetInfo
//
/////////////////////////////////////////////////////////////////////////


	
InsetInfo::InsetInfo(Buffer const & buf, string const & name) 
	: InsetCollapsable(buf), type_(UNKNOWN_INFO), name_()
{
	setAutoBreakRows(true);
	setInfo(name);
	status_ = Collapsed;
}


Inset * InsetInfo::editXY(Cursor & cur, int x, int y)
{
	cur.push(*this);
	return InsetCollapsable::editXY(cur, x, y);
}


string InsetInfo::infoType() const
{
	return nameTranslator().find(type_);
}


docstring InsetInfo::name() const 
{
	return from_ascii("Info:" + infoType());
}


docstring InsetInfo::toolTip(BufferView const &, int, int) const
{
	odocstringstream os;
	os << _("Information regarding ")
	   << _(infoType()) << " " << from_utf8(name_);
	return os.str();
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
			lex.next(true);
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
	setLayout(buffer().params());
	updateInfo();
}


void InsetInfo::write(ostream & os) const
{
	os << "Info\ntype  \"" << infoType() 
	   << "\"\narg   " << Lexer::quoteString(name_);
}


bool InsetInfo::validate(docstring const & arg) const
{
	string type;
	string const name = trim(split(to_utf8(arg), type, ' '));
	switch (nameTranslator().find(type)) {
	case UNKNOWN_INFO:
		return false;
	case SHORTCUT_INFO:
	case SHORTCUTS_INFO:
	case MENU_INFO:
	case ICON_INFO: {
		FuncRequest func = lyxaction.lookupFunc(name);
		return func.action != LFUN_UNKNOWN_ACTION;
	}
	case LYXRC_INFO: {
		ostringstream oss;
		lyxrc.write(oss, true, name);
		return !oss.str().empty();
	}
	case PACKAGE_INFO:
	case TEXTCLASS_INFO:
		return true;
	case BUFFER_INFO:
		return name == "name" || name == "path" || name == "class";
	}
	return false;
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
	case LFUN_MOUSE_PRESS:
	case LFUN_MOUSE_RELEASE:
	case LFUN_MOUSE_MOTION:
	case LFUN_MOUSE_DOUBLE:
	case LFUN_MOUSE_TRIPLE:
	case LFUN_COPY:
	case LFUN_INSET_SETTINGS:
		return InsetCollapsable::getStatus(cur, cmd, flag);

	case LFUN_INSET_MODIFY:
		flag.setEnabled(true);
		break;

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
		InsetCollapsable::doDispatch(cur, cmd);
		break;

	case LFUN_INSET_MODIFY:
		setInfo(to_utf8(cmd.argument()));
		cur.pos() = 0;
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
	setLayout(buffer().params());
	updateInfo();
}


void InsetInfo::error(string const & err)
{
	setText(bformat(_(err), from_utf8(name_)), Font(inherit_font), false);
}


void InsetInfo::setText(docstring const & str)
{
	setText(str, Font(inherit_font), false);
}


void InsetInfo::updateInfo()
{
	BufferParams const & bp = buffer().params();	

	switch (type_) {
	case UNKNOWN_INFO:
		error("Unknown Info: %1$s");
		break;
	case SHORTCUT_INFO:
	case SHORTCUTS_INFO: {
		FuncRequest func = lyxaction.lookupFunc(name_);
		if (func.action == LFUN_UNKNOWN_ACTION) {
			error("Unknown action %1$s");
			break;
		}
		KeyMap::Bindings bindings = theTopLevelKeymap().findBindings(func);
		if (bindings.empty()) {
			// It is impropriate to use error() for undefined shortcut
			setText(_("undefined"));
			break;
		}
		if (type_ == SHORTCUT_INFO)
			setText(bindings.rbegin()->print(KeySequence::Portable),
				Font(getLayout().font()), false);
		else
			setText(theTopLevelKeymap().printBindings(func, KeySequence::Portable), 
				Font(getLayout().font()), false);
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
		setText(from_utf8(result));
		break;
	}
	case PACKAGE_INFO:
		// check in packages.lst
		setText(LaTeXFeatures::isAvailable(name_) ? _("yes") : _("no"));
		break;
	case TEXTCLASS_INFO: {
		// name_ is the class name
		setText(LayoutFileList::get().haveClass(name_) ? _("yes") : _("no"));
		break;
	}
	case MENU_INFO: {
		docstring_list names;
		FuncRequest func = lyxaction.lookupFunc(name_);
		if (func.action == LFUN_UNKNOWN_ACTION) {
			error("Unknown action %1$s");
			break;
		}
		// iterate through the menubackend to find it
		if (!theApp()->searchMenu(func, names)) {
			error("No menu entry for action %1$s");
			break;
		}
		// if found, return its path.
		clear();
		Paragraph & par = paragraphs().front();
		Font const f = Font(getLayout().font());
		//Font fu = f;
		//fu.fontInfo().setUnderbar(FONT_ON);
		docstring_list::const_iterator beg = names.begin();
		docstring_list::const_iterator end = names.end();
		for (docstring_list::const_iterator it = beg ; 
		     it != end ; ++it) {
			// do not insert > for the top level menu item
			if (it != beg)
				par.insertInset(par.size(), new InsetSpecialChar(InsetSpecialChar::MENU_SEPARATOR),
						Change(Change::UNCHANGED));
			//FIXME: add proper underlines here. This
			// involves rewriting searchMenu used above to
			// return a vector of menus. If we do not do
			// that, we might as well use below
			// Paragraph::insert on each string (JMarc)
			for (size_type i = 0; i != it->length(); ++i)
				par.insertChar(par.size(), (*it)[i], 
					       f, Change(Change::UNCHANGED));
		}
		break;
	}
	case ICON_INFO: {
		FuncRequest func = lyxaction.lookupFunc(name_);
		docstring icon_name = theApp()->iconName(func, true);
		//FIXME: We should use the icon directly instead of
		// going through FileName. The code below won't work
		// if the icon is embedded in the executable through
		// the Qt resource system.
		FileName file(to_utf8(icon_name));
		if (!file.exists())
			break;
		InsetGraphics * inset = new InsetGraphics(buffer());
		InsetGraphicsParams igp;
		igp.filename = file;
		inset->setParams(igp);
		clear();
		paragraphs().front().insertInset(0, inset, 
						 Change(Change::UNCHANGED));
		break;
	}
	case BUFFER_INFO: {
		if (name_ == "name")
			setText(from_utf8(buffer().fileName().onlyFileName()));
		else if (name_ == "path")
			setText(from_utf8(buffer().filePath()));
		else if (name_ == "class")
			setText(from_utf8(bp.documentClass().name()));
		else
			setText(_("Unknown buffer info"));
		break;
	}
	}
}


docstring InsetInfo::contextMenu(BufferView const &, int, int) const
{
	return from_ascii("context-info");
}


} // namespace lyx
