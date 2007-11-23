// -*- C++ -*-
/**
 * \file GuiTexInfo.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 * \author Herbert Voß
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUITEXINFO_H
#define GUITEXINFO_H

#include "GuiDialog.h"
#include "ui_TexinfoUi.h"
#include "qt_helpers.h"

#include <map>
#include <vector>


namespace lyx {
namespace frontend {

class GuiTexInfo : public GuiDialog, public Ui::TexinfoUi
{
	Q_OBJECT

public:
	///
	GuiTexInfo(GuiView & lv);
	/// the file extensions. order matters in GuiTexInfo::fileType()
	enum TexFileType { ClsType, StyType, BstType };

public Q_SLOTS:
	///
	void updateView();

private Q_SLOTS:
	///
	void change_adaptor();
	///
	void rescanClicked();
	///
	void viewClicked();
	///
	void enableViewPB();

private:
	///
	void closeEvent(QCloseEvent * e);
	///
	void updateStyles(TexFileType);
	///
	void updateStyles();
	///
	bool warningPosted;
	///
	TexFileType activeStyle;
	/// Nothing to initialise in this case.
	bool initialiseParams(std::string const &) { return true; }
	///
	void clearParams() {}
	///
	void dispatchParams() {}
	///
	bool isBufferDependent() const { return false; }
	///
	void apply() {}

	/// show contents af a file
	void viewFile(std::string const & filename) const;
	/// show all classoptions
	std::string classOptions(std::string const & filename) const;
	/// return file type as string
	std::string fileType(TexFileType type) const;

	typedef std::vector<std::string> ContentsType;
	std::map<TexFileType, ContentsType> texdata_;
};


} // namespace frontend
} // namespace lyx

#endif // GUITEXINFO_H
