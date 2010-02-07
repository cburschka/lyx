/**
 * \file InsetDialog.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "InsetDialog.h"

#include "qt_helpers.h"

#include "Buffer.h"
#include "buffer_funcs.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Cursor.h"
#include "FuncRequest.h"

#include "support/debug.h"
#include "support/lstrings.h"

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {

/////////////////////////////////////////////////////////////////
//
// InsetDialog::Private
//
/////////////////////////////////////////////////////////////////

struct InsetDialog::Private
{
	Private(InsetCode code, FuncCode creation_code)
		: inset_code_(code), creation_code_(creation_code)
	{
	}

	///
	InsetCode inset_code_;
	///
	FuncCode creation_code_;
};

/////////////////////////////////////////////////////////////////
//
// InsetDialog
//
/////////////////////////////////////////////////////////////////

InsetDialog::InsetDialog(GuiView & lv, InsetCode code, FuncCode creation_code,
		char const * name, char const * display_name)
	: DialogView(lv, name, qt_(display_name)), d(new Private(code, creation_code))
{
}


void InsetDialog::on_closePB_clicked()
{
	hide();
}


void InsetDialog::on_newPB_clicked()
{
	docstring argument;
	if (d->creation_code_ == LFUN_INSET_INSERT)
		argument = from_ascii(insetName(d->inset_code_)) + " ";
	argument += dialogToParams();
	dispatch(FuncRequest(d->creation_code_, argument));
}


void InsetDialog::applyView()
{
	if (!checkWidgets())
		return;

	Inset const * i = inset(d->inset_code_);
	if (!i)
		return;
	
	docstring const argument = dialogToParams();
	if (!i->validateModifyArgument(argument))
		return;

	dispatch(FuncRequest(LFUN_INSET_MODIFY, argument));
}


void InsetDialog::updateView()
{
	Inset const * i = inset(d->inset_code_);
	if (i)
		paramsToDialog(i);
	else
		enableView(false);
}

} // namespace frontend
} // namespace lyx

#include "moc_InsetDialog.cpp"
