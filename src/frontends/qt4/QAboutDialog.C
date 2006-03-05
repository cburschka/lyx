/**
 * \file QAboutDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Kalle Dalheimer
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QAboutDialog.h"

namespace lyx {
namespace frontend {

QAboutDialog::QAboutDialog(QWidget * parent, const char * name,
			   bool modal, Qt::WFlags fl)
{
	setupUi(this);

    connect( closePB, SIGNAL( clicked() ), this, SLOT( reject() ) );
}


QAboutDialog::~QAboutDialog()
{}

} // namespace frontend
} // namespace lyx
