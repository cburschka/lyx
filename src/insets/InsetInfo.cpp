/**
 * \file InsetInfo.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Bo Peng
 * \author Jürgen Spitzmüller
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
#include <QDate>
#include <QLocale>

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace {

typedef Translator<InsetInfoParams::info_type, string> NameTranslator;

NameTranslator const initTranslator()
{
	NameTranslator translator(InsetInfoParams::UNKNOWN_INFO, "unknown");

	translator.addPair(InsetInfoParams::SHORTCUTS_INFO, "shortcuts");
	translator.addPair(InsetInfoParams::SHORTCUT_INFO, "shortcut");
	translator.addPair(InsetInfoParams::LYXRC_INFO, "lyxrc");
	translator.addPair(InsetInfoParams::PACKAGE_INFO, "package");
	translator.addPair(InsetInfoParams::TEXTCLASS_INFO, "textclass");
	translator.addPair(InsetInfoParams::MENU_INFO, "menu");
	translator.addPair(InsetInfoParams::L7N_INFO, "l7n");
	translator.addPair(InsetInfoParams::ICON_INFO, "icon");
	translator.addPair(InsetInfoParams::BUFFER_INFO, "buffer");
	translator.addPair(InsetInfoParams::LYX_INFO, "lyxinfo");
	translator.addPair(InsetInfoParams::VCS_INFO, "vcs");
	translator.addPair(InsetInfoParams::DATE_INFO, "date");
	translator.addPair(InsetInfoParams::MODDATE_INFO, "moddate");
	translator.addPair(InsetInfoParams::FIXDATE_INFO, "fixdate");
	translator.addPair(InsetInfoParams::TIME_INFO, "time");
	translator.addPair(InsetInfoParams::MODTIME_INFO, "modtime");
	translator.addPair(InsetInfoParams::FIXTIME_INFO, "fixtime");

	return translator;
}

/// The translator between the information type enum and corresponding string.
NameTranslator const & nameTranslator()
{
	static NameTranslator const translator = initTranslator();
	return translator;
}


typedef Translator<InsetInfoParams::info_type, string> DefaultValueTranslator;

DefaultValueTranslator const initDVTranslator()
{
	DefaultValueTranslator translator(InsetInfoParams::UNKNOWN_INFO, "");

	translator.addPair(InsetInfoParams::SHORTCUTS_INFO, "info-insert");
	translator.addPair(InsetInfoParams::SHORTCUT_INFO, "info-insert");
	translator.addPair(InsetInfoParams::LYXRC_INFO, "user_name");
	translator.addPair(InsetInfoParams::PACKAGE_INFO, "graphics");
	translator.addPair(InsetInfoParams::TEXTCLASS_INFO, "article");
	translator.addPair(InsetInfoParams::MENU_INFO, "info-insert");
	translator.addPair(InsetInfoParams::L7N_INFO, "");
	translator.addPair(InsetInfoParams::ICON_INFO, "info-insert");
	translator.addPair(InsetInfoParams::BUFFER_INFO, "name-noext");
	translator.addPair(InsetInfoParams::LYX_INFO, "version");
	translator.addPair(InsetInfoParams::VCS_INFO, "revision");
	translator.addPair(InsetInfoParams::DATE_INFO, "loclong");
	translator.addPair(InsetInfoParams::MODDATE_INFO, "loclong");
	translator.addPair(InsetInfoParams::FIXDATE_INFO, "loclong");
	translator.addPair(InsetInfoParams::TIME_INFO, "long");
	translator.addPair(InsetInfoParams::MODTIME_INFO, "long");
	translator.addPair(InsetInfoParams::FIXTIME_INFO, "long");

	return translator;
}

/// The translator between the information type enum and some sensible default value.
DefaultValueTranslator const & defaultValueTranslator()
{
	static DefaultValueTranslator const translator = initDVTranslator();
	return translator;
}

} // namespace


/////////////////////////////////////////////////////////////////////
//
// InsetInfoParams
//
///////////////////////////////////////////////////////////////////////

InsetInfoParams infoparams;

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
		doc = subst(doc, from_ascii("\r"), docstring());
		while (contains(doc, from_ascii("//")))
			doc = subst(doc, from_ascii("//"), from_ascii("/"));
		if (!doc.empty())
			list.insert(removeExtension(onlyFileName(to_utf8(doc))));
	}

	// remove duplicates
	return list;
}

bool translateString(docstring const & in, docstring & out, string const & lcode)
{
	out = translateIfPossible(in, lcode);
	return in != out;
}
} // namespace anon


docstring InsetInfoParams::getDate(string const & iname, QDate const date) const
{
	QLocale loc;
	if (lang)
		loc = QLocale(toqstr(lang->code()));
	if (iname == "long")
		return qstring_to_ucs4(loc.toString(date, QLocale::LongFormat));
	else if (iname == "short")
		return qstring_to_ucs4(loc.toString(date, QLocale::ShortFormat));
	else if (iname == "ISO")
		return qstring_to_ucs4(date.toString(Qt::ISODate));
	else if (iname == "loclong")
		return lang ? qstring_to_ucs4(loc.toString(date, toqstr(lang->dateFormat(0))))
			    : _("No long date format (language unknown)!");
	else if (iname == "locmedium")
		return lang ? qstring_to_ucs4(loc.toString(date, toqstr(lang->dateFormat(1))))
			    : _("No medium date format (language unknown)!");
	else if (iname == "locshort")
		return lang ? qstring_to_ucs4(loc.toString(date, toqstr(lang->dateFormat(2))))
				: _("No short date format (language unknown)!");
	else
		return qstring_to_ucs4(loc.toString(date, toqstr(iname)));
}


docstring InsetInfoParams::getTime(string const & iname, QTime const time) const
{
	QLocale loc;
	if (lang)
		loc = QLocale(toqstr(lang->code()));
	if (iname == "long")
		return qstring_to_ucs4(loc.toString(time, QLocale::LongFormat));
	else if (iname == "short")
		return qstring_to_ucs4(loc.toString(time, QLocale::ShortFormat));
	else if (iname == "ISO")
		return qstring_to_ucs4(time.toString(Qt::ISODate));
	else
		return qstring_to_ucs4(loc.toString(time, toqstr(iname)));
}


vector<pair<string,docstring>> InsetInfoParams::getArguments(Buffer const * buf,
							     string const & itype) const
{
	vector<pair<string,docstring>> result;

	switch (nameTranslator().find(itype)) {
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

	case L7N_INFO:
		result.push_back(make_pair("custom", _("Custom")));
		break;

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
		string const filename = (itype == "package") ? "styFiles.lst"
							    : "clsFiles.lst";
		set<string> flist = getTexFileList(filename);
		for (auto const & f : flist)
			result.push_back(make_pair(f, from_utf8(f)));
		break;
	}

	case BUFFER_INFO:
		result.push_back(make_pair("name", _("File name (with extension)")));
		result.push_back(make_pair("name-noext", _("File name (without extension)")));
		result.push_back(make_pair("path", _("File path")));
		result.push_back(make_pair("class", _("Used text class")));
		break;

	case VCS_INFO: {
		if (!buf->lyxvc().inUse()) {
			result.push_back(make_pair("invalid", _("No version control!")));
			break;
		}
		result.push_back(make_pair("revision", _("Revision[[Version Control]]")));
		result.push_back(make_pair("revision-abbrev", _("Abbreviated revision[[Version Control]]")));
		result.push_back(make_pair("tree-revision", _("Tree revision")));
		result.push_back(make_pair("author", _("Author")));
		result.push_back(make_pair("date", _("Date")));
		result.push_back(make_pair("time", _("Time[[of day]]")));
		break;
	}

	case LYX_INFO:
		result.push_back(make_pair("version", _("LyX version")));
		result.push_back(make_pair("layoutformat", _("LyX layout format")));
		break;

	case FIXDATE_INFO:
	case DATE_INFO:
	case MODDATE_INFO: {
		string const dt = split(name, '@');
		QDate date;
		if (itype == "moddate")
			date = QDateTime::fromTime_t(buf->fileName().lastModified()).date();
		else if (itype == "fixdate" && !dt.empty()) {
			QDate const gdate = QDate::fromString(toqstr(dt), Qt::ISODate);
			date = (gdate.isValid()) ? gdate : QDate::currentDate();
		} else
			date = QDate::currentDate();
		result.push_back(make_pair("long",getDate("long", date)));
		result.push_back(make_pair("short", getDate("short", date)));
		result.push_back(make_pair("loclong", getDate("loclong", date)));
		result.push_back(make_pair("locmedium", getDate("locmedium", date)));
		result.push_back(make_pair("locshort", getDate("locshort", date)));
		result.push_back(make_pair("ISO", getDate("ISO", date)));
		result.push_back(make_pair("yyyy", getDate("yyyy", date)));
		result.push_back(make_pair("MMMM", getDate("MMMM", date)));
		result.push_back(make_pair("MMM", getDate("MMM", date)));
		result.push_back(make_pair("dddd", getDate("dddd", date)));
		result.push_back(make_pair("ddd", getDate("ddd", date)));
		result.push_back(make_pair("custom", _("Custom")));
		break;
	}
	case FIXTIME_INFO:
	case TIME_INFO:
	case MODTIME_INFO: {
		string const tt = split(name, '@');
		QTime time;
		if (itype == "modtime")
			time = QDateTime::fromTime_t(buf->fileName().lastModified()).time();
		else if (itype == "fixtime" && !tt.empty()) {
			QTime const gtime = QTime::fromString(toqstr(tt), Qt::ISODate);
			time = (gtime.isValid()) ? gtime : QTime::currentTime();
		} else
			time = QTime::currentTime();
		result.push_back(make_pair("long",getTime("long", time)));
		result.push_back(make_pair("short", getTime("short", time)));
		result.push_back(make_pair("ISO", getTime("ISO", time)));
		result.push_back(make_pair("custom", _("Custom")));
		break;
	}
	}

	return result;
}


bool InsetInfoParams::validateArgument(Buffer const * buf, docstring const & arg,
				       bool const usedefaults) const
{
	string type;
	string name = trim(split(to_utf8(arg), type, ' '));
	if (name.empty() && usedefaults)
		name = defaultValueTranslator().find(type);

	switch (nameTranslator().find(type)) {
	case UNKNOWN_INFO:
		return false;

	case SHORTCUT_INFO:
	case SHORTCUTS_INFO:
	case MENU_INFO: {
		FuncRequest func = lyxaction.lookupFunc(name);
		return func.action() != LFUN_UNKNOWN_ACTION;
	}

	case L7N_INFO:
		return !name.empty();

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
		return (name == "name" || name == "name-noext"
			|| name == "path" || name == "class");

	case VCS_INFO:
		if (name == "revision" || name == "revision-abbrev" || name == "tree-revision"
		    || name == "author" || name == "date" || name == "time")
			return buf->lyxvc().inUse();
		return false;

	case LYX_INFO:
		return name == "version" || name == "layoutformat";

	case FIXDATE_INFO: {
		string date;
		string piece;
		date = split(name, piece, '@');
		if (!date.empty() && !QDate::fromString(toqstr(date), Qt::ISODate).isValid())
			return false;
		if (!piece.empty())
			name = piece;
	}
	// fall through
	case DATE_INFO:
	case MODDATE_INFO: {
		if (name == "long" || name == "short" || name == "ISO")
			return true;
		else {
			QDate date = QDate::currentDate();
			return !date.toString(toqstr(name)).isEmpty();
		}
	}
	case FIXTIME_INFO: {
		string time;
		string piece;
		time = split(name, piece, '@');
		if (!time.empty() && !QTime::fromString(toqstr(time), Qt::ISODate).isValid())
			return false;
		if (!piece.empty())
			name = piece;
	}
	// fall through
	case TIME_INFO:
	case MODTIME_INFO: {
		if (name == "long" || name == "short" || name == "ISO")
			return true;
		else {
			QTime time = QTime::currentTime();
			return !time.toString(toqstr(name)).isEmpty();
		}
	}
	}

	return false;
}




string InsetInfoParams::infoType() const
{
	return nameTranslator().find(type);
}



/////////////////////////////////////////////////////////////////////////
//
// InsetInfo
//
/////////////////////////////////////////////////////////////////////////



InsetInfo::InsetInfo(Buffer * buf, string const & name)
	: InsetCollapsible(buf), initialized_(false)
{
	params_.type = InsetInfoParams::UNKNOWN_INFO;
	params_.force_ltr = false;
	setInfo(name);
	status_ = Collapsed;
}


Inset * InsetInfo::editXY(Cursor & cur, int x, int y)
{
	// do not allow the cursor to be set in this Inset
	return Inset::editXY(cur, x, y);
}


docstring InsetInfo::layoutName() const
{
	return from_ascii("Info:" + params_.infoType());
}


docstring InsetInfo::toolTip(BufferView const &, int, int) const
{
	docstring result;
	switch (nameTranslator().find(params_.infoType())) {
	case InsetInfoParams::UNKNOWN_INFO:
		result = _("Invalid information inset");
		break;
	case InsetInfoParams::SHORTCUT_INFO:
		result = bformat(_("The keybard shortcut for the function '%1$s'"),
				from_utf8(params_.name));
		break;
	case InsetInfoParams::SHORTCUTS_INFO:
		result = bformat(_("The keybard shortcuts for the function '%1$s'"),
				from_utf8(params_.name));
		break;
	case InsetInfoParams::MENU_INFO: 
		result = bformat(_("The menu location for the function '%1$s'"),
				from_utf8(params_.name));
		break;
	case InsetInfoParams::L7N_INFO: 
		result = bformat(_("The localization for the string '%1$s'"),
				from_utf8(params_.name));
		break;
	case InsetInfoParams::ICON_INFO:
		result = bformat(_("The toolbar icon for the function '%1$s'"),
				from_utf8(params_.name));
		break;
	case InsetInfoParams::LYXRC_INFO:
		result = bformat(_("The preference setting for the preference key '%1$s'"),
				from_utf8(params_.name));
		break;
	case InsetInfoParams::PACKAGE_INFO:
		result = bformat(_("Availability of the LaTeX package '%1$s'"),
				from_utf8(params_.name));
		break;
	case InsetInfoParams::TEXTCLASS_INFO:
		result = bformat(_("Availability of the LaTeX class '%1$s'"),
				from_utf8(params_.name));
		break;
	case InsetInfoParams::BUFFER_INFO:
		if (params_.name == "name")
			result = _("The name of this file (incl. extension)");
		else if (params_.name == "name-noext")
			result = _("The name of this file (without extension)");
		else if (params_.name == "path")
			result = _("The path where this file is saved");
		else if (params_.name == "class")
			result = _("The class this document uses");
		break;
	case InsetInfoParams::VCS_INFO:
		if (params_.name == "revision")
			result = _("Version control revision");
		else if (params_.name == "revision-abbrev")
			result = _("Version control abbreviated revision");
		else if (params_.name == "tree-revision")
			result = _("Version control tree revision");
		else if (params_.name == "author")
			 result = _("Version control author");
		else if (params_.name == "date")
			result = _("Version control date");
		else if (params_.name == "time")
			result = _("Version control time");
		break;
	case InsetInfoParams::LYX_INFO:
		if (params_.name == "version")
			result = _("The current LyX version");
		else if (params_.name == "layoutformat")
			result = _("The current LyX layout format");
		break;
	case InsetInfoParams::DATE_INFO:
		result = _("The current date");
		break;
	case InsetInfoParams::MODDATE_INFO:
		result = _("The date of last save");
		break;
	case InsetInfoParams::FIXDATE_INFO:
		result = _("A static date");
		break;
	case InsetInfoParams::TIME_INFO:
		result = _("The current time");
		break;
	case InsetInfoParams::MODTIME_INFO:
		result = _("The time of last save");
		break;
	case InsetInfoParams::FIXTIME_INFO:
		result = _("A static time");
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
			params_.type = nameTranslator().find(token);
		} else if (token == "arg") {
			lex.next(true);
			params_.name = lex.getString();
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
	os << "Info\ntype  \"" << params_.infoType()
	   << "\"\narg   " << Lexer::quoteString(params_.name);
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
	case LFUN_INSET_DISSOLVE:
		flag.setEnabled(true);
		return true;

	case LFUN_INSET_MODIFY:
		if (params_.validateArgument(&buffer(), cmd.argument())) {
			flag.setEnabled(true);
			string typestr;
			string name = trim(split(to_utf8(cmd.argument()), typestr, ' '));
			InsetInfoParams::info_type type = nameTranslator().find(typestr);
			string origname = params_.name;
			if (type == InsetInfoParams::FIXDATE_INFO
			    || type == InsetInfoParams::FIXTIME_INFO)
				split(params_.name, origname, '@');
			flag.setOnOff(type == params_.type && name == origname);
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

	string saved_date_specifier;
	// Store old date specifier for potential re-use
	if (!params_.name.empty())
		saved_date_specifier = split(params_.name, '@');
	// info_type name
	string type;
	params_.name = trim(split(name, type, ' '));
	params_.type = nameTranslator().find(type);
	if (params_.name.empty())
		params_.name = defaultValueTranslator().find(params_.type);
	if (params_.type == InsetInfoParams::FIXDATE_INFO) {
		string const date_specifier = split(params_.name, '@');
		// If an explicit new fix date is specified, use that
		// Otherwise, use the old one or, if there is none,
		// the current date
		if (date_specifier.empty()) {
			if (saved_date_specifier.empty())
				params_.name += "@" + fromqstr(QDate::currentDate().toString(Qt::ISODate));
			else
				params_.name += "@" + saved_date_specifier;
		}
	}
	else if (params_.type == InsetInfoParams::FIXTIME_INFO) {
		string const time_specifier = split(params_.name, '@');
		// If an explicit new fix time is specified, use that
		// Otherwise, use the old one or, if there is none,
		// the current time
		if (time_specifier.empty()) {
			if (saved_date_specifier.empty())
				params_.name += "@" + fromqstr(QTime::currentTime().toString(Qt::ISODate));
			else
				params_.name += "@" + saved_date_specifier;
		}
	}
}


void InsetInfo::error(docstring const & err, Language const * lang)
{
	docstring const res = translateIfPossible(err, lang->code());
	bool const translated = res != err;
	// If the string is not translated, we use default lang (English)
	Font const f = translated ? Font(inherit_font, lang) : Font(inherit_font);
	setText(bformat(res, from_utf8(params_.name)), f, false);
}


void InsetInfo::info(docstring const & err, Language const * lang)
{
	docstring const res = translateIfPossible(err, lang->code());
	bool const translated = res != err;
	// If the string is not translated, we use default lang (English)
	Font const f = translated ? Font(inherit_font, lang) : Font(inherit_font);
	setText(translateIfPossible(err, lang->code()), f, false);
}


void InsetInfo::setText(docstring const & str, Language const * lang)
{
	setText(str, Font(inherit_font, lang), false);
}


bool InsetInfo::forceLTR(OutputParams const &) const
{
	return params_.force_ltr;
}


bool InsetInfo::forceLocalFontSwitch() const
{
	return params_.type == InsetInfoParams::MENU_INFO
		|| params_.type == InsetInfoParams::SHORTCUTS_INFO
		|| params_.type == InsetInfoParams::L7N_INFO;
}


void InsetInfo::updateBuffer(ParIterator const & it, UpdateType utype, bool const deleted) {
	// If the Buffer is a clone, then we neither need nor want to do any
	// of what follows. We want, rather, just to inherit how things were
	// in the original Buffer. This is especially important for VCS.
	// Otherwise, we could in principle have different settings here
	// than in the Buffer we were exporting.
	if (buffer().isClone())
		return;

	BufferParams const & bp = buffer().params();
	params_.lang = it.paragraph().getFontSettings(bp, it.pos()).language();
	Language const * tryguilang = languages.getFromCode(Messages::guiLanguage());
	// Some info insets use the language of the GUI (if available)
	Language const * guilang = tryguilang ? tryguilang : params_.lang;

	params_.force_ltr = !params_.lang->rightToLeft();
	// This is just to get the string into the po files
	docstring gui;
	switch (params_.type) {
	case InsetInfoParams::UNKNOWN_INFO:
		gui = _("Unknown Info!");
		info(from_ascii("Unknown Info!"), params_.lang);
		initialized_ = false;
		break;
	case InsetInfoParams::SHORTCUT_INFO:
	case InsetInfoParams::SHORTCUTS_INFO: {
		// shortcuts can change, so we need to re-do this each time
		FuncRequest const func = lyxaction.lookupFunc(params_.name);
		if (func.action() == LFUN_UNKNOWN_ACTION) {
			gui = _("Unknown action %1$s");
			error(from_ascii("Unknown action %1$s"), params_.lang);
			break;
		}
		KeyMap::Bindings bindings = theTopLevelKeymap().findBindings(func);
		if (bindings.empty()) {
			gui = _("undefined");
			info(from_ascii("undefined"), params_.lang);
			break;
		}
		docstring sequence;
		docstring seq_untranslated;
		if (params_.type == InsetInfoParams::SHORTCUT_INFO) {
			sequence = bindings.begin()->print(KeySequence::ForGui);
			seq_untranslated = bindings.begin()->print(KeySequence::ForGui, true);
		} else {
			sequence = theTopLevelKeymap().printBindings(func, KeySequence::ForGui);
			seq_untranslated = theTopLevelKeymap().printBindings(func, KeySequence::ForGui, true);
		}
		// QKeySequence returns special characters for keys on the mac
		// Since these are not included in many fonts, we
		// re-translate them to textual names (see #10641)
		odocstringstream ods;
		string const lcode = params_.lang->code();
		docstring trans;
		bool is_translated = sequence != seq_untranslated;
		for (size_t n = 0; n < sequence.size(); ++n) {
			char_type const c = sequence[n];
			switch(c) {
			case 0x21b5://Return
				gui = _("Return[[Key]]");
				is_translated = translateString(from_ascii("Return[[Key]]"), trans, lcode);
				ods << trans;
				break;
			case 0x21b9://Tab both directions (Win)
				gui = _("Tab[[Key]]");
				is_translated = translateString(from_ascii("Tab[[Key]]"), trans, lcode);
				ods << trans;
				break;
			case 0x21de://Qt::Key_PageUp
				gui = _("PgUp");
				is_translated = translateString(from_ascii("PgUp"), trans, lcode);
				ods << trans;
				break;
			case 0x21df://Qt::Key_PageDown
				gui = _("PgDown");
				is_translated = translateString(from_ascii("PgDown"), trans, lcode);
				ods << trans;
				break;
			case 0x21e4://Qt::Key_Backtab
				gui = _("Backtab");
				is_translated = translateString(from_ascii("Backtab"), trans, lcode);
				ods << trans;
				break;
			case 0x21e5://Qt::Key_Tab
				gui = _("Tab");
				is_translated = translateString(from_ascii("Tab"), trans, lcode);
				ods << trans;
				break;
			case 0x21e7://Shift
				gui = _("Shift");
				is_translated = translateString(from_ascii("Shift"), trans, lcode);
				ods << trans;
				break;
			case 0x21ea://Qt::Key_CapsLock
				gui = _("CapsLock");
				is_translated = translateString(from_ascii("CapsLock"), trans, lcode);
				ods << trans;
				break;
			case 0x2303://Control
				gui = _("Control[[Key]]");
				is_translated = translateString(from_ascii("Control[[Key]]"), trans, lcode);
				ods << trans;
				break;
			case 0x2318://CMD
				gui = _("Command[[Key]]");
				is_translated = translateString(from_ascii("Command[[Key]]"), trans, lcode);
				ods << trans;
				break;
			case 0x2324://Qt::Key_Enter
				gui = _("Return[[Key]]");
				is_translated = translateString(from_ascii("Return[[Key]]"), trans, lcode);
				ods << trans;
				break;
			case 0x2325://Option key
				gui = _("Option[[Key]]");
				is_translated = translateString(from_ascii("Option[[Key]]"), trans, lcode);
				ods << trans;
				break;
			case 0x2326://Qt::Key_Delete
				gui = _("Delete[[Key]]");
				is_translated = translateString(from_ascii("Delete[[Key]]"), trans, lcode);
				ods << trans;
				break;
			case 0x232b://Qt::Key_Backspace
				gui = _("Fn+Del");
				is_translated = translateString(from_ascii("Fn+Del"), trans, lcode);
				ods << trans;
				break;
			case 0x238b://Qt::Key_Escape
				gui = _("Esc");
				is_translated = translateString(from_ascii("Esc"), trans, lcode);
				ods << trans;
				break;
			default:
				ods.put(c);
			}
		}
		setText(ods.str(), is_translated ? guilang : nullptr);
		params_.force_ltr = !is_translated || (!guilang->rightToLeft() && !params_.lang->rightToLeft());
		break;
	}
	case InsetInfoParams::LYXRC_INFO: {
		// this information could change, if the preferences are changed,
		// so we will recalculate each time through.
		ostringstream oss;
		if (params_.name.empty()) {
			gui = _("undefined");
			info(from_ascii("undefined"), params_.lang);
			break;
		}
		// FIXME this uses the serialization mechanism to get the info
		// we want, which i guess works but is a bit strange.
		lyxrc.write(oss, true, params_.name);
		string result = oss.str();
		if (result.size() < 2) {
			gui = _("undefined");
			info(from_ascii("undefined"), params_.lang);
			break;
		}
		string::size_type loc = result.rfind("\n", result.size() - 2);
		loc = loc == string::npos ? 0 : loc + 1;
		if (result.size() < loc + params_.name.size() + 1
			  || result.substr(loc + 1, params_.name.size()) != params_.name) {
			gui = _("undefined");
			info(from_ascii("undefined"), params_.lang);
			break;
		}
		// remove leading comments and \\name and space
		result = result.substr(loc + params_.name.size() + 2);

		// remove \n and ""
		result = rtrim(result, "\n");
		result = trim(result, "\"");
		gui = _("not set");
		if (result.empty())
			result = "not set";
		setText(from_utf8(result), params_.lang);
		break;
	}
	case InsetInfoParams::PACKAGE_INFO:
		// only need to do this once.
		if (initialized_)
			break;
		// check in packages.lst
		if (LaTeXFeatures::isAvailable(params_.name)) {
			gui = _("yes");
			info(from_ascii("yes"), params_.lang);
		} else {
			gui = _("no");
			info(from_ascii("no"), params_.lang);
		}
		initialized_ = true;
		break;

	case InsetInfoParams::TEXTCLASS_INFO: {
		// the TextClass can change
		LayoutFileList const & list = LayoutFileList::get();
		bool available = false;
		// params_.name is the class name
		if (list.haveClass(params_.name))
			available = list[params_.name].isTeXClassAvailable();
		if (available) {
			gui = _("yes");
			info(from_ascii("yes"), params_.lang);
		} else {
			gui = _("no");
			info(from_ascii("no"), params_.lang);
		}
		break;
	}
	case InsetInfoParams::MENU_INFO: {
		// only need to do this once.
		if (initialized_)
			break;
		docstring_list names;
		FuncRequest func = lyxaction.lookupFunc(params_.name);
		if (func.action() == LFUN_UNKNOWN_ACTION) {
			gui = _("Unknown action %1$s");
			error(from_ascii("Unknown action %1$s"), params_.lang);
			break;
		}
		if (func.action() == LFUN_BUFFER_VIEW || func.action() == LFUN_BUFFER_UPDATE)
			// The default output format is in the menu without argument,
			// so strip it here.
			if (func.argument() == from_ascii(buffer().params().getDefaultOutputFormat()))
				func = FuncRequest(func.action());
		// iterate through the menubackend to find it
		if (!theApp()) {
			gui = _("Can't determine menu entry for action %1$s in batch mode");
			error(from_ascii("Can't determine menu entry for action %1$s in batch mode"), params_.lang);
			initialized_ = true;
			break;
		}
		// and we will not keep trying if we fail
		initialized_ = theApp()->hasBufferView();
		if (!theApp()->searchMenu(func, names)) {
			gui = _("No menu entry for action %1$s");
			error(from_ascii("No menu entry for action %1$s"), params_.lang);
			break;
		}
		// if found, return its path.
		clear();
		Paragraph & par = paragraphs().front();
		Font const f(inherit_font, guilang);
		params_.force_ltr = !guilang->rightToLeft();
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
	case InsetInfoParams::L7N_INFO: {
		docstring locstring = _(params_.name);
		// Remove trailing colons
		locstring = rtrim(locstring, ":");
		// Remove menu accelerators
		if (contains(locstring, from_ascii("|"))) {
			docstring nlocstring;
			rsplit(locstring, nlocstring, '|');
			locstring = nlocstring;
		}
		// Remove Qt accelerators, but keep literal ampersands
		locstring = subst(locstring, from_ascii(" & "), from_ascii("</amp;>"));
		locstring = subst(locstring, from_ascii("&"), docstring());
		locstring = subst(locstring, from_ascii("</amp;>"), from_ascii(" & "));
		setText(locstring, guilang);
		params_.force_ltr = !guilang->rightToLeft() && !params_.lang->rightToLeft();
		break;
	}
	case InsetInfoParams::ICON_INFO: {
		// only need to do this once.
		if (initialized_)
			break;
		// and we will not keep trying if we fail
		initialized_ = true;
		FuncRequest func = lyxaction.lookupFunc(params_.name);
		docstring icon_name = frontend::Application::iconName(func, true);
		FileName file(to_utf8(icon_name));
		if (file.onlyFileNameWithoutExt() == "unknown") {
			string dir = "images";
			FileName file2(imageLibFileSearch(dir, params_.name, "svgz,png"));
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
		Font const f(inherit_font, params_.lang);
		paragraphs().front().insertInset(0, inset, f,
						 Change(Change::UNCHANGED));
		break;
	}
	case InsetInfoParams::BUFFER_INFO: {
		// this could all change, so we will recalculate each time
		if (params_.name == "name")
			setText(from_utf8(buffer().fileName().onlyFileName()), params_.lang);
		else if (params_.name == "name-noext")
			setText(from_utf8(buffer().fileName().onlyFileNameWithoutExt()), params_.lang);
		else if (params_.name == "path")
			setText(from_utf8(os::latex_path(buffer().filePath())), params_.lang);
		else if (params_.name == "class")
			setText(from_utf8(bp.documentClass().name()), params_.lang);
		break;
	}
	case InsetInfoParams::VCS_INFO: {
		// this information could change, in principle, so we will 
		// recalculate each time through
		if (!buffer().lyxvc().inUse()) {
			gui = _("No version control!");
			info(from_ascii("No version control!"), params_.lang);
			break;
		}
		LyXVC::RevisionInfo itype = LyXVC::Unknown;
		if (params_.name == "revision")
			itype = LyXVC::File;
		else if (params_.name == "revision-abbrev")
			itype = LyXVC::FileAbbrev;
		else if (params_.name == "tree-revision")
			itype = LyXVC::Tree;
		else if (params_.name == "author")
			itype = LyXVC::Author;
		else if (params_.name == "time")
			itype = LyXVC::Time;
		else if (params_.name == "date")
			itype = LyXVC::Date;
		string binfo = buffer().lyxvc().revisionInfo(itype);
		if (binfo.empty()) {
			gui = _("%1$s[[vcs data]] unknown");
			error(from_ascii("%1$s[[vcs data]] unknown"), params_.lang);
		} else
			setText(from_utf8(binfo), params_.lang);
		break;
	}
	case InsetInfoParams::LYX_INFO:
		// only need to do this once.
		if (initialized_)
			break;
		if (params_.name == "version")
			setText(from_ascii(lyx_version), params_.lang);
		else if (params_.name == "layoutformat")
			setText(convert<docstring>(LAYOUT_FORMAT), params_.lang);
		initialized_ = true;
		break;
	case InsetInfoParams::DATE_INFO:
	case InsetInfoParams::MODDATE_INFO:
	case InsetInfoParams::FIXDATE_INFO: {
		string date_format = params_.name;
		string const date_specifier = (params_.type == InsetInfoParams::FIXDATE_INFO
					       && contains(params_.name, '@'))
				? split(params_.name, date_format, '@') : string();
		QDate date;
		if (params_.type == InsetInfoParams::MODDATE_INFO)
			date = QDateTime::fromTime_t(buffer().fileName().lastModified()).date();
		else if (params_.type == InsetInfoParams::FIXDATE_INFO && !date_specifier.empty())
			date = QDate::fromString(toqstr(date_specifier), Qt::ISODate);
		else
			date = QDate::currentDate();
		setText(params_.getDate(date_format, date), params_.lang);
		break;
	}
	case InsetInfoParams::TIME_INFO:
	case InsetInfoParams::MODTIME_INFO:
	case InsetInfoParams::FIXTIME_INFO: {
		string time_format = params_.name;
		string const time_specifier = (params_.type == InsetInfoParams::FIXTIME_INFO
					       && contains(params_.name, '@'))
				? split(params_.name, time_format, '@') : string();
		QTime time;
		if (params_.type == InsetInfoParams::MODTIME_INFO)
			time = QDateTime::fromTime_t(buffer().fileName().lastModified()).time();
		else if (params_.type == InsetInfoParams::FIXTIME_INFO && !time_specifier.empty())
			time = QTime::fromString(toqstr(time_specifier), Qt::ISODate);
		else
			time = QTime::currentTime();
		setText(params_.getTime(time_format, time), params_.lang);
		break;
	}
	}

	// Just to do something with that string
	LYXERR(Debug::INFO, "info inset text: " << gui);
	InsetCollapsible::updateBuffer(it, utype, deleted);
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
