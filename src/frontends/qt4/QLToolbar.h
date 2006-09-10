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
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QLTOOLBAR_H
#define QLTOOLBAR_H

#include "frontends/Toolbars.h"
#include <boost/scoped_ptr.hpp>

#include <QToolBar>
#include <vector>

class QComboBox;

namespace lyx {
namespace frontend {

class QLayoutBox;
class GuiView;
class Action;


class QLayoutBox : public QObject, public LayoutBox {
	Q_OBJECT
public:
	QLayoutBox(QToolBar *, GuiView &);

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

private Q_SLOTS:
	void selected(const QString & str);

private:
	QComboBox * combo_;
	GuiView & owner_;
};


class QLToolbar : public QToolBar, public Toolbar {
	Q_OBJECT
public:
	QLToolbar(ToolbarBackend::Toolbar const &, GuiView &);

	//~QLToolbar();

	void add(FuncRequest const & func, std::string const & tooltip);
	void hide(bool);
	void show(bool);
	void update();
	LayoutBox * layout() const { return layout_.get(); }



Q_SIGNALS:
	void updated();

private:

	std::vector<Action *> ActionVector;
	GuiView & owner_;

	boost::scoped_ptr<QLayoutBox> layout_;
};

} // namespace frontend
} // namespace lyx

#endif // NOT QLTOOLBAR_H
