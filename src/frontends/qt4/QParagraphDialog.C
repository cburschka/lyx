/**
 * \file QParagraphDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Edwin Leuven
 * \author Richard Heck
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QParagraphDialog.h"
#include "QParagraph.h"

#include <QLineEdit>
#include <QPushButton>
#include <QValidator>
#include <QWhatsThis>
#include <QCloseEvent>

#include "qt_helpers.h"
#include "frontends/controllers/ControlParagraph.h"

#include "debug.h"

#include <map>

namespace lyx {
namespace frontend {

QParagraphDialog::QParagraphDialog(QParagraph * form)
	: form_(form)
{
	setupUi(this);

	connect(okPB, SIGNAL(clicked()),
		form_, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()),
		form_, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()),
		form_, SLOT(slotClose()));
	connect(restorePB, SIGNAL(clicked()),
		form_, SLOT(slotRestore()));
	connect(alignDefaultCB, SIGNAL( clicked() ), 
		this, SLOT( change_adaptor() ) );
	connect(alignJustRB, SIGNAL( clicked() ), 
		this, SLOT( change_adaptor() ) );
	connect(alignLeftRB, SIGNAL( clicked() ), 
		this, SLOT( change_adaptor() ) );
	connect(alignRightRB, SIGNAL( clicked() ), 
		this, SLOT( change_adaptor() ) );
	connect(alignCenterRB, SIGNAL( clicked() ), 
		this, SLOT( change_adaptor() ) );
	connect(linespacing, SIGNAL( activated(int) ), 
		this, SLOT( change_adaptor() ) );
	connect(linespacing, SIGNAL( activated(int) ), 
		this, SLOT( enableLinespacingValue(int) ) );
	connect(linespacingValue, SIGNAL( textChanged(const QString&) ), 
		this, SLOT( change_adaptor() ) );
	connect(indentCB, SIGNAL( clicked() ), 
		this, SLOT( change_adaptor() ) );
	connect(labelWidth, SIGNAL( textChanged(const QString&) ), 
		this, SLOT( change_adaptor() ) );

	linespacingValue->setValidator(new QDoubleValidator(linespacingValue));

	labelWidth->setWhatsThis( qt_(
		"As described in the User Guide, the length of"
		" this text will determine how wide the label part"
		" of each item is in environments like List and"
		" Description.\n"
		"\n"
		" Normally you won't need to set this,"
		" since the largest label width of all the"
		" items is used. But if you need to, you can"
		" change it here."
	));

	radioMap[LYX_ALIGN_BLOCK] = alignJustRB;
	radioMap[LYX_ALIGN_LEFT] = alignLeftRB;
	radioMap[LYX_ALIGN_RIGHT] = alignRightRB;
	radioMap[LYX_ALIGN_CENTER] = alignCenterRB;
}


void QParagraphDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QParagraphDialog::change_adaptor()
{
	form_->changed();
}


void QParagraphDialog::enableLinespacingValue(int)
{
	bool const enable = linespacing->currentIndex() == 4;
	linespacingValue->setEnabled(enable);
}


void QParagraphDialog::checkAlignmentRadioButtons() {
	if (alignDefaultCB->isChecked()) {
		QPRadioMap::const_iterator it = radioMap.begin();
		for (; it != radioMap.end(); ++it)
			it->second->setDisabled(true);
	} else {
		LyXAlignment alignPossible = form_->controller().alignPossible();
		QPRadioMap::const_iterator it = radioMap.begin();
		for (; it != radioMap.end(); ++it)
			it->second->setEnabled(it->first & alignPossible);
	}
}


void QParagraphDialog::on_alignDefaultCB_toggled(bool)
{
	checkAlignmentRadioButtons();
	alignmentToRadioButtons();
}


void QParagraphDialog::alignmentToRadioButtons(LyXAlignment align)
{
	if (align == LYX_ALIGN_LAYOUT)
		align = form_->controller().alignDefault();

	QPRadioMap::const_iterator it = radioMap.begin();
	for (;it != radioMap.end(); ++it) {
		if (align == it->first) {
			it->second->setChecked(true);
			return;
		}
	}

	lyxerr << BOOST_CURRENT_FUNCTION << "Unknown alignment " 
		<< align << std::endl;
}


LyXAlignment QParagraphDialog::getAlignmentFromDialog()
{
	if (alignDefaultCB->isChecked()) 
		return LYX_ALIGN_LAYOUT;
	LyXAlignment alignment = LYX_ALIGN_NONE;
	QPRadioMap::const_iterator it = radioMap.begin();
	for (; it != radioMap.end(); ++it) {
		if (it->second->isChecked()) {
			alignment = it->first;
			break;
		}
	}
	if (alignment == form_->controller().alignDefault())
		return LYX_ALIGN_LAYOUT;
	else return alignment;
}


} // namespace frontend
} // namespace lyx

#include "QParagraphDialog_moc.cpp"
