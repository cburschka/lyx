/**
 * \file QDocument.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QDocument.h"
#include "QDocumentDialog.h"
#include "Qt2BC.h"
#include "qt_helpers.h"

#include "bufferparams.h"
#include "floatplacement.h"
#include "gettext.h"
#include "helper_funcs.h" // getSecond()
#include "language.h"
#include "lyxrc.h" // defaultUnit
#include "lyxtextclasslist.h"

#include "support/lstrings.h"

#include "controllers/ControlDocument.h"
#include "controllers/frnt_lang.h"

#include "lengthcombo.h"


using lyx::support::bformat;
using lyx::support::getVectorFromString;

using std::distance;
using std::vector;
using std::string;

namespace lyx {
namespace frontend {

typedef QController<ControlDocument, QView<QDocumentDialog> > base_class;


QDocument::QDocument(Dialog & parent)
	: base_class(parent, _("Document Settings"))
{}


void QDocument::build_dialog()
{
	dialog_.reset(new QDocumentDialog(this));
}


void QDocument::showPreamble()
{
	dialog_->showPreamble();
}


void QDocument::apply()
{
	if (!dialog_.get())
		return;

	dialog_->apply(controller().params());
}


void QDocument::update_contents()
{
	if (!dialog_.get())
		return;

	dialog_->update(controller().params());
}

void QDocument::saveDocDefault()
{
	// we have to apply the params first
	apply();
	controller().saveAsDefault();
}


void QDocument::useClassDefaults()
{
	BufferParams & params = controller().params();

	///\todo verify the use of below with lyx-devel:
	params.textclass = dialog_->latexModule->classCO->currentItem();

	params.useClassDefaults();
	update_contents();
}

} // namespace frontend
} // namespace lyx
