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


#include "qt_helpers.h"

#include "ControlMath.h"

#include "QMathDialog.h"
#include "QMath.h"

#include "iconpalette.h"

#include <qwidgetstack.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qpopupmenu.h>

using std::string;


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

} // namespace anon


QMathDialog::QMathDialog(QMath * form)
	: QMathDialogBase(0, 0, false, 0),
	form_(form)
{
	// enlarge the symbols ComboBox (no scrollbar)
	symbolsCO->setSizeLimit(13);

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
	m->setCaption(qt_("LyX: Insert space"));
	m->insertTearOffHandle();
	m->insertItem(qt_("Thin space	\\,"), 1);
	m->insertItem(qt_("Medium space	\\:"), 2);
	m->insertItem(qt_("Thick space	\\;"), 3);
	m->insertItem(qt_("Quadratin space	\\quad"), 4);
	m->insertItem(qt_("Double quadratin space	\\qquad"), 5);
	m->insertItem(qt_("Negative space	\\!"), 6);
	connect(m, SIGNAL(activated(int)), this, SLOT(insertSpace(int)));
	spacePB->setPopup(m);

	m = new QPopupMenu(sqrtPB);
	m->setCaption(qt_("LyX: Insert root"));
	m->insertTearOffHandle();
	m->insertItem(qt_("Square root	\\sqrt"), 1);
	m->insertItem(qt_("Cube root	\\root"), 2);
	m->insertItem(qt_("Other root	\\root"), 3);
	connect(m, SIGNAL(activated(int)), this, SLOT(insertRoot(int)));
	sqrtPB->setPopup(m);

	m = new QPopupMenu(stylePB);
	m->setCaption(qt_("LyX: Set math style"));
	m->insertTearOffHandle();
	m->insertItem(qt_("Display style	\\displaystyle"), 1);
	m->insertItem(qt_("Normal text style	\\textstyle"), 2);
	m->insertItem(qt_("Script (small) style	\\scriptstyle"), 3);
	m->insertItem(qt_("Scriptscript (smaller) style	\\scriptscriptstyle"), 4);
	connect(m, SIGNAL(activated(int)), this, SLOT(insertStyle(int)));
	stylePB->setPopup(m);

	m = new QPopupMenu(fontPB);
	m->setCaption(qt_("LyX: Set math font"));
	m->insertTearOffHandle();
	m->insertItem(qt_("Roman	\\mathrm"), 1);
	m->insertItem(qt_("Bold	\\mathbf"), 2);
	m->insertItem(qt_("Sans serif	\\mathsf"), 3);
	m->insertItem(qt_("Italic	\\mathit"), 4);
	m->insertItem(qt_("Typewriter	\\mathtt"), 5);
	m->insertItem(qt_("Blackboard	\\mathbb"), 6);
	m->insertItem(qt_("Fraktur	\\mathfrak"), 7);
	m->insertItem(qt_("Calligraphic	\\mathcal"), 8);
	m->insertItem(qt_("Normal text mode	\\textrm"), 9);
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
		p->add(QPixmap(toqstr(find_xpm(entries[i]))), entries[i], string("\\") + entries[i]);
	}
	// Leave these std:: qualifications alone !
	connect(p, SIGNAL(button_clicked(const std::string &)),
		this, SLOT(symbol_clicked(const std::string &)));

	return p;
}


void QMathDialog::addPanel(int num)
{
	QScrollViewSingle * view = static_cast<QScrollViewSingle*>(symbolsWS->widget(num));
	IconPalette * p = makePanel(view->viewport(), panels[num]);
	view->setChild(p);
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
	int const id = symbolsWS->id(symbolsWS->visibleWidget());
	IconPalette * p = makePanel(0, panels[id]);
	string s = "LyX: ";
	s += fromqstr(symbolsCO->text(id));
	p->setCaption(toqstr(s));
	p->resize(40 * 5, p->height());
	p->show();
	p->setMaximumSize(p->width(), p->height());
}


void QMathDialog::functionSelected(const QString & str)
{
	form_->controller().dispatchInsert(fromqstr(str));
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
		default: return;
	}
	form_->controller().dispatchInsert(str);
}


void QMathDialog::insertRoot(int id)
{
	switch (id) {
		case 1:
			form_->controller().dispatchInsert("sqrt");
			break;
		case 2:
			form_->controller().dispatchCubeRoot();
			break;
		case 3:
			form_->controller().dispatchInsert("root");
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
		default: return;
	}
	form_->controller().dispatchInsert(str);
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
		default: return;
	}
	form_->controller().dispatchInsert(str);
}
