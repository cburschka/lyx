/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include <algorithm>
#include "MenuBackend.h"
#include "lyxlex.h"
#include "LyXAction.h"
#include "debug.h"
#include "gettext.h"
#include "lastfiles.h"
#include "lyx_main.h" // for lastfiles
#include "bufferlist.h"
#include "converter.h"
#include "exporter.h"
#include "importer.h"
#include "FloatList.h"
#include "support/LAssert.h"
#include "support/filetools.h"
#include "support/lyxfunctional.h"
#include "support/lstrings.h"

extern LyXAction lyxaction;
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
	case ViewFormats:
	case UpdateFormats:
	case ExportFormats:
	case ImportFormats:
	case FloatListInsert:
	case FloatInsert:
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


string const MenuItem::label() const
{
	return token(label_, '|', 0);
}


string const MenuItem::shortcut() const
{
	return token(label_, '|', 1);
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
		md_separator,
		md_submenu,
		md_toc,
		md_updateformats,
		md_viewformats,
		md_floatlistinsert,
		md_floatinsert,
		md_last
	};

	struct keyword_item menutags[md_last - 1] = {
		{ "documents", md_documents },
		{ "end", md_endmenu },
		{ "exportformats", md_exportformats },
		{ "floatinsert", md_floatinsert },
		{ "floatlistinsert", md_floatlistinsert },
		{ "importformats", md_importformats },
		{ "item", md_item },
		{ "lastfiles", md_lastfiles },
		{ "optitem", md_optitem },
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

		case md_submenu: {
			lex.next(true);
			string const mlabel = _(lex.getString());
			lex.next(true);
			string const mname = lex.getString();
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


namespace {

class compare_format {
public:
	bool operator()(Format const * p1, Format const * p2) {
		return *p1 < *p2;
	}
};

} // namespace anon


void Menu::expand(Menu & tomenu, Buffer * buf) const
{
	for (const_iterator cit = begin();
	     cit != end() ; ++cit) {
		switch (cit->kind()) {
		case MenuItem::Lastfiles: {
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
				tomenu.add(MenuItem(MenuItem::Command,
						    label, action));
			}
		}
		break;

		case MenuItem::Documents: {
			typedef vector<string> Strings;

			Strings const names = bufferlist.getFileNames();

			if (names.empty()) {
				tomenu.add(MenuItem(MenuItem::Command,
						    _("No Documents Open!"),
						    LFUN_NOACTION));
				break;
			}

			Strings::const_iterator docit = names.begin();
			Strings::const_iterator end = names.end();
			for (; docit != end ; ++docit) {
				int const action = lyxaction
					.getPseudoAction(LFUN_SWITCHBUFFER,
							 *docit);
				string const label =
					MakeDisplayPath(*docit, 30);
				tomenu.add(MenuItem(MenuItem::Command,
						    label, action));
			}
		}
		break;

		case MenuItem::ImportFormats:
		case MenuItem::ViewFormats:
		case MenuItem::UpdateFormats:
		case MenuItem::ExportFormats: {

			if (!buf && cit->kind() != MenuItem::ImportFormats) {
				tomenu.add(MenuItem(MenuItem::Command,
						    _("No Documents Open!"),
						    LFUN_NOACTION));
				continue;
			}				
			
			typedef vector<Format const *> Formats;

			Formats formats;

			kb_action action;
			switch (cit->kind()) {
			case MenuItem::ImportFormats:
				formats = Importer::GetImportableFormats();
				action = LFUN_IMPORT;
				break;
			case MenuItem::ViewFormats:
				formats = Exporter::GetExportableFormats(buf, true);
				action = LFUN_PREVIEW;
				break;
			case MenuItem::UpdateFormats:
				formats = Exporter::GetExportableFormats(buf, true);
				action = LFUN_UPDATE;
				break;
			default:
				formats = Exporter::GetExportableFormats(buf, false);
				action = LFUN_EXPORT;
			}
			sort(formats.begin(), formats.end(), compare_format());

			Formats::const_iterator fit = formats.begin();
			Formats::const_iterator end = formats.end();

			for (; fit != end ; ++fit) {
				if ((*fit)->dummy())
					continue;
				string label = (*fit)->prettyname();
				if (cit->kind() == MenuItem::ImportFormats)
					if ((*fit)->name() == "text")
						label = _("Ascii text as lines");
					else if ((*fit)->name() == "textparagraph")
						label = _("Ascii text as paragraphs");
				if (!(*fit)->shortcut().empty())
					label += "|" + (*fit)->shortcut();
				int const action2 = lyxaction.
					getPseudoAction(action,
							(*fit)->name());
				tomenu.add(MenuItem(MenuItem::Command,
						    label, action2));
			}
		}
		break;

		case MenuItem::FloatListInsert:
		{
			FloatList::const_iterator cit = floatList.begin();
			FloatList::const_iterator end = floatList.end();
			for (; cit != end; ++cit) {
				int const action =  lyxaction
					.getPseudoAction(LFUN_FLOAT_LIST,
							 cit->second.type());
				tomenu.add(MenuItem(MenuItem::Command,
						    _(cit->second.listName()),
						    action));
			}
		}
		break;

		case MenuItem::FloatInsert:
		{
			FloatList::const_iterator cit = floatList.begin();
			FloatList::const_iterator end = floatList.end();
			for (; cit != end; ++cit) {
				// normal float
				int const action = lyxaction
					.getPseudoAction(LFUN_INSET_FLOAT,
							 cit->second.type());
				string const label = _(cit->second.name());
				tomenu.add(MenuItem(MenuItem::Command,
						    label, action));

				// and the wide version
				int const action2 = lyxaction
					.getPseudoAction(LFUN_INSET_WIDE_FLOAT,
							 cit->second.type());
				string const label2 = label + _(" (wide)");
				tomenu.add(MenuItem(MenuItem::Command,
						    label2, action2));
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


bool Menu::hasSubmenu(string const & name) const
{
	return find_if(begin(), end(),
		       lyx::compare_memfun(&MenuItem::submenu, name)) != end();
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
	if (compare_no_case(lex.getString(), "menuset")) {
		lyxerr << "Menubackend::read: ERROR wrong token:`"
		       << lex.getString() << '\'' << endl;
	}

	lex.pushTable(menutags, md_last - 1);
	if (lyxerr.debugging(Debug::PARSER))
		lex.printTable(lyxerr);

	bool quit = false;
	bool menubar = false;

	while (lex.isOK() && !quit) {
		switch (lex.lex()) {
		case md_menubar:
			menubar = true;
			// fallback to md_menu
		case md_menu: {
			lex.next(true);
			string const name = lex.getString();
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
		.add(MenuItem(MenuItem::Command, _("Open...|O"), "file-open"))
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
		       lyx::compare_memfun(&Menu::name, name)) != end();
}


Menu const & MenuBackend::getMenu(string const & name) const
{
	const_iterator cit = find_if(begin(), end(),
				     lyx::compare_memfun(&Menu::name, name));
	lyx::Assert(cit != end());
	return (*cit);
}


Menu & MenuBackend::getMenu(string const & name)
{
	MenuList::iterator it =
		find_if(menulist_.begin(), menulist_.end(),
			lyx::compare_memfun(&Menu::name, name));
	lyx::Assert(it != menulist_.end());
	return (*it);
}
