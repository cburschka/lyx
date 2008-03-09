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

#include <Qmenu>

#include <vector>

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
	QMenu * menu(QString const & name, GuiView & view);

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


class GuiPopupMenuBase : public QMenu
{
	Q_OBJECT
	
public:
	///
	GuiPopupMenuBase(QWidget * parent = 0)
		: QMenu(parent)
	{
		connect(this, SIGNAL(aboutToShow()), this, SLOT(updateView()));
	}
	
private Q_SLOTS:
	///
	virtual void updateView() = 0;
};

} // namespace frontend
} // namespace lyx

#endif // MENUS_H
