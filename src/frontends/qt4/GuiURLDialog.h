// -*- C++ -*-
/**
 * \file GuiURLDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIURLDIALOG_H
#define GUIURLDIALOG_H

#include "ui_URLUi.h"

#include <QDialog>
#include <QCloseEvent>

namespace lyx {
namespace frontend {

class UrlView;

class GuiURLDialog : public QDialog, public Ui::URLUi {
	Q_OBJECT
public:
	GuiURLDialog(UrlView * form);
public Q_SLOTS:
	void changed_adaptor();
protected:
	void closeEvent(QCloseEvent *);
private:
	UrlView * form_;
};

} // namespace frontend
} // namespace lyx

#endif // GUIURLDIALOG_H
