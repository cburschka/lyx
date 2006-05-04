/**
 * \file QParagraphDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Edwin Leuven
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
	connect(align, SIGNAL( activated(int) ), 
		this, SLOT( change_adaptor() ) );
	connect(linespacing, SIGNAL( activated(int) ), 
		this, SLOT( change_adaptor() ) );
	connect(linespacing, SIGNAL( activated(int) ), 
		this, SLOT( enableLinespacingValue(int) ) );
	connect(linespacingValue, SIGNAL( textChanged(const QString&) ), 
		this, SLOT( change_adaptor() ) );
	connect(indentCB, SIGNAL( toggled(bool) ), 
		this, SLOT( change_adaptor() ) );
	connect(labelWidth, SIGNAL( textChanged(const QString&) ), 
		this, SLOT( change_adaptor() ) );

	linespacingValue->setValidator(new QDoubleValidator(linespacingValue));

	//Q3WhatsThis::add(labelWidth, qt_(
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
	bool const enable = linespacing->currentItem() == 4;
	linespacingValue->setEnabled(enable);
}

} // namespace frontend
} // namespace lyx
