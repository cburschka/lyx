/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 *
 * ====================================================== */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>
#include <memory>
#include "support/LAssert.h"
#include "MenuBackend.h"
#include "lyxlex.h"
#include "LyXAction.h"
#include "debug.h"
#include "gettext.h"
#include "lastfiles.h"
#include "bufferlist.h"
#include "converter.h"
#include "exporter.h"
#include "support/filetools.h"
#include "support/lyxfunctional.h"

extern LyXAction lyxaction;
extern LastFiles * lastfiles; 
extern BufferList bufferlist;

using std::endl;
using std::vector;
using std::pair;
using std::find_if;
using std::sort;

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
	case References:
	case ViewFormats:
	case UpdateFormats:
	case ExportFormats:
	case ImportFormats:
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
		submenu_ = command;
		break;
	}
}


Menu & Menu::add(MenuItem const & i)
{
	items_.push_back(i);
	return *this;
}


Menu & Menu::read(LyXLex & lex)
{
	enum Menutags {
		md_item = 1,
		md_documents,
		md_endmenu,
		md_exportformats,
		md_importformats,
		md_lastfiles,
		md_optitem,
		md_references,
		md_separator,
		md_submenu,
		md_toc,
		md_updateformats,
		md_viewformats,
		md_last
	};

	struct keyword_item menutags[md_last - 1] = {
		{ "documents", md_documents },
		{ "end", md_endmenu },
		{ "exportformats", md_exportformats },
		{ "importformats", md_importformats },
		{ "item", md_item },
		{ "lastfiles", md_lastfiles },
		{ "optitem", md_optitem }, 
		{ "references", md_references },
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

	while (lex.IsOK() && !quit) {
		switch (lex.lex()) {
		case md_optitem:
			optional = true;
			// fallback to md_item
		case md_item: {
			lex.next();
			string name = _(lex.GetString());
			lex.next();
			string const command = lex.GetString();
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

		case md_references:
			add(MenuItem(MenuItem::References));
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

		case md_submenu: {
			lex.next();
			string mlabel = _(lex.GetString());
			lex.next();
			string mname = lex.GetString();
			add(MenuItem(MenuItem::Submenu, mlabel, mname));
			break;
		}

		case md_endmenu:
			quit = true;
			break;

		default:
			lex.printError("menubar::read: "
				       "Unknown menu tag: `$$Token'");
			break;
		}
	}
	lex.popTable();
	return *this;
}

struct compare_formatpair {
	bool operator()(FormatPair const & a, FormatPair const & b) {
		return a.format->prettyname < b.format->prettyname; 
	}
};

void Menu::checkShortcuts() const
{
	// This is a quadratic algorithm, but we do not care because
	// it is used for debugging only.
	for (const_iterator it1 = begin(); it1 != end(); ++it1) {
		string shortcut = it1->shortcut();
		if (shortcut.empty())
			continue;
		if (!contains(it1->label(), shortcut))
			lyxerr << "Menu warning: menu entry \""
			       << it1->label()
			       << "\" does not contain shortcut `"
			       << shortcut << '\'' << endl;
		for (const_iterator it2 = begin(); it2 != it1 ; ++it2) {
			if (!compare_no_case(it2->shortcut(), shortcut)) {
				lyxerr << "Menu warning: menu entries "
				       << '"' << it1->fulllabel()
				       << "\" and \"" << it2->fulllabel()
				       << "\" share the same shortcut."
				       << endl;
			}
		}
	}
}

void Menu::expand(Menu & tomenu, Buffer * buf) const
{
	for (const_iterator cit = begin();
	     cit != end() ; ++cit) {
		switch ((*cit).kind()) {
		case MenuItem::Lastfiles: {
			int ii = 1;
			for (LastFiles::const_iterator lfit = lastfiles->begin();
			     lfit != lastfiles->end() && ii < 10;
			     ++lfit, ++ii) {
				string label = tostr(ii) + ". "
					+ MakeDisplayPath((*lfit), 30)
					+ '|' + tostr(ii);
				int action = lyxaction.
					getPseudoAction(LFUN_FILE_OPEN,
							(*lfit));
				tomenu.add(MenuItem(MenuItem::Command,
						    label, action));
			}
		}
		break;
		
		case MenuItem::Documents: {
			vector<string> names = bufferlist.getFileNames();
			
			if (names.empty()) {
				tomenu.add(MenuItem(MenuItem::Command,
						    _("No Documents Open!"),
						    LFUN_NOACTION));
				break;
			}

			for (vector<string>::const_iterator docit = names.begin();
			     docit != names.end() ; ++docit) {
				int action =
					lyxaction.getPseudoAction(LFUN_SWITCHBUFFER, *docit);
				string label = MakeDisplayPath(*docit, 30);
				tomenu.add(MenuItem(MenuItem::Command,
						    label, action));
			}
		}
		break;

		case MenuItem::ViewFormats:
		case MenuItem::UpdateFormats:
		case MenuItem::ExportFormats: {
			vector<FormatPair> names;
			kb_action action;
			if ((*cit).kind() == MenuItem::ViewFormats) {
				names = Exporter::GetExportableFormats(buf, true);
				action = LFUN_PREVIEW;
			} else if ((*cit).kind() == MenuItem::UpdateFormats) {
				names = Exporter::GetExportableFormats(buf, true);
				action = LFUN_UPDATE;
			} else {
				names = Exporter::GetExportableFormats(buf, false);
				action = LFUN_EXPORT;
			}
			sort(names.begin(), names.end(), compare_formatpair());

			for (vector<FormatPair>::const_iterator fit = names.begin();
			     fit != names.end() ; ++fit) {
				if ((*fit).format->dummy())
					continue;
				string fmt = (*fit).format->name;
				string label = (*fit).format->prettyname;
				bool same_before = 
					fit != names.begin() &&
					(*fit).format == (*(fit-1)).format;
				bool same_after = 
					fit+1 != names.end() &&
					(*fit).format == (*(fit+1)).format;
				if ((*fit).from &&
				    (same_before || same_after)) {
					fmt += ":" + (*fit).from->name;
					string head;
					split((*fit).command, head, ' ');
					label += _(" (using ") + head + ")";
					if (!(*fit).format->shortcut.empty() &&
					    !same_before)
						label += "|" + (*fit).format->shortcut;
				} else if (!(*fit).format->shortcut.empty())
					label += "|" + (*fit).format->shortcut;
				int action2 = lyxaction.getPseudoAction(action, fmt);
				tomenu.add(MenuItem(MenuItem::Command,
						    label, action2));
			}
		}
		break;

		case MenuItem::ImportFormats: {
			vector<FormatPair> names = Converter::GetReachableTo("lyx");
			sort(names.begin(), names.end(), compare_formatpair());

			for (vector<FormatPair>::const_iterator fit = names.begin();
			     fit != names.end() ; ++fit) {
				if ((*fit).format->dummy())
					continue;
				string fmt = (*fit).format->name;
				string label = (*fit).format->prettyname;
				if (!(*fit).format->shortcut.empty())
					label += "|" + (*fit).format->shortcut;
				int action2 = lyxaction.getPseudoAction(LFUN_IMPORT, fmt);
				tomenu.add(MenuItem(MenuItem::Command,
						    label, action2));
			}
		}
		break;
			
		default:
			tomenu.add(*cit);
		}
	}

	// Check whether the shortcuts are unique
	if (lyxerr.debugging(Debug::GUI))
		checkShortcuts();
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
	if (compare_no_case(lex.GetString(), "menuset"))
		lyxerr << "Menubackend::read: ERROR wrong token:`"
		       << lex.GetString() << '\'' << endl;

	lex.pushTable(menutags, md_last - 1);
	if (lyxerr.debugging(Debug::PARSER))
		lex.printTable(lyxerr);

	bool quit = false;
	bool menubar = false;

	while (lex.IsOK() && !quit) {
		switch (lex.lex()) {
		case md_menubar: 
			menubar = true;
			// fallback to md_menu
		case md_menu: {
			lex.next();
			string name = lex.GetString();
			if (hasMenu(name)) {
				if (getMenu(name).menubar() == menubar) {
					getMenu(name).read(lex);
				} else {
					lex.printError("Cannot append to menu `$$Token' unless it is of the same type");
					return;
				}
			} else {				
				Menu menu(name, menubar);
				menu.read(lex);
				add(menu);
			}
			menubar = false;
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


void MenuBackend::defaults()
{
	menulist_.clear();

	lyxerr[Debug::GUI] << "MenuBackend::defaults: using default values" 
			   << endl;

	Menu file("file");
	file
		.add(MenuItem(MenuItem::Command, _("New...|N"), "buffer-new"))
		.add(MenuItem(MenuItem::Command, _("Open...|O"), "buffer-open"))
		.add(MenuItem(MenuItem::Submenu, _("Import|I"), "import"))
		.add(MenuItem(MenuItem::Command, _("Quit|Q"), "lyx-quit"))
		.add(MenuItem(MenuItem::Separator))
		.add(MenuItem(MenuItem::Lastfiles));
	add(file);

	Menu import("import");
	import
		.add(MenuItem(MenuItem::Command,
			      _("LaTeX...|L"), "buffer-import latex"))
		.add(MenuItem(MenuItem::Command,
			      _("LinuxDoc...|L"), "buffer-import linuxdoc"));
	add(import);
 
	Menu edit("edit");
	edit
		.add(MenuItem(MenuItem::Command, _("Cut"), "cut"))
		.add(MenuItem(MenuItem::Command, _("Copy"), "copy"))
		.add(MenuItem(MenuItem::Command, _("Paste"), "paste"))
		.add(MenuItem(MenuItem::Command, _("Emphasize"), "font-emph"));
	add(edit);

	Menu documents("documents");
	documents.add(MenuItem(MenuItem::Documents));
	add(documents);

	Menu main("main", true);
	main
		.add(MenuItem(MenuItem::Submenu, _("File|F"), "file"))
		.add(MenuItem(MenuItem::Submenu, _("Edit|E"), "edit"))
		.add(MenuItem(MenuItem::Submenu,
			      _("Documents|D"), "documents"));
	add(main);

	Menu main_nobuffer("main_nobuffer", true);
	main_nobuffer.add(MenuItem(MenuItem::Submenu, _("File|F"), "file"));
	add(main_nobuffer);

	if (lyxerr.debugging(Debug::GUI)) {
		for (const_iterator cit = begin();
		    cit != end() ; ++cit)
			lyxerr << "Menu name: " << cit->name() 
			       << ", Menubar: " << cit->menubar() 
			       << endl;
	}
}


void MenuBackend::add(Menu const & menu)
{
	menulist_.push_back(menu);
}


bool MenuBackend::hasMenu(string const & name) const
{
	return find_if(begin(), end(),
		       compare_memfun(&Menu::name, name)) != end();
}


Menu const & MenuBackend::getMenu(string const & name) const
{
	const_iterator cit = find_if(begin(), end(),
				     compare_memfun(&Menu::name, name));
	Assert(cit != end());
	return (*cit);
}


Menu & MenuBackend::getMenu(string const & name)
{
	MenuList::iterator it = find_if(menulist_.begin(), menulist_.end(),
					compare_memfun(&Menu::name, name));
	Assert(it != menulist_.end());
	return (*it);
}
