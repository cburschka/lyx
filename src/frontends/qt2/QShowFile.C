/**
 * \file QShowFile.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QShowFile.h"
#include "QShowFileDialog.h"
#include "Qt2BC.h"
#include "qt_helpers.h"

#include "controllers/ControlShowFile.h"

#include <qtextview.h>
#include <qpushbutton.h>

using std::string;

namespace lyx {
namespace frontend {

typedef QController<ControlShowFile, QView<QShowFileDialog> > base_class;


QShowFile::QShowFile(Dialog & parent)
	: base_class(parent, _("LyX: Show File"))
{
}


void QShowFile::build_dialog()
{
	dialog_.reset(new QShowFileDialog(this));

	bcview().setCancel(dialog_->closePB);
}


void QShowFile::update_contents()
{
	dialog_->setName(toqstr(controller().getFileName()));

	string contents = controller().getFileContents();
	if (contents.empty()) {
		contents = "Error -> Cannot load file!";
	}

	dialog_->text->setText(toqstr(contents));
}

} // namespace frontend
} // namespace lyx
