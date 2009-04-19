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

class QListWidgetItem;

namespace lyx {

class docstring_list;

namespace frontend {

class GuiSpellchecker : public DockView
{
	Q_OBJECT

public:
	GuiSpellchecker(GuiView & parent);
	~GuiSpellchecker();

private Q_SLOTS:
	void on_closePB_clicked();
	void on_suggestionsLW_changed(QListWidgetItem *);
	void on_replaceC0_highlighted(const QString & str);
	void on_replaceAllPB_clicked();
	void on_addPB_clicked();
	void on_ignorePB_clicked();
	void on_replacePB_clicked();

private:
	/// update from controller
	void updateSuggestions(docstring_list & words);

	///{
	void updateView();
	bool initialiseParams(std::string const & data);
	void clearParams() {}
	void dispatchParams() {}
	bool isBufferDependent() const { return true; }
	///}

	/// check text until next misspelled/unknown word
	/// returns true when finished
	void check();
	/// show count of checked words at normal exit
	void showSummary();

	struct Private;
	Private * const d;
};

} // namespace frontend
} // namespace lyx

#endif // GUISPELLCHECKER_H
