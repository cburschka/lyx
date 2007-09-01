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

#include "EmbeddedFiles.h"
#include "ControlEmbeddedFiles.h"
#include "ui_EmbeddedFilesUi.h"

namespace lyx {
namespace frontend {

class GuiEmbeddedFiles;

class GuiEmbeddedFilesDialog : public QWidget, public Ui::GuiEmbeddedFilesUi {
	Q_OBJECT
public:
	GuiEmbeddedFilesDialog(GuiEmbeddedFiles * form);

public Q_SLOTS:
	///
	void on_filesLW_itemSelectionChanged();
	///
	void on_filesLW_itemDoubleClicked();
	///
	void update();
	///
	void on_actionPB_clicked();
	///
	void on_actionCB_stateChanged(int);
	///
	void on_enableCB_toggled(bool enable);
	///
	void on_autoRB_clicked();
	void on_embeddedRB_clicked();
	void on_externalRB_clicked();
	///
	void addFile();
private:
	void set_embedding_status(EmbeddedFile::STATUS);
	///
	GuiEmbeddedFiles * form_;
};


class GuiEmbeddedFiles : public QObject, public ControlEmbeddedFiles
{
	Q_OBJECT
public:
	/// Constructor
	GuiEmbeddedFiles(Dialog & dialog)
		: ControlEmbeddedFiles(dialog) {}
	///
	virtual ~GuiEmbeddedFiles() {}
};

} // namespace frontend
} // namespace lyx

#endif // QEMBEDDEDFILES_H
