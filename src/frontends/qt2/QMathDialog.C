/**
 * \file QMathDialog.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#include "support/filetools.h"
#include "gettext.h"
#include "debug.h"
 
#include "QMathDialog.h"
#include "QMath.h"

#include "ControlMath.h"
#include "iconpalette.h"
 
#include <qapplication.h>
#include <qwidgetstack.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qpixmap.h>
#include <qscrollview.h>
#include <qlayout.h>
#include <qpopupmenu.h>
#include <qcursor.h>
 
using std::min;
using std::max;
using std::endl;
 
class QScrollViewSingle : public QScrollView {
public:
	QScrollViewSingle(QWidget * p)
		: QScrollView(p), w_(0) {
		setResizePolicy(Manual);
		setHScrollBarMode(AlwaysOff);
		setVScrollBarMode(AlwaysOn);
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
		// force the resize to get accurate scrollbars
		qApp->processEvents();
		resizeContents(w_->width(), w_->height());
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

bool panel_initialised[nr_panels];
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
		QScrollViewSingle * view = new QScrollViewSingle(symbolsWS);
		symbolsWS->addWidget(view, i);
	}

	// aboutToShow() only fires when != 0 in Qt 2 !
	symbolsWS->raiseWidget(0);
	addPanel(0);
	panel_initialised[0] = true;
 
	connect(symbolsWS, SIGNAL(aboutToShow(int)), this, SLOT(showingPanel(int)));
 
	QPopupMenu * m = new QPopupMenu(spacePB);
	m->insertItem(_("Thin space	\\,"), 1); 
	m->insertItem(_("Medium space	\\:"), 2); 
	m->insertItem(_("Thick space	\\;"), 3); 
	m->insertItem(_("Quadratin space	\\quad"), 4); 
	m->insertItem(_("Double quadratin space	\\qquad"), 5); 
	m->insertItem(_("Negative space	\\!"), 6);
	connect(m, SIGNAL(activated(int)), this, SLOT(insertSpace(int)));
	spacePB->setPopup(m);

	m = new QPopupMenu(sqrtPB);
	m->insertItem(_("Square root	\\sqrt"), 1);
	m->insertItem(_("Cube root	\\root"), 2);
	m->insertItem(_("Other root	\\root"), 3);
	connect(m, SIGNAL(activated(int)), this, SLOT(insertRoot(int))); 
	sqrtPB->setPopup(m);

	m = new QPopupMenu(stylePB);
	m->insertItem(_("Display style	\\displaystyle"), 1);
	m->insertItem(_("Normal text style	\\textstyle"), 2);
	m->insertItem(_("Script (small) style	\\scriptstyle"), 3);
	m->insertItem(_("Scriptscript (smaller) style	\\scriptscriptstyle"), 4);
	connect(m, SIGNAL(activated(int)), this, SLOT(insertStyle(int)));
	stylePB->setPopup(m);

	m = new QPopupMenu(fontPB);
	m->insertItem(_("Roman	\\mathrm"), 1);
	m->insertItem(_("Bold	\\mathbf"), 2);
	m->insertItem(_("San serif	\\mathsf"), 3);
	m->insertItem(_("Italic	\\mathit"), 4);
	m->insertItem(_("Typewriter	\\mathtt"), 5);
	m->insertItem(_("Blackboard	\\mathbb"), 6);
	m->insertItem(_("Fraktur	\\mathfrak"), 7);
	m->insertItem(_("Calligraphic	\\mathcal"), 8);
	m->insertItem(_("Normal text mode	\\textrm"), 9);
	connect(m, SIGNAL(activated(int)), this, SLOT(insertFont(int)));
	fontPB->setPopup(m);
}


void QMathDialog::showingPanel(int num)
{
	if (panel_initialised[num])
		return;

	addPanel(num);

	// Qt needs to catch up. Dunno why. 
	qApp->processEvents();
 
	panel_initialised[num] = true;
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

 
void QMathDialog::addPanel(int num)
{
	QScrollViewSingle * view = static_cast<QScrollViewSingle*>(symbolsWS->visibleWidget());
	IconPalette * p = makePanel(view->viewport(), panels[num]);
	view->setChild(p);
}

 
void QMathDialog::symbol_clicked(string str)
{
	form_->insert(str);
}

 
void QMathDialog::fracClicked()
{
	form_->insert("frac");
}
 

void QMathDialog::delimiterClicked()
{
}

 
void QMathDialog::expandClicked()
{
	int const id = symbolsWS->id(symbolsWS->visibleWidget());
	IconPalette * p = makePanel(0, panels[id]);
	string s = "LyX: "; 
	s += symbolsCO->text(id).latin1();
	p->setCaption(s.c_str());
	p->resize(40 * 5, p->height());
	p->show();
	p->setMaximumSize(p->width(), p->height());
}
 
 
void QMathDialog::functionSelected(const QString & str)
{
	form_->insert(str.latin1()); 
}

 
void QMathDialog::matrixClicked()
{
	form_->insertMatrix();
}

 
void QMathDialog::equationClicked()
{
	form_->toggleDisplay();
}

 
void QMathDialog::subscriptClicked()
{
	form_->subscript();
}

 
void QMathDialog::superscriptClicked()
{
	form_->superscript();
}


void QMathDialog::insertSpace(int id)
{
	string str;
	switch (id) {
		case 1: str = ","; break;
		case 2: str = ":"; break;
		case 3: str = ";"; break;
		case 4: str = "quad"; break;
		case 5: str = "qquad"; break;
		case 6: str = "!"; break;
	}
	form_->insert(str);
}

 
void QMathDialog::insertRoot(int id)
{
	switch (id) {
		case 1:
			form_->insert("sqrt");
			break;
		case 2:
			form_->insertCubeRoot();
			break;
		case 3:
			form_->insert("root");
			break;
	}
}

 
void QMathDialog::insertStyle(int id)
{
	string str;
	switch (id) {
		case 1: str = "displaystyle"; break;
		case 2: str = "textstyle"; break;
		case 3: str = "scriptstyle"; break;
		case 4: str = "scriptscriptstyle"; break;
	} 
	form_->insert(str);
}

 
void QMathDialog::insertFont(int id)
{
	string str;
	switch (id) {
		case 1: str = "mathrm"; break;
		case 2: str = "mathbf"; break;
		case 3: str = "mathsf"; break;
		case 4: str = "mathit"; break;
		case 5: str = "mathtt"; break;
		case 6: str = "mathbb"; break;
		case 7: str = "mathfrak"; break;
		case 8: str = "mathcal"; break;
		case 9: str = "textrm"; break;
	}
	form_->insert(str);
}
