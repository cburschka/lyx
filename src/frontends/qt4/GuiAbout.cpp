/**
 * \file GuiAbout.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Kalle Dalheimer
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiAbout.h"

#include "ui_AboutUi.h"

#include "qt_helpers.h"
#include "version.h"

#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/Package.h"

#include <QDate>
#include <QFile>
#include <QTextStream>

using namespace lyx::support;
using lyx::support::package;
using lyx::support::makeDisplayPath;


namespace lyx {
namespace frontend {


static QDate release_date()
{
	return QDate::fromString(QString(lyx_release_date), Qt::ISODate);
}


static QString credits()
{
	QString res;
	QFile file(toqstr(package().system_support().absFileName()) + "/CREDITS");
	QTextStream out(&res);

	if (!file.exists()) {
		out << qt_("ERROR: LyX wasn't able to find the CREDITS file\n");
		out << qt_("Please install correctly to estimate the great\n");
		out << qt_("amount of work other people have done for the LyX project.");
	} else {
		file.open(QIODevice::ReadOnly);
		if (!file.isReadable()) {
			out << qt_("ERROR: LyX wasn't able to read the CREDITS file\n");
			out << qt_("Please install correctly to estimate the great\n");
			out << qt_("amount of work other people have done for the LyX project.");
		} else {
			QTextStream ts(&file);
			ts.setCodec("UTF-8");
			QString line;
			do {
				line = ts.readLine();
				if (line.startsWith("@b"))
					out << "<b>" << line.mid(2) << "</b>";
				else if (line.startsWith("@i")) {
					if (line.startsWith("@iE-mail")) {
						// unmask email
						line.replace(QString(" () "), QString("@"));
						line.replace(QString(" ! "), QString("."));
					}
					out << "<i>" << line.mid(2) << "</i>";
				} else
					out << line;
				out << "<br>";
			} while (!line.isNull());
		}
	}
	out.flush();
	return res;
}


static QString release_notes()
{
	QString res;
	QFile file(toqstr(package().system_support().absFileName()) + "/RELEASE-NOTES");
	QTextStream out(&res);

	if (!file.exists()) {
		out << qt_("ERROR: LyX wasn't able to find the RELEASE-NOTES file\n");
		out << qt_("Please install correctly to see what has changed\n");
		out << qt_("for this version of LyX.");
	} else {
		file.open(QIODevice::ReadOnly);
		if (!file.isReadable()) {
			out << qt_("ERROR: LyX wasn't able to read the RELEASE-NOTES file\n");
			out << qt_("Please install correctly to see what has changed\n");
			out << qt_("for this version of LyX.");
		} else {
			QTextStream ts(&file);
			ts.setCodec("UTF-8");
			QString line;
			bool incomment = false;
			bool inlist = false;
			do {
				// a simple markdown parser
				line = ts.readLine();
				// skipe empty lines
				if (line.isEmpty())
					continue;
				// parse (:comments:)
				if (line.startsWith("(:")) {
					if (!line.endsWith(":)"))
						incomment = true;
					continue;
				} if (line.endsWith(":)") && incomment) {
					incomment = false;
					continue;
				} if (incomment)
					continue;

				// detect links to the tracker
				line.replace(QRegExp("(bug )(\\#)(\\d+)*"),
					     "<a href=\"http://www.lyx.org/trac/ticket/\\3\">\\1\\3</a>");

				// headings
				if (line.startsWith("!!!")) {
					if (inlist) {
					    out << "</li>";
					    out << "</ul><br>";
					    inlist = false;
					}
					out << "<h4>" << line.mid(3) << "</h4>";
				}
				else if (line.startsWith("!!")) {
					if (inlist) {
					    out << "</li>";
					    out << "</ul><br>";
					    inlist = false;
					}
					out << "<h3>" << line.mid(2) << "</h3>";
				} else if (line.startsWith("!")) {
					if (inlist) {
					    out << "</li>";
					    out << "</ul><br>";
					    inlist = false;
					}
					out << "<h2>" << line.mid(1) << "</h2>";
				// lists
				} else if (line.startsWith("* ")) {
					if (inlist)
						out << "</li>";
					else
						out << "<ul>";
					inlist = true;
					out << "<li>" << line.mid(2);
				} else if (inlist && line.startsWith("  ")) {
					out << line.mid(2);
				} else if (inlist) {
					inlist = false;
					out << "</li>";
					out << "</ul><br>";
					out << line;
				} else
					out << line;

				out << " ";
			} while (!line.isNull());
		}
	}
	out.flush();
	return res;
}


static QString copyright()
{
	QString release_year = release_date().toString("yyyy");
	docstring copy_message =
		bformat(_("LyX is Copyright (C) 1995 by Matthias Ettrich,\n1995--%1$s LyX Team"),
			qstring_to_ucs4(release_year));
	return toqstr(copy_message);
}


static QString license()
{
	return qt_("This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.");
}


static QString disclaimer()
{
	return qt_("LyX is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\nSee the GNU General Public License for more details.\nYou should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.");
}


static QString version()
{
	QString loc_release_date;
	QDate date = release_date();
	if (date.isValid()) {
		QLocale loc;
		loc_release_date = loc.toString(date, QLocale::LongFormat);
	} else {
		if (QString(lyx_release_date) == "not released yet")
			loc_release_date = qt_("not released yet");
		else
			loc_release_date = toqstr(lyx_release_date);
	}
	docstring version_date =
		bformat(_("LyX Version %1$s\n(%2$s)"),
			from_ascii(lyx_version),
			qstring_to_ucs4(loc_release_date))+"\n";
	if (std::string(lyx_git_commit_hash) != "none")
		version_date += _("Built from git commit hash ")
			+ from_utf8(lyx_git_commit_hash).substr(0,8);
	version_date += "\n";

	QString res;
	QTextStream out(&res);
	out << toqstr(version_date);
	out << qt_("Library directory: ");
	out << toqstr(makeDisplayPath(package().system_support().absFileName()));
	out << "\n";
	out << qt_("User directory: ");
	out << toqstr(makeDisplayPath(package().user_support().absFileName()));
	out << "\n";
	out << toqstr(bformat(_("Qt Version (run-time): %1$s"), from_ascii(qVersion()))) << "\n";
	out << toqstr(bformat(_("Qt Version (compile-time): %1$s"), from_ascii(QT_VERSION_STR))) << "\n";
	return res;
}

static QString buildinfo()
{
	QString res;
	QTextStream out(&res);
	out << "LyX " << lyx_version
		<< " (" << lyx_release_date << ")" << endl;
	if (std::string(lyx_git_commit_hash) != "none")
		out << qt_("  Git commit hash ")
		    << QString(lyx_git_commit_hash).left(8) << endl;

	out << lyx_version_info << endl;
	return res;
}


struct GuiAbout::Private
{
	Ui::AboutUi ui;
};


GuiAbout::GuiAbout(GuiView & lv)
	: DialogView(lv, "aboutlyx", qt_("About LyX")),
	d(new GuiAbout::Private)
{
	d->ui.setupUi(this);

	d->ui.copyrightTB->setPlainText(copyright());
	d->ui.copyrightTB->append(QString());
	d->ui.copyrightTB->append(license());
	d->ui.copyrightTB->append(QString());
	d->ui.copyrightTB->append(disclaimer());

	d->ui.versionLA->setText(version());
	d->ui.buildinfoTB->setText(buildinfo());
	d->ui.releasenotesTB->setHtml(release_notes());
	d->ui.releasenotesTB->setOpenExternalLinks(true);
	d->ui.creditsTB->setHtml(credits());

	d->ui.tab->setUsesScrollButtons(false);
}


void GuiAbout::on_closePB_clicked()
{
	close();
}


Dialog * createGuiAbout(GuiView & lv) { return new GuiAbout(lv); }


} // namespace frontend
} // namespace lyx

#include "moc_GuiAbout.cpp"
