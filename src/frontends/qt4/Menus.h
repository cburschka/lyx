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

#include "support/docstring.h"

#include <vector>

class QMenu;
class QString;

namespace lyx {

class Lexer;
class FuncRequest;

namespace frontend {

class GuiView;

class Menus
{
public:
	Menus();
	
	///
	bool searchMenu(FuncRequest const & func,
		std::vector<docstring> & names) const;
	///
	void fillMenuBar(GuiView * view);

	/// \return a top-level submenu given its name.
	QMenu * menu(QString const & name);

	///
	void read(Lexer &);

	///
	void updateMenu(QString const & name);

private:
	/// Use the Pimpl idiom to hide the internals.
	struct Impl;
	/// The pointer never changes although *d's contents may.
	Impl * const d;
};

} // namespace frontend
} // namespace lyx

#endif // MENUS_H
