/**
 * \file QParagraph.C
 * Copyright 2000-2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Edwin Leuven <leuven@fee.uva.nl>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlParagraph.h"
#include "QParagraph.h"
#include "QParagraphDialog.h"
#include "Qt2BC.h"
#include "ParagraphParameters.h"
#include "lyxrc.h" // to set the deafult length values
#include "gettext.h"
#include "layout.h" // LyXAlignment
#include "helper_funcs.h"
#include "lyxgluelength.h"
#include "vspace.h"

#include "support/lstrings.h"
#include "support/LAssert.h"

#include <functional>

#include <qcombobox.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qbuttongroup.h>

using std::vector;
using std::bind2nd;
using std::remove_if;

typedef Qt2CB<ControlParagraph, Qt2DB<QParagraphDialog> > base_class;

QParagraph::QParagraph(ControlParagraph & c, Dialogs &)
	: base_class(c, _("Paragraph Layout"))
{}

void QParagraph::build_dialog()
{
	// the tabbed folder
	dialog_.reset(new QParagraphDialog(this));

	// Create the contents of the unit choices
	// Don't include the "%" terms...
	units_ = getLatexUnits();
	vector<string>::iterator del = remove_if(units_.begin(), units_.end(),
						 bind2nd(contains_functor(), "%"));
	units_.erase(del, units_.end());

	for (vector<string>::const_iterator it = units_.begin();
		it != units_.end(); ++it) {
		dialog_->unitAbove->insertItem(it->c_str());
		dialog_->unitBelow->insertItem(it->c_str());
	}

	// Manage the ok, apply, restore and cancel/close buttons
	bc().setOK(dialog_->okPB);
	bc().setApply(dialog_->applyPB);
	bc().setCancel(dialog_->closePB);
	bc().setRestore(dialog_->restorePB);
	bc().addReadOnly(dialog_->paragraphTab);
}

namespace {

VSpace setVSpaceFromWidgets(int spacing,
			    string value,
			    string unit,
			    bool keep)
{
	VSpace space;

	switch (spacing) {
	case 0:
		space = VSpace(VSpace::NONE);
		break;
	case 1:
		space = VSpace(VSpace::DEFSKIP);
		break;
	case 2:
		space = VSpace(VSpace::SMALLSKIP);
		break;
	case 3:
		space = VSpace(VSpace::MEDSKIP);
		break;
	case 4:
		space = VSpace(VSpace::BIGSKIP);
		break;
	case 5:
		space = VSpace(VSpace::VFILL);
		break;
	case 6:
		string s;
		string const length = trim(value);
		if (isValidGlueLength(length)) {
			s = length;
		} else if (!length.empty()){
			string u = trim(unit);
			u = subst(u, "%%", "%");
			s = length + u;
		}
		space = VSpace(LyXGlueLength(s));
		break;
	}

	space.setKeep(keep);

	return space;
}

} // namespace anon

void QParagraph::apply()
{
	/* SPACING ABOVE */
	// If a vspace kind is "Length" but there's no text in
	// the input field, reset the kind to "None".
	if (dialog_->spacingAbove->currentItem()==5
	    && dialog_->valueAbove->text().isEmpty())
		dialog_->spacingAbove->setCurrentItem(0);

	VSpace const space_top =
		setVSpaceFromWidgets(dialog_->spacingAbove->currentItem(),
				     string(dialog_->valueAbove->text()),
				     string(dialog_->unitAbove->currentText()),
				     dialog_->keepAbove->isChecked());


	controller().params().spaceTop(space_top);

	/* SPACING BELOW */
	if (dialog_->spacingBelow->currentItem()==5
	    && dialog_->valueBelow->text().isEmpty())
		dialog_->spacingBelow->setCurrentItem(0);

	VSpace const space_bottom =
	setVSpaceFromWidgets(dialog_->spacingBelow->currentItem(),
			     string(dialog_->valueBelow->text()),
			     string(dialog_->unitBelow->currentText()),
			     dialog_->keepBelow->isChecked());

	controller().params().spaceBottom(space_bottom);

	/* alignment */
	LyXAlignment align;
	switch (dialog_->align->currentItem()) {
	case 0:
		align = LYX_ALIGN_BLOCK;
		break;
	case 1:
		align = LYX_ALIGN_LEFT;
		break;
	case 2:
		align = LYX_ALIGN_RIGHT;
		break;
	case 3:
		align = LYX_ALIGN_CENTER;
		break;
	default:
		align = LYX_ALIGN_BLOCK;
	}
	controller().params().align(align);

	/* get spacing */
	Spacing::Space linespacing = Spacing::Default;
	string other;
	switch (dialog_->linespacing->currentItem()) {
	case 0:
		linespacing = Spacing::Default;
		break;
	case 1:
		linespacing = Spacing::Single;
		break;
	case 2:
		linespacing = Spacing::Onehalf;
		break;
	case 3:
		linespacing = Spacing::Double;
		break;
	case 4:
		linespacing = Spacing::Other;
		other = dialog_->linespacingValue->text();
		break;
	}

	Spacing const spacing(linespacing, other);
	controller().params().spacing(spacing);

	/* lines and pagebreaks */
	controller().params().lineTop(dialog_->lineAbove->isChecked());
	controller().params().lineBottom(dialog_->lineBelow->isChecked());
	controller().params().pagebreakTop(dialog_->pagebreakAbove->isChecked());
	controller().params().pagebreakBottom(dialog_->pagebreakBelow->isChecked());
	/* label width */
	controller().params().labelWidthString(string(dialog_->labelWidth->text()));
	/* indendation */
	controller().params().noindent(dialog_->noindent->isChecked());

}

namespace {

void setWidgetsFromVSpace(VSpace const & space,
			  QComboBox * spacing,
			  QLineEdit * value,
			  QComboBox * unit,
			  QCheckBox * keep, vector<string> units_)
{
	value->setText("");
	value->setEnabled(false);
	unit->setEnabled(false);

	int item;
	switch (space.kind()) {
	case VSpace::NONE:
		item = 0;
		break;
	case VSpace::DEFSKIP:
		item = 1;
		break;
	case VSpace::SMALLSKIP:
		item = 2;
		break;
	case VSpace::MEDSKIP:
		item = 3;
		break;
	case VSpace::BIGSKIP:
		item = 4;
		break;
	case VSpace::VFILL:
		item = 5;
		break;
	case VSpace::LENGTH:
		item = 6;
		value->setEnabled(true);
		unit->setEnabled(true);
		string length = space.length().asString();
		string const default_unit = (lyxrc.default_papersize>3) ? "cm" : "in";
		string supplied_unit = default_unit;
		LyXLength len(length);
		if ((isValidLength(length) || isStrDbl(length)) && !len.zero()) {
			ostringstream buffer;
			buffer << len.value();
			length = buffer.str();
			supplied_unit = subst(stringFromUnit(len.unit()),"%","%%");
		}
		int unit_item = 0;
		int i = 0;
		for (vector<string>::const_iterator it = units_.begin();
		     it != units_.end(); ++it) {
			if (*it==default_unit) {
				unit_item = i;
			}
			if (*it==supplied_unit) {
				unit_item = i;
				break;
			}
			i += 1;
		}
		value->setText(length.c_str());
		unit->setCurrentItem(unit_item);
		break;
	}
	spacing->setCurrentItem(item);
	keep->setChecked(space.keep());
}

} // namespace anon

void QParagraph::update_contents()
{
	/* label width */
	string labelwidth = controller().params().labelWidthString();
	dialog_->labelWidth->setText(labelwidth.c_str());
	dialog_->labelwidthGB->setEnabled(
		labelwidth != _("Senseless with this layout!"));

	/* alignment */
	int i;
	switch (controller().params().align()) {
	case LYX_ALIGN_LEFT:
		i = 1;
		break;
	case LYX_ALIGN_RIGHT:
		i = 2;
		break;
	case LYX_ALIGN_CENTER:
		i = 3;
		break;
	default:
		i = 0;
		break;
	}
	dialog_->align->setCurrentItem(i);


	//LyXAlignment alignpos = controller().alignPossible();

	// no inset-text-owned paragraph may have pagebreaks
	bool ininset = controller().inInset();
	dialog_->pagebreakAbove->setEnabled(!ininset);
	dialog_->pagebreakBelow->setEnabled(!ininset);

	/* lines, pagebreaks and indent */
	dialog_->lineAbove->setChecked(controller().params().lineTop());
	dialog_->lineBelow->setChecked(controller().params().lineBottom());
	dialog_->pagebreakAbove->setChecked(controller().params().pagebreakTop());
	dialog_->pagebreakBelow->setChecked(controller().params().pagebreakBottom());
	dialog_->noindent->setChecked(controller().params().noindent());

	/* linespacing */
	int linespacing;
	Spacing const space = controller().params().spacing();
	switch (space.getSpace()) {
	case Spacing::Single:
		linespacing = 1;
		break;
	case Spacing::Onehalf:
		linespacing = 2;
		break;
	case Spacing::Double:
		linespacing = 3;
		break;
	case Spacing::Other:
		linespacing = 4;
		break;
	default:
		linespacing = 0;
		break;
	}
	dialog_->linespacing->setCurrentItem(linespacing);
	if (space.getSpace() == Spacing::Other) {
		string const sp = tostr(space.getValue());
		dialog_->linespacingValue->setText(sp.c_str());
		dialog_->linespacingValue->setEnabled(true);
	} else {
		dialog_->linespacingValue->setText("");
		dialog_->linespacingValue->setEnabled(false);
	}

	/* vspace top */
	setWidgetsFromVSpace(controller().params().spaceTop(),
			     dialog_->spacingAbove,
			     dialog_->valueAbove,
			     dialog_->unitAbove,
			     dialog_->keepAbove,units_);

	/* vspace bottom */
	setWidgetsFromVSpace(controller().params().spaceBottom(),
			     dialog_->spacingBelow,
			     dialog_->valueBelow,
			     dialog_->unitBelow,
			     dialog_->keepBelow,units_);

	/* no indent */
	dialog_->noindent->setChecked(controller().params().noindent());
}
