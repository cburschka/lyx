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

extern LyXAction lyxaction;

using std::endl;

// This is the global menu definition
MenuBackend menubackend;


MenuItem::MenuItem(Kind kind, string const & label, 
		   string const & command, bool optional) 
	: kind_(kind), label_(label), optional_(optional)
{
	switch(kind) {
	case Separator:
	case Documents:
	case Lastfiles:
	case Toc:
	case References:
	case ViewFormats:
	case UpdateFormats:
	case ExportFormats:
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

	struct keyword_item menutags[md_last-1] = {
		{ "documents", md_documents },
		{ "end", md_endmenu },
		{ "exportformats", md_exportformats },
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
		switch(lex.lex()) {
		case md_optitem:
			optional = true;
			// fallback to md_item
		case md_item: {
			lex.next();
			char * tmp = strdup(lex.GetString().c_str());
			string name = _(tmp);
			free(tmp);
			lex.next();
			string command = lex.GetString();
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

		case md_submenu: {
			lex.next();
			char * tmp = strdup(lex.GetString().c_str());
			string mlabel = _(tmp);
			free(tmp);
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
		switch(lex.lex()) {
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
		for(const_iterator cit = begin();
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
	const_iterator mend = end();
	for (const_iterator cit = begin(); cit != mend; ++cit) {
		if ((*cit).name() == name)
			return true;
	}
	return false;
}


Menu const & MenuBackend::getMenu(string const & name) const
{
	const_iterator mend = end();
	for (const_iterator cit = begin(); cit != mend; ++cit) {
		if ((*cit).name() == name)
			return (*cit);
	}
	Assert(false); // we actually require the name to exist.
	return menulist_.front();
}


Menu & MenuBackend::getMenu(string const & name)
{
	MenuList::iterator end = menulist_.end();
	for (MenuList::iterator cit = menulist_.begin(); 
	     cit != end; ++cit) {
		if ((*cit).name() == name)
			return (*cit);
	}
	Assert(false); // we actually require the name to exist.
	return menulist_.front();
}

