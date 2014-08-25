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
#include "GuiView.h"
#include "GuiWorkArea.h"
#include "qt_helpers.h"

#include "BiblioInfo.h"
#include "BranchList.h"
#include "Buffer.h"
#include "BufferList.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Converter.h"
#include "CutAndPaste.h"
#include "Floating.h"
#include "FloatList.h"
#include "Format.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "IndicesList.h"
#include "KeyMap.h"
#include "Language.h"
#include "Layout.h"
#include "Lexer.h"
#include "LyXAction.h"
#include "LyX.h"
#include "LyXRC.h"
#include "lyxfind.h"
#include "Paragraph.h"
#include "ParagraphParameters.h"
#include "ParIterator.h"
#include "Session.h"
#include "SpellChecker.h"
#include "TextClass.h"
#include "Text.h"
#include "TocBackend.h"
#include "Toolbars.h"
#include "WordLangTuple.h"

#include "insets/Inset.h"
#include "insets/InsetCitation.h"
#include "insets/InsetGraphics.h"

#include "support/lassert.h"
#include "support/convert.h"
#include "support/debug.h"
#include "support/docstring_list.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"

#include <QCursor>
#include <QHash>
#include <QList>
#include <QMenuBar>
#include <QString>
#if QT_VERSION >= 0x040600
#include <QProxyStyle>
#endif

#include "support/shared_ptr.h"

#include <algorithm>
#include <vector>

using namespace std;
using namespace lyx::support;


namespace lyx {
namespace frontend {

namespace {

// MacOSX specific stuff is at the end.

class MenuDefinition;

///
class MenuItem {
public:
	/// The type of elements that can be in a menu
	enum Kind {
		///
		Command,
		///
		Submenu,
		///
		Separator,
		/** This type of item explains why something is unavailable. If this
		    menuitem is in a submenu, the submenu is enabled to make sure the
		    user sees the information. */
		Help,
		/** This type of item merely shows that there might be a list or
		    something alike at this position, but the list is still empty.
		    If this item is in a submenu, the submenu will not always be
		    enabled. */
		Info,
		/** This is the list of last opened file,
		    typically for the File menu. */
		Lastfiles,
		/** This is the list of opened Documents,
		    typically for the Documents menu. */
		Documents,
		/** This is the bookmarks */
		Bookmarks,
		///
		Toc,
		/** This is a list of viewable formats
		    typically for the File->View menu. */
		ViewFormats,
		/** This is a list of updatable formats
		    typically for the File->Update menu. */
		UpdateFormats,
		/** This is a list of exportable formats
		    typically for the File->Export menu. */
		ExportFormats,
		/** This is a list of importable formats
		    typically for the File->Import menu. */
		ImportFormats,
		/** This is the list of elements available
		 * for insertion into document. */
		CharStyles,
		/** This is the list of user-configurable
		insets to insert into document */
		Custom,
		/** This is the list of XML elements to
		insert into the document */
		Elements,
		/** This is the list of floats that we can
		    insert a list for. */
		FloatListInsert,
		/** This is the list of floats that we can
		    insert. */
		FloatInsert,
		/** This is the list of selections that can
		    be pasted. */
		PasteRecent,
		/** toolbars */
		Toolbars,
		/** Available branches in document */
		Branches,
		/** Available indices in document */
		Indices,
		/** Context menu for indices in document */
		IndicesContext,
		/** Available index lists in document */
		IndicesLists,
		/** Context menu for available indices lists in document */
		IndicesListsContext,
		/** Available citation styles for a given citation */
		CiteStyles,
		/** Available graphics groups */
		GraphicsGroups,
		/// Words suggested by the spellchecker.
		SpellingSuggestions,
		/** Used Languages */
		LanguageSelector,
		/** This is the list of arguments available
		    for insertion into the current layout. */
		Arguments,
		/** This is the list of arguments available
		    in the InsetArgument context menu. */
		SwitchArguments,
		/** This is the list of captions available
		in the current layout. */
		Captions,
		/** This is the list of captions available
		in the InsetCaption context menu. */
		SwitchCaptions,
		/** Commands to separate environments. */
		EnvironmentSeparators
	};

	explicit MenuItem(Kind kind) : kind_(kind), optional_(false) {}

	MenuItem(Kind kind,
		 QString const & label,
		 QString const & submenu = QString(),
		 QString const & tooltip = QString(),
		 bool optional = false)
		: kind_(kind), label_(label), submenuname_(submenu),
		  tooltip_(tooltip), optional_(optional)
	{
		LATTEST(kind == Submenu || kind == Help || kind == Info);
	}

	MenuItem(Kind kind,
		 QString const & label,
		 FuncRequest const & func,
		 QString const & tooltip = QString(),
		 bool optional = false,
		 FuncRequest::Origin origin = FuncRequest::MENU)
		: kind_(kind), label_(label), func_(func),
		  tooltip_(tooltip), optional_(optional)
	{
		func_.setOrigin(origin);
	}

	// shared_ptr<MenuDefinition> needs this apprently...
	~MenuItem() {}

	/// The label of a given menuitem
	QString label() const
	{
		int const index = label_.lastIndexOf('|');
		return index == -1 ? label_ : label_.left(index);
	}

	/// The keyboard shortcut (usually underlined in the entry)
	QString shortcut() const
	{
		int const index = label_.lastIndexOf('|');
		return index == -1 ? QString() : label_.mid(index + 1);
	}
	/// The complete label, with label and shortcut separated by a '|'
	QString fulllabel() const { return label_; }
	/// The kind of entry
	Kind kind() const { return kind_; }
	/// the action (if relevant)
	FuncRequest const & func() const { return func_; }
	/// the tooltip
	QString const & tooltip() const { return tooltip_; }
	/// returns true if the entry should be omitted when disabled
	bool optional() const { return optional_; }
	/// returns the status of the lfun associated with this entry
	FuncStatus const & status() const { return status_; }
	/// returns the status of the lfun associated with this entry
	FuncStatus & status() { return status_; }
	/// returns the status of the lfun associated with this entry
	void status(FuncStatus const & status) { status_ = status; }

	///returns the binding associated to this action.
	QString binding() const
	{
		if (kind_ != Command)
			return QString();
		// Get the keys bound to this action, but keep only the
		// first one later
		KeyMap::Bindings bindings = theTopLevelKeymap().findBindings(func_);
		if (!bindings.empty())
			return toqstr(bindings.begin()->print(KeySequence::ForGui));

		LYXERR(Debug::KBMAP, "No binding for "
			<< lyxaction.getActionName(func_.action())
			<< '(' << func_.argument() << ')');
		return QString();
	}

	/// the description of the  submenu (if relevant)
	QString const & submenuname() const { return submenuname_; }
	/// set the description of the  submenu
	void submenuname(QString const & name) { submenuname_ = name; }
	///
	bool hasSubmenu() const { return !submenu_.isEmpty(); }
	///
	MenuDefinition const & submenu() const { return submenu_.at(0); }
	MenuDefinition & submenu() { return submenu_[0]; }
	///
	void setSubmenu(MenuDefinition const & menu)
	{
		submenu_.clear();
		submenu_.append(menu);
	}

private:
	///
	Kind kind_;
	///
	QString label_;
	///
	FuncRequest func_;
	///
	QString submenuname_;
	///
	QString tooltip_;
	///
	bool optional_;
	///
	FuncStatus status_;
	/// contains 0 or 1 item.
	QList<MenuDefinition> submenu_;
};

///
class MenuDefinition {
public:
	///
	typedef std::vector<MenuItem> ItemList;
	///
	typedef ItemList::const_iterator const_iterator;
	///
	explicit MenuDefinition(QString const & name = QString()) : name_(name) {}

	///
	void read(Lexer &);
	///
	QString const & name() const { return name_; }
	///
	bool empty() const { return items_.empty(); }
	/// Clear the menu content.
	void clear() { items_.clear(); }
	///
	size_t size() const { return items_.size(); }
	///
	MenuItem const & operator[](size_t) const;
	///
	const_iterator begin() const { return items_.begin(); }
	///
	const_iterator end() const { return items_.end(); }
	///
	void cat(MenuDefinition const & other);
	///
	void catSub(docstring const & name);

	// search for func in this menu iteratively, and put menu
	// names in a stack.
	bool searchMenu(FuncRequest const & func, docstring_list & names)
		const;
	///
	bool hasFunc(FuncRequest const &) const;
	/// Add the menu item unconditionally
	void add(MenuItem const & item) { items_.push_back(item); }
	/// Checks the associated FuncRequest status before adding the
	/// menu item.
	void addWithStatusCheck(MenuItem const &);
	// Check whether the menu shortcuts are unique
	void checkShortcuts() const;
	///
	void expandLastfiles();
	void expandDocuments();
	void expandBookmarks();
	void expandFormats(MenuItem::Kind const kind, Buffer const * buf);
	void expandFloatListInsert(Buffer const * buf);
	void expandFloatInsert(Buffer const * buf);
	void expandFlexInsert(Buffer const * buf, InsetLayout::InsetLyXType type);
	void expandToc2(Toc const & toc_list, size_t from, size_t to, int depth);
	void expandToc(Buffer const * buf);
	void expandPasteRecent(Buffer const * buf);
	void expandToolbars();
	void expandBranches(Buffer const * buf);
	void expandIndices(Buffer const * buf, bool listof = false);
	void expandIndicesContext(Buffer const * buf, bool listof = false);
	void expandCiteStyles(BufferView const *);
	void expandGraphicsGroups(BufferView const *);
	void expandSpellingSuggestions(BufferView const *);
	void expandLanguageSelector(Buffer const * buf);
	void expandArguments(BufferView const *, bool switcharg = false);
	void expandCaptions(Buffer const * buf, bool switchcap = false);
	void expandEnvironmentSeparators(BufferView const *);
	///
	ItemList items_;
	///
	QString name_;
};


/// Helper for std::find_if
class MenuNamesEqual
{
public:
	MenuNamesEqual(QString const & name) : name_(name) {}
	bool operator()(MenuDefinition const & menu) const { return menu.name() == name_; }
private:
	QString name_;
};


///
typedef std::vector<MenuDefinition> MenuList;
///
typedef MenuList::const_iterator const_iterator;
///
typedef MenuList::iterator iterator;

/////////////////////////////////////////////////////////////////////
// MenuDefinition implementation
/////////////////////////////////////////////////////////////////////

void MenuDefinition::addWithStatusCheck(MenuItem const & i)
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
		bool enabled = false;
		if (i.hasSubmenu()) {
			for (const_iterator cit = i.submenu().begin();
				  cit != i.submenu().end(); ++cit) {
				// Only these kind of items affect the status of the submenu
				if ((cit->kind() == MenuItem::Command
					|| cit->kind() == MenuItem::Submenu
					|| cit->kind() == MenuItem::Help)) {
					enabled = true;
					break;
				}
			}
		}
		if (enabled || !i.optional()) {
			items_.push_back(i);
			items_.back().status().setEnabled(enabled);
		}
		break;
	}

	case MenuItem::Separator:
		if (!items_.empty() && items_.back().kind() != MenuItem::Separator)
			items_.push_back(i);
		break;

	default:
		items_.push_back(i);
	}
}


void MenuDefinition::read(Lexer & lex)
{
	enum {
		md_item = 1,
		md_branches,
		md_citestyles,
		md_documents,
		md_bookmarks,
		md_charstyles,
		md_custom,
		md_elements,
		md_endmenu,
		md_exportformats,
		md_importformats,
		md_indices,
		md_indicescontext,
		md_indiceslists,
		md_indiceslistscontext,
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
		md_graphicsgroups,
		md_spellingsuggestions,
		md_languageselector,
		md_arguments,
		md_switcharguments,
		md_captions,
		md_switchcaptions,
		md_env_separators
	};

	LexerKeyword menutags[] = {
		{ "arguments", md_arguments },
		{ "bookmarks", md_bookmarks },
		{ "branches", md_branches },
		{ "captions", md_captions },
		{ "charstyles", md_charstyles },
		{ "citestyles", md_citestyles },
		{ "custom", md_custom },
		{ "documents", md_documents },
		{ "elements", md_elements },
		{ "end", md_endmenu },
		{ "environmentseparators", md_env_separators },
		{ "exportformats", md_exportformats },
		{ "floatinsert", md_floatinsert },
		{ "floatlistinsert", md_floatlistinsert },
		{ "graphicsgroups", md_graphicsgroups },
		{ "importformats", md_importformats },
		{ "indices", md_indices },
		{ "indicescontext", md_indicescontext },
		{ "indiceslists", md_indiceslists },
		{ "indiceslistscontext", md_indiceslistscontext },
		{ "item", md_item },
		{ "languageselector", md_languageselector },
		{ "lastfiles", md_lastfiles },
		{ "optitem", md_optitem },
		{ "optsubmenu", md_optsubmenu },
		{ "pasterecent", md_pasterecent },
		{ "separator", md_separator },
		{ "spellingsuggestions", md_spellingsuggestions },
		{ "submenu", md_submenu },
		{ "switcharguments", md_switcharguments },
		{ "switchcaptions", md_switchcaptions },
		{ "toc", md_toc },
		{ "toolbars", md_toolbars },
		{ "updateformats", md_updateformats },
		{ "viewformats", md_viewformats }
	};

	lex.pushTable(menutags);
	lex.setContext("MenuDefinition::read: ");

	int md_type = 0;
	while (lex.isOK() && md_type != md_endmenu) {
		switch (md_type = lex.lex()) {
		case md_optitem:
		case md_item: {
			lex.next(true);
			docstring const name = translateIfPossible(lex.getDocString());
			lex.next(true);
			string const command = lex.getString();
			FuncRequest func = lyxaction.lookupFunc(command);
			FuncRequest::Origin origin = FuncRequest::MENU;
			if (name_.startsWith("context-toc-"))
				origin = FuncRequest::TOC;
			bool const optional = (md_type == md_optitem);
			add(MenuItem(MenuItem::Command, toqstr(name), func, QString(), optional, origin));
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

		case md_citestyles:
			add(MenuItem(MenuItem::CiteStyles));
			break;

		case md_graphicsgroups:
			add(MenuItem(MenuItem::GraphicsGroups));
			break;

		case md_spellingsuggestions:
			add(MenuItem(MenuItem::SpellingSuggestions));
			break;

		case md_languageselector:
			add(MenuItem(MenuItem::LanguageSelector));
			break;

		case md_indices:
			add(MenuItem(MenuItem::Indices));
			break;

		case md_indicescontext:
			add(MenuItem(MenuItem::IndicesContext));
			break;

		case md_indiceslists:
			add(MenuItem(MenuItem::IndicesLists));
			break;

		case md_indiceslistscontext:
			add(MenuItem(MenuItem::IndicesListsContext));
			break;

		case md_arguments:
			add(MenuItem(MenuItem::Arguments));
			break;

		case md_switcharguments:
			add(MenuItem(MenuItem::SwitchArguments));
			break;

		case md_captions:
			add(MenuItem(MenuItem::Captions));
			break;

		case md_switchcaptions:
			add(MenuItem(MenuItem::SwitchCaptions));
			break;

		case md_env_separators:
			add(MenuItem(MenuItem::EnvironmentSeparators));
			break;

		case md_optsubmenu:
		case md_submenu: {
			lex.next(true);
			docstring const mlabel = translateIfPossible(lex.getDocString());
			lex.next(true);
			docstring const mname = lex.getDocString();
			bool const optional = (md_type == md_optsubmenu);
			add(MenuItem(MenuItem::Submenu,
				toqstr(mlabel), toqstr(mname), QString(), optional));
			break;
		}

		case md_endmenu:
			break;

		default:
			lex.printError("Unknown menu tag");
			break;
		}
	}
	lex.popTable();
}


MenuItem const & MenuDefinition::operator[](size_type i) const
{
	return items_[i];
}


bool MenuDefinition::hasFunc(FuncRequest const & func) const
{
	for (const_iterator it = begin(), et = end(); it != et; ++it)
		if (it->func() == func)
			return true;
	return false;
}


void MenuDefinition::catSub(docstring const & name)
{
	add(MenuItem(MenuItem::Submenu,
		     qt_("More...|M"), toqstr(name), QString(), false));
}

void MenuDefinition::cat(MenuDefinition const & other)
{
	const_iterator et = other.end();
	for (const_iterator it = other.begin(); it != et; ++it)
		add(*it);
}


void MenuDefinition::checkShortcuts() const
{
	// This is a quadratic algorithm, but we do not care because
	// menus are short enough
	for (const_iterator it1 = begin(); it1 != end(); ++it1) {
		QString shortcut = it1->shortcut();
		if (shortcut.isEmpty())
			continue;
		if (!it1->label().contains(shortcut))
			LYXERR0("Menu warning: menu entry \""
			       << it1->label()
			       << "\" does not contain shortcut `"
			       << shortcut << "'.");
		for (const_iterator it2 = begin(); it2 != it1 ; ++it2) {
			if (!it2->shortcut().compare(shortcut, Qt::CaseInsensitive)) {
				LYXERR0("Menu warning: menu entries "
				       << '"' << it1->fulllabel()
				       << "\" and \"" << it2->fulllabel()
				       << "\" share the same shortcut.");
			}
		}
	}
}


bool MenuDefinition::searchMenu(FuncRequest const & func, docstring_list & names) const
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
			if (!m->hasSubmenu()) {
				LYXERR(Debug::GUI, "Warning: non existing sub menu label="
					<< m->label() << " name=" << m->submenuname());
				names.pop_back();
				continue;
			}
			if (m->submenu().searchMenu(func, names))
				return true;
			names.pop_back();
		}
	}
	return false;
}


QString limitStringLength(docstring const & str)
{
	size_t const max_item_length = 45;

	if (str.size() > max_item_length)
		return toqstr(str.substr(0, max_item_length - 3) + "...");

	return toqstr(str);
}


void MenuDefinition::expandGraphicsGroups(BufferView const * bv)
{
	if (!bv)
		return;
	set<string> grp;
	graphics::getGraphicsGroups(bv->buffer(), grp);
	if (grp.empty())
		return;

	set<string>::const_iterator it = grp.begin();
	set<string>::const_iterator end = grp.end();
	add(MenuItem(MenuItem::Command, qt_("No Group"),
		     FuncRequest(LFUN_SET_GRAPHICS_GROUP)));
	for (; it != end; ++it) {
		addWithStatusCheck(MenuItem(MenuItem::Command, toqstr(*it) + '|',
				FuncRequest(LFUN_SET_GRAPHICS_GROUP, *it)));
	}
}


void MenuDefinition::expandSpellingSuggestions(BufferView const * bv)
{
	if (!bv)
		return;
	Cursor const & cur = bv->cursor();
	if (!cur.inTexted())
		return;
	WordLangTuple wl;
	docstring_list suggestions;
	Paragraph const & par = cur.paragraph();
	pos_type from = cur.pos();
	pos_type to = from;
	SpellChecker::Result res = par.spellCheck(from, to, wl, suggestions, true, true);
	switch (res) {
	case SpellChecker::UNKNOWN_WORD:
		if (lyxrc.spellcheck_continuously) {
			LYXERR(Debug::GUI, "Misspelled Word! Suggested Words = ");
			docstring const & selection = cur.selectionAsString(false);
			if (!cur.selection() || selection == wl.word()) {
				size_t i = 0;
				size_t m = 10; // first submenu index
				MenuItem item(MenuItem::Submenu, qt_("More Spelling Suggestions"));
				item.setSubmenu(MenuDefinition(qt_("More Spelling Suggestions")));
				for (; i != suggestions.size(); ++i) {
					docstring const & suggestion = suggestions[i];
					LYXERR(Debug::GUI, suggestion);
					MenuItem w(MenuItem::Command, toqstr(suggestion),
						FuncRequest(LFUN_WORD_REPLACE,
							replace2string(suggestion, selection,
								true,     // case sensitive
								true,     // match word
								false,    // all words
								true,     // forward
								false))); // find next
					if (i < m)
						add(w);
					else
						item.submenu().add(w);
				}
				if (i > m)
					add(item);
				if (i > 0)
					add(MenuItem(MenuItem::Separator));
				docstring const arg = wl.word() + " " + from_ascii(wl.lang()->lang());
				add(MenuItem(MenuItem::Command, qt_("Add to personal dictionary|n"),
						FuncRequest(LFUN_SPELLING_ADD, arg)));
				add(MenuItem(MenuItem::Command, qt_("Ignore all|I"),
						FuncRequest(LFUN_SPELLING_IGNORE, arg)));
			}
		}
		break;
	case SpellChecker::LEARNED_WORD: {
			LYXERR(Debug::GUI, "Learned Word.");
			docstring const arg = wl.word() + " " + from_ascii(wl.lang()->lang());
			add(MenuItem(MenuItem::Command, qt_("Remove from personal dictionary|r"),
					FuncRequest(LFUN_SPELLING_REMOVE, arg)));
		}
		break;
	case SpellChecker::NO_DICTIONARY:
		LYXERR(Debug::GUI, "No dictionary for language " + from_ascii(wl.lang()->lang()));
		// FALLTHROUGH
	case SpellChecker::WORD_OK:
	case SpellChecker::COMPOUND_WORD:
	case SpellChecker::ROOT_FOUND:
	case SpellChecker::IGNORED_WORD:
		break;
	}
}

struct sortLanguageByName {
	bool operator()(const Language * a, const Language * b) const {
		return qt_(a->display()).localeAwareCompare(qt_(b->display())) < 0;
	}
};

void MenuDefinition::expandLanguageSelector(Buffer const * buf)
{
	if (!buf)
		return;

	std::set<Language const *> languages_buffer =
		buf->masterBuffer()->getLanguages();

	if (languages_buffer.size() < 2)
		return;

	std::set<Language const *, sortLanguageByName> languages;

	std::set<Language const *>::const_iterator const beg =
		languages_buffer.begin();
	for (std::set<Language const *>::const_iterator cit = beg;
	     cit != languages_buffer.end(); ++cit) {
		languages.insert(*cit);
	}

	MenuItem item(MenuItem::Submenu, qt_("Language|L"));
	item.setSubmenu(MenuDefinition(qt_("Language")));
	QString morelangs = qt_("More Languages ...|M");
	QStringList accelerators;
	if (morelangs.contains('|'))
		accelerators.append(morelangs.section('|', -1));
	std::set<Language const *, sortLanguageByName>::const_iterator const begin = languages.begin();
	for (std::set<Language const *, sortLanguageByName>::const_iterator cit = begin;
	     cit != languages.end(); ++cit) {
		QString label = qt_((*cit)->display());
		// try to add an accelerator
		bool success = false;
		// try capitals first
		for (int i = 0; i < label.size(); ++i) {
			QChar const ch = label[i];
			if (!ch.isUpper())
				continue;
			if (!accelerators.contains(ch, Qt::CaseInsensitive)) {
				label = label + toqstr("|") + ch;
				accelerators.append(ch);
				success = true;
				break;
			}
		}
		// if all capitals are taken, try the rest
		if (!success) {
			for (int i = 0; i < label.size(); ++i) {
				if (label[i].isSpace())
					continue;
				QString const ch = QString(label[i]);
				if (!accelerators.contains(ch, Qt::CaseInsensitive)) {
					label = label + toqstr("|") + ch;
					accelerators.append(ch);
					break;
				}
			}
		}
		MenuItem w(MenuItem::Command, label,
			FuncRequest(LFUN_LANGUAGE, (*cit)->lang() + " set"));
		item.submenu().addWithStatusCheck(w);
	}
	item.submenu().add(MenuItem(MenuItem::Separator));
	item.submenu().add(MenuItem(MenuItem::Command, morelangs,
			FuncRequest(LFUN_DIALOG_SHOW, "character")));
	add(item);
}


void MenuDefinition::expandLastfiles()
{
	LastFilesSection::LastFiles const & lf = theSession().lastFiles().lastFiles();
	LastFilesSection::LastFiles::const_iterator lfit = lf.begin();

	unsigned int ii = 1;

	for (; lfit != lf.end() && ii <= lyxrc.num_lastfiles; ++lfit, ++ii) {
		string const file = lfit->absFileName();
		QString const short_path = toqstr(makeDisplayPath(file, 30));
		QString const long_path = toqstr(makeDisplayPath(file));
		QString label;
		if (ii < 10)
			label = QString("%1. %2|%3").arg(ii).arg(short_path).arg(ii);
		else
			label = QString("%1. %2").arg(ii).arg(short_path);
		add(MenuItem(MenuItem::Command, label,
			FuncRequest(LFUN_FILE_OPEN, file), long_path));
	}
}


void MenuDefinition::expandDocuments()
{
	MenuItem item(MenuItem::Submenu, qt_("Hidden|H"));
	item.setSubmenu(MenuDefinition(qt_("Hidden|H")));

	Buffer * first = theBufferList().first();
	if (!first) {
		add(MenuItem(MenuItem::Info, qt_("<No Documents Open>")));
		return;
	}

	int i = 0;
	while (true) {
		if (!guiApp->currentView())
			break;
		GuiWorkArea * wa = guiApp->currentView()->workArea(i);
		if (!wa)
			break;
		Buffer const & b = wa->bufferView().buffer();
		QString label = toqstr(b.fileName().displayName(20));
		if (!b.isClean())
			label += "*";
		if (i < 10)
			label = QString::number(i) + ". " + label + '|' + QString::number(i);
		add(MenuItem(MenuItem::Command, label,
			FuncRequest(LFUN_BUFFER_SWITCH, b.absFileName())));
		++i;
	}


	i = 0;
	Buffer * b = first;
	// We cannot use a for loop as the buffer list cycles.
	do {
		if (!(guiApp->currentView()
		    && guiApp->currentView()->workArea(*b))) {
			QString label = toqstr(b->fileName().displayName(20));
			if (!b->isClean())
				label += "*";
			if (i < 10)
				label = QString::number(i) + ". " + label + '|' + QString::number(i);
			item.submenu().add(MenuItem(MenuItem::Command, label,
				FuncRequest(LFUN_BUFFER_SWITCH, b->absFileName())));
			++i;
		}
		b = theBufferList().next(b);
	} while (b != first);

	if (!item.submenu().empty())
		add(item);
}


void MenuDefinition::expandBookmarks()
{
	lyx::BookmarksSection const & bm = theSession().bookmarks();

	bool empty = true;
	for (size_t i = 1; i <= bm.size(); ++i) {
		if (bm.isValid(i)) {
			string const file = bm.bookmark(i).filename.absFileName();
			QString const label = QString("%1. %2|%3").arg(i)
				.arg(toqstr(makeDisplayPath(file, 20))).arg(i);
			add(MenuItem(MenuItem::Command, label,
				FuncRequest(LFUN_BOOKMARK_GOTO, convert<docstring>(i))));
			empty = false;
		}
	}
	if (empty)
		add(MenuItem(MenuItem::Info, qt_("<No Bookmarks Saved Yet>")));
}


void MenuDefinition::expandFormats(MenuItem::Kind const kind, Buffer const * buf)
{
	if (!buf && kind != MenuItem::ImportFormats)
		return;

	typedef vector<Format const *> Formats;
	Formats formats;
	FuncCode action = LFUN_NOACTION;

	switch (kind) {
	case MenuItem::ImportFormats:
		formats = theConverters().importableFormats();
		action = LFUN_BUFFER_IMPORT;
		break;
	case MenuItem::ViewFormats:
		formats = buf->params().exportableFormats(true);
		action = LFUN_BUFFER_VIEW;
		break;
	case MenuItem::UpdateFormats:
		formats = buf->params().exportableFormats(true);
		action = LFUN_BUFFER_UPDATE;
		break;
	case MenuItem::ExportFormats:
		formats = buf->params().exportableFormats(false);
		action = LFUN_BUFFER_EXPORT;
		break;
	default:
		LATTEST(false);
		return;
	}
	sort(formats.begin(), formats.end(), Format::formatSorter);

	bool const view_update = (kind == MenuItem::ViewFormats
			|| kind == MenuItem::UpdateFormats);

	QString smenue;
	if (view_update)
		smenue = (kind == MenuItem::ViewFormats
			? qt_("View (Other Formats)|F")
			: qt_("Update (Other Formats)|p"));
	MenuItem item(MenuItem::Submenu, smenue);
	item.setSubmenu(MenuDefinition(smenue));

	Formats::const_iterator fit = formats.begin();
	Formats::const_iterator end = formats.end();
	for (; fit != end ; ++fit) {
		if ((*fit)->dummy())
			continue;

		docstring lab = from_utf8((*fit)->prettyname());
		docstring const scut = from_utf8((*fit)->shortcut());
		docstring const tmplab = lab;

		if (!scut.empty())
			lab += char_type('|') + scut;
		docstring const lab_i18n = translateIfPossible(lab);
		docstring const shortcut = split(lab_i18n, lab, '|');

		bool const untranslated = (lab == lab_i18n);
		docstring label = untranslated ? translateIfPossible(tmplab) : lab;

		switch (kind) {
		case MenuItem::ImportFormats:
			label += from_ascii("...");
			break;
		case MenuItem::ViewFormats:
		case MenuItem::UpdateFormats:
			if ((*fit)->name() == buf->params().getDefaultOutputFormat()) {
				docstring lbl = (kind == MenuItem::ViewFormats
					? bformat(_("View [%1$s]|V"), label)
					: bformat(_("Update [%1$s]|U"), label));
				add(MenuItem(MenuItem::Command, toqstr(lbl), FuncRequest(action)));
				continue;
			}
		// fall through
		case MenuItem::ExportFormats:
			if (!(*fit)->inExportMenu())
				continue;
			break;
		default:
			// we already asserted earlier in this case
			// LATTEST(false);
			continue;
		}
		if (!shortcut.empty())
			label += '|' + shortcut;

		if (view_update) {
			if (buf)
				item.submenu().addWithStatusCheck(MenuItem(MenuItem::Command,
					toqstr(label), FuncRequest(action, (*fit)->name())));
			else
				item.submenu().add(MenuItem(MenuItem::Command, toqstr(label),
					FuncRequest(action, (*fit)->name())));
		} else {
			if (buf)
				addWithStatusCheck(MenuItem(MenuItem::Command, toqstr(label),
					FuncRequest(action, (*fit)->name())));
			else
				add(MenuItem(MenuItem::Command, toqstr(label),
					FuncRequest(action, (*fit)->name())));
		}
	}
	if (view_update)
		add(item);
}


void MenuDefinition::expandFloatListInsert(Buffer const * buf)
{
	if (!buf)
		return;

	FloatList const & floats = buf->params().documentClass().floats();
	FloatList::const_iterator cit = floats.begin();
	FloatList::const_iterator end = floats.end();
	set<string> seen;
	for (; cit != end; ++cit) {
		if (!cit->second.usesFloatPkg()) {
			// Different floats could declare the same ListCommand. We only
			// want it on the list once, though.
			string const & list_cmd = cit->second.listCommand();
			if (list_cmd.empty())
				// we do not know how to generate such a list
				continue;
			// This form of insert returns an iterator pointing to the newly
			// inserted element OR the existing element with that value, and
			// a bool indicating whether we inserted a new element. So we can
			// see if one is there and insert it if not all at once.
			pair<set<string>::iterator, bool> ret = seen.insert(list_cmd);
			if (!ret.second)
				continue;
		}
		string const & list_name = cit->second.listName();
		addWithStatusCheck(MenuItem(MenuItem::Command, qt_(list_name),
			FuncRequest(LFUN_FLOAT_LIST_INSERT, cit->second.floattype())));
	}
}


void MenuDefinition::expandFloatInsert(Buffer const * buf)
{
	if (!buf)
		return;

	FloatList const & floats = buf->params().documentClass().floats();
	FloatList::const_iterator cit = floats.begin();
	FloatList::const_iterator end = floats.end();
	for (; cit != end; ++cit) {
		// normal float
		QString const label = qt_(cit->second.name());
		addWithStatusCheck(MenuItem(MenuItem::Command, label,
				    FuncRequest(LFUN_FLOAT_INSERT,
						cit->second.floattype())));
	}
}


void MenuDefinition::expandFlexInsert(
		Buffer const * buf, InsetLayout::InsetLyXType type)
{
	if (!buf)
		return;

	TextClass::InsetLayouts const & insetLayouts =
		buf->params().documentClass().insetLayouts();
	TextClass::InsetLayouts::const_iterator cit = insetLayouts.begin();
	TextClass::InsetLayouts::const_iterator end = insetLayouts.end();
	for (; cit != end; ++cit) {
		if (cit->second.lyxtype() == type) {
			docstring label = cit->first;
			// we remove the "Flex:" prefix, if it is present
			if (prefixIs(label, from_utf8("Flex:")))
				label = label.substr(5);
			addWithStatusCheck(MenuItem(MenuItem::Command,
				toqstr(translateIfPossible(label)),
				FuncRequest(LFUN_FLEX_INSERT, Lexer::quoteString(label))));
		}
	}
	// FIXME This is a little clunky.
	if (items_.empty() && type == InsetLayout::CUSTOM && !buf->isReadonly())
		add(MenuItem(MenuItem::Help, qt_("No Custom Insets Defined!")));
}


size_t const max_number_of_items = 25;

void MenuDefinition::expandToc2(Toc const & toc_list,
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
			if (toc_list[i].depth() == depth) {
				label += '|';
			    if (shortcut_count < 9) {
					if (label.contains(QString::number(shortcut_count + 1)))
						label += QString::number(++shortcut_count);
				}
			}
			add(MenuItem(MenuItem::Command, label,
					    FuncRequest(toc_list[i].action())));
		}
	} else {
		size_t pos = from;
		while (pos < to) {
			size_t new_pos = pos + 1;
			while (new_pos < to && toc_list[new_pos].depth() > depth)
				++new_pos;

			QString label(4 * max(0, toc_list[pos].depth() - depth), ' ');
			label += limitStringLength(toc_list[pos].str());
			if (toc_list[pos].depth() == depth) {
				label += '|';
			    if (shortcut_count < 9) {
					if (label.contains(QString::number(shortcut_count + 1)))
						label += QString::number(++shortcut_count);
				}
			}
			if (new_pos == pos + 1) {
				add(MenuItem(MenuItem::Command,
						    label, FuncRequest(toc_list[pos].action())));
			} else {
				MenuDefinition sub;
				sub.expandToc2(toc_list, pos, new_pos, depth + 1);
				MenuItem item(MenuItem::Submenu, label);
				item.setSubmenu(sub);
				add(item);
			}
			pos = new_pos;
		}
	}
}


void MenuDefinition::expandToc(Buffer const * buf)
{
	// To make things very cleanly, we would have to pass buf to
	// all MenuItem constructors and to expandToc2. However, we
	// know that all the entries in a TOC will be have status_ ==
	// OK, so we avoid this unnecessary overhead (JMarc)

	if (!buf) {
		add(MenuItem(MenuItem::Info, qt_("<No Document Open>")));
		return;
	}

	// Add an entry for the master doc if this is a child doc
	Buffer const * const master = buf->masterBuffer();
	if (buf != master) {
		ParIterator const pit = par_iterator_begin(master->inset());
		string const arg = convert<string>(pit->id());
		FuncRequest f(LFUN_PARAGRAPH_GOTO, arg);
		add(MenuItem(MenuItem::Command, qt_("Master Document"), f));
	}

	MenuDefinition other_lists;

	FloatList const & floatlist = buf->params().documentClass().floats();
	TocList const & toc_list = buf->tocBackend().tocs();
	TocList::const_iterator cit = toc_list.begin();
	TocList::const_iterator end = toc_list.end();
	for (; cit != end; ++cit) {
		// Handle this later
		if (cit->first == "tableofcontents")
			continue;

		MenuDefinition submenu;
		if (cit->second.size() >= 30) {
			FuncRequest f(LFUN_DIALOG_SHOW, "toc " + cit->first);
			submenu.add(MenuItem(MenuItem::Command, qt_("Open Navigator..."), f));
		} else {
			TocIterator ccit = cit->second.begin();
			TocIterator eend = cit->second.end();
			for (; ccit != eend; ++ccit) {
				submenu.add(MenuItem(MenuItem::Command,
					limitStringLength(ccit->str()) + '|',
					FuncRequest(ccit->action())));
			}
		}

		MenuItem item(MenuItem::Submenu, guiName(cit->first, buf->params()));
		item.setSubmenu(submenu);
		if (floatlist.typeExist(cit->first) || cit->first == "child") {
			// Those two types deserve to be in the main menu.
			item.setSubmenu(submenu);
			add(item);
		} else
			other_lists.add(item);
	}
	if (!other_lists.empty()) {
		MenuItem item(MenuItem::Submenu, qt_("Other Lists"));
		item.setSubmenu(other_lists);
		add(item);
	}

	// Handle normal TOC
	cit = toc_list.find("tableofcontents");
	if (cit == end)
		LYXERR(Debug::GUI, "No table of contents.");
	else {
		if (!cit->second.empty())
			expandToc2(cit->second, 0, cit->second.size(), 0);
		else
			add(MenuItem(MenuItem::Info, qt_("<Empty Table of Contents>")));
	}
}


void MenuDefinition::expandPasteRecent(Buffer const * buf)
{
	docstring_list const sel = cap::availableSelections(buf);

	docstring_list::const_iterator cit = sel.begin();
	docstring_list::const_iterator end = sel.end();

	for (unsigned int index = 0; cit != end; ++cit, ++index) {
		add(MenuItem(MenuItem::Command, toqstr(*cit) + '|',
				    FuncRequest(LFUN_PASTE, convert<string>(index))));
	}
}


void MenuDefinition::expandToolbars()
{
	MenuDefinition other_lists;
	// extracts the toolbars from the backend
	Toolbars::Infos::const_iterator cit = guiApp->toolbars().begin();
	Toolbars::Infos::const_iterator end = guiApp->toolbars().end();
	for (; cit != end; ++cit) {
		MenuItem const item(MenuItem::Command, toqstr(cit->gui_name),
				FuncRequest(LFUN_TOOLBAR_TOGGLE, cit->name));
		if (guiApp->toolbars().isMainToolbar(cit->name))
			add(item);
		else
			other_lists.add(item);
	}

	if (!other_lists.empty()) {
		MenuItem item(MenuItem::Submenu, qt_("Other Toolbars"));
		item.setSubmenu(other_lists);
		add(item);
	}
}


void MenuDefinition::expandBranches(Buffer const * buf)
{
	if (!buf || buf->isReadonly())
		return;

	BufferParams const & master_params = buf->masterBuffer()->params();
	BufferParams const & params = buf->params();
	if (params.branchlist().empty() && master_params.branchlist().empty() ) {
		add(MenuItem(MenuItem::Help, qt_("No Branches Set for Document!")));
		return;
	}

	BranchList::const_iterator cit = master_params.branchlist().begin();
	BranchList::const_iterator end = master_params.branchlist().end();

	for (int ii = 1; cit != end; ++cit, ++ii) {
		docstring label = cit->branch();
		if (ii < 10) {
			label = convert<docstring>(ii) + ". " + label
				+ char_type('|') + convert<docstring>(ii);
		}
		addWithStatusCheck(MenuItem(MenuItem::Command, toqstr(label),
				    FuncRequest(LFUN_BRANCH_INSERT,
						cit->branch())));
	}

	if (buf == buf->masterBuffer())
		return;

	MenuDefinition child_branches;

	BranchList::const_iterator ccit = params.branchlist().begin();
	BranchList::const_iterator cend = params.branchlist().end();

	for (int ii = 1; ccit != cend; ++ccit, ++ii) {
		docstring label = ccit->branch();
		if (ii < 10) {
			label = convert<docstring>(ii) + ". " + label
				+ char_type('|') + convert<docstring>(ii);
		} else
			label += char_type('|');
		child_branches.addWithStatusCheck(MenuItem(MenuItem::Command,
				    toqstr(label),
				    FuncRequest(LFUN_BRANCH_INSERT,
						ccit->branch())));
	}

	if (!child_branches.empty()) {
		MenuItem item(MenuItem::Submenu, qt_("Child Document"));
		item.setSubmenu(child_branches);
		add(item);
	}
}


void MenuDefinition::expandIndices(Buffer const * buf, bool listof)
{
	if (!buf)
		return;

	BufferParams const & params = buf->masterBuffer()->params();
	if (!params.use_indices) {
		if (listof)
			addWithStatusCheck(MenuItem(MenuItem::Command,
					   qt_("Index List|I"),
					   FuncRequest(LFUN_INDEX_PRINT,
						  from_ascii("idx"))));
		else
			addWithStatusCheck(MenuItem(MenuItem::Command,
					   qt_("Index Entry|d"),
					   FuncRequest(LFUN_INDEX_INSERT,
						  from_ascii("idx"))));
		return;
	}

	if (params.indiceslist().empty())
		return;

	IndicesList::const_iterator cit = params.indiceslist().begin();
	IndicesList::const_iterator end = params.indiceslist().end();

	for (int ii = 1; cit != end; ++cit, ++ii) {
		if (listof) {
			docstring const label =
				bformat(_("Index: %1$s"), cit->index());
			addWithStatusCheck(MenuItem(MenuItem::Command, toqstr(label),
					   FuncRequest(LFUN_INDEX_PRINT, cit->shortcut())));
		} else {
			docstring const label =
				bformat(_("Index Entry (%1$s)"), cit->index());
			addWithStatusCheck(MenuItem(MenuItem::Command, toqstr(label),
					   FuncRequest(LFUN_INDEX_INSERT, cit->shortcut())));
		}
	}
}


void MenuDefinition::expandIndicesContext(Buffer const * buf, bool listof)
{
	if (!buf)
		return;

	BufferParams const & params = buf->masterBuffer()->params();
	if (!params.use_indices || params.indiceslist().empty())
		return;

	IndicesList::const_iterator cit = params.indiceslist().begin();
	IndicesList::const_iterator end = params.indiceslist().end();

	for (int ii = 1; cit != end; ++cit, ++ii) {
		if (listof) {
			InsetCommandParams p(INDEX_PRINT_CODE);
			p["type"] = cit->shortcut();
			string const data = InsetCommand::params2string(p);
			addWithStatusCheck(MenuItem(MenuItem::Command, toqstr(cit->index()),
					   FuncRequest(LFUN_INSET_MODIFY, data)));
		} else {
			docstring const label =
					bformat(_("Index Entry (%1$s)"), cit->index());
			addWithStatusCheck(MenuItem(MenuItem::Command, toqstr(label),
					   FuncRequest(LFUN_INSET_MODIFY,
						  from_ascii("changetype ") + cit->shortcut())));
		}
	}
}


void MenuDefinition::expandCiteStyles(BufferView const * bv)
{
	if (!bv)
		return;

	Inset const * inset = bv->cursor().nextInset();
	if (!inset || inset->lyxCode() != CITE_CODE) {
		add(MenuItem(MenuItem::Command,
				    qt_("No Citation in Scope!"),
				    FuncRequest(LFUN_NOACTION)));
		return;
	}
	InsetCommand const * citinset =
				static_cast<InsetCommand const *>(inset);

	Buffer const * buf = &bv->buffer();
	string const cmd = citinset->params().getCmdName();

	docstring const & key = citinset->getParam("key");
	if (key.empty()) {
		add(MenuItem(MenuItem::Command,
				    qt_("No citations selected!"),
				    FuncRequest(LFUN_NOACTION)));
		return;
	}

	docstring const & before = citinset->getParam("before");
	docstring const & after = citinset->getParam("after");

	size_t const n = cmd.size();
	bool const force = cmd[0] == 'C';
	bool const full = cmd[n] == '*';

	vector<docstring> const keys = getVectorFromString(key);

	vector<CitationStyle> const citeStyleList = buf->params().citeStyles();
	static const size_t max_length = 40;
	vector<docstring> citeStrings =
		buf->masterBibInfo().getCiteStrings(keys, citeStyleList, bv->buffer(),
		before, after, from_utf8("dialog"), max_length);

	vector<docstring>::const_iterator cit = citeStrings.begin();
	vector<docstring>::const_iterator end = citeStrings.end();

	for (int ii = 1; cit != end; ++cit, ++ii) {
		docstring label = *cit;
		CitationStyle cs = citeStyleList[ii - 1];
		cs.forceUpperCase &= force;
		cs.fullAuthorList &= full;
		addWithStatusCheck(MenuItem(MenuItem::Command, toqstr(label),
				    FuncRequest(LFUN_INSET_MODIFY,
						"changetype " + from_utf8(citationStyleToString(cs)))));
	}
}


void MenuDefinition::expandArguments(BufferView const * bv, bool switcharg)
{
	if (!bv)
		return;

	if (!bv->cursor().inTexted())
		return;

	Inset const * inset = &bv->cursor().inset();
	Layout::LaTeXArgMap args = bv->cursor().paragraph().layout().args();
	if (inset && args.empty())
		args = inset->getLayout().args();
	if (args.empty() || (switcharg && args.size() == 1))
		return;
	Layout::LaTeXArgMap::const_iterator lait = args.begin();
	Layout::LaTeXArgMap::const_iterator const laend = args.end();
	for (; lait != laend; ++lait) {
		Layout::latexarg arg = (*lait).second;
		docstring str = arg.menustring.empty()? arg.labelstring : arg.menustring;
		QString item = toqstr(translateIfPossible(str));
		if (switcharg)
			add(MenuItem(MenuItem::Command, item,
				     FuncRequest(LFUN_INSET_MODIFY,
						 from_ascii("changetype ")
						 + from_ascii((*lait).first))));
		else
			add(MenuItem(MenuItem::Command, item,
				     FuncRequest(LFUN_ARGUMENT_INSERT,
						 from_ascii((*lait).first))));
	}
}


void MenuDefinition::expandCaptions(Buffer const * buf, bool switchcap)
{
	if (!buf)
		return;

	vector<docstring> caps;
	DocumentClass const & dc = buf->params().documentClass();
	TextClass::InsetLayouts::const_iterator lit = dc.insetLayouts().begin();
	TextClass::InsetLayouts::const_iterator len = dc.insetLayouts().end();
	for (; lit != len; ++lit) {
		if (prefixIs(lit->first, from_ascii("Caption:")))
			caps.push_back(lit->first);
	}

	if (caps.empty() || (switchcap && caps.size() == 1))
		return;
	if (caps.size() == 1) {
		docstring dummy;
		docstring const type = split(*caps.begin(), dummy, ':');
		add(MenuItem(MenuItem::Command, qt_("Caption"),
			 FuncRequest(LFUN_CAPTION_INSERT, translateIfPossible(type))));
		return;
	}

	MenuDefinition captions;

	vector<docstring>::const_iterator cit = caps.begin();
	vector<docstring>::const_iterator end = caps.end();

	for (int ii = 1; cit != end; ++cit, ++ii) {
		docstring dummy;
		docstring const type = split(*cit, dummy, ':');
		docstring const trtype = translateIfPossible(type);
		docstring const cmitem = bformat(_("Caption (%1$s)"), trtype);
		// make menu item optional, otherwise we would also see
		// forbidden caption types
		if (switchcap)
			addWithStatusCheck(MenuItem(MenuItem::Command, toqstr(cmitem),
				     FuncRequest(LFUN_INSET_MODIFY,
						 from_ascii("changetype ")
						 + type), QString(), true));
		else
			captions.addWithStatusCheck(MenuItem(MenuItem::Command,
							     toqstr(trtype),
							     FuncRequest(LFUN_CAPTION_INSERT,
							     type), QString(), true));
	}
	if (!captions.empty()) {
		MenuItem item(MenuItem::Submenu, qt_("Caption"));
		item.setSubmenu(captions);
		add(item);
	}
}


void MenuDefinition::expandEnvironmentSeparators(BufferView const * bv)
{
	if (!bv)
		return;
	Text const * text = bv->cursor().text();
	// no paragraphs and no separators exist in math
	if (!text)
		return;

	pit_type pit = bv->cursor().selBegin().pit();
	Paragraph const & par = text->getPar(pit);
	docstring const curlayout = par.layout().name();
	docstring outerlayout;
	depth_type current_depth = par.params().depth();
	// check if we have an environment in our nesting hierarchy
	Paragraph cpar = par;
	while (true) {
		if (pit == 0 || cpar.params().depth() == 0)
			break;
		--pit;
		cpar = text->getPar(pit);
		if (cpar.params().depth() < current_depth
		    && cpar.layout().isEnvironment()) {
				outerlayout = cpar.layout().name();
				current_depth = cpar.params().depth();
		}
	}
	if (par.layout().isEnvironment()) {
		docstring const label =
			bformat(_("Start New Environment (%1$s)"),
				translateIfPossible(curlayout));
		add(MenuItem(MenuItem::Command, toqstr(label),
			     FuncRequest(LFUN_ENVIRONMENT_SPLIT)));
	}
	if (!outerlayout.empty()) {
		docstring const label =
			bformat(_("Start New Parent Environment (%1$s)"),
				translateIfPossible(outerlayout));
		add(MenuItem(MenuItem::Command, toqstr(label),
			     FuncRequest(LFUN_ENVIRONMENT_SPLIT,
					 from_ascii("outer"))));
	}
}

} // namespace anon


/////////////////////////////////////////////////////////////////////
// Menu::Impl definition and implementation
/////////////////////////////////////////////////////////////////////

struct Menu::Impl
{
	/// populates the menu or one of its submenu
	/// This is used as a recursive function
	void populate(QMenu & qMenu, MenuDefinition const & menu);

	/// Only needed for top level menus.
	MenuDefinition * top_level_menu;
	/// our owning view
	GuiView * view;
	/// the name of this menu
	QString name;
};



/// Get a MenuDefinition item label from the menu backend
static QString label(MenuItem const & mi)
{
	QString label = mi.label();
	label.replace("&", "&&");

	QString shortcut = mi.shortcut();
	if (!shortcut.isEmpty()) {
		int pos = label.indexOf(shortcut);
		if (pos != -1)
			//label.insert(pos, 1, char_type('&'));
			label.replace(pos, 0, "&");
	}

	QString const binding = mi.binding();
	if (!binding.isEmpty())
		label += '\t' + binding;

	return label;
}

void Menu::Impl::populate(QMenu & qMenu, MenuDefinition const & menu)
{
	LYXERR(Debug::GUI, "populating menu " << menu.name());
	if (menu.empty()) {
		LYXERR(Debug::GUI, "\tERROR: empty menu " << menu.name());
		return;
	}
	LYXERR(Debug::GUI, " *****  menu entries " << menu.size());
	MenuDefinition::const_iterator m = menu.begin();
	MenuDefinition::const_iterator end = menu.end();
	for (; m != end; ++m) {
		if (m->kind() == MenuItem::Separator)
			qMenu.addSeparator();
		else if (m->kind() == MenuItem::Submenu) {
			QMenu * subMenu = qMenu.addMenu(label(*m));
			populate(*subMenu, m->submenu());
			subMenu->setEnabled(m->status().enabled());
		} else {
			// we have a MenuItem::Command
			qMenu.addAction(new Action(QIcon(), label(*m),
				m->func(), m->tooltip(), &qMenu));
		}
	}
}

#if (defined(Q_OS_WIN) || defined(Q_CYGWIN_WIN)) && (QT_VERSION >= 0x040600)
class AlwaysMnemonicStyle : public QProxyStyle {
public:
	int styleHint(StyleHint hint, const QStyleOption *opt = 0, const QWidget *widget = 0,
		QStyleHintReturn *returnData = 0) const
	{
		if (hint == QStyle::SH_UnderlineShortcut)
			return 1;
		return QProxyStyle::styleHint(hint, opt, widget, returnData);
	}
};
#endif

/////////////////////////////////////////////////////////////////////
// Menu implementation
/////////////////////////////////////////////////////////////////////

Menu::Menu(GuiView * gv, QString const & name, bool top_level, bool keyboard)
: QMenu(gv), d(new Menu::Impl)
{
#if (defined(Q_OS_WIN) || defined(Q_CYGWIN_WIN)) && (QT_VERSION >= 0x040600)
	if (keyboard)
		setStyle(new AlwaysMnemonicStyle);
#else
	(void) keyboard;
#endif
	d->top_level_menu = top_level? new MenuDefinition : 0;
	d->view = gv;
	d->name = name;
	setTitle(name);
	if (d->top_level_menu)
		connect(this, SIGNAL(aboutToShow()), this, SLOT(updateView()));
}


Menu::~Menu()
{
	delete d->top_level_menu;
	delete d;
}


void Menu::updateView()
{
	guiApp->menus().updateMenu(this);
}


void Menu::clear()
{
	QList<QAction *> items = actions();
	for (int i = 0; i != items.size(); ++i) {
		// QAction::menu() returns 0 if there's no submenu.
		delete items.at(i)->menu();
	}
	QMenu::clear();
}


/////////////////////////////////////////////////////////////////////
// Menus::Impl definition and implementation
/////////////////////////////////////////////////////////////////////

struct Menus::Impl {
	///
	bool hasMenu(QString const &) const;
	///
	MenuDefinition & getMenu(QString const &);
	///
	MenuDefinition const & getMenu(QString const &) const;

	/// Expands some special entries of the menu
	/** The entries with the following kind are expanded to a
	    sequence of Command MenuItems: Lastfiles, Documents,
	    ViewFormats, ExportFormats, UpdateFormats, Branches,
	    Indices, Arguments, SwitchArguments, Captions, SwitchCaptions,
	    EnvironmentSeparators
	*/
	void expand(MenuDefinition const & frommenu, MenuDefinition & tomenu,
		BufferView const *) const;

	/// Initialize specific MACOS X menubar
	void macxMenuBarInit(QMenuBar * qmb);

	/// Mac special menu.
	/** This defines a menu whose entries list the FuncRequests
	    that will be removed by expand() in other menus. This is
	    used by the Qt/Mac code.

	    NOTE: Qt does not remove the menu items when clearing a QMenuBar,
	    such that the items will keep accessing the FuncRequests in
	    the MenuDefinition. While Menus::Impl might be recreated,
	    we keep mac_special_menu_ in memory by making it static.
	*/
	static MenuDefinition mac_special_menu_;

	///
	MenuList menulist_;
	///
	MenuDefinition menubar_;

	typedef QMap<GuiView *, QHash<QString, Menu*> > NameMap;

	/// name to menu for \c menu() method.
	NameMap name_map_;
};


MenuDefinition Menus::Impl::mac_special_menu_;


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
void Menus::Impl::macxMenuBarInit(QMenuBar * qmb)
{
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
		FuncCode action;
		char const * arg;
		char const * label;
		QAction::MenuRole role;
	};

	static MacMenuEntry entries[] = {
		{LFUN_DIALOG_SHOW, "aboutlyx", "About LyX",
		 QAction::AboutRole},
		{LFUN_DIALOG_SHOW, "prefs", "Preferences",
		 QAction::PreferencesRole},
#if !(defined(QT_MAC_USE_COCOA) || (QT_VERSION >= 0x050000))
		/* This doesn't work with Cocoa. */
		{LFUN_RECONFIGURE, "", "Reconfigure",
		 QAction::ApplicationSpecificRole},
#endif
		{LFUN_LYX_QUIT, "", "Quit LyX", QAction::QuitRole}
	};
	const size_t num_entries = sizeof(entries) / sizeof(entries[0]);
	const bool first_call = mac_special_menu_.empty();

	// the special menu for Menus. Fill it up only once.
	if (first_call) {
		for (size_t i = 0 ; i < num_entries ; ++i) {
			FuncRequest const func(entries[i].action,
				from_utf8(entries[i].arg));
			mac_special_menu_.add(MenuItem(MenuItem::Command,
				entries[i].label, func));
		}
	}

	// add the entries to a QMenu that will eventually be empty
	// and therefore invisible.
	QMenu * qMenu = qmb->addMenu("special");
	MenuDefinition::const_iterator cit = mac_special_menu_.begin();
	MenuDefinition::const_iterator end = mac_special_menu_.end();
	for (size_t i = 0 ; cit != end ; ++cit, ++i) {
		Action * action = new Action(QIcon(), cit->label(),
			cit->func(), QString(), qMenu);
		action->setMenuRole(entries[i].role);
		qMenu->addAction(action);
	}
}


void Menus::Impl::expand(MenuDefinition const & frommenu,
	MenuDefinition & tomenu, BufferView const * bv) const
{
	if (!tomenu.empty())
		tomenu.clear();

	for (MenuDefinition::const_iterator cit = frommenu.begin();
	     cit != frommenu.end() ; ++cit) {
		Buffer const * buf = bv ? &bv->buffer() : 0;
		switch (cit->kind()) {
		case MenuItem::Lastfiles:
			tomenu.expandLastfiles();
			break;

		case MenuItem::Documents:
			tomenu.expandDocuments();
			break;

		case MenuItem::Bookmarks:
			tomenu.expandBookmarks();
			break;

		case MenuItem::ImportFormats:
		case MenuItem::ViewFormats:
		case MenuItem::UpdateFormats:
		case MenuItem::ExportFormats:
			tomenu.expandFormats(cit->kind(), buf);
			break;

		case MenuItem::CharStyles:
			tomenu.expandFlexInsert(buf, InsetLayout::CHARSTYLE);
			break;

		case MenuItem::Custom:
			tomenu.expandFlexInsert(buf, InsetLayout::CUSTOM);
			break;

		case MenuItem::Elements:
			tomenu.expandFlexInsert(buf, InsetLayout::ELEMENT);
			break;

		case MenuItem::FloatListInsert:
			tomenu.expandFloatListInsert(buf);
			break;

		case MenuItem::FloatInsert:
			tomenu.expandFloatInsert(buf);
			break;

		case MenuItem::PasteRecent:
			tomenu.expandPasteRecent(buf);
			break;

		case MenuItem::Toolbars:
			tomenu.expandToolbars();
			break;

		case MenuItem::Branches:
			tomenu.expandBranches(buf);
			break;

		case MenuItem::Indices:
			tomenu.expandIndices(buf);
			break;

		case MenuItem::IndicesContext:
			tomenu.expandIndicesContext(buf);
			break;

		case MenuItem::IndicesLists:
			tomenu.expandIndices(buf, true);
			break;

		case MenuItem::IndicesListsContext:
			tomenu.expandIndicesContext(buf, true);
			break;

		case MenuItem::CiteStyles:
			tomenu.expandCiteStyles(bv);
			break;

		case MenuItem::Toc:
			tomenu.expandToc(buf);
			break;

		case MenuItem::GraphicsGroups:
			tomenu.expandGraphicsGroups(bv);
			break;

		case MenuItem::SpellingSuggestions:
			tomenu.expandSpellingSuggestions(bv);
			break;

		case MenuItem::LanguageSelector:
			tomenu.expandLanguageSelector(buf);
			break;

		case MenuItem::Arguments:
			tomenu.expandArguments(bv, false);
			break;

		case MenuItem::SwitchArguments:
			tomenu.expandArguments(bv, true);
			break;

		case MenuItem::Captions:
			tomenu.expandCaptions(buf, false);
			break;

		case MenuItem::SwitchCaptions:
			tomenu.expandCaptions(buf, true);
			break;

		case MenuItem::EnvironmentSeparators:
			tomenu.expandEnvironmentSeparators(bv);
			break;

		case MenuItem::Submenu: {
			MenuItem item(*cit);
			item.setSubmenu(MenuDefinition(cit->submenuname()));
			expand(getMenu(cit->submenuname()), item.submenu(), bv);
			tomenu.addWithStatusCheck(item);
		}
		break;

		case MenuItem::Info:
		case MenuItem::Help:
		case MenuItem::Separator:
			tomenu.addWithStatusCheck(*cit);
			break;

		case MenuItem::Command:
			if (!mac_special_menu_.hasFunc(cit->func()))
				tomenu.addWithStatusCheck(*cit);
		}
	}

	// we do not want the menu to end with a separator
	if (!tomenu.empty() && tomenu.items_.back().kind() == MenuItem::Separator)
		tomenu.items_.pop_back();

	// Check whether the shortcuts are unique
	tomenu.checkShortcuts();
}


bool Menus::Impl::hasMenu(QString const & name) const
{
	return find_if(menulist_.begin(), menulist_.end(),
		MenuNamesEqual(name)) != menulist_.end();
}


MenuDefinition const & Menus::Impl::getMenu(QString const & name) const
{
	const_iterator cit = find_if(menulist_.begin(), menulist_.end(),
		MenuNamesEqual(name));
	if (cit == menulist_.end()) {
		LYXERR0("No submenu named " << name);
		LASSERT(false, { static const MenuDefinition m; return m; });
	}
	return (*cit);
}


MenuDefinition & Menus::Impl::getMenu(QString const & name)
{
	iterator it = find_if(menulist_.begin(), menulist_.end(),
		MenuNamesEqual(name));
	if (it == menulist_.end()) {
		LYXERR0("No submenu named " << name);
		LASSERT(false, { static MenuDefinition m; return m; });
	}
	return (*it);
}


/////////////////////////////////////////////////////////////////////
//
// Menus
//
/////////////////////////////////////////////////////////////////////

Menus::Menus() : d(new Impl) {}


Menus::~Menus()
{
  delete d;
}


void Menus::reset()
{
	delete d;
	d = new Impl;
}


void Menus::read(Lexer & lex)
{
	enum {
		md_menu,
		md_menubar,
		md_endmenuset
	};

	LexerKeyword menutags[] = {
		{ "end", md_endmenuset },
		{ "menu", md_menu },
		{ "menubar", md_menubar }
	};

	// consistency check
	if (compare_ascii_no_case(lex.getString(), "menuset"))
		LYXERR0("Menus::read: ERROR wrong token: `" << lex.getString() << '\'');

	lex.pushTable(menutags);
	lex.setContext("Menus::read");

	bool quit = false;

	while (lex.isOK() && !quit) {
		switch (lex.lex()) {
		case md_menubar:
			d->menubar_.read(lex);
			break;
		case md_menu: {
			lex.next(true);
			QString const name = toqstr(lex.getDocString());
			if (d->hasMenu(name))
				d->getMenu(name).read(lex);
			else {
				MenuDefinition menu(name);
				menu.read(lex);
				d->menulist_.push_back(menu);
			}
			break;
		}
		case md_endmenuset:
			quit = true;
			break;
		default:
			lex.printError("Unknown menu tag");
			break;
		}
	}
	lex.popTable();
}


bool Menus::searchMenu(FuncRequest const & func,
	docstring_list & names) const
{
	MenuDefinition menu;
	d->expand(d->menubar_, menu, 0);
	return menu.searchMenu(func, names);
}


void Menus::fillMenuBar(QMenuBar * qmb, GuiView * view, bool initial)
{
	if (initial) {
#ifdef Q_OS_MAC
		// setup special mac specific menu items, but only do this
		// the first time a QMenuBar is created. Otherwise Qt will
		// create duplicate items in the application menu. It seems
		// that Qt does not remove them when the QMenubar is cleared.
		LYXERR(Debug::GUI, "Creating Mac OS X special menu bar");
		d->macxMenuBarInit(qmb);
#endif
	} else {
		// Clear all menubar contents before filling it.
		qmb->clear();
	}

	LYXERR(Debug::GUI, "populating menu bar" << d->menubar_.name());

	if (d->menubar_.empty()) {
		LYXERR(Debug::GUI, "\tERROR: empty menu bar"
			<< d->menubar_.name());
		return;
	}
	LYXERR(Debug::GUI, "menu bar entries " << d->menubar_.size());

	MenuDefinition menu;
	BufferView * bv = 0;
	if (view)
		bv = view->currentBufferView();
	d->expand(d->menubar_, menu, bv);

	MenuDefinition::const_iterator m = menu.begin();
	MenuDefinition::const_iterator end = menu.end();

	for (; m != end; ++m) {

		if (m->kind() != MenuItem::Submenu) {
			LYXERR(Debug::GUI, "\tERROR: not a submenu " << m->label());
			continue;
		}

		LYXERR(Debug::GUI, "menu bar item " << m->label()
			<< " is a submenu named " << m->submenuname());

		QString name = m->submenuname();
		if (!d->hasMenu(name)) {
			LYXERR(Debug::GUI, "\tERROR: " << name
				<< " submenu has no menu!");
			continue;
		}

		Menu * menu = new Menu(view, m->submenuname(), true);
		menu->setTitle(label(*m));

#if defined(Q_OS_MAC) && (defined(QT_MAC_USE_COCOA) || (QT_VERSION >= 0x050000))
		// On Mac OS with QT/cocoa, the menu is not displayed if there is no action
		// so we create a temporary one here
		QAction * action = new QAction(menu);
		menu->addAction(action);
#endif

		qmb->addMenu(menu);

		d->name_map_[view][name] = menu;
	}
}


void Menus::updateMenu(Menu * qmenu)
{
	LYXERR(Debug::GUI, "Triggered menu: " << qmenu->d->name);
	qmenu->clear();

	if (qmenu->d->name.isEmpty())
		return;

	docstring identifier = qstring_to_ucs4(qmenu->d->name);
	MenuDefinition fromLyxMenu(qmenu->d->name);
	while (!identifier.empty()) {
		docstring menu_name;
		identifier = split(identifier, menu_name, ';');

		if (!d->hasMenu(toqstr(menu_name))) {
			LYXERR(Debug::GUI, "\tWARNING: non existing menu: "
				<< menu_name);
			continue;
		}

		MenuDefinition cat_menu = d->getMenu(toqstr(menu_name));
		//FIXME: 50 is a wild guess. We should take into account here
		//the expansion of menu items, disabled optional items etc.
		bool const in_sub_menu = !fromLyxMenu.empty()
			&& fromLyxMenu.size() + cat_menu.size() > 50 ;
		if (in_sub_menu)
			fromLyxMenu.catSub(menu_name);
		else
			fromLyxMenu.cat(cat_menu);
		fromLyxMenu.add(MenuItem(MenuItem::Separator));
	}

	if (fromLyxMenu.empty()) {
		qmenu->addAction(qt_("No Action Defined!"));
		return;
	}

	BufferView * bv = 0;
	if (qmenu->d->view)
		bv = qmenu->d->view->currentBufferView();
	d->expand(fromLyxMenu, *qmenu->d->top_level_menu, bv);
	qmenu->d->populate(*qmenu, *qmenu->d->top_level_menu);
}


Menu * Menus::menu(QString const & name, GuiView & view, bool keyboard)
{
	LYXERR(Debug::GUI, "Context menu requested: " << name);
	Menu * menu = d->name_map_[&view].value(name, 0);
	if (!menu && !name.startsWith("context-")) {
		LYXERR0("requested context menu not found: " << name);
		return 0;
	}

	menu = new Menu(&view, name, true, keyboard);
	d->name_map_[&view][name] = menu;
	return menu;
}

} // namespace frontend
} // namespace lyx

#include "moc_Menus.cpp"
