// -*- C++ -*-
/**
 * \file GuiExternal.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIEXTERNAL_H
#define GUIEXTERNAL_H

#include "GuiDialog.h"
#include "ui_ExternalUi.h"

#include "support/types.h"

#include "insets/InsetExternal.h"

#include <QHash>

namespace lyx {
namespace frontend {

class GuiExternal : public GuiDialog, public Ui::ExternalUi
{
	Q_OBJECT

public:
	GuiExternal(GuiView & lv);

private Q_SLOTS:
	void bbChanged();
	void browseClicked();
	void change_adaptor();
	void editClicked();
	void extraChanged(const QString &);
	void formatChanged(const QString &);
	void getbbClicked();
	void sizeChanged();
	void templateChanged();
	void widthUnitChanged();

private:
	///
	typedef QHash<QString, QString> MapType;

	///
	bool activateAspectratio() const;
	/// Apply changes
	void applyView();
	/// update
	void updateContents();
	/// Helper function called when the template is changed.
	void updateTemplate();
	/// get bounding box from file
	void getBB();

	///
	MapType extra_;
	///
	bool initialiseParams(std::string const & data);
	/// clean-up on hide.
	void clearParams();
	/// clean-up on hide.
	void dispatchParams();
	///
	bool isBufferDependent() const { return true; }

	///
	void editExternal();
	///
	QString browse(QString const & input_file,
				 QString const & template_name) const;

	/// Read the Bounding Box from a eps or ps-file
	std::string const readBB(std::string const & file);
	///
private:
	///
	InsetExternalParams params_;
	///
	bool bbChanged_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIEXTERNAL_H
