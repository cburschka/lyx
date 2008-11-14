// -*- C++ -*-
/**
 * \file GuiSymbols.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUISYMBOLSDIALOG_H
#define GUISYMBOLSDIALOG_H

#include "DialogView.h"
#include "ui_SymbolsUi.h"

#include <map>
#include <string>
#include <vector>

namespace lyx {
namespace frontend {

class GuiSymbols : public DialogView, public Ui::SymbolsUi
{
	Q_OBJECT

public:
	GuiSymbols(GuiView & lv);

	/// Dialog inherited methods
	//@{
	void updateView();
	void dispatchParams();
	void enableView(bool enable);
	bool isBufferDependent() const { return true; }
	FuncCode getLfun() const { return LFUN_SELF_INSERT; }
	//@}

public Q_SLOTS:
	void on_applyPB_clicked();
	void on_okPB_clicked();
	void on_closePB_clicked();
	void on_symbolsLW_activated(QModelIndex const & index);
	void on_symbolsLW_clicked(QModelIndex const & index);
	void on_categoryCO_activated(QString const & text);
	void on_categoryFilterCB_toggled(bool);
	void on_chosenLE_returnPressed();
	void on_chosenLE_textChanged(QString const &);

private:
	void scrollToItem(QString const & category);
	/** update the widgets (symbol browser, category combo)
	 *  \p combo indicates if the combo box has to be refreshed
	 *  as well (which is rather expensive)
	**/
	void updateSymbolList(bool update_combo = true);
	/// the encoding at cursor position
	std::string encoding_;
	/// which blocks do we actually include?
	typedef std::map<QString, int> UsedBlocks;
	///
	UsedBlocks used_blocks;
	/// list of all symbols
	typedef std::vector<char_type> SymbolsList;
	///
	SymbolsList symbols_;
	/// custom model for symbol list view
	class Model;
	friend class Model;
	Model * model_;
};

} // namespace frontend
} // namespace lyx

#endif // GUISYMBOLSDIALOG_H
