/**
 * \file QToc.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QToc.h"
#include "QTocDialog.h"
#include "Qt2BC.h"
#include "qt_helpers.h"

#include "debug.h"

#include "controllers/ControlToc.h"

using std::endl;

using std::pair;
using std::vector;
using std::string;

namespace lyx {
namespace frontend {

typedef QController<ControlToc, QView<QTocDialog> > base_class;

QToc::QToc(Dialog & parent)
	: base_class(parent, _("Table of Contents"))
{}


void QToc::build_dialog()
{
	dialog_.reset(new QTocDialog(this));

	// Manage the cancel/close button
	bcview().setCancel(dialog_->closePB);
}


void QToc::update_contents()
{
	dialog_->updateType();
	dialog_->updateToc();
}


void QToc::select(string const & text)
{
	toc::Toc::const_iterator iter = toclist.begin();

	for (; iter != toclist.end(); ++iter) {
		if (iter->str == text)
			break;
	}

	if (iter == toclist.end()) {
		lyxerr[Debug::GUI] << "Couldn't find highlighted TOC entry: "
			<< text << endl;
		return;
	}

	controller().goTo(*iter);
}

} // namespace frontend
} // namespace lyx
