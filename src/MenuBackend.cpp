/**
 * \file MenuBackend.cpp
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
#include "Buffer.h"
#include "BufferList.h"
#include "BufferParams.h"
#include "CutAndPaste.h"
#include "debug.h"
#include "Exporter.h"
#include "Floating.h"
#include "FloatList.h"
#include "Format.h"
#include "gettext.h"
#include "Importer.h"
#include "KeyMap.h"
#include "Session.h"
#include "LyXAction.h"
#include "LyX.h" // for lastfiles
#include "LyXFunc.h"
#include "Lexer.h"
#include "Paragraph.h"
#include "TocBackend.h"
#include "ToolbarBackend.h"

#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/convert.h"

#include <boost/bind.hpp>

#include <algorithm>


namespace lyx {

using support::compare_ascii_no_case;
using support::contains;
using support::makeDisplayPath;
using support::split;
using support::token;

using boost::bind;

using std::auto_ptr;
using std::endl;
using std::equal_to;
using std::find_if;
using std::max;
using std::sort;
using std::string;
using std::vector;


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
	func_.origin = FuncRequest::MENU;
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


docstring const MenuItem::binding(bool forgui) const
{
	if (kind_ != Command)
		return docstring();

	// Get the keys bound to this action, but keep only the
	// first one later
	KeyMap::Bindings bindings = theTopLevelKeymap().findbindings(func_);

	if (bindings.size()) {
		return bindings.begin()->print(forgui);
	} else {
		LYXERR(Debug::KBMAP)
			<< "No binding for "
			<< lyxaction.getActionName(func_.action)
			<< '(' << to_utf8(func_.argument()) << ')' << endl;
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
		FuncStatus status = lyx::getStatus(i.func());
		if (status.unknown() || (!status.enabled() && i.optional()))
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


Menu & Menu::read(Lexer & lex)
{
	enum Menutags {
		md_item = 1,
		md_branches,
		md_documents,
		md_bookmarks,
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
		md_toolbars,
		md_last
	};

	struct keyword_item menutags[md_last - 1] = {
		{ "bookmarks", md_bookmarks },
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
		{ "toolbars", md_toolbars },
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
			docstring const name = translateIfPossible(lex.getDocString());
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

		case md_bookmarks:
			add(MenuItem(MenuItem::Bookmarks));
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

		case md_toolbars:
			add(MenuItem(MenuItem::Toolbars));
			break;

		case md_branches:
			add(MenuItem(MenuItem::Branches));
			break;

		case md_optsubmenu:
			optional = true;
			// fallback to md_submenu
		case md_submenu: {
			lex.next(true);
			docstring const mlabel = translateIfPossible(lex.getDocString());
			lex.next(true);
			docstring const mname = lex.getDocString();
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
			       << to_utf8(it1->label())
			       << "\" does not contain shortcut `"
			       << to_utf8(shortcut) << "'." << endl;
		for (const_iterator it2 = begin(); it2 != it1 ; ++it2) {
			if (!compare_ascii_no_case(it2->shortcut(), shortcut)) {
				lyxerr << "Menu warning: menu entries "
				       << '"' << to_utf8(it1->fulllabel())
				       << "\" and \"" << to_utf8(it2->fulllabel())
				       << "\" share the same shortcut."
				       << endl;
			}
		}
	}
}


void MenuBackend::specialMenu(Menu const & menu)
{
	specialmenu_ = menu;
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
	lyx::LastFilesSection::LastFiles const & lf = LyX::cref().session().lastFiles().lastFiles();
	lyx::LastFilesSection::LastFiles::const_iterator lfit = lf.begin();

	int ii = 1;

	for (; lfit != lf.end() && ii < 10; ++lfit, ++ii) {
		string const file = lfit->absFilename();
		docstring const label = convert<docstring>(ii) + ". "
			+ makeDisplayPath(file, 30)
			+ char_type('|') + convert<docstring>(ii);
		tomenu.add(MenuItem(MenuItem::Command, label, FuncRequest(LFUN_FILE_OPEN, file)));
	}
}


void expandDocuments(Menu & tomenu)
{
	Buffer * first = theBufferList().first();
	if (first) {
		Buffer * b = first;
		int ii = 1;
		
		// We cannot use a for loop as the buffer list cycles.
		do {
			docstring label = makeDisplayPath(b->fileName(), 20);
			if (!b->isClean()) label = label + "*";
			if (ii < 10)
				label = convert<docstring>(ii) + ". " + label + '|' + convert<docstring>(ii);
			tomenu.add(MenuItem(MenuItem::Command, label, FuncRequest(LFUN_BUFFER_SWITCH, b->fileName())));
			
			b = theBufferList().next(b);
			++ii;
		} while (b != first); 
	} else {
		tomenu.add(MenuItem(MenuItem::Command, _("No Documents Open!"),
		           FuncRequest(LFUN_NOACTION)));
		return;
	}
}


void expandBookmarks(Menu & tomenu)
{
	lyx::BookmarksSection const & bm = LyX::cref().session().bookmarks();

	for (size_t i = 1; i <= bm.size(); ++i) {
		if (bm.isValid(i)) {
			docstring const label = convert<docstring>(i) + ". "
				+ makeDisplayPath(bm.bookmark(i).filename.absFilename(), 20)
				+ char_type('|') + convert<docstring>(i);
			tomenu.add(MenuItem(MenuItem::Command, label, FuncRequest(LFUN_BOOKMARK_GOTO,
				convert<docstring>(i))));
		}
	}
}


void expandFormats(MenuItem::Kind kind, Menu & tomenu, Buffer const * buf)
{
	if (!buf && kind != MenuItem::ImportFormats) {
		tomenu.add(MenuItem(MenuItem::Command,
				    _("No Document Open!"),
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
		docstring label = from_utf8((*fit)->prettyname());
		docstring shortcut = from_utf8((*fit)->shortcut());

		if (!shortcut.empty())
			label += char_type('|') + shortcut;
		docstring label_i18n = translateIfPossible(label);
		bool const untranslated = (label == label_i18n);
		shortcut = split(label_i18n, label, '|');
		if (untranslated)
			// this might happen if the shortcut
			// has been redefined
			label = translateIfPossible(label);

		switch (kind) {
		case MenuItem::ImportFormats:
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
		if (!shortcut.empty())
			label += char_type('|') + shortcut;

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
				    _("No Document Open!"),
				    FuncRequest(LFUN_NOACTION)));
		return;
	}

	FloatList const & floats =
		buf->params().getTextClass().floats();
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
				    _("No Document Open!"),
				    FuncRequest(LFUN_NOACTION)));
		return;
	}

	FloatList const & floats =
		buf->params().getTextClass().floats();
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
				    _("No Document Open!"),
				    FuncRequest(LFUN_NOACTION)));
		return;
	}
	CharStyles & charstyles =
		buf->params().getTextClass().charstyles();
	CharStyles::iterator cit = charstyles.begin();
	CharStyles::iterator end = charstyles.end();
	for (; cit != end; ++cit) {
		docstring const label = from_utf8(cit->name);
		tomenu.addWithStatusCheck(MenuItem(MenuItem::Command, label,
				    FuncRequest(LFUN_CHARSTYLE_INSERT,
						label)));
	}
}


Menu::size_type const max_number_of_items = 25;

void expandToc2(Menu & tomenu,
		Toc const & toc_list,
		Toc::size_type from,
		Toc::size_type to, int depth)
{
	int shortcut_count = 0;

	// check whether depth is smaller than the smallest depth in toc.
	int min_depth = 1000;
	for (Toc::size_type i = from; i < to; ++i)
		min_depth = std::min(min_depth, toc_list[i].depth());
	if (min_depth > depth)
		depth = min_depth;


	if (to - from <= max_number_of_items) {
		for (Toc::size_type i = from; i < to; ++i) {
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
		Toc::size_type pos = from;
		while (pos < to) {
			Toc::size_type new_pos = pos + 1;
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
				    _("No Document Open!"),
				    FuncRequest(LFUN_NOACTION)));
		return;
	}

	Buffer* cbuf = const_cast<Buffer*>(buf);
	cbuf->tocBackend().update();
	cbuf->structureChanged();

	// Add an entry for the master doc if this is a child doc
	Buffer const * const master = buf->getMasterBuffer();
	if (buf != master) {
		ParIterator const pit = par_iterator_begin(master->inset());
		string const arg = convert<string>(pit->id());
		FuncRequest f(LFUN_PARAGRAPH_GOTO, arg);
		tomenu.add(MenuItem(MenuItem::Command, _("Master Document"), f));
	}

	FloatList const & floatlist = buf->params().getTextClass().floats();
	TocList const & toc_list = buf->tocBackend().tocs();
	TocList::const_iterator cit = toc_list.begin();
	TocList::const_iterator end = toc_list.end();
	for (; cit != end; ++cit) {
		// Handle this later
		if (cit->first == "tableofcontents")
			continue;

		// All the rest is for floats
		auto_ptr<Menu> menu(new Menu);
		TocIterator ccit = cit->second.begin();
		TocIterator eend = cit->second.end();
		for (; ccit != eend; ++ccit) {
			docstring const label = limit_string_length(ccit->str());
			menu->add(MenuItem(MenuItem::Command,
					   label,
					   FuncRequest(ccit->action())));
		}
		string const & floatName = floatlist.getType(cit->first).listName();
		docstring label;
		if (!floatName.empty())
			label = _(floatName);
		// BUG3633: listings is not a proper float so its name
		// is not shown in floatlist.
		else if (cit->first == "listing")
			label = _("List of listings");
		// this should not happen now, but if something else like
		// listings is added later, this can avoid an empty menu name.
		else
			label = _("Other floats");
		MenuItem item(MenuItem::Submenu, label);
		item.submenu(menu.release());
		tomenu.add(item);
	}

	// Handle normal TOC
	cit = toc_list.find("tableofcontents");
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
		cap::availableSelections(*buf);

	vector<docstring>::const_iterator cit = sel.begin();
	vector<docstring>::const_iterator end = sel.end();

	for (unsigned int index = 0; cit != end; ++cit, ++index) {
		tomenu.add(MenuItem(MenuItem::Command, *cit,
				    FuncRequest(LFUN_PASTE, convert<string>(index))));
	}
}


void expandToolbars(Menu & tomenu)
{
	//
	// extracts the toolbars from the backend
	ToolbarBackend::Toolbars::const_iterator cit = toolbarbackend.begin();
	ToolbarBackend::Toolbars::const_iterator end = toolbarbackend.end();

	for (; cit != end; ++cit) {
		docstring label = _(cit->gui_name);
		// frontends are not supposed to turn on/off toolbars,
		// if they cannot update ToolbarBackend::flags. That
		// is to say, ToolbarsBackend::flags should reflect
		// the true state of toolbars.
		//
		// menu is displayed as
		//       on/off review
		// and
		//              review (auto)
		// in the case of auto.
		if (cit->flags & ToolbarInfo::AUTO)
			label += _(" (auto)");
		tomenu.add(MenuItem(MenuItem::Command, label,
				    FuncRequest(LFUN_TOOLBAR_TOGGLE, cit->name + " allowauto")));
	}
}


void expandBranches(Menu & tomenu, Buffer const * buf)
{
	if (!buf) {
		tomenu.add(MenuItem(MenuItem::Command,
				    _("No Document Open!"),
				    FuncRequest(LFUN_NOACTION)));
		return;
	}

	BufferParams const & params = buf->getMasterBuffer()->params();
	if (params.branchlist().empty()) {
		tomenu.add(MenuItem(MenuItem::Command,
				    _("No Branch in Document!"),
				    FuncRequest(LFUN_NOACTION)));
		return;
	}

	BranchList::const_iterator cit = params.branchlist().begin();
	BranchList::const_iterator end = params.branchlist().end();

	for (int ii = 1; cit != end; ++cit, ++ii) {
		docstring label = cit->getBranch();
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

		case MenuItem::Bookmarks:
			expandBookmarks(tomenu);
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

		case MenuItem::Toolbars:
			expandToolbars(tomenu);
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
			if (!specialmenu_.hasFunc(cit->func()))
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


void MenuBackend::read(Lexer & lex)
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
			docstring const name = lex.getDocString();
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
		lyxerr << "No submenu named " << to_utf8(name) << endl;
	BOOST_ASSERT(cit != end());
	return (*cit);
}


Menu & MenuBackend::getMenu(docstring const & name)
{
	iterator it = find_if(begin(), end(), MenuNamesEqual(name));
	if (it == end())
		lyxerr << "No submenu named " << to_utf8(name) << endl;
	BOOST_ASSERT(it != end());
	return (*it);
}


Menu const & MenuBackend::getMenubar() const
{
	return menubar_;
}


} // namespace lyx
