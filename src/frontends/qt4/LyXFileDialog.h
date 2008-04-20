// -*- C++ -*-
/**
 * \file LyXFileDialog.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYXFILEDIALOG_H
#define LYXFILEDIALOG_H

#include "FileDialog.h"

#include <QFileDialog>

class QToolButton;

namespace lyx {

class LyXFileDialog : public QFileDialog
{
	Q_OBJECT

public:
	LyXFileDialog(QString const & title,
		      QString const & path,
		      QStringList const & filters,
		      FileDialog::Button const & b1,
		      FileDialog::Button const & b2);

public Q_SLOTS:
	void button1Clicked();
	void button2Clicked();

private:
	QString b1_dir_;
	QString b2_dir_;
};

} // namespace lyx


#endif // LYXFILEDIALOG_H
