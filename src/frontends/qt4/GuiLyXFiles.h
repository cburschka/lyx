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

private Q_SLOTS:
	void changed_adaptor();
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
	bool isValid();
	/// Apply from dialog
	void applyView();
	/// Update the dialog
	void updateContents();

	///
	bool initialiseParams(std::string const & data);
	///
	void paramsToDialog();
	///
	void clearParams() {}
	///
	void dispatchParams();
	///
	bool isBufferDependent() const { return false; }
	///
	FuncCode getLfun() const;
	///
	QString const getSuffix();
	///
	bool translateName() const;
	///
	void getFiles(QMap<QString, QString> &, QString const);
	/// Get the full file path in the selected localization
	QString const getRealPath();
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
