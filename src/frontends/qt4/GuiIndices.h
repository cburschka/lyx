// -*- C++ -*-
/**
 * \file GuiIndices.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIINDICES_H
#define GUIINDICES_H

#include "IndicesList.h"

#include "ui_IndicesUi.h"

#include <QWidget>

class QTreeWidgetItem;

namespace lyx {

class BufferParams;

namespace frontend {

class GuiIndices : public QWidget, public Ui::IndicesUi
{
	Q_OBJECT
public:
	GuiIndices(QWidget * parent = 0);

	void update(BufferParams const & params, bool const readonly);
	void apply(BufferParams & params) const;

Q_SIGNALS:
	void changed();

protected:
	void toggleColor(QTreeWidgetItem *);
	void updateView();

protected Q_SLOTS:
	void on_indexCO_activated(int n);
	void on_newIndexLE_textChanged(QString);
	void on_indexOptionsLE_textChanged(QString);
	void on_addIndexPB_pressed();
	void on_renamePB_clicked();
	void on_removePB_pressed();
	void on_indicesTW_itemDoubleClicked(QTreeWidgetItem *, int);
	void on_indicesTW_itemSelectionChanged();
	void on_colorPB_clicked();
	void on_multipleIndicesCB_toggled(bool);

private:
	///
	void updateWidgets();
	/// Contains all legal indices for this doc
	IndicesList indiceslist_;
	///
	bool readonly_;
	///
	bool use_indices_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIINDICES_H
