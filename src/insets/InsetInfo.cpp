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
#include "LyX.h"
#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "CutAndPaste.h"
#include "Font.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "InsetGraphics.h"
#include "InsetSpecialChar.h"
#include "KeyMap.h"
#include "LaTeXFeatures.h"
#include "Language.h"
#include "LayoutFile.h"
#include "Length.h"
#include "LyXAction.h"
#include "LyXRC.h"
#include "LyXVC.h"
#include "Lexer.h"
#include "Paragraph.h"
#include "ParIterator.h"
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
#include "support/Messages.h"
#include "support/lstrings.h"
#include "support/qstring_helpers.h"
#include "support/Translator.h"

#include <sstream>

#include <QtGui/QImage>

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
	translator.addPair(InsetInfo::VCS_INFO, "vcs");

	return translator;
}

/// The translator between the information type enum and corresponding string.
NameTranslator const & nameTranslator()
{
	static NameTranslator const translator = initTranslator();
	return translator;
}

} // namespace

/////////////////////////////////////////////////////////////////////////
//
// InsetInfo
//
/////////////////////////////////////////////////////////////////////////



InsetInfo::InsetInfo(Buffer * buf, string const & name)
	: InsetCollapsible(buf), initialized_(false), 
	  type_(UNKNOWN_INFO), name_(), force_ltr_(false)
{
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
	docstring result;
	switch (nameTranslator().find(infoType())) {
	case UNKNOWN_INFO:
		result = _("Invalid information inset");
		break;
	case SHORTCUT_INFO:
		result = bformat(_("The keybard shortcut for the function '%1$s'"),
				from_utf8(name_));
		break;
	case SHORTCUTS_INFO:
		result = bformat(_("The keybard shortcuts for the function '%1$s'"),
				from_utf8(name_));
		break;
	case MENU_INFO: 
		result = bformat(_("The menu location for the function '%1$s'"),
				from_utf8(name_));
		break;
	case ICON_INFO:
		result = bformat(_("The toolbar icon for the function '%1$s'"),
				from_utf8(name_));
		break;
	case LYXRC_INFO:
		result = bformat(_("The preference setting for the preference key '%1$s'"),
				from_utf8(name_));
		break;
	case PACKAGE_INFO:
		result = bformat(_("Availability of the LaTeX package '%1$s'"),
				from_utf8(name_));
		break;
	case TEXTCLASS_INFO:
		result = bformat(_("Availability of the LaTeX class '%1$s'"),
				from_utf8(name_));
		break;
	case BUFFER_INFO:
		if (name_ == "name")
			result = _("The name of this file");
		else if (name_ == "path")
			result = _("The path where this file is saved");
		else if (name_ == "class")
			result = _("The class this document uses");
		break;
	case VCS_INFO:
		if (name_ == "revision")
			result = _("Version control revision");
		else if (name_ == "tree-revision")
			result = _("Version control tree revision");
		else if (name_ == "author")
			 result = _("Version control author");
		else if (name_ == "date")
			result = _("Version control date");
		else if (name_ == "time")
			result = _("Version control time");
		break;
	case LYX_INFO:
		result = _("The current LyX version");
		break;
	}

	return result;
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
	case MENU_INFO: {
		FuncRequest func = lyxaction.lookupFunc(name);
		return func.action() != LFUN_UNKNOWN_ACTION;
	}

	case ICON_INFO: {
		FuncCode const action = lyxaction.lookupFunc(name).action();
		if (action == LFUN_UNKNOWN_ACTION) {
			string dir = "images";
			return !imageLibFileSearch(dir, name, "svgz,png").empty();
		}
		return true;
	}

	case LYXRC_INFO: {
		set<string> rcs = lyxrc.getRCs();
		return rcs.find(name) != rcs.end();
	}

	case PACKAGE_INFO:
	case TEXTCLASS_INFO:
		return true;

	case BUFFER_INFO:
		return (name == "name" || name == "path" || name == "class");

	case VCS_INFO:
		if (name == "revision" || name == "tree-revision"
		    || name == "author" || name == "date" || name == "time")
			return buffer().lyxvc().inUse();
		return false;

	case LYX_INFO:
		return name == "version";
	}

	return false;
}


namespace{
set<string> getTexFileList(string const & filename)
{
	set<string> list;
	FileName const file = libFileSearch(string(), filename);
	if (file.empty())
		return list;

	// FIXME Unicode.
	vector<docstring> doclist =
		getVectorFromString(file.fileContents("UTF-8"), from_ascii("\n"));

	// Normalise paths like /foo//bar ==> /foo/bar
	for (auto doc : doclist) {
		subst(doc, from_ascii("\r"), docstring());
		while (contains(doc, from_ascii("//")))
			subst(doc, from_ascii("//"), from_ascii("/"));
		if (!doc.empty())
			list.insert(removeExtension(onlyFileName(to_utf8(doc))));
	}

	// remove duplicates
	return list;
}
} // namespace anon


vector<pair<string,docstring>> InsetInfo::getArguments(string const & type) const
{
	vector<pair<string,docstring>> result;

	switch (nameTranslator().find(type)) {
	case UNKNOWN_INFO:
		result.push_back(make_pair("invalid", _("Please select a valid type!")));
		break;

	case SHORTCUT_INFO:
	case SHORTCUTS_INFO:
	case MENU_INFO:
	case ICON_INFO: {
		result.push_back(make_pair("custom", _("Custom")));
		LyXAction::const_iterator fit = lyxaction.func_begin();
		LyXAction::const_iterator const fen = lyxaction.func_end();
		for (; fit != fen; ++fit) {
			string const lfun = fit->first;
			if (!lfun.empty())
				result.push_back(make_pair(lfun, from_ascii(lfun)));
		}
		break;
	}

	case LYXRC_INFO: {
		result.push_back(make_pair("custom", _("Custom")));
		set<string> rcs = lyxrc.getRCs();
		for (auto const & rc : rcs)
			result.push_back(make_pair(rc, from_ascii(rc)));
		break;
	}

	case PACKAGE_INFO:
	case TEXTCLASS_INFO: {
		result.push_back(make_pair("custom", _("Custom")));
		string const filename = (type == "package") ? "styFiles.lst"
							    : "clsFiles.lst";
		set<string> flist = getTexFileList(filename);
		for (auto const & f : flist)
			result.push_back(make_pair(f, from_utf8(f)));
		break;
	}

	case BUFFER_INFO:
		result.push_back(make_pair("name", _("File name")));
		result.push_back(make_pair("path", _("File path")));
		result.push_back(make_pair("class", _("Used text class")));
		break;

	case VCS_INFO: {
		if (!buffer().lyxvc().inUse()) {
			result.push_back(make_pair("invalid", _("No version control!")));
			break;
		}
		result.push_back(make_pair("revision", _("Revision[[Version Control]]")));
		result.push_back(make_pair("tree-revision", _("Tree revision")));
		result.push_back(make_pair("author", _("Author")));
		result.push_back(make_pair("date", _("Date")));
		result.push_back(make_pair("time", _("Time")));
		break;
	}

	case LYX_INFO:
		result.push_back(make_pair("version", _("LyX version")));
		break;
	}

	return result;
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
		return InsetCollapsible::getStatus(cur, cmd, flag);

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
		cur.forceBufferUpdate();
		initialized_ = false;
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
		InsetCollapsible::doDispatch(cur, cmd);
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


void InsetInfo::error(docstring const & err, Language const * lang)
{
	setText(bformat(translateIfPossible(err, lang->code()), from_utf8(name_)),
		Font(inherit_font, lang), false);
}


void InsetInfo::info(docstring const & err, Language const * lang)
{
	setText(translateIfPossible(err, lang->code()),
			Font(inherit_font, lang), false);
}


void InsetInfo::setText(docstring const & str, Language const * lang)
{
	setText(str, Font(inherit_font, lang), false);
}


bool InsetInfo::forceLTR() const
{
	return force_ltr_;
}


void InsetInfo::updateBuffer(ParIterator const & it, UpdateType utype) {
	// If the Buffer is a clone, then we neither need nor want to do any
	// of what follows. We want, rather, just to inherit how things were
	// in the original Buffer. This is especially important for VCS.
	// Otherwise, we could in principle have different settings here
	// than in the Buffer we were exporting.
	if (buffer().isClone())
		return;

	BufferParams const & bp = buffer().params();
	Language const * lang = it.paragraph().getFontSettings(bp, it.pos()).language();
	Language const * tryguilang = languages.getFromCode(Messages::guiLanguage());
	// Some info insets use the language of the GUI (if available)
	Language const * guilang = tryguilang ? tryguilang : lang;

	force_ltr_ = !lang->rightToLeft();
	// This is just to get the string into the po files
	docstring gui;
	switch (type_) {
	case UNKNOWN_INFO:
		gui = _("Unknown Info!");
		info(from_ascii("Unknown Info!"), lang);
		initialized_ = false;
		break;
	case SHORTCUT_INFO:
	case SHORTCUTS_INFO: {
		// shortcuts can change, so we need to re-do this each time
		FuncRequest const func = lyxaction.lookupFunc(name_);
		if (func.action() == LFUN_UNKNOWN_ACTION) {
			gui = _("Unknown action %1$s");
			error(from_ascii("Unknown action %1$s"), lang);
			break;
		}
		KeyMap::Bindings bindings = theTopLevelKeymap().findBindings(func);
		if (bindings.empty()) {
			gui = _("undefined");
			info(from_ascii("undefined"), lang);
			break;
		}
		if (type_ == SHORTCUT_INFO)
			setText(bindings.begin()->print(KeySequence::Portable), guilang);
		else
			setText(theTopLevelKeymap().printBindings(func, KeySequence::Portable), guilang);
		force_ltr_ = !guilang->rightToLeft() && !lang->rightToLeft();
		break;
	}
	case LYXRC_INFO: {
		// this information could change, if the preferences are changed,
		// so we will recalculate each time through.
		ostringstream oss;
		if (name_.empty()) {
			gui = _("undefined");
			info(from_ascii("undefined"), lang);
			break;
		}
		// FIXME this uses the serialization mechanism to get the info
		// we want, which i guess works but is a bit strange.
		lyxrc.write(oss, true, name_);
		string result = oss.str();
		if (result.size() < 2) {
			gui = _("undefined");
			info(from_ascii("undefined"), lang);
			break;
		}
		string::size_type loc = result.rfind("\n", result.size() - 2);
		loc = loc == string::npos ? 0 : loc + 1;
		if (result.size() < loc + name_.size() + 1
			  || result.substr(loc + 1, name_.size()) != name_) {
			gui = _("undefined");
			info(from_ascii("undefined"), lang);
			break;
		}
		// remove leading comments and \\name and space
		result = result.substr(loc + name_.size() + 2);

		// remove \n and ""
		result = rtrim(result, "\n");
		result = trim(result, "\"");
		setText(from_utf8(result), lang);
		break;
	}
	case PACKAGE_INFO:
		// only need to do this once.
		if (initialized_)
			break;
		// check in packages.lst
		if (LaTeXFeatures::isAvailable(name_)) {
			gui = _("yes");
			info(from_ascii("yes"), lang);
		} else {
			gui = _("no");
			info(from_ascii("no"), lang);
		}
		initialized_ = true;
		break;

	case TEXTCLASS_INFO: {
		// the TextClass can change
		LayoutFileList const & list = LayoutFileList::get();
		bool available = false;
		// name_ is the class name
		if (list.haveClass(name_))
			available = list[name_].isTeXClassAvailable();
		if (available) {
			gui = _("yes");
			info(from_ascii("yes"), lang);
		} else {
			gui = _("no");
			info(from_ascii("no"), lang);
		}
		break;
	}
	case MENU_INFO: {
		// only need to do this once.
		if (initialized_)
			break;
		// and we will not keep trying if we fail
		initialized_ = true;
		docstring_list names;
		FuncRequest const func = lyxaction.lookupFunc(name_);
		if (func.action() == LFUN_UNKNOWN_ACTION) {
			gui = _("Unknown action %1$s");
			error(from_ascii("Unknown action %1$s"), lang);
			break;
		}
		// iterate through the menubackend to find it
		if (!theApp()) {
			gui = _("Can't determine menu entry for action %1$s in batch mode");
			error(from_ascii("Can't determine menu entry for action %1$s in batch mode"), lang);
			break;
		}
		if (!theApp()->searchMenu(func, names)) {
			gui = _("No menu entry for action %1$s");
			error(from_ascii("No menu entry for action %1$s"), lang);
			break;
		}
		// if found, return its path.
		clear();
		Paragraph & par = paragraphs().front();
		Font const f(inherit_font, guilang);
		force_ltr_ = !guilang->rightToLeft();
		//Font fu = f;
		//fu.fontInfo().setUnderbar(FONT_ON);
		for (docstring const & name : names) {
			// do not insert > for the top level menu item
			if (&name != &names.front())
				par.insertInset(par.size(), new InsetSpecialChar(InsetSpecialChar::MENU_SEPARATOR),
						f, Change(Change::UNCHANGED));
			//FIXME: add proper underlines here. This
			// involves rewriting searchMenu used above to
			// return a vector of menus. If we do not do
			// that, we might as well use below
			// Paragraph::insert on each string (JMarc)
			for (char_type c : name)
				par.insertChar(par.size(), c, f, Change(Change::UNCHANGED));
		}
		break;
	}
	case ICON_INFO: {
		// only need to do this once.
		if (initialized_)
			break;
		// and we will not keep trying if we fail
		initialized_ = true;
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
		if (file.onlyFileNameWithoutExt() == "unknown") {
			string dir = "images";
			FileName file2(imageLibFileSearch(dir, name_, "svgz,png"));
			if (!file2.empty())
				file = file2;
		}
		if (!file.exists())
			break;
		int percent_scale = 100;
		if (use_gui) {
			// Compute the scale factor for the icon such that its
			// width on screen is equal to 1em in pixels.
			// The scale factor is rounded to the integer nearest
			// to the float value of the ratio 100*iconsize/imgsize.
			int imgsize = QImage(toqstr(file.absFileName())).width();
			if (imgsize > 0) {
				int iconsize = Length(1, Length::EM).inPixels(1);
				percent_scale = (100 * iconsize + imgsize / 2)/imgsize;
			}
		}
		InsetGraphics * inset = new InsetGraphics(buffer_);
		InsetGraphicsParams igp;
		igp.filename = file;
		igp.lyxscale = percent_scale;
		igp.scale = string();
		igp.width = Length(1, Length::EM);
		inset->setParams(igp);
		clear();
		Font const f(inherit_font, lang);
		paragraphs().front().insertInset(0, inset, f,
						 Change(Change::UNCHANGED));
		break;
	}
	case BUFFER_INFO: {
		// this could all change, so we will recalculate each time
		if (name_ == "name")
			setText(from_utf8(buffer().fileName().onlyFileName()), lang);
		else if (name_ == "path")
			setText(from_utf8(os::latex_path(buffer().filePath())), lang);
		else if (name_ == "class")
			setText(from_utf8(bp.documentClass().name()), lang);
		break;
	}
	case VCS_INFO: {
		// this information could change, in principle, so we will 
		// recalculate each time through
		if (!buffer().lyxvc().inUse()) {
			gui = _("No version control!");
			info(from_ascii("No version control!"), lang);
			break;
		}
		LyXVC::RevisionInfo itype = LyXVC::Unknown;
		if (name_ == "revision")
			itype = LyXVC::File;
		else if (name_ == "tree-revision")
			itype = LyXVC::Tree;
		else if (name_ == "author")
			itype = LyXVC::Author;
		else if (name_ == "time")
			itype = LyXVC::Time;
		else if (name_ == "date")
			itype = LyXVC::Date;
		string binfo = buffer().lyxvc().revisionInfo(itype);
		if (binfo.empty()) {
			gui = _("%1$s[[vcs data]] unknown");
			error(from_ascii("%1$s[[vcs data]] unknown"), lang);
		} else
			setText(from_utf8(binfo), lang);
		break;
	}
	case LYX_INFO:
		// only need to do this once.
		if (initialized_)
			break;
		if (name_ == "version")
			setText(from_ascii(lyx_version), lang);
		initialized_ = true;
		break;
	}
	// Just to do something with that string
	LYXERR(Debug::INFO, "info inset text: " << gui);
	InsetCollapsible::updateBuffer(it, utype);
}


string InsetInfo::contextMenu(BufferView const &, int, int) const
{
	//FIXME: We override the implementation of InsetCollapsible,
	//because this inset is not a collapsible inset.
	return contextMenuName();
}


string InsetInfo::contextMenuName() const
{
	return "context-info";
}


} // namespace lyx
