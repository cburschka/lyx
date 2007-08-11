// -*- C++ -*-
/**
 * \file QExternal.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QEXTERNAL_H
#define QEXTERNAL_H

#include "QDialogView.h"

#include "ui_ExternalUi.h"

#include <QCloseEvent>
#include <QDialog>

#include <map>

namespace lyx {
namespace frontend {

class QExternal;

class QExternalDialog : public QDialog, public Ui::QExternalUi {
	Q_OBJECT
public:
	QExternalDialog(QExternal * form);

	virtual void show();
protected Q_SLOTS:
	virtual void bbChanged();
	virtual void browseClicked();
	virtual void change_adaptor();
	virtual void editClicked();
	virtual void extraChanged(const QString&);
	virtual void formatChanged(const QString&);
	virtual void getbbClicked();
	virtual void sizeChanged();
	virtual void templateChanged();
	virtual void widthUnitChanged();

protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	bool activateAspectratio() const;
	QExternal * form_;
};


class ControlExternal;

class QExternal
	: public QController<ControlExternal, QView<QExternalDialog> >
{
public:
	friend class QExternalDialog;

	QExternal(Dialog &);

	typedef std::map<std::string, QString> MapType;

private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();

	/// Helper function called when the template is changed.
	void updateTemplate();
	/// get bounding box from file
	void getBB();

	MapType extra_;
};

} // namespace frontend
} // namespace lyx

#endif // QEXTERNAL_H
