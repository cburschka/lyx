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

using kde_helpers::setSizeHint;
using kde_helpers::setComboFromStr;
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
		settings->combo_docclass->insertItem((*cit).description().c_str());
	setSizeHint(settings->combo_docclass);
	
	settings->combo_pagestyle->insertItem(_("default"));
	settings->combo_pagestyle->insertItem(_("empty"));
	settings->combo_pagestyle->insertItem(_("plain"));
	settings->combo_pagestyle->insertItem(_("headings"));
	settings->combo_pagestyle->insertItem(_("fancy"));
	setSizeHint(settings->combo_pagestyle);
	
	// available fonts
	for (int i=0; tex_fonts[i][0]; i++)
		settings->combo_font->insertItem(tex_fonts[i]);
	setSizeHint(settings->combo_font);

	settings->combo_fontsize->insertItem(_("default"));
	settings->combo_fontsize->insertItem(_("10 point"));
	settings->combo_fontsize->insertItem(_("11 point"));
	settings->combo_fontsize->insertItem(_("12 point"));
	setSizeHint(settings->combo_fontsize);
	
	settings->combo_linespacing->insertItem(_("single"));
	settings->combo_linespacing->insertItem(_("1 1/2 spacing"));
	settings->combo_linespacing->insertItem(_("double"));
	settings->combo_linespacing->insertItem(_("custom"));
	setSizeHint(settings->combo_linespacing);

	connect(settings->combo_linespacing, SIGNAL(highlighted(const char *)),
		this, SLOT(linespacingChanged(const char *)));

	settings->combo_paraspacing->insertItem(_("small"));
	settings->combo_paraspacing->insertItem(_("medium"));
	settings->combo_paraspacing->insertItem(_("big"));
	settings->combo_paraspacing->insertItem(_("custom"));
	setSizeHint(settings->combo_paraspacing);

	connect(settings->combo_paraspacing, SIGNAL(highlighted(const char *)),
		this, SLOT(paraspacingChanged(const char *)));

	connect(settings->check_addspace, SIGNAL(toggled(bool)),
		this, SLOT(addspaceChanged(bool)));

	extra->combo_first->insertItem(_("Here"));
	extra->combo_first->insertItem(_("Bottom of page"));
	extra->combo_first->insertItem(_("Top of page"));
	extra->combo_first->insertItem(_("Separate page"));
	extra->combo_first->insertItem(_("Not set"));
	extra->combo_second->insertItem(_("Here"));
	extra->combo_second->insertItem(_("Bottom of page"));
	extra->combo_second->insertItem(_("Top of page"));
	extra->combo_second->insertItem(_("Separate page"));
	extra->combo_second->insertItem(_("Not set"));
	extra->combo_third->insertItem(_("Here"));
	extra->combo_third->insertItem(_("Bottom of page"));
	extra->combo_third->insertItem(_("Top of page"));
	extra->combo_third->insertItem(_("Separate page"));
	extra->combo_third->insertItem(_("Not set"));
	extra->combo_fourth->insertItem(_("Here"));
	extra->combo_fourth->insertItem(_("Bottom of page"));
	extra->combo_fourth->insertItem(_("Top of page"));
	extra->combo_fourth->insertItem(_("Separate page"));
	extra->combo_fourth->insertItem(_("Not set"));

	// ps driver options
	for (int i = 0; tex_graphics[i][0]; i++)
		extra->combo_psdriver->insertItem(tex_graphics[i]);
	setSizeHint(extra->combo_psdriver);

	geometry->combo_papersize->insertItem(_("default"));
	geometry->combo_papersize->insertItem(_("US letter"));
	geometry->combo_papersize->insertItem(_("US legal"));
	geometry->combo_papersize->insertItem(_("US executive"));
	geometry->combo_papersize->insertItem("A3");
	geometry->combo_papersize->insertItem("A4");
	geometry->combo_papersize->insertItem("A5");
	geometry->combo_papersize->insertItem("B3");
	geometry->combo_papersize->insertItem("B4");
	geometry->combo_papersize->insertItem("B5");
	setSizeHint(geometry->combo_papersize);
	
	geometry->combo_margins->insertItem(_("default"));
	geometry->combo_margins->insertItem(_("A4 small margins"));
	geometry->combo_margins->insertItem(_("A4 very small margins"));
	geometry->combo_margins->insertItem(_("A4 very wide margins"));
	setSizeHint(geometry->combo_margins);
	
	for (Languages::const_iterator cit = languages.begin();
		cit != languages.end(); ++cit)
		language->combo_language->insertItem((*cit).second.lang().c_str());
	setSizeHint(language->combo_language);
	
	language->combo_encoding->insertItem(_("default"));
	language->combo_encoding->insertItem(_("auto"));
	language->combo_encoding->insertItem(_("latin1"));
	language->combo_encoding->insertItem(_("latin2"));
	language->combo_encoding->insertItem(_("latin5"));
	language->combo_encoding->insertItem(_("koi8-r"));
	language->combo_encoding->insertItem(_("koi8-u"));
	language->combo_encoding->insertItem(_("cp866"));
	language->combo_encoding->insertItem(_("cp1251"));
	language->combo_encoding->insertItem(_("iso88595"));
	setSizeHint(language->combo_encoding);
	
	language->combo_quotes->insertItem(_("`text'"));
	language->combo_quotes->insertItem(_("``text''"));
	language->combo_quotes->insertItem(_("'text'"));
	language->combo_quotes->insertItem(_("''text''"));
	language->combo_quotes->insertItem(_(",text`"));
	language->combo_quotes->insertItem(_(",,text``"));
	language->combo_quotes->insertItem(_(",text'"));
	language->combo_quotes->insertItem(_(",,text''"));
	language->combo_quotes->insertItem(_("<text>"));
	language->combo_quotes->insertItem(_("«text»"));
	language->combo_quotes->insertItem(_(">text<"));
	language->combo_quotes->insertItem(_("»text«"));
	setSizeHint(language->combo_quotes);

	/* FIXME: bullets */
	
	QToolTip::add(settings->combo_pagestyle, _("Specify header + footer style etc"));
	QToolTip::add(settings->check_addspace, _("Add spacing between paragraphs rather\n than indenting"));
	QToolTip::add(settings->combo_linespacing, _("Custom line spacing in line units"));
	QToolTip::add(settings->line_extraoptions, _("Additional LaTeX options"));
	QToolTip::add(extra->combo_first, _("Specify preferred order for\nplacing floats"));
	QToolTip::add(extra->combo_second, _("Specify preferred order for\nplacing floats"));
	QToolTip::add(extra->combo_third, _("Specify preferred order for\nplacing floats"));
	QToolTip::add(extra->combo_fourth, _("Specify preferred order for\nplacing floats"));
	QToolTip::add(extra->check_ignore, _("Tell LaTeX to ignore usual rules\n for float placement"));
	QToolTip::add(extra->spin_sectiondepth, _("How far in the (sub)sections are numbered"));
	QToolTip::add(extra->spin_tocdepth, _("How detailed the Table of Contents is"));
	QToolTip::add(extra->combo_psdriver, _("Program to produce PostScript output"));
	QToolTip::add(extra->check_amsmath, _("FIXME please !"));
	QToolTip::add(geometry->length_headheight, _("FIXME please !"));
	QToolTip::add(geometry->length_headsep, _("FIXME please !"));
	QToolTip::add(geometry->length_footskip, _("FIXME please !"));

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
	button_cancel->setText(readonly ? _("&Close") : _("&Cancel"));
}


void DocDialog::setFromParams(BufferParams const & params)
{
	setUpdatesEnabled(false);
 
	if (!setComboFromStr(settings->combo_docclass, textclasslist.DescOfClass(params.textclass)))
		lyxerr[Debug::GUI] << "Couldn't set docclass " << textclasslist.DescOfClass(params.textclass) << endl;

	if (!setComboFromStr(settings->combo_font, params.fonts))
		lyxerr[Debug::GUI] << "Couldn't set font " << params.fonts << endl;

	LyXTextClass const & tclass = textclasslist.TextClass(params.textclass);
	
	// opt_fontsize is a string like "10|11|12"
	settings->combo_fontsize->setCurrentItem(tokenPos(tclass.opt_fontsize(), '|', params.fontsize) + 1);

	// "empty|plain|headings|fancy"
	settings->combo_pagestyle->setCurrentItem(tokenPos(tclass.opt_pagestyle(), '|', params.pagestyle) + 1);
	
	settings->check_addspace->setChecked(params.paragraph_separation == BufferParams::PARSEP_SKIP);

	bool const isskip = (params.paragraph_separation == BufferParams::PARSEP_SKIP);

	settings->combo_paraspacing->setEnabled(isskip);
	settings->length_paraspacing->setEnabled(isskip);
	settings->length_paraspacingstretch->setEnabled(isskip);
	settings->length_paraspacingshrink->setEnabled(isskip);

	int item=0;

	switch (params.getDefSkip().kind()) {
		case VSpace::SMALLSKIP: item = 0; break;
		case VSpace::MEDSKIP: item = 1; break;
		case VSpace::BIGSKIP: item = 2; break;
		case VSpace::LENGTH: item = 3; break;
		default:
			lyxerr[Debug::GUI] << "Unknown defskip " << int(params.getDefSkip().kind()) << endl;
	}

	settings->combo_paraspacing->setCurrentItem(item);

	settings->length_paraspacing->setEnabled(item == 3);
	settings->length_paraspacingstretch->setEnabled(item == 3);
	settings->length_paraspacingshrink->setEnabled(item == 3);

	if (item == 3) {
		LyXGlueLength const len = params.getDefSkip().length();
		settings->length_paraspacing->setValue(len.value());
		settings->length_paraspacing->setUnits(len.unit());
		settings->length_paraspacingstretch->setValue(len.plusValue());
		settings->length_paraspacingstretch->setUnits(len.plusUnit());
		settings->length_paraspacingshrink->setValue(len.minusValue());
		settings->length_paraspacingshrink->setUnits(len.minusUnit());
		lyxerr[Debug::GUI] << params.getDefSkip().asLyXCommand() << endl;;
	} else {
		settings->length_paraspacing->setFromLengthStr("0cm");
		settings->length_paraspacingstretch->setFromLengthStr("0cm");
		settings->length_paraspacingshrink->setFromLengthStr("0cm");
	}

	settings->check_sides->setChecked(params.sides == LyXTextClass::TwoSides);
	settings->check_columns->setChecked(params.columns == 2);

	switch (params.spacing.getSpace()) {
		case Spacing::Default:
		case Spacing::Single: item = 0; break;
		case Spacing::Onehalf: item = 1; break;
		case Spacing::Double: item = 2; break;
		case Spacing::Other: item = 3; break;
		default:
			lyxerr[Debug::GUI] << "Unknown line spacing " << int(params.spacing.getSpace()) << endl;
	}

	settings->combo_linespacing->setCurrentItem(item);
	settings->line_linespacing->setEnabled(item == 3);
	
	if (item == 3)
		settings->line_linespacing->setText(tostr(params.spacing.getValue()).c_str());
	else
		settings->line_linespacing->setText("");

	if (params.options.empty())
		settings->line_extraoptions->setText("");
	else
		settings->line_extraoptions->setText(params.options.c_str());

	// geometry page

	geometry->combo_papersize->setCurrentItem(params.papersize2);
	
	geometry->combo_margins->setCurrentItem(params.paperpackage);
	
	geometry->radio_portrait->setChecked(params.orientation == BufferParams::ORIENTATION_PORTRAIT);
	geometry->radio_landscape->setChecked(params.orientation != BufferParams::ORIENTATION_PORTRAIT);
	
	geometry->length_width->setFromLengthStr(params.paperwidth);
	geometry->length_height->setFromLengthStr(params.paperheight);
	geometry->length_left->setFromLengthStr(params.leftmargin);
	geometry->length_right->setFromLengthStr(params.rightmargin);
	geometry->length_top->setFromLengthStr(params.topmargin);
	geometry->length_bottom->setFromLengthStr(params.bottommargin);
	geometry->length_headheight->setFromLengthStr(params.headheight);
	geometry->length_headsep->setFromLengthStr(params.headsep);
	geometry->length_footskip->setFromLengthStr(params.footskip);
	
	// language page

	if (!setComboFromStr(language->combo_language, params.language->lang()))
		lyxerr[Debug::GUI] << "Couldn't set language " << params.language->lang() << endl;
	
	if (!setComboFromStr(language->combo_encoding, params.inputenc))
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

	language->combo_quotes->setCurrentItem(item);
	
	// extra page

	if (!setComboFromStr(extra->combo_psdriver, params.graphicsDriver))
		lyxerr[Debug::GUI] << "Couldn't set psdriver " << params.graphicsDriver << endl;
	
	extra->check_amsmath->setChecked(params.use_amsmath);
	extra->spin_sectiondepth->setValue(params.secnumdepth);
	extra->spin_tocdepth->setValue(params.tocdepth);

	string const place = params.float_placement;
	int count = 0;
	QComboBox * box;

	extra->check_ignore->setChecked(false);
	setComboFromStr(extra->combo_first, _("Not set"));
	setComboFromStr(extra->combo_second, _("Not set"));
	setComboFromStr(extra->combo_third, _("Not set"));
	setComboFromStr(extra->combo_fourth, _("Not set"));

	for (string::const_iterator iter = place.begin(); iter != place.end(); ++count, ++iter) {
		switch (count) {
			case 0: box = extra->combo_first; break;
			case 1: box = extra->combo_second; break;
			case 2: box = extra->combo_third; break;
			default: box = extra->combo_fourth; break;
		};

		if (*iter == '!') {
			extra->check_ignore->setChecked(true);
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

	params.fonts = string(settings->combo_font->currentText());
	LyXTextClass const & tclass = textclasslist.TextClass(params.textclass);
	params.fontsize = token(tclass.opt_fontsize(), '|', settings->combo_fontsize->currentItem() - 1);
	params.pagestyle = token(tclass.opt_pagestyle(), '|', settings->combo_pagestyle->currentItem() - 1);

	// set and update class

	unsigned int const new_class = settings->combo_docclass->currentItem();

	if (new_class != params.textclass) {
		if (!form_->changeClass(params, new_class)) {
			// FIXME: error msg
			// restore old class
			if (!setComboFromStr(settings->combo_docclass, textclasslist.DescOfClass(params.textclass)))
				lyxerr[Debug::GUI] << "Couldn't set docclass " << textclasslist.DescOfClass(params.textclass) << endl;
		} else
			redo = true;
	}

	BufferParams::PARSEP tmpsep = params.paragraph_separation;

	(settings->check_addspace->isChecked())
		? params.paragraph_separation = BufferParams::PARSEP_SKIP
		: params.paragraph_separation = BufferParams::PARSEP_INDENT;
		
	redo = (tmpsep != params.paragraph_separation) || redo;

	// the skip spacing

	VSpace tmpskip;

	switch (settings->combo_paraspacing->currentItem()) {
		case 0: tmpskip = VSpace(VSpace::SMALLSKIP); break;
		case 1: tmpskip = VSpace(VSpace::MEDSKIP); break;
		case 2: tmpskip = VSpace(VSpace::BIGSKIP); break;
		default:
			lyxerr[Debug::GUI] << "Unknown skip spacing " <<
				settings->combo_paraspacing->currentItem() << endl;
			break;
		case 3:
			string const val = settings->length_paraspacing->getLengthStr()
				+ "+" + settings->length_paraspacingstretch->getLengthStr()
				+ "-" + settings->length_paraspacingshrink->getLengthStr();
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

	(settings->check_sides->isChecked())
		? params.sides = LyXTextClass::TwoSides
		: params.sides = LyXTextClass::OneSide;

	(settings->check_columns->isChecked())
		? params.columns = 2
		: params.columns = 1;

	// line spacing

	Spacing tmpspacing = params.spacing;

	switch (settings->combo_linespacing->currentItem()) {
		case 0: params.spacing.set(Spacing::Single); break;
		case 1: params.spacing.set(Spacing::Onehalf); break;
		case 2: params.spacing.set(Spacing::Double); break;
		case 3:
			params.spacing.set(Spacing::Other, settings->line_linespacing->text());
			break;
		default:
			lyxerr[Debug::GUI] << "Unknown line spacing " <<
				settings->combo_linespacing->currentItem();
	}

	if (tmpspacing != params.spacing)
		redo = true;

	// extra options

	params.options = settings->line_extraoptions->text();
		
	
	// paper package and margin package
	params.papersize2 = static_cast<char>(geometry->combo_papersize->currentItem());
	params.paperpackage = static_cast<char>(geometry->combo_margins->currentItem());
	if (geometry->radio_landscape->isChecked())
		params.orientation = BufferParams::ORIENTATION_LANDSCAPE;
	else
		params.orientation = BufferParams::ORIENTATION_PORTRAIT;

	params.paperwidth = geometry->length_width->getLengthStr();
	params.paperheight = geometry->length_height->getLengthStr();
	params.leftmargin = geometry->length_left->getLengthStr();
	params.rightmargin = geometry->length_right->getLengthStr();
	params.topmargin = geometry->length_top->getLengthStr();
	params.bottommargin = geometry->length_bottom->getLengthStr();
	params.headheight = geometry->length_headheight->getLengthStr();
	params.headsep = geometry->length_headsep->getLengthStr();
	params.footskip = geometry->length_footskip->getLengthStr();
	
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

	switch (language->combo_quotes->currentItem()) {
		case 0: case 2: case 4: case 6: case 8: case 10:
			params.quotes_times = InsetQuotes::SingleQ;
			break;
		default:
			params.quotes_times = InsetQuotes::DoubleQ;
	}

	switch (language->combo_quotes->currentItem()) {
		case 0: case 1: lga = InsetQuotes::EnglishQ; break;
		case 2: case 3: lga = InsetQuotes::SwedishQ; break;
		case 4: case 5: lga = InsetQuotes::GermanQ; break;
		case 6: case 7: lga = InsetQuotes::PolishQ; break;
		case 8: case 9: lga = InsetQuotes::FrenchQ; break;
		case 10: case 11: lga = InsetQuotes::DanishQ; break;
		default:
			lyxerr[Debug::GUI] << "unknown quotes style" <<
				language->combo_quotes->currentItem() << endl;
	}

	params.quotes_language = lga;

	/* wow, tongue twister */
	Language const * old_language = params.language;
	Language const * new_language = languages.getLanguage(language->combo_language->currentText());

	/* FIXME */
	if (old_language != new_language
		&& old_language->RightToLeft() == new_language->RightToLeft()
		/*&& !lv_->buffer()->isMultiLingual()*/) {
		//lv_->buffer()->ChangeLanguage(old_language, new_language);
	}
	
	redo = (old_language != new_language) || redo;
	params.language = new_language;

	params.inputenc = language->combo_encoding->currentText();

	// extra dialog

	params.graphicsDriver = extra->combo_psdriver->currentText();
	params.use_amsmath = extra->check_amsmath->isChecked();
	
	if (extra->spin_sectiondepth->value() != params.secnumdepth) {
		redo = true;
		params.secnumdepth = extra->spin_sectiondepth->value();
	}

	params.tocdepth = extra->spin_tocdepth->value();

	string place;

	place += placementString(extra->combo_first);
	place += placementString(extra->combo_second);
	place += placementString(extra->combo_third);
	place += placementString(extra->combo_fourth);
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

	settings->line_linespacing->setEnabled(custom);
}


void DocDialog::paraspacingChanged(const char * sel)
{
	bool const custom = !compare(sel, _("custom"));
	settings->length_paraspacing->setEnabled(custom);
	settings->length_paraspacingstretch->setEnabled(custom);
	settings->length_paraspacingshrink->setEnabled(custom);
}

void DocDialog::addspaceChanged(bool on)
{
	settings->combo_paraspacing->setEnabled(on);
	on = (on && !compare(settings->combo_paraspacing->currentText(),
			     _("custom")));
	settings->length_paraspacing->setEnabled(on);
	settings->length_paraspacingstretch->setEnabled(on);
	settings->length_paraspacingshrink->setEnabled(on);
}
