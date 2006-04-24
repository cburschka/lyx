// -*- C++ -*-
/**
 * \file QLToolbar.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QLTOOLBAR_H
#define QLTOOLBAR_H

#include "frontends/Toolbars.h"
#include <boost/scoped_ptr.hpp>

#include <qobject.h>

class QComboBox;
class QToolBar;
class QToolButton;

namespace lyx {
namespace frontend {

class QLayoutBox;
class QtView;
class QLToolbar;


class QLayoutBox : public QObject, public LayoutBox {
	Q_OBJECT
public:
	QLayoutBox(QWidget *, QtView &);

	/// select the right layout in the combox.
	void set(std::string const & layout);
	/// Populate the layout combox.
	void update();
	/// Erase the layout list.
	void clear();
	/// Display the layout list.
	void open();
	///
	void setEnabled(bool);

private slots:
	void selected(const QString & str);

private:
	QComboBox * combo_;
	QtView & owner_;
};


class QLToolbar : public QObject, public Toolbar {
	Q_OBJECT
public:
	QLToolbar(ToolbarBackend::Toolbar const &, LyXView &);
	void add(FuncRequest const & func, std::string const & tooltip);
	void hide(bool);
	void show(bool);
	void update();
	LayoutBox * layout() const { return layout_.get(); }

private slots:
	void clicked();

private:
	typedef std::map<QToolButton *, FuncRequest> ButtonMap;

	QtView & owner_;
	QToolBar * toolbar_;
	ButtonMap map_;

	boost::scoped_ptr<QLayoutBox> layout_;
};

} // namespace frontend
} // namespace lyx

#endif // NOT QLTOOLBAR_H
