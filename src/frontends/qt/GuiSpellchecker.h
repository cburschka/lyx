// -*- C++ -*-
/**
 * \file GuiSpellchecker.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Kalle Dalheimer
 * \author Edwin Leuven
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUISPELLCHECKER_H
#define GUISPELLCHECKER_H

#include "DockView.h"
#include <QTabWidget>

class QListWidgetItem;

namespace lyx {

class docstring_list;

namespace frontend {

class SpellcheckerWidget : public QTabWidget
{
	Q_OBJECT

public:
	SpellcheckerWidget(GuiView * gv, DockView * dv, QWidget * parent = 0);
	~SpellcheckerWidget();
	///
	void updateView();
	///
	bool initialiseParams(std::string const &);

private Q_SLOTS:
	void on_findNextPB_clicked();
	void on_replaceAllPB_clicked();
	void on_suggestionsLW_itemClicked(QListWidgetItem *);
	void on_replaceCO_highlighted(const QString & str);
	void on_languageCO_activated(int index);
	void on_ignoreAllPB_clicked();
	void on_addPB_clicked();
	void on_ignorePB_clicked();
	void on_replacePB_clicked();

private:
	///
	bool eventFilter(QObject *obj, QEvent *event) override;
	struct Private;
	Private * const d;
};


class GuiSpellchecker : public DockView
{
	Q_OBJECT

public:
	GuiSpellchecker(
		GuiView & parent, ///< the main window where to dock.
		Qt::DockWidgetArea area = Qt::RightDockWidgetArea, ///< Position of the dock (and also drawer)
		Qt::WindowFlags flags = 0);
	~GuiSpellchecker();

private:
	///{
	void updateView() override;
	bool initialiseParams(std::string const & sdata) override
		{ return widget_->initialiseParams(sdata); }
	void clearParams() override {}
	void dispatchParams() override {}
	bool isBufferDependent() const override { return false; }
	///}
	/// The encapsulated widget.
	SpellcheckerWidget * widget_;
};

} // namespace frontend
} // namespace lyx

#endif // GUISPELLCHECKER_H
