/**
 * \file QMathDialog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#include "support/filetools.h"
#include "debug.h"
 
#include "QMathDialog.h"
#include "QMath.h"

#include "ControlMath.h"
#include "iconpalette.h"
 
#include <qwidgetstack.h>
#include <qcombobox.h>
#include <qlistbox.h>
#include <qpixmap.h>
#include <qscrollview.h>
#include <qlayout.h>
 
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
	addPanel("dots", latex_dots);
	addPanel("deco", latex_deco);
	addPanel("misc", latex_misc);
	addPanel("amsoperators", latex_ams_ops);
	addPanel("amsrelations", latex_ams_rel);
	addPanel("amsnegrelations", latex_ams_nrel);
	addPanel("amsarrows", latex_ams_arrows);
	addPanel("amsmisc", latex_ams_misc);
	symbolsWS->raiseWidget(0);
	symbolsWS->resize(symbolsWS->sizeHint());
}

 
void QMathDialog::addPanel(string const & name, char const ** entries)
{
	static int id = 0;
 
	QScrollView * view = new QScrollView(symbolsWS);
	view->setHScrollBarMode(QScrollView::AlwaysOff);
	view->setVScrollBarMode(QScrollView::AlwaysOn);
 
	IconPalette * p = new IconPalette(view->viewport(), name.c_str());
	view->addChild(p);
	for (int i = 0; *entries[i]; ++i) {
		string xpm_name = LibFileSearch("images/math/", entries[i], "xpm");
		p->add(QPixmap(xpm_name.c_str()), entries[i], string("\\") + entries[i]);
	}
	connect(p, SIGNAL(button_clicked(string)), this, SLOT(symbol_clicked(string)));
 
	symbolsWS->addWidget(view, id++);
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

 
void QMathDialog::spaceClicked()
{
}

 
void QMathDialog::styleClicked()
{
}

 
void QMathDialog::subscriptClicked()
{
	form_->subscript();
}

 
void QMathDialog::superscriptClicked()
{
	form_->superscript();
}
