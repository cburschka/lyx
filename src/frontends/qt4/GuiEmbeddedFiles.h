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
#include "DockView.h"
#include "EmbeddedFiles.h"
#include "ui_EmbeddedFilesUi.h"

namespace lyx {
namespace frontend {

class EmbeddedFilesWidget;

class GuiEmbeddedFiles : public DockView
{
	Q_OBJECT

public:
	///
	GuiEmbeddedFiles(
		GuiViewBase & parent, ///< the main window where to dock.
		Qt::DockWidgetArea area = Qt::LeftDockWidgetArea, ///< Position of the dock (and also drawer)
		Qt::WindowFlags flags = 0);
	///
	void updateView();
	
	///
	EmbeddedFiles & embeddedFiles();
	///
	bool initialiseParams(std::string const &);
	/// obtain embedded files from buffer
	void updateEmbeddedFiles();
	///
	void clearParams() {}
	///
	bool isBufferDependent() const { return true; }
	///
	bool canApply() const { return true; }
	///
	bool canApplyToReadOnly() const { return false; }
	///
	void dispatchMessage(std::string const & msg);
	///
	void dispatchParams() {}
	///
	bool isReadonly();
	///
	void setEmbedding(bool enable);
	///
	void goTo(EmbeddedFile const & item, int idx);
	///
	void view(EmbeddedFile const & item);
	///
	void setEmbed(EmbeddedFile & item, bool embed, bool update);
	///
	bool browseAndAddFile();
	///
	bool extract(EmbeddedFile const & item);
	///
	bool update(EmbeddedFile const & item);

protected:
	///
	EmbeddedFilesWidget * widget_;
	///
	std::string message_;
};


class EmbeddedFilesWidget : public QWidget, public Ui::GuiEmbeddedFilesUi
{
	Q_OBJECT

public:
	EmbeddedFilesWidget(GuiEmbeddedFiles &);
	std::string name() const { return "embedding"; }

public Q_SLOTS:
	///
	void on_filesLW_itemChanged(QListWidgetItem* item);
	void on_filesLW_itemSelectionChanged();
	void on_filesLW_itemClicked(QListWidgetItem* item);
	void on_filesLW_itemDoubleClicked(QListWidgetItem* item);
	///
	void updateView();
	///
	void on_enableCB_toggled(bool enable);
	///
	void on_selectPB_clicked();
	void on_unselectPB_clicked();
	void on_addPB_clicked();
	void on_extractPB_clicked();
	void on_updatePB_clicked();

private:
	GuiEmbeddedFiles & controller_;
	void set_embedding_status(bool embed);
};


} // namespace frontend
} // namespace lyx

#endif // QEMBEDDEDFILES_H
