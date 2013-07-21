/**
 * \file GuiSearch.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Edwin Leuven
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "lyxfind.h"
#include "qt_helpers.h"
#include "FuncRequest.h"
#include "BufferView.h"
#include "Buffer.h"
#include "Cursor.h"
#include "GuiSearch.h"
#include "GuiView.h"

#include "support/gettext.h"
#include "frontends/alert.h"

#include <QLineEdit>
#include <QShowEvent>

using namespace std;

namespace lyx {
namespace frontend {

static void uniqueInsert(QComboBox * box, QString const & text)
{
	for (int i = box->count(); --i >= 0; )
		if (box->itemText(i) == text)
			return;

	box->insertItem(0, text);
}


GuiSearch::GuiSearch(GuiView & lv)
	: GuiDialog(lv, "findreplace", qt_("Find and Replace"))
{
	setupUi(this);

	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(findPB, SIGNAL(clicked()), this, SLOT(findClicked()));
	connect(replacePB, SIGNAL(clicked()), this, SLOT(replaceClicked()));
	connect(replaceallPB, SIGNAL(clicked()), this, SLOT(replaceallClicked()));
	connect(findCO, SIGNAL(editTextChanged(QString)),
		this, SLOT(findChanged()));

	setFocusProxy(findCO);

	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);
	bc().setCancel(closePB);
	bc().addReadOnly(replaceCO);
	bc().addReadOnly(replacePB);
	bc().addReadOnly(replaceallPB);

	replacePB->setEnabled(false);
	replaceallPB->setEnabled(false);
}


void GuiSearch::showEvent(QShowEvent * e)
{
	findPB->setFocus();
	findCO->lineEdit()->selectAll();
	GuiDialog::showEvent(e);
}


void GuiSearch::findChanged()
{
	if (findCO->currentText().isEmpty()) {
		findPB->setEnabled(false);
		replacePB->setEnabled(false);
		replaceallPB->setEnabled(false);
	} else {
		findPB->setEnabled(true);
		replacePB->setEnabled(!isBufferReadonly());
		replaceallPB->setEnabled(!isBufferReadonly());
	}
}


void GuiSearch::findClicked()
{
	docstring const needle = qstring_to_ucs4(findCO->currentText());
	find(needle, caseCB->isChecked(), wordsCB->isChecked(),
		!backwardsCB->isChecked());
	uniqueInsert(findCO, findCO->currentText());
	findCO->lineEdit()->selectAll();
}


void GuiSearch::replaceClicked()
{
	docstring const needle = qstring_to_ucs4(findCO->currentText());
	docstring const repl = qstring_to_ucs4(replaceCO->currentText());
	replace(needle, repl, caseCB->isChecked(), wordsCB->isChecked(),
		!backwardsCB->isChecked(), false);
	uniqueInsert(findCO, findCO->currentText());
	uniqueInsert(replaceCO, replaceCO->currentText());
}


void GuiSearch::replaceallClicked()
{
	replace(qstring_to_ucs4(findCO->currentText()),
		qstring_to_ucs4(replaceCO->currentText()),
		caseCB->isChecked(), wordsCB->isChecked(), true, true);
	uniqueInsert(findCO, findCO->currentText());
	uniqueInsert(replaceCO, replaceCO->currentText());
}


void GuiSearch::wrap_dispatch(const FuncRequest & func, bool forward) {
	dispatch(func);

	BufferView * bv = const_cast<BufferView *>(bufferview());
	GuiView & lv = *const_cast<GuiView *>(&lyxview());
	if (!bv->cursor().result().dispatched()) {
		docstring q;
		if (forward)
			q = _("End of file reached while searching forward.\n"
			  "Continue searching from the beginning?");
		else
			q = _("End of file reached while searching backward.\n"
			  "Continue searching from the end?");
		int wrap_answer = frontend::Alert::prompt(_("Wrap search?"),
			q, 0, 1, _("&Yes"), _("&No"));
		if (wrap_answer == 0) {
			if (forward) {
				bv->cursor().clear();
				bv->cursor().push_back(CursorSlice(bv->buffer().inset()));
			} else {
				bv->cursor().setCursor(doc_iterator_end(&bv->buffer()));
				bv->cursor().backwardPos();
			}
			bv->clearSelection();
			dispatch(func);
			if (bv->cursor().result().dispatched())
				return;
		}
		lv.message(_("String not found."));
	}
}


void GuiSearch::find(docstring const & search, bool casesensitive,
			 bool matchword, bool forward)
{
	docstring const data =
		find2string(search, casesensitive, matchword, forward);
	wrap_dispatch(FuncRequest(LFUN_WORD_FIND, data), forward);
}


void GuiSearch::replace(docstring const & search, docstring const & replace,
			    bool casesensitive, bool matchword,
			    bool forward, bool all)
{
	docstring const data =
		replace2string(replace, search, casesensitive,
				     matchword, all, forward);
	wrap_dispatch(FuncRequest(LFUN_WORD_REPLACE, data), forward);
}


Dialog * createGuiSearch(GuiView & lv) { return new GuiSearch(lv); }


} // namespace frontend
} // namespace lyx


#include "moc_GuiSearch.cpp"
