// -*- C++ -*-
/**
 * \file GuiLyXFiles.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUILYXFILES_H
#define GUILYXFILES_H

#include "GuiDialog.h"
#include "FancyLineEdit.h"
#include "ui_LyXFilesUi.h"

#include <vector>

class QListWidgetItem;

namespace lyx {

class Format;

namespace frontend {

class GuiLyXFiles : public GuiDialog, public Ui::LyXFilesUi
{
	Q_OBJECT

public:
	GuiLyXFiles(GuiView & lv);
	/// A way to pass params to the dialog directly
	void passParams(std::string const & data);
	/// A way to pass params to the dialog directly
	void selectItem(QString const item);
Q_SIGNALS:
	void fileSelected(QString const file);

private Q_SLOTS:
	void fileSelectionChanged();
	void on_fileTypeCO_activated(int);
	void on_languageCO_activated(int);
	void on_filesLW_itemDoubleClicked(QTreeWidgetItem *, int);
	void on_filesLW_itemClicked(QTreeWidgetItem *, int);
	void on_browsePB_pressed();
	void slotButtonBox(QAbstractButton *);
	void filterLabels();
	void resetFilter();

private:
	///
	bool isValid() override;
	/// Apply from dialog
	void applyView() override;
	/// Update the dialog
	void updateContents() override;

	///
	bool initialiseParams(std::string const & data) override;
	///
	void paramsToDialog();
	///
	void clearParams() override {}
	///
	void dispatchParams() override;
	///
	bool isBufferDependent() const override { return false; }
	///
	FuncCode getLfun() const override;
	///
	QString const getSuffix();
	///
	bool translateName() const;
	///
	QMap<QString, QString> getFiles();
	/// Get the full file path in the selected localization
	QString const getRealPath(QString relpath = QString());
	///
	void setLanguage();

private:
	/// contains the search box
	FancyLineEdit * filter_;
	///
	QString type_;
	///
	QString file_;
	/// All languages (code, guiname)
	QMap<QString, QString> all_languages_;
	/// Languages available for files (code. guiname)
	QMap<QString, QString> available_languages_;
	///
	typedef QMap<QString, QMap<QString, QString>> localizationsMap;
	///
	localizationsMap localizations_;
	/// Possible GUI language representations
	std::vector<std::string> guilangs_;
	/// The real GUI lang as stored in available_languages_
	QString guilang_;
	/// The last used language
	QString savelang_;
};

} // namespace frontend
} // namespace lyx

#endif // GUILYXFILES_H
