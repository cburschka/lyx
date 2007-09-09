// -*- C++ -*-
/**
 * \file GuiExternal.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIEXTERNAL_H
#define GUIEXTERNAL_H

#include "GuiDialog.h"
#include "ControlExternal.h"
#include "ui_ExternalUi.h"

#include <map>

namespace lyx {
namespace frontend {

class GuiExternalDialog : public GuiDialog, public Ui::ExternalUi
{
	Q_OBJECT

public:
	GuiExternalDialog(LyXView & lv);

private Q_SLOTS:
	void bbChanged();
	void browseClicked();
	void change_adaptor();
	void editClicked();
	void extraChanged(const QString&);
	void formatChanged(const QString&);
	void getbbClicked();
	void sizeChanged();
	void templateChanged();
	void widthUnitChanged();

private:
public:
	void closeEvent(QCloseEvent * e);
	//
	bool activateAspectratio() const;
	/// parent controller
	ControlExternal & controller() const;
	/// Apply changes
	void applyView();
	/// update
	void update_contents();
	/// Helper function called when the template is changed.
	void updateTemplate();
	/// get bounding box from file
	void getBB();

	///
	typedef std::map<std::string, QString> MapType;
	///
	MapType extra_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIEXTERNAL_H
