/**
 * \file docdlg.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>

#include <qtooltip.h>

#include "docdlg.h"

#include "support/lstrings.h"

#include "layout.h"
#include "tex-strings.h"
#include "bufferparams.h"

#include "dlg/helpers.h"

#include "gettext.h"
#include "debug.h"

#ifdef CXX_WORKING_NAMESPACES
using kde_helpers::setSizeHint;
using kde_helpers::setComboFromStr;
#endif

using std::endl;

DocDialog::DocDialog(FormDocument * form, QWidget * parent, char const * name, bool, WFlags)
	: DocDialogData(parent,name), form_(form)
{
	setCaption(name);

	setUpdatesEnabled(false);

	settings = new DocSettingsDialogData(this, "settings");
	extra = new DocExtraDialogData(this, "extra");
	geometry = new DocGeometryDialogData(this, "geometry");
	language = new DocLanguageDialogData(this, "language");
	bullets = new DocBulletsDialogData(this, "bullets");
	
	tabstack->addTabPage(settings, _("&Settings"));
	tabstack->addTabPage(extra, _("&Extra"));
	tabstack->addTabPage(geometry, _("&Geometry"));
	tabstack->addTabPage(language, _("&Language"));
	tabstack->addTabPage(bullets, _("&Bullets"));
	
	// document classes
	for (LyXTextClassList::const_iterator cit = textclasslist.begin();
		cit != textclasslist.end(); ++cit)
		settings->docclass->insertItem((*cit).description().c_str());
	setSizeHint(settings->docclass);
	
	settings->pagestyle->insertItem(_("default"));
	settings->pagestyle->insertItem(_("empty"));
	settings->pagestyle->insertItem(_("plain"));
	settings->pagestyle->insertItem(_("headings"));
	settings->pagestyle->insertItem(_("fancy"));
	setSizeHint(settings->pagestyle);
	
	// available fonts
	for (int i=0; tex_fonts[i][0]; i++)
		settings->font->insertItem(tex_fonts[i]);
	setSizeHint(settings->font);

	settings->fontsize->insertItem(_("default"));
	settings->fontsize->insertItem(_("10 point"));
	settings->fontsize->insertItem(_("11 point"));
	settings->fontsize->insertItem(_("12 point"));
	setSizeHint(settings->fontsize);
	
	settings->linespacing->insertItem(_("single"));
	settings->linespacing->insertItem(_("1 1/2 spacing"));
	settings->linespacing->insertItem(_("double"));
	settings->linespacing->insertItem(_("custom"));
	setSizeHint(settings->linespacing);

	connect(settings->linespacing, SIGNAL(highlighted(const char *)),
		this, SLOT(linespacingChanged(const char *)));

	settings->paraspacing->insertItem(_("small"));
	settings->paraspacing->insertItem(_("medium"));
	settings->paraspacing->insertItem(_("big"));
	settings->paraspacing->insertItem(_("custom"));
	setSizeHint(settings->paraspacing);

	connect(settings->paraspacing, SIGNAL(highlighted(const char *)),
		this, SLOT(paraspacingChanged(const char *)));

	connect(settings->addspace, SIGNAL(toggled(bool)),
		this, SLOT(addspaceChanged(bool)));

	extra->first->insertItem(_("Here"));
	extra->first->insertItem(_("Bottom of page"));
	extra->first->insertItem(_("Top of page"));
	extra->first->insertItem(_("Separate page"));
	extra->first->insertItem(_("Not set"));
	extra->second->insertItem(_("Here"));
	extra->second->insertItem(_("Bottom of page"));
	extra->second->insertItem(_("Top of page"));
	extra->second->insertItem(_("Separate page"));
	extra->second->insertItem(_("Not set"));
	extra->third->insertItem(_("Here"));
	extra->third->insertItem(_("Bottom of page"));
	extra->third->insertItem(_("Top of page"));
	extra->third->insertItem(_("Separate page"));
	extra->third->insertItem(_("Not set"));
	extra->fourth->insertItem(_("Here"));
	extra->fourth->insertItem(_("Bottom of page"));
	extra->fourth->insertItem(_("Top of page"));
	extra->fourth->insertItem(_("Separate page"));
	extra->fourth->insertItem(_("Not set"));

	// ps driver options
	for (int i = 0; tex_graphics[i][0]; i++)
		extra->psdriver->insertItem(tex_graphics[i]);
	setSizeHint(extra->psdriver);

	geometry->papersize->insertItem(_("default"));
	geometry->papersize->insertItem(_("US letter"));
	geometry->papersize->insertItem(_("US legal"));
	geometry->papersize->insertItem(_("US executive"));
	geometry->papersize->insertItem("A3");
	geometry->papersize->insertItem("A4");
	geometry->papersize->insertItem("A5");
	geometry->papersize->insertItem("B3");
	geometry->papersize->insertItem("B4");
	geometry->papersize->insertItem("B5");
	setSizeHint(geometry->papersize);
	
	geometry->margins->insertItem(_("default"));
	geometry->margins->insertItem(_("A4 small margins"));
	geometry->margins->insertItem(_("A4 very small margins"));
	geometry->margins->insertItem(_("A4 very wide margins"));
	setSizeHint(geometry->margins);
	
#ifdef DO_USE_DEFAULT_LANGUAGE
	language->language->insertItem(_("default"));
#endif
	for (Languages::const_iterator cit = languages.begin();
		cit != languages.end(); ++cit)
		language->language->insertItem((*cit).second.lang().c_str());
	setSizeHint(language->language);
	
	language->encoding->insertItem(_("default"));
	language->encoding->insertItem(_("auto"));
	language->encoding->insertItem(_("latin1"));
	language->encoding->insertItem(_("latin2"));
	language->encoding->insertItem(_("latin5"));
	language->encoding->insertItem(_("koi8-r"));
	language->encoding->insertItem(_("koi8-u"));
	language->encoding->insertItem(_("cp866"));
	language->encoding->insertItem(_("cp1251"));
	language->encoding->insertItem(_("iso88595"));
	setSizeHint(language->encoding);
	
	language->quotes->insertItem(_("`text'"));
	language->quotes->insertItem(_("``text''"));
	language->quotes->insertItem(_("'text'"));
	language->quotes->insertItem(_("''text''"));
	language->quotes->insertItem(_(",text`"));
	language->quotes->insertItem(_(",,text``"));
	language->quotes->insertItem(_(",text'"));
	language->quotes->insertItem(_(",,text''"));
	language->quotes->insertItem(_("<text>"));
	language->quotes->insertItem(_("«text»"));
	language->quotes->insertItem(_(">text<"));
	language->quotes->insertItem(_("»text«"));
	setSizeHint(language->quotes);

	/* FIXME: bullets */
	
	QToolTip::add(settings->pagestyle, _("Specify header + footer style etc"));
	QToolTip::add(settings->addspace, _("Add spacing between paragraphs rather\n than indenting"));
	QToolTip::add(settings->linespacingVal, _("Custom line spacing in line units"));
	QToolTip::add(settings->extraoptions, _("Additional LaTeX options"));
	QToolTip::add(extra->first, _("Specify preferred order for\nplacing floats"));
	QToolTip::add(extra->second, _("Specify preferred order for\nplacing floats"));
	QToolTip::add(extra->third, _("Specify preferred order for\nplacing floats"));
	QToolTip::add(extra->fourth, _("Specify preferred order for\nplacing floats"));
	QToolTip::add(extra->ignore, _("Tell LaTeX to ignore usual rules\n for float placement"));
	QToolTip::add(extra->sectiondepth, _("How far in the (sub)sections are numbered"));
	QToolTip::add(extra->tocdepth, _("How detailed the Table of Contents is"));
	QToolTip::add(extra->psdriver, _("Program to produce PostScript output"));
	QToolTip::add(extra->amsmath, _("FIXME please !"));
	QToolTip::add(geometry->headheight, _("FIXME please !"));
	QToolTip::add(geometry->headsep, _("FIXME please !"));
	QToolTip::add(geometry->footskip, _("FIXME please !"));

	setUpdatesEnabled(true);
	update();
}


DocDialog::~DocDialog()
{
}


void DocDialog::closeEvent(QCloseEvent * e)
{
	form_->close();
	e->accept();
}


void DocDialog::setReadOnly(bool readonly)
{
	/* FIXME */
	cancel->setText(readonly ? _("&Close") : _("&Cancel"));
}


void DocDialog::setFromParams(BufferParams const & params)
{
	setUpdatesEnabled(false);
 
	if (!setComboFromStr(settings->docclass, textclasslist.DescOfClass(params.textclass)))
		lyxerr[Debug::GUI] << "Couldn't set docclass " << textclasslist.DescOfClass(params.textclass) << endl;

	if (!setComboFromStr(settings->font, params.fonts))
		lyxerr[Debug::GUI] << "Couldn't set font " << params.fonts << endl;

	LyXTextClass const & tclass = textclasslist.TextClass(params.textclass);
	
	// opt_fontsize is a string like "10|11|12"
	settings->fontsize->setCurrentItem(tokenPos(tclass.opt_fontsize(), '|', params.fontsize) + 1);

	// "empty|plain|headings|fancy"
	settings->pagestyle->setCurrentItem(tokenPos(tclass.opt_pagestyle(), '|', params.pagestyle) + 1);
	
	settings->addspace->setChecked(params.paragraph_separation == BufferParams::PARSEP_SKIP);

	bool const isskip = (params.paragraph_separation == BufferParams::PARSEP_SKIP);

	settings->paraspacing->setEnabled(isskip);
	settings->paraspacingValue->setEnabled(isskip);
	settings->paraspacingStretch->setEnabled(isskip);
	settings->paraspacingShrink->setEnabled(isskip);

	int item=0;

	switch (params.getDefSkip().kind()) {
		case VSpace::SMALLSKIP: item = 0; break;
		case VSpace::MEDSKIP: item = 1; break;
		case VSpace::BIGSKIP: item = 2; break;
		case VSpace::LENGTH: item = 3; break;
		default:
			lyxerr[Debug::GUI] << "Unknown defskip " << int(params.getDefSkip().kind()) << endl;
	}

	settings->paraspacing->setCurrentItem(item);

	settings->paraspacingValue->setEnabled(item == 3);
	settings->paraspacingStretch->setEnabled(item == 3);
	settings->paraspacingShrink->setEnabled(item == 3);

	if (item == 3) {
		LyXGlueLength const len = params.getDefSkip().length();
		settings->paraspacingValue->setValue(len.value());
		settings->paraspacingValue->setUnits(len.unit());
		settings->paraspacingStretch->setValue(len.plusValue());
		settings->paraspacingStretch->setUnits(len.plusUnit());
		settings->paraspacingShrink->setValue(len.minusValue());
		settings->paraspacingShrink->setUnits(len.minusUnit());
		lyxerr[Debug::GUI] << params.getDefSkip().asLyXCommand() << endl;;
	} else {
		settings->paraspacingValue->setFromLengthStr("0cm");
		settings->paraspacingStretch->setFromLengthStr("0cm");
		settings->paraspacingShrink->setFromLengthStr("0cm");
	}

	settings->sides->setChecked(params.sides == LyXTextClass::TwoSides);
	settings->columns->setChecked(params.columns == 2);

	switch (params.spacing.getSpace()) {
		case Spacing::Default:
		case Spacing::Single: item = 0; break;
		case Spacing::Onehalf: item = 1; break;
		case Spacing::Double: item = 2; break;
		case Spacing::Other: item = 3; break;
		default:
			lyxerr[Debug::GUI] << "Unknown line spacing " << int(params.spacing.getSpace()) << endl;
	}

	settings->linespacing->setCurrentItem(item);
	settings->linespacingVal->setEnabled(item == 3);
	
	if (item == 3)
		settings->linespacingVal->setText(tostr(params.spacing.getValue()).c_str());
	else
		settings->linespacingVal->setText("");

	if (params.options.empty())
		settings->extraoptions->setText("");
	else
		settings->extraoptions->setText(params.options.c_str());

	// geometry page

	geometry->papersize->setCurrentItem(params.papersize2);
	
	geometry->margins->setCurrentItem(params.paperpackage);
	
	geometry->portrait->setChecked(params.orientation == BufferParams::ORIENTATION_PORTRAIT);
	geometry->landscape->setChecked(params.orientation != BufferParams::ORIENTATION_PORTRAIT);
	
	geometry->width->setFromLengthStr(params.paperwidth);
	geometry->height->setFromLengthStr(params.paperheight);
	geometry->left->setFromLengthStr(params.leftmargin);
	geometry->right->setFromLengthStr(params.rightmargin);
	geometry->top->setFromLengthStr(params.topmargin);
	geometry->bottom->setFromLengthStr(params.bottommargin);
	geometry->headheight->setFromLengthStr(params.headheight);
	geometry->headsep->setFromLengthStr(params.headsep);
	geometry->footskip->setFromLengthStr(params.footskip);
	
	// language page

	if (!setComboFromStr(language->language, params.language->lang()))
		lyxerr[Debug::GUI] << "Couldn't set language " << params.language->lang() << endl;
	
	if (!setComboFromStr(language->encoding, params.inputenc))
		lyxerr[Debug::GUI] << "Couldn't set encoding " << params.inputenc << endl;

	switch (params.quotes_language) {
		case InsetQuotes::EnglishQ: item = 0; break;
		case InsetQuotes::SwedishQ: item = 2; break;
		case InsetQuotes::GermanQ: item = 4; break;
		case InsetQuotes::PolishQ: item = 6; break;
		case InsetQuotes::FrenchQ: item = 8; break;
		case InsetQuotes::DanishQ: item = 10; break;
		default:
			lyxerr[Debug::GUI] << "Unknown quote style " << int(params.quotes_language) << endl;
	}

	if (params.quotes_times == InsetQuotes::DoubleQ)
		item++;

	language->quotes->setCurrentItem(item);
	
	// extra page

	if (!setComboFromStr(extra->psdriver, params.graphicsDriver))
		lyxerr[Debug::GUI] << "Couldn't set psdriver " << params.graphicsDriver << endl;
	
	extra->amsmath->setChecked(params.use_amsmath);
	extra->sectiondepth->setValue(params.secnumdepth);
	extra->tocdepth->setValue(params.tocdepth);

	string const place = params.float_placement;
	int count = 0;
	QComboBox * box;

	extra->ignore->setChecked(false);
	setComboFromStr(extra->first, _("Not set"));
	setComboFromStr(extra->second, _("Not set"));
	setComboFromStr(extra->third, _("Not set"));
	setComboFromStr(extra->fourth, _("Not set"));

	for (string::const_iterator iter = place.begin(); iter != place.end(); ++count, ++iter) {
		switch (count) {
			case 0: box = extra->first; break;
			case 1: box = extra->second; break;
			case 2: box = extra->third; break;
			default: box = extra->fourth; break;
		};

		if (*iter == '!') {
			extra->ignore->setChecked(true);
			continue;
		}

		switch (*iter) {
			case 'h': setComboFromStr(box, _("Here")); break;
			case 'b': setComboFromStr(box, _("Bottom of page")); break;
			case 't': setComboFromStr(box, _("Top of page")); break;
			case 'p': setComboFromStr(box, _("Separate page")); break;
			default:
				lyxerr[Debug::GUI] << "Unknown float placement \'" << *iter << endl;
		}
	}

	/* FIXME: bullets ! */
 
	setUpdatesEnabled(true);
	update();
}


bool DocDialog::updateParams(BufferParams & params)
{
	bool redo = false;

	params.fonts = string(settings->font->currentText());
	LyXTextClass const & tclass = textclasslist.TextClass(params.textclass);
	params.fontsize = token(tclass.opt_fontsize(), '|', settings->fontsize->currentItem() - 1);
	params.pagestyle = token(tclass.opt_pagestyle(), '|', settings->pagestyle->currentItem() - 1);

	// set and update class

	unsigned int const new_class = settings->docclass->currentItem();

	if (new_class != params.textclass) {
		if (!form_->changeClass(params, new_class)) {
			// FIXME: error msg
			// restore old class
			if (!setComboFromStr(settings->docclass, textclasslist.DescOfClass(params.textclass)))
				lyxerr[Debug::GUI] << "Couldn't set docclass " << textclasslist.DescOfClass(params.textclass) << endl;
		} else
			redo = true;
	}

	BufferParams::PARSEP tmpsep = params.paragraph_separation;

	(settings->addspace->isChecked())
		? params.paragraph_separation = BufferParams::PARSEP_SKIP
		: params.paragraph_separation = BufferParams::PARSEP_INDENT;
		
	redo = (tmpsep != params.paragraph_separation) || redo;

	// the skip spacing

	VSpace tmpskip;

	switch (settings->paraspacing->currentItem()) {
		case 0: tmpskip = VSpace(VSpace::SMALLSKIP); break;
		case 1: tmpskip = VSpace(VSpace::MEDSKIP); break;
		case 2: tmpskip = VSpace(VSpace::BIGSKIP); break;
		default:
			lyxerr[Debug::GUI] << "Unknown skip spacing " <<
				settings->paraspacing->currentItem() << endl;
			break;
		case 3:
			string const val = settings->paraspacingValue->getLengthStr()
				+ "+" + settings->paraspacingStretch->getLengthStr()
				+ "-" + settings->paraspacingShrink->getLengthStr();
			lyxerr[Debug::GUI] << "Read para spacing of \"" << val << "\"" << endl;
			LyXGlueLength length(0.0, LyXLength::PT);
			isValidGlueLength(val, &length);
			tmpskip = VSpace(length);
			break;
	}
	
	if (!(tmpskip == params.getDefSkip())) {
		redo = true;
		params.setDefSkip(tmpskip);
	}

	// columns and sides

	(settings->sides->isChecked())
		? params.sides = LyXTextClass::TwoSides
		: params.sides = LyXTextClass::OneSide;

	(settings->columns->isChecked())
		? params.columns = 2
		: params.columns = 1;

	// line spacing

	Spacing tmpspacing = params.spacing;

	switch (settings->linespacing->currentItem()) {
		case 0: params.spacing.set(Spacing::Single); break;
		case 1: params.spacing.set(Spacing::Onehalf); break;
		case 2: params.spacing.set(Spacing::Double); break;
		case 3:
			params.spacing.set(Spacing::Other, settings->linespacingVal->text());
			break;
		default:
			lyxerr[Debug::GUI] << "Unknown line spacing " <<
				settings->linespacing->currentItem();
	}

	if (tmpspacing != params.spacing)
		redo = true;

	// extra options

	params.options = settings->extraoptions->text();
		
	
	// paper package and margin package
	params.papersize2 = static_cast<char>(geometry->papersize->currentItem());
	params.paperpackage = static_cast<char>(geometry->margins->currentItem());
	if (geometry->landscape->isChecked())
		params.orientation = BufferParams::ORIENTATION_LANDSCAPE;
	else
		params.orientation = BufferParams::ORIENTATION_PORTRAIT;

	params.paperwidth = geometry->width->getLengthStr();
	params.paperheight = geometry->height->getLengthStr();
	params.leftmargin = geometry->left->getLengthStr();
	params.rightmargin = geometry->right->getLengthStr();
	params.topmargin = geometry->top->getLengthStr();
	params.bottommargin = geometry->bottom->getLengthStr();
	params.headheight = geometry->headheight->getLengthStr();
	params.headsep = geometry->headsep->getLengthStr();
	params.footskip = geometry->footskip->getLengthStr();
	
	/* FIXME: is geometry required for headheight,sep,footskip ? */
	params.use_geometry =
		(params.paperwidth != "" ||
		params.paperheight != "" ||
		params.leftmargin != "" ||
		params.rightmargin != "" ||
		params.topmargin != "" ||
		params.bottommargin != "");


	// language dialog

	InsetQuotes::quote_language lga = InsetQuotes::EnglishQ;

	switch (language->quotes->currentItem()) {
		case 0: case 2: case 4: case 6: case 8: case 10:
			params.quotes_times = InsetQuotes::SingleQ;
			break;
		default:
			params.quotes_times = InsetQuotes::DoubleQ;
	}

	switch (language->quotes->currentItem()) {
		case 0: case 1: lga = InsetQuotes::EnglishQ; break;
		case 2: case 3: lga = InsetQuotes::SwedishQ; break;
		case 4: case 5: lga = InsetQuotes::GermanQ; break;
		case 6: case 7: lga = InsetQuotes::PolishQ; break;
		case 8: case 9: lga = InsetQuotes::FrenchQ; break;
		case 10: case 11: lga = InsetQuotes::DanishQ; break;
		default:
			lyxerr[Debug::GUI] << "unknown quotes style" <<
				language->quotes->currentItem() << endl;
	}

	params.quotes_language = lga;

	/* wow, tongue twister */
	Language const * old_language = params.language;
	Language const * new_language = languages.getLanguage(language->language->currentText());

	/* FIXME */
	if (old_language != new_language
		&& old_language->RightToLeft() == new_language->RightToLeft()
		/*&& !lv_->buffer()->isMultiLingual()*/) {
		//lv_->buffer()->ChangeLanguage(old_language, new_language);
	}
	
	redo = (old_language != new_language) || redo;
	params.language = new_language;

	params.inputenc = language->encoding->currentText();

	// extra dialog

	params.graphicsDriver = extra->psdriver->currentText();
	params.use_amsmath = extra->amsmath->isChecked();
	
	if (extra->sectiondepth->value() != params.secnumdepth) {
		redo = true;
		params.secnumdepth = extra->sectiondepth->value();
	}

	params.tocdepth = extra->tocdepth->value();

	string place;

	place += placementString(extra->first);
	place += placementString(extra->second);
	place += placementString(extra->third);
	place += placementString(extra->fourth);
	params.float_placement = place;

	/* FIXME: bullets */
	
	return redo;
}

string DocDialog::placementString(QComboBox * box) const
{
	if (!compare(box->currentText(), _("Here")))
		return "h";
	if (!compare(box->currentText(), _("Bottom of page")))
		return "b";
	if (!compare(box->currentText(), _("Top of page")))
		return "t";
	if (!compare(box->currentText(), _("Separate page")))
		return "p";
	return "";
}

void DocDialog::linespacingChanged(const char * sel)
{
	bool const custom = !compare(sel, _("custom"));

	settings->linespacingVal->setEnabled(custom);
}


void DocDialog::paraspacingChanged(const char * sel)
{
	bool const custom = !compare(sel, _("custom"));
	settings->paraspacingValue->setEnabled(custom);
	settings->paraspacingStretch->setEnabled(custom);
	settings->paraspacingShrink->setEnabled(custom);
}

void DocDialog::addspaceChanged(bool on)
{
	settings->paraspacing->setEnabled(on);
	on = (on && !compare(settings->paraspacing->currentText(),
			     _("custom")));
	settings->paraspacingValue->setEnabled(on);
	settings->paraspacingStretch->setEnabled(on);
	settings->paraspacingShrink->setEnabled(on);
}
