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
#include "MenuBackend.h"
#include "lyxlex.h"
#include "LyXAction.h"
#include "debug.h"

extern LyXAction lyxaction;

using std::endl;

// This is the global menu definition
MenuBackend menubackend;


MenuItem::MenuItem(Kind kind, string const & label, string const & command) 
	: kind_(kind), label_(label) 
{
	switch(kind) {
	case Separator:
	case Documents:
	case Lastfiles:
		break;
	case Command:
		action_ = lyxaction.LookupFunc(command);

		if (action_ == LFUN_UNKNOWN_ACTION) {
			lyxerr << "MenuItem(): LyX command `"
			       << command << "' does not exist." << endl; 
		}
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


void Menu::read(LyXLex & lex)
{
	enum Menutags {
		md_item = 1,
		md_documents,
		md_endmenu,
		md_lastfiles,
		md_submenu,
		md_separator,
		md_last
	};

	struct keyword_item menutags[md_last-1] = {
		{ "documents", md_documents },
		{ "end", md_endmenu },
		{ "item", md_item },
		{ "lastfiles", md_lastfiles },
		{ "separator", md_separator },
		{ "submenu", md_submenu }
	};

	lex.pushTable(menutags, md_last - 1);
	if (lyxerr.debugging(Debug::PARSER))
		lex.printTable(lyxerr);

	bool quit = false;

	while (lex.IsOK() && !quit) {
		switch(lex.lex()) {
		case md_item: {
			lex.next();
			string name = lex.GetString();
			lex.next();
			string command = lex.GetString();
			add(MenuItem(MenuItem::Command, name, command));
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
		case md_submenu: {
			lex.next();
			string mlabel = lex.GetString();
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

	while (lex.IsOK() && !quit) {
		switch(lex.lex()) {
		case md_menu: {
			lex.next();
			string name = lex.GetString();
			Menu menu(name, false);
			menu.read(lex);
			add(menu);
			break;
		}
		case md_menubar: {
			lex.next();
			string name = lex.GetString();
			Menu menubar(name, true);
			menubar.read(lex);
			add(menubar);
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
		.add(MenuItem(MenuItem::Command, "New...|N", "buffer-new"))
		.add(MenuItem(MenuItem::Command, "Open...|O", "buffer-open"))
		.add(MenuItem(MenuItem::Submenu, "Import|I", "import"))
		.add(MenuItem(MenuItem::Command, "Quit|Q", "lyx-quit"))
		.add(MenuItem(MenuItem::Separator))
		.add(MenuItem(MenuItem::Lastfiles));
	add(file);

	Menu import("import");
	import
		.add(MenuItem(MenuItem::Command,
			      "LaTeX...|L", "buffer-import latex"))
		.add(MenuItem(MenuItem::Command,
			      "LinuxDoc...|L", "buffer-import linuxdoc"));
	add(import);
 
	Menu edit("edit");
	edit
		.add(MenuItem(MenuItem::Command, "Cut", "cut"))
		.add(MenuItem(MenuItem::Command, "Copy", "copy"))
		.add(MenuItem(MenuItem::Command, "Paste", "paste"))
		.add(MenuItem(MenuItem::Command, "Emphasize", "font-emph"));
	add(edit);

	Menu documents("documents");
	documents.add(MenuItem(MenuItem::Documents));
	add(documents);

	Menu main("main", true);
	main
		.add(MenuItem(MenuItem::Submenu, "File|F", "file"))
		.add(MenuItem(MenuItem::Submenu, "Edit|E", "edit"))
		.add(MenuItem(MenuItem::Submenu, 
			      "Documents|D", "documents"));
	add(main);

	Menu main_nobuffer("main_nobuffer", true);
	main_nobuffer.add(MenuItem(MenuItem::Submenu, "File|F", "file"));
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
	for (const_iterator cit = begin(); cit != end(); ++cit) {
		if ((*cit).name() == name)
			return true;
	}
	return false;
}


Menu const & MenuBackend::getMenu(string const & name) const
{
	for (const_iterator cit = begin(); cit != end(); ++cit) {
		if ((*cit).name() == name)
			return (*cit);
	}
	Assert(false); // we actually require the name to exist.
	return menulist_.front();
}
