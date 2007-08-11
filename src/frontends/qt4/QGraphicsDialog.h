// -*- C++ -*-
/**
 * \file QGraphicsDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Herbert Voﬂ
 * \author Richard Heck
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QGRAPHICSDIALOG_H
#define QGRAPHICSDIALOG_H

#include "ui_GraphicsUi.h"

#include <QCloseEvent>
#include <QDialog>
#include <QString>

namespace lyx {
namespace frontend {

class QGraphics;

class QGraphicsDialog : public QDialog, public Ui::QGraphicsUi {
	Q_OBJECT
public:
	QGraphicsDialog(QGraphics * form);
	virtual void setAutoText();
	virtual void show();
protected Q_SLOTS:
	virtual void change_adaptor();
	virtual void change_bb();
	virtual void on_browsePB_clicked();
	virtual void on_getPB_clicked();
	virtual void on_editPB_clicked();
	virtual void on_filename_textChanged(const QString &);
	virtual void on_scaleCB_toggled(bool);
	virtual void on_WidthCB_toggled(bool);
	virtual void on_HeightCB_toggled(bool);
	virtual void on_angle_textChanged(const QString &);
protected:
	virtual void closeEvent(QCloseEvent * e);
private:
	QGraphics * form_;
};

} // namespace frontend
} // namespace lyx

#endif // QGRAPHICSDIALOG_H
