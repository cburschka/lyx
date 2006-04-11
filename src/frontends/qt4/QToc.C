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

void QToc::moveUp()
{
	controller().outline(toc::UP);
	update_contents();
}


void QToc::moveDown()
{
	controller().outline(toc::DOWN);
	update_contents();
}


void QToc::moveIn()
{
	controller().outline(toc::IN);
	update_contents();
}


void QToc::moveOut()
{
	controller().outline(toc::OUT);
	update_contents();
}


} // namespace frontend
} // namespace lyx
