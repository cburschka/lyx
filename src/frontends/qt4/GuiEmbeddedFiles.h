// -*- C++ -*-
/**
 * \file GuiEmbeddedFiles.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Bo Peng
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIEMBEDDEDFILES_H
#define GUIEMBEDDEDFILES_H

#include "GuiDialog.h"
#include "EmbeddedFiles.h"
#include "ControlEmbeddedFiles.h"
#include "ui_EmbeddedFilesUi.h"

namespace lyx {
namespace frontend {

class GuiEmbeddedFilesDialog : public GuiDialog, public Ui::GuiEmbeddedFilesUi
{
	Q_OBJECT

public:
	GuiEmbeddedFilesDialog(LyXView & lv);

public Q_SLOTS:
	///
	void on_filesLW_itemSelectionChanged();
	///
	void on_filesLW_itemDoubleClicked();
	///
	void updateView();
	///
	void on_enableCB_toggled(bool enable);
	///
	void on_embeddedRB_clicked();
	void on_externalRB_clicked();
	///
	void on_addPB_clicked();
	//
	void on_extractPB_clicked();

private:
	ControlEmbeddedFiles & controller() const;
	void set_embedding_status(bool embed);
};

} // namespace frontend
} // namespace lyx

#endif // QEMBEDDEDFILES_H
