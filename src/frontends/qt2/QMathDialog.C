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
 
using std::min;
using std::max;
using std::endl;
 
class QScrollViewSingle : public QScrollView {
public:
	QScrollViewSingle(QWidget * p)
		: QScrollView(p) {
		setResizePolicy(Manual);
		setHScrollBarMode(AlwaysOff);
		setVScrollBarMode(AlwaysOn);
		setMinimumHeight(200);
		setBackgroundMode(PaletteBackground);
		viewport()->setBackgroundMode(PaletteBackground);
	}
 
	void setChild(QWidget * w) {
		w_ = w; 
		setMinimumWidth(verticalScrollBar()->width() + w_->width() + 4);
		addChild(w_);
	}

protected:
	virtual void resizeEvent(QResizeEvent * e) {
		QScrollView::resizeEvent(e);
		if (!w_)
			return;
 
		w_->resize(viewport()->width(), w_->height());
		resizeContents(w_->width(), w_->height());
		setMinimumHeight(200);
	}

private:
	QWidget * w_; 
};
 
namespace { 
	char const ** panels[] = {
		latex_bop, latex_varsz, latex_brel, latex_greek, latex_arrow,
		latex_dots, latex_deco, latex_misc, latex_ams_ops,
		latex_ams_rel, latex_ams_nrel, latex_ams_arrows,
		latex_ams_misc
	};
	int const nr_panels = sizeof(panels)/sizeof(panels[0]);
}
 
 
QMathDialog::QMathDialog(QMath * form)
	: QMathDialogBase(0, 0, false, 0),
	form_(form)
{
	connect(symbolsCO, SIGNAL(activated(int)), symbolsWS, SLOT(raiseWidget(int))); 

	for (int i = 0; *function_names[i]; ++i) {
		functionsLB->insertItem(function_names[i]);
	}
 
	for (int i = 0; i < nr_panels; ++i) {
		addPanel(panels[i]);
	}
	symbolsWS->raiseWidget(0);
	symbolsWS->resize(symbolsWS->sizeHint());
}

 
IconPalette * QMathDialog::makePanel(QWidget * parent, char const ** entries)
{
	IconPalette * p = new IconPalette(parent);
	for (int i = 0; *entries[i]; ++i) {
		string xpm_name = LibFileSearch("images/math/", entries[i], "xpm");
		p->add(QPixmap(xpm_name.c_str()), entries[i], string("\\") + entries[i]);
	}
	connect(p, SIGNAL(button_clicked(string)), this, SLOT(symbol_clicked(string)));
	return p;
}

 
void QMathDialog::addPanel(char const ** entries)
{
	static int id = 0;
 
	QScrollViewSingle * view = new QScrollViewSingle(symbolsWS);
	IconPalette * p = makePanel(view->viewport(), entries);
	view->setChild(p);
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
	int const id = symbolsWS->id(symbolsWS->visibleWidget());
	IconPalette * p = makePanel(0, panels[id]);
	p->show();
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
