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
#include "session.h"
#include "LyXAction.h"
#include "lyx_main.h" // for lastfiles
#include "lyxfunc.h"
#include "lyxlex.h"
#include "toc.h"

#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/convert.h"

#include <boost/bind.hpp>

#include <algorithm>

using lyx::char_type;
using lyx::docstring;
using lyx::support::compare_no_case;
using lyx::support::compare_ascii_no_case;
using lyx::support::contains;
using lyx::support::makeDisplayPath;
using lyx::support::token;

using boost::bind;

using std::auto_ptr;
using std::endl;
using std::equal_to;
using std::find_if;
using std::max;
using std::sort;
using std::string;
using std::vector;


extern boost::scoped_ptr<kb_keymap> toplevel_keymap;

namespace {

class MenuNamesEqual : public std::unary_function<Menu, bool> {
public:
	MenuNamesEqual(docstring const & name)
		: name_(name) {}
	bool operator()(Menu const & menu) const
	{
		return menu.name() == name_;
	}
private:
	docstring name_;
};

} // namespace anon


// This is the global menu definition
MenuBackend menubackend;


MenuItem::MenuItem(Kind kind)
	: kind_(kind), optional_(false)
{}


MenuItem::MenuItem(Kind kind, docstring const & label,
		   docstring const & submenu, bool optional)
	: kind_(kind), label_(label),
	  submenuname_(submenu), optional_(optional)
{
	BOOST_ASSERT(kind == Submenu);
}


MenuItem::MenuItem(Kind kind, docstring const & label,
		   FuncRequest const & func, bool optional)
	: kind_(kind), label_(label), func_(func), optional_(optional)
{
	func_.origin = FuncRequest::UI;
}


MenuItem::~MenuItem()
{}


void MenuItem::submenu(Menu * menu)
{
	submenu_.reset(menu);
}


docstring const MenuItem::label() const
{
	return token(label_, char_type('|'), 0);
}


docstring const MenuItem::shortcut() const
{
	return token(label_, char_type('|'), 1);
}


docstring const MenuItem::binding() const
{
	if (kind_ != Command)
		return docstring();

	// Get the keys bound to this action, but keep only the
	// first one later
	kb_keymap::Bindings bindings = toplevel_keymap->findbindings(func_);

	if (bindings.size()) {
		return lyx::from_utf8(bindings.begin()->print());
	} else {
		lyxerr[Debug::KBMAP]
			<< "No binding for "
			<< lyxaction.getActionName(func_.action)
			<< '(' << lyx::to_utf8(func_.argument()) << ')' << endl;
		return docstring();
	}

}


Menu & Menu::add(MenuItem const & i)
{
	items_.push_back(i);
	return *this;
}


Menu & Menu::addWithStatusCheck(MenuItem const & i)
{
	switch (i.kind()) {

	case MenuItem::Command: {
		FuncStatus status =
			lyx::getStatus(i.func());
		if (status.unknown()
		    || (!status.enabled() && i.optional()))
			break;
		items_.push_back(i);
		items_.back().status(status);
		break;
	}

	case MenuItem::Submenu: {
		if (i.submenu()) {
			bool enabled = false;
			for (const_iterator cit = i.submenu()->begin();
			     cit != i.submenu()->end(); ++cit) {
				if ((cit->kind() == MenuItem::Command
				     || cit->kind() == MenuItem::Submenu)
				    && cit->status().enabled()) {
					enabled = true;
					break;
				}
			}
			if (enabled || !i.optional()) {
				items_.push_back(i);
				items_.back().status().enabled(enabled);
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
		md_charstyles,
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
		{ "charstyles", md_charstyles },
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
			docstring const name = _(lex.getString());
			lex.next(true);
			string const command = lex.getString();
			FuncRequest func = lyxaction.lookupFunc(command);
			add(MenuItem(MenuItem::Command, name, func, optional));
			optional = false;
			break;
		}

		case md_separator:
			add(MenuItem(MenuItem::Separator));
			break;

		case md_lastfiles:
			add(MenuItem(MenuItem::Lastfiles));
			break;

		case md_charstyles:
			add(MenuItem(MenuItem::CharStyles));
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
			docstring const mlabel = _(lex.getString());
			lex.next(true);
			docstring const mname = lyx::from_utf8(lex.getString());
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


MenuItem const & Menu::operator[](size_type i) const
{
	return items_[i];
}


bool Menu::hasFunc(FuncRequest const & func) const
{
	return find_if(begin(), end(),
		       bind(std::equal_to<FuncRequest>(),
			    bind(&MenuItem::func, _1),
			    func)) != end();
}

void Menu::checkShortcuts() const
{
	// This is a quadratic algorithm, but we do not care because
	// menus are short enough
	for (const_iterator it1 = begin(); it1 != end(); ++it1) {
		docstring shortcut = it1->shortcut();
		if (shortcut.empty())
			continue;
		if (!contains(it1->label(), shortcut))
			lyxerr << "Menu warning: menu entry \""
			       << lyx::to_utf8(it1->label())
			       << "\" does not contain shortcut `"
			       << lyx::to_utf8(shortcut) << "'." << endl;
		for (const_iterator it2 = begin(); it2 != it1 ; ++it2) {
			if (!compare_no_case(it2->shortcut(), shortcut)) {
				lyxerr << "Menu warning: menu entries "
				       << '"' << lyx::to_utf8(it1->fulllabel())
				       << "\" and \"" << lyx::to_utf8(it2->fulllabel())
				       << "\" share the same shortcut."
				       << endl;
			}
		}
	}
}


void MenuBackend::specialMenu(docstring const &name)
{
	if (hasMenu(name))
		specialmenu_ = &getMenu(name);
}


namespace {

class compare_format {
public:
	bool operator()(Format const * p1, Format const * p2) {
		return *p1 < *p2;
	}
};

docstring const limit_string_length(docstring const & str)
{
	docstring::size_type const max_item_length = 45;

	if (str.size() > max_item_length)
		return str.substr(0, max_item_length - 3) + "...";
	else
		return str;
}


void expandLastfiles(Menu & tomenu)
{
	lyx::Session::LastFiles const & lf = LyX::cref().session().lastFiles();
	lyx::Session::LastFiles::const_iterator lfit = lf.begin();

	int ii = 1;

	for (; lfit != lf.end() && ii < 10; ++lfit, ++ii) {
		docstring const label = convert<docstring>(ii) + ". "
			+ makeDisplayPath((*lfit), 30)
			+ char_type('|') + convert<docstring>(ii);
		tomenu.add(MenuItem(MenuItem::Command, label, FuncRequest(LFUN_FILE_OPEN, (*lfit))));
	}
}


void expandDocuments(Menu & tomenu)
{
	typedef vector<string> Strings;
	Strings const names = theBufferList().getFileNames();

	if (names.empty()) {
		tomenu.add(MenuItem(MenuItem::Command, _("No Documents Open!"),
				    FuncRequest(LFUN_NOACTION)));
		return;
	}

	int ii = 1;
	Strings::const_iterator docit = names.begin();
	Strings::const_iterator end = names.end();
	for (; docit != end; ++docit, ++ii) {
		docstring label = makeDisplayPath(*docit, 20);
		if (ii < 10)
			label = convert<docstring>(ii) + ". " + label + char_type('|') + convert<docstring>(ii);
		tomenu.add(MenuItem(MenuItem::Command, label, FuncRequest(LFUN_BUFFER_SWITCH, *docit)));
	}
}


void expandFormats(MenuItem::Kind kind, Menu & tomenu, Buffer const * buf)
{
	if (!buf && kind != MenuItem::ImportFormats) {
		tomenu.add(MenuItem(MenuItem::Command,
				    _("No Documents Open!"),
				    FuncRequest(LFUN_NOACTION)));
		return;
	}

	typedef vector<Format const *> Formats;
	Formats formats;
	kb_action action;

	switch (kind) {
	case MenuItem::ImportFormats:
		formats = Importer::GetImportableFormats();
		action = LFUN_BUFFER_IMPORT;
		break;
	case MenuItem::ViewFormats:
		formats = Exporter::getExportableFormats(*buf, true);
		action = LFUN_BUFFER_VIEW;
		break;
	case MenuItem::UpdateFormats:
		formats = Exporter::getExportableFormats(*buf, true);
		action = LFUN_BUFFER_UPDATE;
		break;
	default:
		formats = Exporter::getExportableFormats(*buf, false);
		action = LFUN_BUFFER_EXPORT;
	}
	sort(formats.begin(), formats.end(), compare_format());

	Formats::const_iterator fit = formats.begin();
	Formats::const_iterator end = formats.end();
	for (; fit != end ; ++fit) {
		if ((*fit)->dummy())
			continue;
		docstring label = lyx::from_utf8((*fit)->prettyname());

		switch (kind) {
		case MenuItem::ImportFormats:
			if ((*fit)->name() == "text")
				label = _("Plain Text as Lines");
			else if ((*fit)->name() == "textparagraph")
				label = _("Plain Text as Paragraphs");
			label += "...";
			break;
		case MenuItem::ViewFormats:
		case MenuItem::ExportFormats:
		case MenuItem::UpdateFormats:
			if (!(*fit)->documentFormat())
				continue;
			break;
		default:
			BOOST_ASSERT(false);
			break;
		}
		if (!(*fit)->shortcut().empty())
			label += char_type('|') + lyx::from_utf8((*fit)->shortcut());

		if (buf)
			tomenu.addWithStatusCheck(MenuItem(MenuItem::Command, label,
				FuncRequest(action, (*fit)->name())));
		else
			tomenu.add(MenuItem(MenuItem::Command, label,
				FuncRequest(action, (*fit)->name())));
	}
}


void expandFloatListInsert(Menu & tomenu, Buffer const * buf)
{
	if (!buf) {
		tomenu.add(MenuItem(MenuItem::Command,
				    _("No Documents Open!"),
				    FuncRequest(LFUN_NOACTION)));
		return;
	}

	FloatList const & floats =
		buf->params().getLyXTextClass().floats();
	FloatList::const_iterator cit = floats.begin();
	FloatList::const_iterator end = floats.end();
	for (; cit != end; ++cit) {
		tomenu.addWithStatusCheck(MenuItem(MenuItem::Command,
				    _(cit->second.listName()),
				    FuncRequest(LFUN_FLOAT_LIST,
						cit->second.type())));
	}
}


void expandFloatInsert(Menu & tomenu, Buffer const * buf)
{
	if (!buf) {
		tomenu.add(MenuItem(MenuItem::Command,
				    _("No Documents Open!"),
				    FuncRequest(LFUN_NOACTION)));
		return;
	}

	FloatList const & floats =
		buf->params().getLyXTextClass().floats();
	FloatList::const_iterator cit = floats.begin();
	FloatList::const_iterator end = floats.end();
	for (; cit != end; ++cit) {
		// normal float
		docstring const label = _(cit->second.name());
		tomenu.addWithStatusCheck(MenuItem(MenuItem::Command, label,
				    FuncRequest(LFUN_FLOAT_INSERT,
						cit->second.type())));
	}
}


void expandCharStyleInsert(Menu & tomenu, Buffer const * buf)
{
	if (!buf) {
		tomenu.add(MenuItem(MenuItem::Command,
				    _("No Documents Open!"),
				    FuncRequest(LFUN_NOACTION)));
		return;
	}
	CharStyles & charstyles =
		buf->params().getLyXTextClass().charstyles();
	CharStyles::iterator cit = charstyles.begin();
	CharStyles::iterator end = charstyles.end();
	for (; cit != end; ++cit) {
		docstring const label = lyx::from_utf8(cit->name);
		tomenu.addWithStatusCheck(MenuItem(MenuItem::Command, label,
				    FuncRequest(LFUN_CHARSTYLE_INSERT,
						cit->name)));
	}
}


Menu::size_type const max_number_of_items = 25;

void expandToc2(Menu & tomenu,
		lyx::toc::Toc const & toc_list,
		lyx::toc::Toc::size_type from,
		lyx::toc::Toc::size_type to, int depth)
{
	int shortcut_count = 0;

	// check whether depth is smaller than the smallest depth in toc.
	int min_depth = 1000;
	for (lyx::toc::Toc::size_type i = from; i < to; ++i)
		min_depth = std::min(min_depth, toc_list[i].depth());
	if (min_depth > depth)
		depth = min_depth;


	if (to - from <= max_number_of_items) {
		for (lyx::toc::Toc::size_type i = from; i < to; ++i) {
			docstring label(4 * max(0, toc_list[i].depth() - depth), char_type(' '));
			label += limit_string_length(toc_list[i].str());
			if (toc_list[i].depth() == depth
			    && shortcut_count < 9) {
				if (label.find(convert<docstring>(shortcut_count + 1)) != docstring::npos)
					label += char_type('|') + convert<docstring>(++shortcut_count);
			}
			tomenu.add(MenuItem(MenuItem::Command, label,
					    FuncRequest(toc_list[i].action())));
		}
	} else {
		lyx::toc::Toc::size_type pos = from;
		while (pos < to) {
			lyx::toc::Toc::size_type new_pos = pos + 1;
			while (new_pos < to &&
			       toc_list[new_pos].depth() > depth)
				++new_pos;

			docstring label(4 * max(0, toc_list[pos].depth() - depth), ' ');
			label += limit_string_length(toc_list[pos].str());
			if (toc_list[pos].depth() == depth &&
			    shortcut_count < 9) {
				if (label.find(convert<docstring>(shortcut_count + 1)) != docstring::npos)
					label += char_type('|') + convert<docstring>(++shortcut_count);
			}
			if (new_pos == pos + 1) {
				tomenu.add(MenuItem(MenuItem::Command,
						    label, FuncRequest(toc_list[pos].action())));
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


void expandToc(Menu & tomenu, Buffer const * buf)
{
	// To make things very cleanly, we would have to pass buf to
	// all MenuItem constructors and to expandToc2. However, we
	// know that all the entries in a TOC will be have status_ ==
	// OK, so we avoid this unnecessary overhead (JMarc)

	if (!buf) {
		tomenu.add(MenuItem(MenuItem::Command,
				    _("No Documents Open!"),
				    FuncRequest(LFUN_NOACTION)));
		return;
	}

	FloatList const & floatlist = buf->params().getLyXTextClass().floats();
	lyx::toc::TocList const & toc_list = lyx::toc::getTocList(*buf);
	lyx::toc::TocList::const_iterator cit = toc_list.begin();
	lyx::toc::TocList::const_iterator end = toc_list.end();
	for (; cit != end; ++cit) {
		// Handle this later
		if (cit->first == "TOC")
			continue;

		// All the rest is for floats
		auto_ptr<Menu> menu(new Menu);
		lyx::toc::Toc::const_iterator ccit = cit->second.begin();
		lyx::toc::Toc::const_iterator eend = cit->second.end();
		for (; ccit != eend; ++ccit) {
			docstring const label = limit_string_length(ccit->str());
			menu->add(MenuItem(MenuItem::Command,
					   label,
					   FuncRequest(ccit->action())));
		}
		string const & floatName = floatlist.getType(cit->first).listName();
		MenuItem item(MenuItem::Submenu, _(floatName));
		item.submenu(menu.release());
		tomenu.add(item);
	}

	// Handle normal TOC
	cit = toc_list.find("TOC");
	if (cit == end) {
		tomenu.addWithStatusCheck(MenuItem(MenuItem::Command,
				    _("No Table of contents"),
				    FuncRequest()));
	} else {
		expandToc2(tomenu, cit->second, 0, cit->second.size(), 0);
	}
}


void expandPasteRecent(Menu & tomenu, Buffer const * buf)
{
	if (!buf)
		return;

	vector<docstring> const sel =
		lyx::cap::availableSelections(*buf);

	vector<docstring>::const_iterator cit = sel.begin();
	vector<docstring>::const_iterator end = sel.end();

	for (unsigned int index = 0; cit != end; ++cit, ++index) {
		tomenu.add(MenuItem(MenuItem::Command, *cit,
				    FuncRequest(LFUN_PASTE, convert<string>(index))));
	}
}


void expandBranches(Menu & tomenu, Buffer const * buf)
{
	if (!buf)
		return;

	BufferParams const & params = buf->getMasterBuffer()->params();

	BranchList::const_iterator cit = params.branchlist().begin();
	BranchList::const_iterator end = params.branchlist().end();

	for (int ii = 1; cit != end; ++cit, ++ii) {
		docstring label = lyx::from_utf8(cit->getBranch());
		if (ii < 10)
			label = convert<docstring>(ii) + ". " + label + char_type('|') + convert<docstring>(ii);
		tomenu.addWithStatusCheck(MenuItem(MenuItem::Command, label,
				    FuncRequest(LFUN_BRANCH_INSERT,
						cit->getBranch())));
	}
}


} // namespace anon


void MenuBackend::expand(Menu const & frommenu, Menu & tomenu,
			 Buffer const * buf) const
{
	if (!tomenu.empty())
		tomenu.clear();

	for (Menu::const_iterator cit = frommenu.begin();
	     cit != frommenu.end() ; ++cit) {
		switch (cit->kind()) {
		case MenuItem::Lastfiles:
			expandLastfiles(tomenu);
			break;

		case MenuItem::Documents:
			expandDocuments(tomenu);
			break;

		case MenuItem::ImportFormats:
		case MenuItem::ViewFormats:
		case MenuItem::UpdateFormats:
		case MenuItem::ExportFormats:
			expandFormats(cit->kind(), tomenu, buf);
			break;

		case MenuItem::CharStyles:
			expandCharStyleInsert(tomenu, buf);
			break;

		case MenuItem::FloatListInsert:
			expandFloatListInsert(tomenu, buf);
			break;

		case MenuItem::FloatInsert:
			expandFloatInsert(tomenu, buf);
			break;

		case MenuItem::PasteRecent:
			expandPasteRecent(tomenu, buf);
			break;

		case MenuItem::Branches:
			expandBranches(tomenu, buf);
			break;

		case MenuItem::Toc:
			expandToc(tomenu, buf);
			break;

		case MenuItem::Submenu: {
			MenuItem item(*cit);
			item.submenu(new Menu(cit->submenuname()));
			expand(getMenu(cit->submenuname()),
			       *item.submenu(), buf);
			tomenu.addWithStatusCheck(item);
		}
		break;

		case MenuItem::Separator:
			tomenu.addWithStatusCheck(*cit);
			break;

		case MenuItem::Command:
			if (!specialmenu_
			    || !specialmenu_->hasFunc(cit->func()))
				tomenu.addWithStatusCheck(*cit);
		}
	}

	// we do not want the menu to end with a separator
	if (!tomenu.empty()
	    && tomenu.items_.back().kind() == MenuItem::Separator)
		tomenu.items_.pop_back();

	// Check whether the shortcuts are unique
	tomenu.checkShortcuts();
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
			docstring const name = lyx::from_utf8(lex.getString());
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


bool MenuBackend::hasMenu(docstring const & name) const
{
	return find_if(begin(), end(), MenuNamesEqual(name)) != end();
}


Menu const & MenuBackend::getMenu(docstring const & name) const
{
	const_iterator cit = find_if(begin(), end(), MenuNamesEqual(name));
	if (cit == end())
		lyxerr << "No submenu named " << lyx::to_utf8(name) << endl;
	BOOST_ASSERT(cit != end());
	return (*cit);
}


Menu & MenuBackend::getMenu(docstring const & name)
{
	iterator it = find_if(begin(), end(), MenuNamesEqual(name));
	if (it == end())
		lyxerr << "No submenu named " << lyx::to_utf8(name) << endl;
	BOOST_ASSERT(it != end());
	return (*it);
}


Menu const & MenuBackend::getMenubar() const
{
	return menubar_;
}
