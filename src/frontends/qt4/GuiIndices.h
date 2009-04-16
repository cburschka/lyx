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

#include "GuiDocument.h"
#include "ui_IndicesUi.h"
#include "IndicesList.h"

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

	void update(BufferParams const & params);
	void apply(BufferParams & params) const;

Q_SIGNALS:
	void changed();

protected:
	void toggleColor(QTreeWidgetItem *);
	void updateView();

protected Q_SLOTS:
	void on_addIndexPB_pressed();
	void on_renamePB_clicked();
	void on_removePB_pressed();
	void on_indicesTW_itemDoubleClicked(QTreeWidgetItem *, int);
	void on_colorPB_clicked();
	void on_multipleIndicesCB_toggled(bool);

private:
	/// Contains all legal indices for this doc
	IndicesList indiceslist_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIINDICES_H
