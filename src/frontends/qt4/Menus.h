// -*- C++ -*-
/**
 * \file Menus.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MENUS_H
#define MENUS_H

#include <QMenu>

class QString;
class QMenuBar;

namespace lyx {

class docstring_list;
class Lexer;
class FuncRequest;

namespace frontend {

class GuiView;

class Menu : public QMenu
{
	Q_OBJECT
public:
	///
	Menu(GuiView * gv, QString const & name, bool top_level,
		bool keyboard = false);

	///
	~Menu();

	/// QMenu::clear() reimplementation to handle the deletion of submenus.
	void clear();

private Q_SLOTS:
	///
	void updateView();

private:
	friend class Menus;

	/// Use the Pimpl idiom to hide the internals.
	struct Impl;
	/// The pointer never changes although *d's contents may.
	Impl * const d;
};


class Menus
{
	/// noncopyable
	Menus(Menus const &);
	void operator=(Menus const &);
public:
	Menus();
	~Menus();

	///
	void reset();

	///
	bool searchMenu(FuncRequest const & func,
		docstring_list & names) const;
	///
	void fillMenuBar(QMenuBar * qmb, GuiView * view, bool initial = false);

	/// \return a top-level submenu given its name.
	Menu * menu(QString const & name, GuiView & view, bool keyboard = false);

	///
	void read(Lexer &);

	///
	void updateMenu(Menu * qmenu);

private:
	/// Use the Pimpl idiom to hide the internals.
	struct Impl;
	///
	Impl * d;
};

} // namespace frontend
} // namespace lyx

#endif // MENUS_H
