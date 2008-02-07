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

class QListWidgetItem;

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
	virtual kb_action getLfun() const { return LFUN_SELF_INSERT; }
	//@}

public Q_SLOTS:
	void on_applyPB_clicked();
	void on_okPB_clicked();
	void on_closePB_clicked();
	void on_symbolsLW_itemActivated(QListWidgetItem *);
	void on_symbolsLW_itemClicked(QListWidgetItem * item);
	void on_categoryCO_activated(QString const & text);
	void on_chosenLE_returnPressed();
	void on_chosenLE_textChanged(QString const &);

private:
	/// update the widgets (symbol browser, category combo)
	void updateSymbolList();
	/// get the unicode block associated with \p c
	QString const getBlock(char_type c) const;
	/// the encoding at cursor position
	std::string encoding_;
	/// which blocks do we actually include?
	typedef std::map<QString, QListWidgetItem *> UsedBlocks;
	///
	UsedBlocks used_blocks;
};

} // namespace frontend
} // namespace lyx

#endif // GUISYMBOLSDIALOG_H
