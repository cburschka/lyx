/**
 * \file qt4/alert_pimpl.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Alert_pimpl.h"
#include "alert.h"

#include "qt_helpers.h"

#include "ui_AskForTextUi.h"

#include "frontends/Application.h"

#include "gettext.h"

#include <QApplication>
#include <QMessageBox>
#include <QLabel>
#include <QLineEdit>
#include <QDialog>
#include <QInputDialog>

#include <algorithm>

using std::pair;
using std::make_pair;
using lyx::support::bformat;

namespace lyx {

static docstring const formatted(docstring const & text)
{
	const int w = 80;
	docstring sout;

	if (text.empty())
		return sout;

	docstring::size_type curpos = 0;
	docstring line;

	for (;;) {
		docstring::size_type const nxtpos1 = text.find(' ',  curpos);
		docstring::size_type const nxtpos2 = text.find('\n', curpos);
		docstring::size_type const nxtpos = std::min(nxtpos1, nxtpos2);

		docstring const word =
			nxtpos == docstring::npos ?
			text.substr(curpos) :
			text.substr(curpos, nxtpos - curpos);

		bool const newline = (nxtpos2 != docstring::npos &&
				      nxtpos2 < nxtpos1);

		docstring const line_plus_word =
			line.empty() ? word : line + char_type(' ') + word;

		// FIXME: make w be size_t
		if (int(line_plus_word.length()) >= w) {
			sout += line + char_type('\n');
			if (newline) {
				sout += word + char_type('\n');
				line.erase();
			} else {
				line = word;
			}

		} else if (newline) {
			sout += line_plus_word + char_type('\n');
			line.erase();

		} else {
			if (!line.empty())
				line += char_type(' ');
			line += word;
		}

		if (nxtpos == docstring::npos) {
			if (!line.empty())
				sout += line;
			break;
		}

		curpos = nxtpos + 1;
	}

	return sout;
}


int prompt_pimpl(docstring const & tit, docstring const & question,
		 int default_button, int cancel_button,
		 docstring const & b1, docstring const & b2, docstring const & b3)
{
	docstring const title = bformat(_("LyX: %1$s"), tit);

	QMessageBox mb;

	// For some reason, sometimes Qt uses an hourglass or watch cursor when
	// displaying the alert. Hence, we ask for the standard cursor shape.
	// This call has no effect if the cursor has not been overridden.
	qApp->changeOverrideCursor(Qt::ArrowCursor);

	// FIXME replace that with theApp->gui()->currentView()
	int res = QMessageBox::information(qApp->focusWidget(),
					   toqstr(title),
					   toqstr(formatted(question)),
					   toqstr(b1),
					   toqstr(b2),
					   b3.empty() ? QString::null : toqstr(b3),
					   default_button, cancel_button);

	// Qt bug: can return -1 on cancel or WM close, despite the docs.
	if (res == -1)
		res = cancel_button;
	return res;
}


void warning_pimpl(docstring const & tit, docstring const & message)
{
	docstring const title = bformat(_("LyX: %1$s"), tit);

	if (theApp() == 0) {
		int argc = 1;
		char * argv[1];
		QApplication app(argc, argv);
		QMessageBox::warning(0,
			toqstr(title),
			toqstr(formatted(message)));
		return;
	}
	QMessageBox::warning(qApp->focusWidget(),
			     toqstr(title),
			     toqstr(formatted(message)));
}


void error_pimpl(docstring const & tit, docstring const & message)
{
	docstring const title = bformat(_("LyX: %1$s"), tit);
	if (theApp() == 0) {
		int argc = 1;
		char * argv[1];
		QApplication app(argc, argv);
		QMessageBox::critical(0,
			toqstr(title),
			toqstr(formatted(message)));
		return;
	}
	QMessageBox::critical(qApp->focusWidget(),
			      toqstr(title),
			      toqstr(formatted(message)));
}


void information_pimpl(docstring const & tit, docstring const & message)
{
	docstring const title = bformat(_("LyX: %1$s"), tit);
	QMessageBox::information(qApp->focusWidget(),
				 toqstr(title),
				 toqstr(formatted(message)));
}


pair<bool, docstring> const
askForText_pimpl(docstring const & msg, docstring const & dflt)
{
	docstring const title = bformat(_("LyX: %1$s"), msg);

	bool ok;
	QString text = QInputDialog::getText(qApp->focusWidget(),
		toqstr(title),
		toqstr(char_type('&') + msg),
		QLineEdit::Normal,
		toqstr(dflt), &ok);

	if (ok && !text.isEmpty())
		return make_pair<bool, docstring>(true, qstring_to_ucs4(text));
	else
		return make_pair<bool, docstring>(false, docstring());
}


} // namespace lyx
