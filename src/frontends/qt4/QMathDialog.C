/**
 * \file QMathDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QMathDialog.h"
#include "QMath.h"

#include <QPixmap>
#include <QResizeEvent>
#include <QScrollArea>
#include <QMenu>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>
#include <QIcon>

#include "iconpalette.h"
#include "qt_helpers.h"
#include "controllers/ControlMath.h"
#include "frontends/lyx_gui.h"

using std::string;

namespace lyx {
namespace frontend {


namespace {

char const ** panels[] = {
	latex_bop, latex_varsz, latex_brel, latex_greek, latex_arrow,
	latex_dots, latex_deco, latex_misc, latex_ams_ops,
	latex_ams_rel, latex_ams_nrel, latex_ams_arrows,
	latex_ams_misc
};

int const nr_panels = sizeof(panels)/sizeof(panels[0]);

bool panel_initialised[nr_panels];

} // namespace anon


QMathDialog::QMathDialog(QMath * form)
	: form_(form)
{
	setupUi(this);

	// enlarge the symbols ComboBox (no scrollbar)
	//symbolsCO->setSizeLimit(13);

	connect( tearoffPB, SIGNAL( clicked() ), this, SLOT( expandClicked() ) );
	connect( closePB, SIGNAL( clicked() ), this, SLOT( accept() ) );
	connect( fracPB, SIGNAL( clicked() ), this, SLOT( fracClicked() ) );
	connect( superscriptPB, SIGNAL( clicked() ), this, SLOT( superscriptClicked() ) );
	connect( subscriptPB, SIGNAL( clicked() ), this, SLOT( subscriptClicked() ) );
	connect( delimitersPB, SIGNAL( clicked() ), this, SLOT( delimiterClicked() ) );
	connect( matrixPB, SIGNAL( clicked() ), this, SLOT( matrixClicked() ) );
	connect( functionsLW, SIGNAL( itemActivated(QListWidgetItem *)  ), this, SLOT( functionSelected(QListWidgetItem *) ) );
	connect( equationPB, SIGNAL( clicked() ), this, SLOT( equationClicked() ) );
	connect( symbolsCO, SIGNAL(activated(int)), this, SLOT(showingPanel(int)));

	// function list
	for (int i = 0; *function_names[i]; ++i) {
		functionsLW->addItem(function_names[i]);
	}
	//functionsLW->setFixedWidth(functionsLW->sizeHint().width());

	// add first symbol panel
	addPanel(0);
	showingPanel(0);
	// 5 buttons + 2 margins + 1 scrollbar
	symbolWS->setFixedWidth(5*40 + 2*10 + 15 );

	// add menu's to the buttons
	QMenu * m = new QMenu(spacePB);
	m->setTitle(qt_("LyX: Math Spacing"));
	m->setTearOffEnabled(true);
	addMenuItem(m, qt_("Thin space	\\,"), ",");
	addMenuItem(m, qt_("Medium space	\\:"), ":");
	addMenuItem(m, qt_("Thick space	\\;"), ";");
	addMenuItem(m, qt_("Quadratin space	\\quad"), "quad");
	addMenuItem(m, qt_("Double quadratin space	\\qquad"), "qquad");
	addMenuItem(m, qt_("Negative space	\\!"), "!");
	spacePB->setMenu(m);

	m = new QMenu(sqrtPB);
	m->setTitle(qt_("LyX: Math Roots"));
	m->setTearOffEnabled(true);
	addMenuItem(m, qt_("Square root	\\sqrt"), "sqrt");
	QAction * ma = new QAction(qt_("Cube root	\\root"), this);
	connect(ma, SIGNAL(triggered()), this, SLOT(insertCubeRoot()));
	m->addAction(ma);
	addMenuItem(m, qt_("Other root	\\root"), "root");
	sqrtPB->setPopup(m);

	m = new QMenu(stylePB);
	m->setTitle(qt_("LyX: Math Styles"));
	m->setTearOffEnabled(true);
	addMenuItem(m, qt_("Display style	\\displaystyle"), "displaystyle");
	addMenuItem(m, qt_("Normal text style	\\textstyle"), "textstyle");
	addMenuItem(m, qt_("Script (small) style	\\scriptstyle"), "scriptstyle");
	addMenuItem(m, qt_("Scriptscript (smaller) style	\\scriptscriptstyle"), "scriptscriptstyle");
	stylePB->setPopup(m);

	m = new QMenu(fontPB);
	m->setTitle(qt_("LyX: Math Fonts"));
	m->setTearOffEnabled(true);
	addMenuItem(m, qt_("Roman	\\mathrm"), "mathrm");
	addMenuItem(m, qt_("Bold	\\mathbf"), "mathbf");
	addMenuItem(m, qt_("Bold symbol	\\boldsymbol"), "boldsymbol");
	addMenuItem(m, qt_("Sans serif	\\mathsf"), "mathsf");
	addMenuItem(m, qt_("Italic	\\mathit"), "mathit");
	addMenuItem(m, qt_("Typewriter	\\mathtt"), "mathtt");
	addMenuItem(m, qt_("Blackboard	\\mathbb"), "mathbb");
	addMenuItem(m, qt_("Fraktur	\\mathfrak"), "mathfrak");
	addMenuItem(m, qt_("Calligraphic	\\mathcal"), "mathcal");
	addMenuItem(m, qt_("Normal text mode	\\textrm"), "textrm");
	fontPB->setPopup(m);
}

void QMathDialog::addMenuItem(QMenu * menu, const QString & label, const std::string & action)
{
	QMAction * ma = new QMAction(label,action, this);
	connect(ma, SIGNAL(action(const std::string &)), this, SLOT(symbol_clicked(const std::string &)));
	menu->addAction(ma);
}

void QMathDialog::showingPanel(int num)
{
	if (!panel_initialised[num])
		addPanel(num);

	symbolWS->setCurrentIndex(num);
}


IconPalette * QMathDialog::makePanel(QWidget * parent, char const ** entries)
{
	IconPalette * p = new IconPalette(parent);
	for (int i = 0; *entries[i]; ++i) {
		p->add(QPixmap(toqstr(find_xpm(entries[i]))), entries[i], string("\\") + entries[i]);
	}
	// Leave these std:: qualifications alone !
	connect(p, SIGNAL(button_clicked(const std::string &)),
		this, SLOT(symbol_clicked(const std::string &)));

	return p;
}


void QMathDialog::addPanel(int num)
{
	QScrollArea * sc = new QScrollArea(symbolWS);
	IconPalette * p = makePanel(this, panels[num]);
	p->resize(40 * 5, p->height());
	sc->setWidget(p);
	symbolWS->insertWidget(num,sc);
	panel_initialised[num] = true;
}


void QMathDialog::symbol_clicked(const string & str)
{
	form_->controller().dispatchInsert(str);
}


void QMathDialog::fracClicked()
{
	form_->controller().dispatchInsert("frac");
}


void QMathDialog::delimiterClicked()
{
	form_->controller().showDialog("mathdelimiter");
}


void QMathDialog::expandClicked()
{
	int const id = symbolsCO->currentIndex();
	IconPalette * p = makePanel(0, panels[id]);
	string s = "LyX: ";
	s += fromqstr(symbolsCO->currentText());
	p->setCaption(toqstr(s));
	p->resize(40 * 5, p->height());
	p->show();
	p->setMaximumSize(p->width(), p->height());
}


void QMathDialog::functionSelected(QListWidgetItem * item)
{
	form_->controller().dispatchInsert(fromqstr(item->text()));
}


void QMathDialog::matrixClicked()
{
	form_->controller().showDialog("mathmatrix");
}


void QMathDialog::equationClicked()
{
	form_->controller().dispatchToggleDisplay();
}


void QMathDialog::subscriptClicked()
{
	form_->controller().dispatchSubscript();
}


void QMathDialog::superscriptClicked()
{
	form_->controller().dispatchSuperscript();
}


void QMathDialog::insertCubeRoot()
{
	form_->controller().dispatchCubeRoot();
}


} // namespace frontend
} // namespace lyx
