// -*- C++ -*-
/**
 * \file GuiFloatDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QFLOATDIALOG_H
#define QFLOATDIALOG_H

#include "ui_FloatUi.h"

#include <QCloseEvent>
#include <QDialog>

namespace lyx {
namespace frontend {

class GuiFloat;

class GuiFloatDialog : public QDialog, public Ui::FloatUi {
	Q_OBJECT
public:
	GuiFloatDialog(GuiFloat * form);

protected Q_SLOTS:
	virtual void change_adaptor();

protected:
	virtual void closeEvent(QCloseEvent * e);

private:
	GuiFloat * form_;
};

} // namespace frontend
} // namespace lyx

#endif // QFLOATDIALOG_H
