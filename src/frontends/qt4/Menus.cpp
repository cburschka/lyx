/**
 * \file qt4/Menus.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
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

#include "Menus.h"

#include "Action.h"
#include "GuiApplication.h"
#include "GuiPopupMenu.h"
#include "GuiView.h"

#include "BranchList.h"
#include "Buffer.h"
#include "BufferList.h"
#include "BufferParams.h"
#include "Converter.h"
#include "CutAndPaste.h"
#include "Floating.h"
#include "FloatList.h"
#include "Format.h"
#include "KeyMap.h"
#include "Session.h"
#include "LyXAction.h"
#include "LyX.h" // for lastfiles
#include "LyXFunc.h"
#include "Lexer.h"
#include "Paragraph.h"
#include "TextClass.h"
#include "TocBackend.h"
#include "ToolbarBackend.h"

#include "frontends/Application.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"

#include <boost/bind.hpp>

#include "qt_helpers.h"

#include <QCursor>
#include <QMenuBar>

#include <algorithm>
#include <ostream>

using namespace std;
using boost::bind;
using namespace lyx::support;


namespace lyx {
namespace frontend {

namespace {

class MenuNamesEqual
{
public:
	MenuNamesEqual(QString const & name) : name_(name) {}
	bool operator()(Menu const & menu) const { return menu.name() == name_; }
private:
	QString name_;
};

} // namespace anon

// MacOSX specific stuff is at the end.

void Menus::fillMenuBar(GuiView * view)
{
	// Clear all menubar contents before filling it.
	view->menuBar()->clear();
	
#ifdef Q_WS_MACX
	// setup special mac specific menu item
	macxMenuBarInit(view);
#endif

	LYXERR(Debug::GUI, "populating menu bar" << fromqstr(getMenubar().name()));

	if (getMenubar().size() == 0) {
		LYXERR(Debug::GUI, "\tERROR: empty menu bar"
			<< fromqstr(getMenubar().name()));
		return;
	}
	else {
		LYXERR(Debug::GUI, "menu bar entries "
			<< getMenubar().size());
	}

	Menu menu;
	expand(getMenubar(), menu, view->buffer());

	Menu::const_iterator m = menu.begin();
	Menu::const_iterator end = menu.end();

	for (; m != end; ++m) {

		if (m->kind() != MenuItem::Submenu) {
			LYXERR(Debug::GUI, "\tERROR: not a submenu " << fromqstr(m->label()));
			continue;
		}

		LYXERR(Debug::GUI, "menu bar item " << fromqstr(m->label())
			<< " is a submenu named " << fromqstr(m->submenuname()));

		QString name = m->submenuname();
		if (!hasMenu(name)) {
			LYXERR(Debug::GUI, "\tERROR: " << fromqstr(name)
				<< " submenu has no menu!");
			continue;
		}

		GuiPopupMenu * qMenu = new GuiPopupMenu(view, *m, true);
		view->menuBar()->addMenu(qMenu);

		name_map_[name] = qMenu;
	}
}


QMenu * Menus::menu(QString const & name)
{
	LYXERR(Debug::GUI, "Context menu requested: " << fromqstr(name));
	GuiPopupMenu * menu = name_map_.value(name, 0);
	if (!menu)
		LYXERR0("resquested context menu not found: " << fromqstr(name));
	return menu;
}


/// Some special Qt/Mac support hacks

/*
  Here is what the Qt documentation says about how a menubar is chosen:

     1) If the window has a QMenuBar then it is used. 2) If the window
     is a modal then its menubar is used. If no menubar is specified
     then a default menubar is used (as documented below) 3) If the
     window has no parent then the default menubar is used (as
     documented below).

     The above 3 steps are applied all the way up the parent window
     chain until one of the above are satisifed. If all else fails a
     default menubar will be created, the default menubar on Qt/Mac is
     an empty menubar, however you can create a different default
     menubar by creating a parentless QMenuBar, the first one created
     will thus be designated the default menubar, and will be used
     whenever a default menubar is needed.

  Thus, for Qt/Mac, we add the menus to a free standing menubar, so
  that this menubar will be used also when one of LyX' dialogs has
  focus. (JMarc)
*/

void Menus::macxMenuBarInit(GuiView * view)
{
	// The Mac menubar initialisation must be done only once!
	static bool done = false;
	if (done)
		return;
	done = true;

	/* Since Qt 4.2, the qt/mac menu code has special code for
	   specifying the role of a menu entry. However, it does not
	   work very well with our scheme of creating menus on demand,
	   and therefore we need to put these entries in a special
	   invisible menu. (JMarc)
	*/

	/* The entries of our special mac menu. If we add support for
	 * special entries in Menus, we could imagine something
	 * like
	 *    SpecialItem About " "About LyX" "dialog-show aboutlyx"
	 * and therefore avoid hardcoding. I am not sure it is worth
	 * the hassle, though. (JMarc)
	 */
	struct MacMenuEntry {
		kb_action action;
		char const * arg;
		char const * label;
		QAction::MenuRole role;
	};

	MacMenuEntry entries[] = {
		{LFUN_DIALOG_SHOW, "aboutlyx", "About LyX",
		 QAction::AboutRole},
		{LFUN_DIALOG_SHOW, "prefs", "Preferences",
		 QAction::PreferencesRole},
		{LFUN_RECONFIGURE, "", "Reconfigure",
		 QAction::ApplicationSpecificRole},
		{LFUN_LYX_QUIT, "", "Quit LyX", QAction::QuitRole}
	};
	const size_t num_entries = sizeof(entries) / sizeof(MacMenuEntry);

	// the special menu for Menus.
	Menu special;
	for (size_t i = 0 ; i < num_entries ; ++i) {
		FuncRequest const func(entries[i].action,
				       from_utf8(entries[i].arg));
		special.add(MenuItem(MenuItem::Command, entries[i].label, func));
	}
	specialMenu(special);

	// add the entries to a QMenu that will eventually be empty
	// and therefore invisible.
	QMenu * qMenu = view->menuBar()->addMenu("special");

	// we do not use 'special' because it is a temporary variable,
	// whereas Menus::specialMenu points to a persistent
	// copy.
	Menu::const_iterator cit = specialMenu().begin();
	Menu::const_iterator end = specialMenu().end();
	for (size_t i = 0 ; cit != end ; ++cit, ++i) {
		Action * action = new Action(*view, QIcon(), cit->label(),
					     cit->func(), QString());
		action->setMenuRole(entries[i].role);
		qMenu->addAction(action);

	}
}


MenuItem::MenuItem(Kind kind)
	: kind_(kind), optional_(false)
{}


MenuItem::MenuItem(Kind kind, QString const & label,
		   QString const & submenu, bool optional)
	: kind_(kind), label_(label),
	  submenuname_(submenu), optional_(optional)
{
	BOOST_ASSERT(kind == Submenu);
}


MenuItem::MenuItem(Kind kind, QString const & label,
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


QString MenuItem::label() const
{
	return label_.split('|')[0];
}


QString MenuItem::shortcut() const
{
	return label_.contains('|') ? label_.split('|')[1] : QString();
}


QString MenuItem::binding() const
{
	if (kind_ != Command)
		return QString();

	// Get the keys bound to this action, but keep only the
	// first one later
	KeyMap::Bindings bindings = theTopLevelKeymap().findBindings(func_);

	if (bindings.size())
		return toqstr(bindings.begin()->print(KeySequence::ForGui));

	LYXERR(Debug::KBMAP, "No binding for "
		<< lyxaction.getActionName(func_.action)
		<< '(' << func_.argument() << ')');
	return QString();
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
		if (!items_.empty() && items_.back().kind() != MenuItem::Separator)
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
		md_custom,
		md_elements,
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
		{ "custom", md_custom },
		{ "documents", md_documents },
		{ "elements", md_elements },
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
			add(MenuItem(MenuItem::Command, toqstr(name), func, optional));
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

		case md_custom:
			add(MenuItem(MenuItem::Custom));
			break;

		case md_elements:
			add(MenuItem(MenuItem::Elements));
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
			add(MenuItem(MenuItem::Submenu,
				toqstr(mlabel), toqstr(mname), optional));
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
		       bind(equal_to<FuncRequest>(),
			    bind(&MenuItem::func, _1),
			    func)) != end();
}

void Menu::checkShortcuts() const
{
	// This is a quadratic algorithm, but we do not care because
	// menus are short enough
	for (const_iterator it1 = begin(); it1 != end(); ++it1) {
		QString shortcut = it1->shortcut();
		if (shortcut.isEmpty())
			continue;
		if (!it1->label().contains(shortcut))
			lyxerr << "Menu warning: menu entry \""
			       << fromqstr(it1->label())
			       << "\" does not contain shortcut `"
			       << fromqstr(shortcut) << "'." << endl;
		for (const_iterator it2 = begin(); it2 != it1 ; ++it2) {
			if (!it2->shortcut().compare(shortcut, Qt::CaseInsensitive)) {
				lyxerr << "Menu warning: menu entries "
				       << '"' << fromqstr(it1->fulllabel())
				       << "\" and \"" << fromqstr(it2->fulllabel())
				       << "\" share the same shortcut."
				       << endl;
			}
		}
	}
}


bool Menu::searchMenu(FuncRequest const & func, vector<docstring> & names) const
{
	const_iterator m = begin();
	const_iterator m_end = end();
	for (; m != m_end; ++m) {
		if (m->kind() == MenuItem::Command && m->func() == func) {
			names.push_back(qstring_to_ucs4(m->label()));
			return true;
		}
		if (m->kind() == MenuItem::Submenu) {
			names.push_back(qstring_to_ucs4(m->label()));
			Menu const & submenu = *m->submenu();
			if (submenu.searchMenu(func, names))
				return true;
			names.pop_back();
		}
	}
	return false;
}


void Menus::specialMenu(Menu const & menu)
{
	specialmenu_ = menu;
}


namespace {

bool compareFormat(Format const * p1, Format const * p2)
{
	return *p1 < *p2;
}


QString limitStringLength(docstring const & str)
{
	size_t const max_item_length = 45;

	if (str.size() > max_item_length)
		return toqstr(str.substr(0, max_item_length - 3) + "...");

	return toqstr(str);
}


void expandLastfiles(Menu & tomenu)
{
	LastFilesSection::LastFiles const & lf = LyX::cref().session().lastFiles().lastFiles();
	LastFilesSection::LastFiles::const_iterator lfit = lf.begin();

	int ii = 1;

	for (; lfit != lf.end() && ii < 10; ++lfit, ++ii) {
		string const file = lfit->absFilename();
		QString const label = QString("%1. %2|%3").arg(ii)
			.arg(toqstr(makeDisplayPath(file, 30))).arg(ii);
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
			QString label = toqstr(b->fileName().displayName(20));
			if (!b->isClean())
				label += "*";
			if (ii < 10)
				label = QString::number(ii) + ". " + label + '|' + QString::number(ii);
			tomenu.add(MenuItem(MenuItem::Command, label,
				FuncRequest(LFUN_BUFFER_SWITCH, b->absFileName())));
			
			b = theBufferList().next(b);
			++ii;
		} while (b != first); 
	} else {
		tomenu.add(MenuItem(MenuItem::Command, qt_("No Documents Open!"),
		           FuncRequest(LFUN_NOACTION)));
	}
}


void expandBookmarks(Menu & tomenu)
{
	lyx::BookmarksSection const & bm = LyX::cref().session().bookmarks();

	for (size_t i = 1; i <= bm.size(); ++i) {
		if (bm.isValid(i)) {
			string const file = bm.bookmark(i).filename.absFilename();
			QString const label = QString("%1. %2|%3").arg(i)
				.arg(toqstr(makeDisplayPath(file, 20))).arg(i);
			tomenu.add(MenuItem(MenuItem::Command, label,
				FuncRequest(LFUN_BOOKMARK_GOTO, convert<docstring>(i))));
		}
	}
}


void expandFormats(MenuItem::Kind kind, Menu & tomenu, Buffer const * buf)
{
	if (!buf && kind != MenuItem::ImportFormats) {
		tomenu.add(MenuItem(MenuItem::Command,
				    qt_("No Document Open!"),
				    FuncRequest(LFUN_NOACTION)));
		return;
	}

	typedef vector<Format const *> Formats;
	Formats formats;
	kb_action action;

	switch (kind) {
	case MenuItem::ImportFormats:
		formats = theConverters().importableFormats();
		action = LFUN_BUFFER_IMPORT;
		break;
	case MenuItem::ViewFormats:
		formats = buf->exportableFormats(true);
		action = LFUN_BUFFER_VIEW;
		break;
	case MenuItem::UpdateFormats:
		formats = buf->exportableFormats(true);
		action = LFUN_BUFFER_UPDATE;
		break;
	default:
		formats = buf->exportableFormats(false);
		action = LFUN_BUFFER_EXPORT;
	}
	sort(formats.begin(), formats.end(), &compareFormat);

	Formats::const_iterator fit = formats.begin();
	Formats::const_iterator end = formats.end();
	for (; fit != end ; ++fit) {
		if ((*fit)->dummy())
			continue;
		QString label = toqstr((*fit)->prettyname());
		QString const shortcut = toqstr((*fit)->shortcut());

		switch (kind) {
		case MenuItem::ImportFormats:
			// FIXME: This is a hack, we should rather solve
			// FIXME: bug 2488 instead.
			if ((*fit)->name() == "text")
				label = qt_("Plain Text");
			else if ((*fit)->name() == "textparagraph")
				label = qt_("Plain Text, Join Lines");
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
		// FIXME: if we had proper support for translating the
		// format names defined in configure.py, there would
		// not be a need to check whether the shortcut is
		// correct. If we add it uncondiitonally, it would
		// create useless warnings on bad shortcuts
		if (!shortcut.isEmpty() && label.contains(shortcut))
			label += '|' + shortcut;

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
		tomenu.add(MenuItem(MenuItem::Command, qt_("No Document Open!"),
				    FuncRequest(LFUN_NOACTION)));
		return;
	}

	FloatList const & floats = buf->params().documentClass().floats();
	FloatList::const_iterator cit = floats.begin();
	FloatList::const_iterator end = floats.end();
	for (; cit != end; ++cit) {
		tomenu.addWithStatusCheck(MenuItem(MenuItem::Command,
				    qt_(cit->second.listName()),
				    FuncRequest(LFUN_FLOAT_LIST,
						cit->second.type())));
	}
}


void expandFloatInsert(Menu & tomenu, Buffer const * buf)
{
	if (!buf) {
		tomenu.add(MenuItem(MenuItem::Command, qt_("No Document Open!"),
				    FuncRequest(LFUN_NOACTION)));
		return;
	}

	FloatList const & floats = buf->params().documentClass().floats();
	FloatList::const_iterator cit = floats.begin();
	FloatList::const_iterator end = floats.end();
	for (; cit != end; ++cit) {
		// normal float
		QString const label = qt_(cit->second.name());
		tomenu.addWithStatusCheck(MenuItem(MenuItem::Command, label,
				    FuncRequest(LFUN_FLOAT_INSERT,
						cit->second.type())));
	}
}


void expandFlexInsert(Menu & tomenu, Buffer const * buf, string s)
{
	if (!buf) {
		tomenu.add(MenuItem(MenuItem::Command, qt_("No Document Open!"),
				    FuncRequest(LFUN_NOACTION)));
		return;
	}
	TextClass::InsetLayouts const & insetLayouts =
		buf->params().documentClass().insetLayouts();
	TextClass::InsetLayouts::const_iterator cit = insetLayouts.begin();
	TextClass::InsetLayouts::const_iterator end = insetLayouts.end();
	for (; cit != end; ++cit) {
		docstring const label = cit->first;
		if (cit->second.lyxtype() == s)
			tomenu.addWithStatusCheck(MenuItem(MenuItem::Command, 
				toqstr(label), FuncRequest(LFUN_FLEX_INSERT,
						label)));
	}
}


size_t const max_number_of_items = 25;

void expandToc2(Menu & tomenu, Toc const & toc_list,
		size_t from, size_t to, int depth)
{
	int shortcut_count = 0;

	// check whether depth is smaller than the smallest depth in toc.
	int min_depth = 1000;
	for (size_t i = from; i < to; ++i)
		min_depth = min(min_depth, toc_list[i].depth());
	if (min_depth > depth)
		depth = min_depth;

	if (to - from <= max_number_of_items) {
		for (size_t i = from; i < to; ++i) {
			QString label(4 * max(0, toc_list[i].depth() - depth), ' ');
			label += limitStringLength(toc_list[i].str());
			if (toc_list[i].depth() == depth
			    && shortcut_count < 9) {
				if (label.contains(QString::number(shortcut_count + 1)))
					label += '|' + QString::number(++shortcut_count);
			}
			tomenu.add(MenuItem(MenuItem::Command, label,
					    FuncRequest(toc_list[i].action())));
		}
	} else {
		size_t pos = from;
		while (pos < to) {
			size_t new_pos = pos + 1;
			while (new_pos < to &&
			       toc_list[new_pos].depth() > depth)
				++new_pos;

			QString label(4 * max(0, toc_list[pos].depth() - depth), ' ');
			label += limitStringLength(toc_list[pos].str());
			if (toc_list[pos].depth() == depth &&
			    shortcut_count < 9) {
				if (label.contains(QString::number(shortcut_count + 1)))
					label += '|' + QString::number(++shortcut_count);
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
		tomenu.add(MenuItem(MenuItem::Command, qt_("No Document Open!"),
				    FuncRequest(LFUN_NOACTION)));
		return;
	}

	Buffer* cbuf = const_cast<Buffer*>(buf);
	cbuf->tocBackend().update();
	cbuf->structureChanged();

	// Add an entry for the master doc if this is a child doc
	Buffer const * const master = buf->masterBuffer();
	if (buf != master) {
		ParIterator const pit = par_iterator_begin(master->inset());
		string const arg = convert<string>(pit->id());
		FuncRequest f(LFUN_PARAGRAPH_GOTO, arg);
		tomenu.add(MenuItem(MenuItem::Command, qt_("Master Document"), f));
	}

	FloatList const & floatlist = buf->params().documentClass().floats();
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
			QString const label = limitStringLength(ccit->str());
			menu->add(MenuItem(MenuItem::Command, label,
					   FuncRequest(ccit->action())));
		}
		string const & floatName = floatlist.getType(cit->first).listName();
		QString label;
		if (!floatName.empty())
			label = qt_(floatName);
		// BUG3633: listings is not a proper float so its name
		// is not shown in floatlist.
		else if (cit->first == "equation")
			label = qt_("List of Equations");
		else if (cit->first == "index")
			label = qt_("List of Indexes");
		else if (cit->first == "listing")
			label = qt_("List of Listings");
		else if (cit->first == "marginalnote")
			label = qt_("List of Marginal notes");
		else if (cit->first == "note")
			label = qt_("List of Notes");
		else if (cit->first == "footnote")
			label = qt_("List of Foot notes");
		else if (cit->first == "label")
			label = qt_("Labels and References");
		else if (cit->first == "citation")
			label = qt_("List of Citations");
		// this should not happen now, but if something else like
		// listings is added later, this can avoid an empty menu name.
		else
			label = qt_("Other floats");
		MenuItem item(MenuItem::Submenu, label);
		item.submenu(menu.release());
		tomenu.add(item);
	}

	// Handle normal TOC
	cit = toc_list.find("tableofcontents");
	if (cit == end) {
		tomenu.addWithStatusCheck(MenuItem(MenuItem::Command,
				    qt_("No Table of contents"),
				    FuncRequest()));
	} else {
		expandToc2(tomenu, cit->second, 0, cit->second.size(), 0);
	}
}


void expandPasteRecent(Menu & tomenu)
{
	vector<docstring> const sel = cap::availableSelections();

	vector<docstring>::const_iterator cit = sel.begin();
	vector<docstring>::const_iterator end = sel.end();

	for (unsigned int index = 0; cit != end; ++cit, ++index) {
		tomenu.add(MenuItem(MenuItem::Command, toqstr(*cit),
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
		QString label = qt_(cit->gui_name);
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
			label += qt_(" (auto)");
		tomenu.add(MenuItem(MenuItem::Command, label,
				    FuncRequest(LFUN_TOOLBAR_TOGGLE, cit->name + " allowauto")));
	}
}


void expandBranches(Menu & tomenu, Buffer const * buf)
{
	if (!buf) {
		tomenu.add(MenuItem(MenuItem::Command,
				    qt_("No Document Open!"),
				    FuncRequest(LFUN_NOACTION)));
		return;
	}

	BufferParams const & params = buf->masterBuffer()->params();
	if (params.branchlist().empty()) {
		tomenu.add(MenuItem(MenuItem::Command,
				    qt_("No Branch in Document!"),
				    FuncRequest(LFUN_NOACTION)));
		return;
	}

	BranchList::const_iterator cit = params.branchlist().begin();
	BranchList::const_iterator end = params.branchlist().end();

	for (int ii = 1; cit != end; ++cit, ++ii) {
		docstring label = cit->getBranch();
		if (ii < 10)
			label = convert<docstring>(ii) + ". " + label + char_type('|') + convert<docstring>(ii);
		tomenu.addWithStatusCheck(MenuItem(MenuItem::Command, toqstr(label),
				    FuncRequest(LFUN_BRANCH_INSERT,
						cit->getBranch())));
	}
}


} // namespace anon


void Menus::expand(Menu const & frommenu, Menu & tomenu,
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
			expandFlexInsert(tomenu, buf, "charstyle");
			break;

		case MenuItem::Custom:
			expandFlexInsert(tomenu, buf, "custom");
			break;

		case MenuItem::Elements:
			expandFlexInsert(tomenu, buf, "element");
			break;

		case MenuItem::FloatListInsert:
			expandFloatListInsert(tomenu, buf);
			break;

		case MenuItem::FloatInsert:
			expandFloatInsert(tomenu, buf);
			break;

		case MenuItem::PasteRecent:
			expandPasteRecent(tomenu);
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


void Menus::read(Lexer & lex)
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
			QString const name = toqstr(lex.getDocString());
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


void Menus::add(Menu const & menu)
{
	menulist_.push_back(menu);
}


bool Menus::hasMenu(QString const & name) const
{
	return find_if(begin(), end(), MenuNamesEqual(name)) != end();
}


Menu const & Menus::getMenu(QString const & name) const
{
	const_iterator cit = find_if(begin(), end(), MenuNamesEqual(name));
	if (cit == end())
		lyxerr << "No submenu named " << fromqstr(name) << endl;
	BOOST_ASSERT(cit != end());
	return (*cit);
}


Menu & Menus::getMenu(QString const & name)
{
	iterator it = find_if(begin(), end(), MenuNamesEqual(name));
	if (it == end())
		lyxerr << "No submenu named " << fromqstr(name) << endl;
	BOOST_ASSERT(it != end());
	return (*it);
}


Menu const & Menus::getMenubar() const
{
	return menubar_;
}


} // namespace frontend
} // namespace lyx

#include "Menus_moc.cpp"
