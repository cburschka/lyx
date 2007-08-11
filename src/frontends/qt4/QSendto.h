// -*- C++ -*-
/**
 * \file QSendto.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QSENDTO_H
#define QSENDTO_H

#include "QDialogView.h"
#include "ui_SendtoUi.h"

#include <QDialog>

#include <vector>

class QCloseEvent;
class QListWidgetItem;

namespace lyx {

class Format;

namespace frontend {

class QSendto;

class QSendtoDialog : public QDialog, public Ui::QSendtoUi {
	Q_OBJECT
public:
	QSendtoDialog(QSendto * form);
protected Q_SLOTS:
	virtual void changed_adaptor();
	virtual void slotFormatHighlighted(QListWidgetItem *) {}
	virtual void slotFormatSelected(QListWidgetItem *) {}
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	QSendto * form_;
};


class ControlSendto;

/** This class provides a Qt implementation of the Custom Export Dialog.
 */
class QSendto
	: public QController<ControlSendto, QView<QSendtoDialog> >
{
public:
	///
	friend class QSendtoDialog;
	///
	QSendto(Dialog &);
protected:
	virtual bool isValid();
private:
	/// Apply from dialog
	virtual void apply();
	/// Update the dialog
	virtual void update_contents();
	/// Build the dialog
	virtual void build_dialog();
	///
	std::vector<Format const *> all_formats_;
};

} // namespace frontend
} // namespace lyx

#endif // QSENDTO_H
