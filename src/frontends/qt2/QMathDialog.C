/**
 * \file QMathDialog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#include "support/filetools.h"
 
#include "QMathDialog.h"
#include "QMath.h"

#include "ControlMath.h"
#include "iconpalette.h"
 
#include <qwidgetstack.h>
#include <qcombobox.h>
#include <qlistbox.h>
#include <qpixmap.h>
 
QMathDialog::QMathDialog(QMath * form)
	: QMathDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(symbolsCO, SIGNAL(activated(int)), symbolsWS, SLOT(raiseWidget(int))); 

	for (int i = 0; *function_names[i]; ++i) {
		functionsLB->insertItem(function_names[i]);
	}
 
	addPanel("operators", latex_bop);
	addPanel("bigoperators", latex_varsz);
	addPanel("relations", latex_brel);
	addPanel("greek", latex_greek);
	addPanel("arrows", latex_arrow);
	addPanel("misc", latex_misc);
	addPanel("amsoperators", latex_ams_ops);
	addPanel("amsrelations", latex_ams_rel);
	addPanel("amsnegrelations", latex_ams_nrel);
	addPanel("amsarrows", latex_ams_arrows);
	addPanel("amsmisc", latex_ams_misc);
	symbolsWS->raiseWidget(0);
}

 
void QMathDialog::addPanel(string const & name, char const ** entries)
{
	static int id = 0;
 
	IconPalette * p = new IconPalette(this, name.c_str());
	for (int i = 0; *entries[i]; ++i) {
		string xpm_name = LibFileSearch("images/math/", entries[i], "xpm");
		p->add(QPixmap(xpm_name.c_str()), entries[i], string("\\") + entries[i]);
	}
	connect(p, SIGNAL(button_clicked(string)), this, SLOT(symbol_clicked(string)));
	symbolsWS->addWidget(p, id++);
}

 
void QMathDialog::symbol_clicked(string str)
{
	form_->insert_symbol(str);
}

 
void QMathDialog::fracClicked()
{
	form_->insert_symbol("frac");
}
 

void QMathDialog::sqrtClicked()
{
	form_->insert_symbol("sqrt");
}

 
void QMathDialog::decoClicked()
{
}

 
void QMathDialog::delimiterClicked()
{
}

 
void QMathDialog::expandClicked()
{
}

 
 
void QMathDialog::functionSelected(const QString & str)
{
	form_->insert_symbol(str.latin1()); 
}

 
void QMathDialog::matrixClicked()
{
}

 
void QMathDialog::equationClicked()
{
}

 
void QMathDialog::spaceClicked()
{
}

 
void QMathDialog::styleClicked()
{
}

 
void QMathDialog::subscriptClicked()
{
}

 
void QMathDialog::superscriptClicked()
{
}
