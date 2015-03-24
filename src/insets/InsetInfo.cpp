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
#include "CutAndPaste.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "InsetGraphics.h"
#include "InsetSpecialChar.h"
#include "KeyMap.h"
#include "LaTeXFeatures.h"
#include "LayoutFile.h"
#include "LyXAction.h"
#include "LyXRC.h"
#include "LyXVC.h"
#include "Lexer.h"
#include "ParagraphParameters.h"
#include "version.h"

#include "frontends/Application.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/docstream.h"
#include "support/docstring_list.h"
#include "support/ExceptionMessage.h"
#include "support/FileName.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/Translator.h"

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
	translator.addPair(InsetInfo::LYX_INFO, "lyxinfo");

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


	
InsetInfo::InsetInfo(Buffer * buf, string const & name) 
	: InsetCollapsable(buf), type_(UNKNOWN_INFO), name_()
{
	setAutoBreakRows(true);
	setInfo(name);
	status_ = Collapsed;
}


Inset * InsetInfo::editXY(Cursor & cur, int x, int y)
{
	// do not allow the cursor to be set in this Inset
	return Inset::editXY(cur, x, y);
}


string InsetInfo::infoType() const
{
	return nameTranslator().find(type_);
}


docstring InsetInfo::layoutName() const
{
	return from_ascii("Info:" + infoType());
}


docstring InsetInfo::toolTip(BufferView const &, int, int) const
{
	return bformat(_("Information regarding %1$s '%2$s'"),
			_(infoType()), from_utf8(name_));
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
	updateInfo();
}


void InsetInfo::write(ostream & os) const
{
	os << "Info\ntype  \"" << infoType() 
	   << "\"\narg   " << Lexer::quoteString(name_);
}


bool InsetInfo::validateModifyArgument(docstring const & arg) const
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
		return func.action() != LFUN_UNKNOWN_ACTION;
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
		if (name == "name" || name == "path" || name == "class")
			return true;
		if (name == "vcs-revision" || name == "vcs-tree-revision" ||
		       name == "vcs-author" || name == "vcs-date" || name == "vcs-time")
			return buffer().lyxvc().inUse();
		return false;

	case LYX_INFO:
		return name == "version";
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
	switch (cmd.action()) {
	case LFUN_INSET_SETTINGS:
		return InsetCollapsable::getStatus(cur, cmd, flag);

	case LFUN_INSET_DIALOG_UPDATE:
	case LFUN_INSET_COPY_AS:
		flag.setEnabled(true);
		return true;

	case LFUN_INSET_MODIFY:
		if (validateModifyArgument(cmd.argument())) {
			flag.setEnabled(true);
			return true;
		}
		//fall through

	default:
		return false;
	}
}


void InsetInfo::doDispatch(Cursor & cur, FuncRequest & cmd)
{
	switch (cmd.action()) {
	case LFUN_INSET_MODIFY:
		cur.recordUndo();
		setInfo(to_utf8(cmd.argument()));
		break;

	case LFUN_INSET_COPY_AS: {
		cap::clearSelection();
		Cursor copy(cur);
		copy.pushBackward(*this);
		copy.pit() = 0;
		copy.pos() = 0;
		copy.resetAnchor();
		copy.pit() = copy.lastpit();
		copy.pos() = copy.lastpos();
		copy.setSelection();
		cap::copySelection(copy);
		break;
	}

	default:
		InsetCollapsable::doDispatch(cur, cmd);
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


void InsetInfo::error(string const & err)
{
	setText(bformat(_(err), from_utf8(name_)), 
		Font(inherit_font, buffer().params().language), false);
}


void InsetInfo::setText(docstring const & str)
{
	setText(str, Font(inherit_font, buffer().params().language), false);
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
		FuncRequest const func = lyxaction.lookupFunc(name_);
		if (func.action() == LFUN_UNKNOWN_ACTION) {
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
			setText(bindings.begin()->print(KeySequence::Portable));
		else
			setText(theTopLevelKeymap().printBindings(func, KeySequence::Portable));
		break;
	}
	case LYXRC_INFO: {
		ostringstream oss;
		if (name_.empty()) {
			setText(_("undefined"));
			break;
		}
		lyxrc.write(oss, true, name_);
		string result = oss.str();
		if (result.size() < 2) {
			setText(_("undefined"));
			break;
		}
		string::size_type loc = result.rfind("\n", result.size() - 2);
		loc = loc == string::npos ? 0 : loc + 1;
		if (result.size() < loc + name_.size() + 1
			  || result.substr(loc + 1, name_.size()) != name_) {
			setText(_("undefined"));
			break;
		}
		// remove leading comments and \\name and space
		result = result.substr(loc + name_.size() + 2);
		
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
		LayoutFileList const & list = LayoutFileList::get();
		bool available = false;
		if (list.haveClass(name_))
			available = list[name_].isTeXClassAvailable();
		setText(available ? _("yes") : _("no"));
		break;
	}
	case MENU_INFO: {
		docstring_list names;
		FuncRequest const func = lyxaction.lookupFunc(name_);
		if (func.action() == LFUN_UNKNOWN_ACTION) {
			error("Unknown action %1$s");
			break;
		}
		// iterate through the menubackend to find it
		if (!theApp()) {
			error("Can't determine menu entry for action %1$s in batch mode");
			break;
		}
		if (!theApp()->searchMenu(func, names)) {
			error("No menu entry for action %1$s");
			break;
		}
		// if found, return its path.
		clear();
		Paragraph & par = paragraphs().front();
		Font const f(inherit_font, buffer().params().language);
		//Font fu = f;
		//fu.fontInfo().setUnderbar(FONT_ON);
		docstring_list::const_iterator beg = names.begin();
		docstring_list::const_iterator end = names.end();
		for (docstring_list::const_iterator it = beg ; 
		     it != end ; ++it) {
			// do not insert > for the top level menu item
			if (it != beg)
				par.insertInset(par.size(), new InsetSpecialChar(InsetSpecialChar::MENU_SEPARATOR),
						f, Change(Change::UNCHANGED));
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
		docstring icon_name = frontend::Application::iconName(func, true);
		// FIXME: We should use the icon directly instead of
		// going through FileName. The code below won't work
		// if the icon is embedded in the executable through
		// the Qt resource system.
		// This is only a negligible performance problem:
		// If the installed icon differs from the resource icon the
		// installed one is preferred anyway, and all icons that are
		// embedded in the resources are installed as well.
		FileName file(to_utf8(icon_name));
		if (!file.exists())
			break;
		InsetGraphics * inset = new InsetGraphics(buffer_);
		InsetGraphicsParams igp;
		igp.filename = file;
		igp.lyxscale = iconScaleFactor(file);
		igp.scale = convert<string>(igp.lyxscale);
		inset->setParams(igp);
		clear();
		Font const f(inherit_font, buffer().params().language);
		paragraphs().front().insertInset(0, inset, f,
						 Change(Change::UNCHANGED));
		break;
	}
	case BUFFER_INFO: {
		if (name_ == "name") {
			setText(from_utf8(buffer().fileName().onlyFileName()));
			break;
		}
		if (name_ == "path") {
			setText(from_utf8(os::latex_path(buffer().filePath())));
			break;
		}
		if (name_ == "class") {
			setText(from_utf8(bp.documentClass().name()));
			break;
		}
		
		// everything that follows is for version control.
		// nothing that isn't version control should go below this line.
		if (!buffer().lyxvc().inUse()) {
			setText(_("No version control"));
			break;
		}
		LyXVC::RevisionInfo itype = LyXVC::Unknown;
		if (name_ == "vcs-revision")
			itype = LyXVC::File;
		else if (name_ == "vcs-tree-revision")
			itype = LyXVC::Tree;
		else if (name_ == "vcs-author")
			itype = LyXVC::Author;
		else if (name_ == "vcs-time")
			itype = LyXVC::Time;
		else if (name_ == "vcs-date")
			itype = LyXVC::Date;
		string binfo = buffer().lyxvc().revisionInfo(itype);
		if (binfo.empty())
			setText(from_ascii(name_) + " unknown");
		else
			setText(from_utf8(binfo));
		break;
	}
	case LYX_INFO:
		if (name_ == "version")
			setText(from_ascii(lyx_version));
		break;
	}
}


string InsetInfo::contextMenu(BufferView const &, int, int) const
{
	//FIXME: We override the implementation of InsetCollapsable,
	//because this inset is not a collapsable inset.
	return contextMenuName();
}


string InsetInfo::contextMenuName() const
{
	return "context-info";
}


} // namespace lyx
