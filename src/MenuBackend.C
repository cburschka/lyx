/**
 * \file MenuBackend.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author André Pönitz
 * \author Dekel Tsur
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "MenuBackend.h"

#include "BranchList.h"
#include "buffer.h"
#include "bufferlist.h"
#include "bufferparams.h"
#include "CutAndPaste.h"
#include "debug.h"
#include "exporter.h"
#include "Floating.h"
#include "FloatList.h"
#include "format.h"
#include "gettext.h"
#include "importer.h"
#include "kbmap.h"
#include "lastfiles.h"
#include "LyXAction.h"
#include "lyx_main.h" // for lastfiles
#include "lyxfunc.h"
#include "lyxlex.h"
#include "toc.h"

#include "frontends/LyXView.h"

#include "support/filetools.h"
#include "support/lyxfunctional.h"
#include "support/lstrings.h"
#include "support/tostr.h"

#include <algorithm>

extern BufferList bufferlist;
extern boost::scoped_ptr<kb_keymap> toplevel_keymap;

using namespace lyx::support;

using std::endl;
using std::find_if;
using std::max;
using std::sort;

using std::vector;

// This is the global menu definition
MenuBackend menubackend;


MenuItem::MenuItem(Kind kind, string const & label,
		   string const & command, bool optional)
	: kind_(kind), label_(label), optional_(optional)
{
	switch (kind) {
	case Separator:
	case Documents:
	case Lastfiles:
	case Toc:
	case ViewFormats:
	case UpdateFormats:
	case ExportFormats:
	case ImportFormats:
	case FloatListInsert:
	case FloatInsert:
	case PasteRecent:
	case Branches:
		break;
	case Command:
		action_ = lyxaction.LookupFunc(command);

		if (action_ == LFUN_UNKNOWN_ACTION) {
			lyxerr << "MenuItem(): LyX command `"
			       << command << "' does not exist." << endl;
		}
		if (optional_)
			lyxerr[Debug::GUI] << "Optional item "
					   << command << endl;
		break;
	case Submenu:
		submenuname_ = command;
		break;
	}
}


MenuItem::MenuItem(Kind kind, string const & label, int action, bool optional)
	: kind_(kind), label_(label), action_(action), submenuname_(),
	  optional_(optional)
{}


MenuItem::~MenuItem()
{}


void MenuItem::submenu(Menu * menu)
{
	submenu_.reset(menu);
}


string const MenuItem::label() const
{
	return token(label_, '|', 0);
}


string const MenuItem::shortcut() const
{
	return token(label_, '|', 1);
}

string const MenuItem::binding() const
{
	if (kind_ != Command)
		return string();

	// Get the keys bound to this action, but keep only the
	// first one later
	string bindings = toplevel_keymap->findbinding(action_);

	if (!bindings.empty()) {
		return bindings.substr(1, bindings.find(']') - 1);
	} else
		return string();
}


Menu & Menu::add(MenuItem const & i, LyXView const * view)
{
	if (!view) {
		items_.push_back(i);
		return *this;
	}

	switch (i.kind()) {
	case MenuItem::Command:
	{
		FuncStatus status =
			view->getLyXFunc().getStatus(i.action());
		if (status.unknown()
		    || (status.disabled() && i.optional()))
			break;
		items_.push_back(i);
		items_.back().status(status);
		break;
	}
	case MenuItem::Submenu:
	{
		if (i.submenu()) {
			bool disabled = true;
			for (const_iterator cit = i.submenu()->begin();
			     cit != i.submenu()->end(); ++cit) {
				if ((cit->kind() == MenuItem::Command
				     || cit->kind() == MenuItem::Submenu)
				    && !cit->status().disabled()) {
					disabled = false;
					break;
				}
			}
			if (!disabled || !i.optional()) {
				items_.push_back(i);
				items_.back().status().disabled(disabled);
			}
		}
		else
			items_.push_back(i);
		break;
	}
	case MenuItem::Separator:
		if (!items_.empty()
		    && items_.back().kind() != MenuItem::Separator)
			items_.push_back(i);
		break;
	default:
		items_.push_back(i);
	}

	return *this;
}


Menu & Menu::read(LyXLex & lex)
{
	enum Menutags {
		md_item = 1,
		md_branches,
		md_documents,
		md_endmenu,
		md_exportformats,
		md_importformats,
		md_lastfiles,
		md_optitem,
		md_optsubmenu,
		md_separator,
		md_submenu,
		md_toc,
		md_updateformats,
		md_viewformats,
		md_floatlistinsert,
		md_floatinsert,
		md_pasterecent,
		md_last
	};

	struct keyword_item menutags[md_last - 1] = {
		{ "branches", md_branches },
		{ "documents", md_documents },
		{ "end", md_endmenu },
		{ "exportformats", md_exportformats },
		{ "floatinsert", md_floatinsert },
		{ "floatlistinsert", md_floatlistinsert },
		{ "importformats", md_importformats },
		{ "item", md_item },
		{ "lastfiles", md_lastfiles },
		{ "optitem", md_optitem },
		{ "optsubmenu", md_optsubmenu },
		{ "pasterecent", md_pasterecent },
		{ "separator", md_separator },
		{ "submenu", md_submenu },
		{ "toc", md_toc },
		{ "updateformats", md_updateformats },
		{ "viewformats", md_viewformats }
	};

	lex.pushTable(menutags, md_last - 1);
	if (lyxerr.debugging(Debug::PARSER))
		lex.printTable(lyxerr);

	bool quit = false;
	bool optional = false;

	while (lex.isOK() && !quit) {
		switch (lex.lex()) {
		case md_optitem:
			optional = true;
			// fallback to md_item
		case md_item: {
			lex.next(true);
			string const name = _(lex.getString());
			lex.next(true);
			string const command = lex.getString();
			add(MenuItem(MenuItem::Command, name,
				     command, optional));
			optional = false;
			break;
		}

		case md_separator:
			add(MenuItem(MenuItem::Separator));
			break;

		case md_lastfiles:
			add(MenuItem(MenuItem::Lastfiles));
			break;

		case md_documents:
			add(MenuItem(MenuItem::Documents));
			break;

		case md_toc:
			add(MenuItem(MenuItem::Toc));
			break;

		case md_viewformats:
			add(MenuItem(MenuItem::ViewFormats));
			break;

		case md_updateformats:
			add(MenuItem(MenuItem::UpdateFormats));
			break;

		case md_exportformats:
			add(MenuItem(MenuItem::ExportFormats));
			break;

		case md_importformats:
			add(MenuItem(MenuItem::ImportFormats));
			break;

		case md_floatlistinsert:
			add(MenuItem(MenuItem::FloatListInsert));
			break;

		case md_floatinsert:
			add(MenuItem(MenuItem::FloatInsert));
			break;

		case md_pasterecent:
			add(MenuItem(MenuItem::PasteRecent));
			break;

		case md_branches:
			add(MenuItem(MenuItem::Branches));
			break;

		case md_optsubmenu:
			optional = true;
			// fallback to md_submenu
		case md_submenu: {
			lex.next(true);
			string const mlabel = _(lex.getString());
			lex.next(true);
			string const mname = lex.getString();
			add(MenuItem(MenuItem::Submenu, mlabel, mname,
				     optional));
			optional = false;
			break;
		}

		case md_endmenu:
			quit = true;
			break;

		default:
			lex.printError("Menu::read: "
				       "Unknown menu tag: `$$Token'");
			break;
		}
	}
	lex.popTable();
	return *this;
}


void Menu::checkShortcuts() const
{
	// This is a quadratic algorithm, but we do not care because
	// menus are short enough
	for (const_iterator it1 = begin(); it1 != end(); ++it1) {
		string shortcut = it1->shortcut();
		if (shortcut.empty())
			continue;
		if (!contains(it1->label(), shortcut))
			lyxerr << "Menu warning: menu entry \""
			       << it1->label()
			       << "\" does not contain shortcut `"
			       << shortcut << "'." << endl;
		for (const_iterator it2 = begin(); it2 != it1 ; ++it2) {
			if (!compare_ascii_no_case(it2->shortcut(), shortcut)) {
				lyxerr << "Menu warning: menu entries "
				       << '"' << it1->fulllabel()
				       << "\" and \"" << it2->fulllabel()
				       << "\" share the same shortcut."
				       << endl;
			}
		}
	}
}


namespace {

class compare_format {
public:
	bool operator()(Format const * p1, Format const * p2) {
		return *p1 < *p2;
	}
};

string const limit_string_length(string const & str)
{
	string::size_type const max_item_length = 45;

	if (str.size() > max_item_length)
		return str.substr(0, max_item_length - 3) + "...";
	else
		return str;
}


void expandLastfiles(Menu & tomenu, LyXView const * view)
{
	int ii = 1;
	LastFiles::const_iterator lfit = lastfiles->begin();
	LastFiles::const_iterator end = lastfiles->end();

	for (; lfit != end && ii < 10; ++lfit, ++ii) {
		string const label = tostr(ii) + ". "
			+ MakeDisplayPath((*lfit), 30)
			+ '|' + tostr(ii);
		int const action = lyxaction.
			getPseudoAction(LFUN_FILE_OPEN,
					(*lfit));
		tomenu.add(MenuItem(MenuItem::Command, label, action), view);
	}
}

void expandDocuments(Menu & tomenu, LyXView const * view)
{
	typedef vector<string> Strings;
	Strings const names = bufferlist.getFileNames();

	if (names.empty()) {
		tomenu.add(MenuItem(MenuItem::Command, _("No Documents Open!"),
				    LFUN_NOACTION), view);
		return;
	}

	int ii = 1;
	Strings::const_iterator docit = names.begin();
	Strings::const_iterator end = names.end();
	for (; docit != end; ++docit, ++ii) {
		int const action =
			lyxaction.getPseudoAction(LFUN_SWITCHBUFFER, *docit);
		string label = MakeDisplayPath(*docit, 20);
		if (ii < 10)
			label = tostr(ii) + ". " + label + '|' + tostr(ii);
		tomenu.add(MenuItem(MenuItem::Command, label, action), view);
	}
}


void expandFormats(MenuItem::Kind kind, Menu & tomenu, LyXView const * view)
{
	if (!view->buffer() && kind != MenuItem::ImportFormats) {
		tomenu.add(MenuItem(MenuItem::Command,
				    _("No Documents Open!"), LFUN_NOACTION),
				    view);
		return;
	}

	typedef vector<Format const *> Formats;
	Formats formats;
	kb_action action;

	switch (kind) {
	case MenuItem::ImportFormats:
		formats = Importer::GetImportableFormats();
		action = LFUN_IMPORT;
		break;
	case MenuItem::ViewFormats:
		formats = Exporter::GetExportableFormats(*view->buffer(), true);
		action = LFUN_PREVIEW;
		break;
	case MenuItem::UpdateFormats:
		formats = Exporter::GetExportableFormats(*view->buffer(), true);
		action = LFUN_UPDATE;
		break;
	default:
		formats = Exporter::GetExportableFormats(*view->buffer(), false);
		action = LFUN_EXPORT;
	}
	sort(formats.begin(), formats.end(), compare_format());

	Formats::const_iterator fit = formats.begin();
	Formats::const_iterator end = formats.end();
	for (; fit != end ; ++fit) {
		if ((*fit)->dummy())
			continue;
		string label = (*fit)->prettyname();
		// we need to hide the default graphic export formats
		// from the external menu, because we need them only
		// for the internal lyx-view and external latex run
		if (label == "EPS" || label == "XPM" || label == "PNG")
			continue;

		if (kind == MenuItem::ImportFormats) {
			if ((*fit)->name() == "text")
				label = _("ASCII text as lines");
			else if ((*fit)->name() == "textparagraph")
				label = _("ASCII text as paragraphs");
			label += "...";
		}
		if (!(*fit)->shortcut().empty())
			label += '|' + (*fit)->shortcut();
		int const action2 = lyxaction.
			getPseudoAction(action,	(*fit)->name());
		tomenu.add(MenuItem(MenuItem::Command, label, action2),
			   view);
	}
}


void expandFloatListInsert(Menu & tomenu, LyXView const * view)
{
	if (!view->buffer()) {
		tomenu.add(MenuItem(MenuItem::Command,
				    _("No Documents Open!"), LFUN_NOACTION),
			   view);
		return;
	}

	FloatList const & floats =
		view->buffer()->params().getLyXTextClass().floats();
	FloatList::const_iterator cit = floats.begin();
	FloatList::const_iterator end = floats.end();
	for (; cit != end; ++cit) {
		int const action =  lyxaction
			.getPseudoAction(LFUN_FLOAT_LIST, cit->second.type());
		tomenu.add(MenuItem(MenuItem::Command,
				    _(cit->second.listName()), action),
			   view);
	}
}


void expandFloatInsert(Menu & tomenu, LyXView const * view)
{
	if (!view->buffer()) {
		tomenu.add(MenuItem(MenuItem::Command,
				    _("No Documents Open!"), LFUN_NOACTION),
			   view);
		return;
	}

	FloatList const & floats =
		view->buffer()->params().getLyXTextClass().floats();
	FloatList::const_iterator cit = floats.begin();
	FloatList::const_iterator end = floats.end();
	for (; cit != end; ++cit) {
		// normal float
		int const action =
			lyxaction.getPseudoAction(LFUN_INSET_FLOAT,
						  cit->second.type());
		string const label = _(cit->second.name());
		tomenu.add(MenuItem(MenuItem::Command, label, action),
			   view);
	}
}


Menu::size_type const max_number_of_items = 25;

void expandToc2(Menu & tomenu,
		lyx::toc::Toc const & toc_list,
		lyx::toc::Toc::size_type from,
		lyx::toc::Toc::size_type to, int depth)
{
	int shortcut_count = 0;
	if (to - from <= max_number_of_items) {
		for (lyx::toc::Toc::size_type i = from; i < to; ++i) {
			int const action = toc_list[i].action();
			string label(4 * max(0, toc_list[i].depth - depth),' ');
			label += limit_string_length(toc_list[i].str);
			if (toc_list[i].depth == depth
			    && ++shortcut_count <= 9) {
				label += '|' + tostr(shortcut_count);
			}
			tomenu.add(MenuItem(MenuItem::Command, label, action));
		}
	} else {
		lyx::toc::Toc::size_type pos = from;
		while (pos < to) {
			lyx::toc::Toc::size_type new_pos = pos + 1;
			while (new_pos < to &&
			       toc_list[new_pos].depth > depth)
				++new_pos;

			int const action = toc_list[pos].action();
			string label(4 * max(0, toc_list[pos].depth - depth), ' ');
			label += limit_string_length(toc_list[pos].str);
			if (toc_list[pos].depth == depth &&
			    ++shortcut_count <= 9)
				label += '|' + tostr(shortcut_count);

			if (new_pos == pos + 1) {
				tomenu.add(MenuItem(MenuItem::Command,
						    label, action));
			} else {
				MenuItem item(MenuItem::Submenu, label);
				item.submenu(new Menu);
				expandToc2(*item.submenu(),
					   toc_list, pos, new_pos, depth + 1);
				tomenu.add(item);
			}
			pos = new_pos;
		}
	}
}


void expandToc(Menu & tomenu, LyXView const * view)
{
	// To make things very cleanly, we would have to pass view to
	// all MenuItem constructors and to expandToc2. However, we
	// know that all the entries in a TOC will be have status_ ==
	// OK, so we avoid this unnecessary overhead (JMarc)

	if (!view->buffer()) {
		tomenu.add(MenuItem(MenuItem::Command,
				    _("No Documents Open!"), LFUN_NOACTION),
			   view);
		return;
	}

	lyx::toc::TocList toc_list = lyx::toc::getTocList(*view->buffer());
	lyx::toc::TocList::const_iterator cit = toc_list.begin();
	lyx::toc::TocList::const_iterator end = toc_list.end();
	for (; cit != end; ++cit) {
		// Handle this later
		if (cit->first == "TOC")
			continue;

		// All the rest is for floats
		Menu * menu = new Menu;
		lyx::toc::Toc::const_iterator ccit = cit->second.begin();
		lyx::toc::Toc::const_iterator eend = cit->second.end();
		for (; ccit != eend; ++ccit) {
			string const label = limit_string_length(ccit->str);
			menu->add(MenuItem(MenuItem::Command,
					   label, ccit->action()));
		}
		string const & floatName = cit->first;
		// Is the _(...) really needed here? (Lgb)
		MenuItem item(MenuItem::Submenu, _(floatName));
		item.submenu(menu);
		tomenu.add(item);
	}

	// Handle normal TOC
	cit = toc_list.find("TOC");
	if (cit == end) {
		tomenu.add(MenuItem(MenuItem::Command,
				    _("No Table of contents")),
			   view);
	} else {
		expandToc2(tomenu, cit->second, 0, cit->second.size(), 0);
	}
}


void expandPasteRecent(Menu & tomenu, LyXView const * view)
{
	if (!view || !view->buffer())
		return;

	vector<string> const selL =
		CutAndPaste::availableSelections(*view->buffer());

	vector<string>::const_iterator cit = selL.begin();
	vector<string>::const_iterator end = selL.end();

	for (unsigned int index = 0; cit != end; ++cit, ++index) {
		int const action = lyxaction.getPseudoAction(LFUN_PASTE,
							     tostr(index));
		tomenu.add(MenuItem(MenuItem::Command, *cit, action));
	}
}


void expandBranches(Menu & tomenu, LyXView const * view)
{
	if (!view || !view->buffer())
		return;

	BufferParams const & params = view->buffer()->params();

	std::list<Branch>::const_iterator cit = params.branchlist().begin();
	std::list<Branch>::const_iterator end = params.branchlist().end();

	for (int ii = 1; cit != end; ++cit, ++ii) {
		string label = cit->getBranch();
		int const action = lyxaction.
			getPseudoAction(LFUN_INSERT_BRANCH,
					(cit->getBranch()));
		if (ii < 10)
			label = tostr(ii) + ". " + label + "|" + tostr(ii);
		tomenu.add(MenuItem(MenuItem::Command, label, action), view);
	}
}


} // namespace anon


void MenuBackend::expand(Menu const & frommenu, Menu & tomenu,
			 LyXView const * view) const
{
	for (Menu::const_iterator cit = frommenu.begin();
	     cit != frommenu.end() ; ++cit) {
		switch (cit->kind()) {
		case MenuItem::Lastfiles:
			expandLastfiles(tomenu, view);
			break;

		case MenuItem::Documents:
			expandDocuments(tomenu, view);
			break;

		case MenuItem::ImportFormats:
		case MenuItem::ViewFormats:
		case MenuItem::UpdateFormats:
		case MenuItem::ExportFormats:
			expandFormats(cit->kind(), tomenu, view);
			break;

		case MenuItem::FloatListInsert:
			expandFloatListInsert(tomenu, view);
			break;

		case MenuItem::FloatInsert:
			expandFloatInsert(tomenu, view);
			break;

		case MenuItem::PasteRecent:
			expandPasteRecent(tomenu, view);
			break;

		case MenuItem::Branches:
			expandBranches(tomenu, view);
			break;

		case MenuItem::Toc:
			expandToc(tomenu, view);
			break;

		case MenuItem::Submenu: {
			MenuItem item(*cit);
			item.submenu(new Menu(cit->submenuname()));
			expand(getMenu(cit->submenuname()),
			       *item.submenu(), view);
			tomenu.add(item, view);
		}
		break;

		default:
			tomenu.add(*cit, view);
		}
	}

	// we do not want the menu to end with a separator
	if (!tomenu.empty()
	    && tomenu.items_.back().kind() == MenuItem::Separator)
		tomenu.items_.pop_back();

	// Check whether the shortcuts are unique
	tomenu.checkShortcuts();
}


bool Menu::hasSubmenu(string const & name) const
{
	return find_if(begin(), end(),
		       lyx::compare_memfun(&MenuItem::submenuname,
					   name)) != end();
}


void MenuBackend::read(LyXLex & lex)
{
	enum Menutags {
		md_menu = 1,
		md_menubar,
		md_endmenuset,
		md_last
	};

	struct keyword_item menutags[md_last - 1] = {
		{ "end", md_endmenuset },
		{ "menu", md_menu },
		{ "menubar", md_menubar }
	};

	//consistency check
	if (compare_ascii_no_case(lex.getString(), "menuset")) {
		lyxerr << "Menubackend::read: ERROR wrong token:`"
		       << lex.getString() << '\'' << endl;
	}

	lex.pushTable(menutags, md_last - 1);
	if (lyxerr.debugging(Debug::PARSER))
		lex.printTable(lyxerr);

	bool quit = false;

	while (lex.isOK() && !quit) {
		switch (lex.lex()) {
		case md_menubar:
			menubar_.read(lex);
			break;
		case md_menu: {
			lex.next(true);
			string const name = lex.getString();
			if (hasMenu(name)) {
				getMenu(name).read(lex);
			} else {
				Menu menu(name);
				menu.read(lex);
				add(menu);
			}
			break;
		}
		case md_endmenuset:
			quit = true;
			break;
		default:
			lex.printError("menubackend::read: "
				       "Unknown menu tag: `$$Token'");
			break;
		}
	}
	lex.popTable();
}


void MenuBackend::add(Menu const & menu)
{
	menulist_.push_back(menu);
}


bool MenuBackend::hasMenu(string const & name) const
{
	return find_if(begin(), end(),
		       lyx::compare_memfun(&Menu::name, name)) != end();
}


Menu const & MenuBackend::getMenu(string const & name) const
{
	const_iterator cit = find_if(begin(), end(),
				     lyx::compare_memfun(&Menu::name, name));
	if (cit == end())
		lyxerr << "No submenu named " << name << endl;
	BOOST_ASSERT(cit != end());
	return (*cit);
}


Menu & MenuBackend::getMenu(string const & name)
{
	MenuList::iterator it =
		find_if(menulist_.begin(), menulist_.end(),
			lyx::compare_memfun(&Menu::name, name));
	BOOST_ASSERT(it != menulist_.end());
	return (*it);
}


Menu const & MenuBackend::getMenubar() const
{
	return menubar_;
}
