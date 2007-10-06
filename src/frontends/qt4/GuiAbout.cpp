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

#include "qt_helpers.h"
#include "gettext.h"
#include "version.h"

#include "support/filetools.h"
#include "support/Package.h"

#include <QtCore>
#include <QtGui>

using lyx::support::package;
using lyx::support::makeDisplayPath;


namespace lyx {
namespace frontend {

static QString credits()
{
	QString res;
	QFile file(toqstr(package().system_support().absFilename()) + "CREDITS");
	QTextStream out(&res);

	if (file.isReadable()) {
		out << toqstr(_("ERROR: LyX wasn't able to read CREDITS file\n"));
		out << toqstr(_("Please install correctly to estimate the great\n"));
		out << toqstr(_("amount of work other people have done for the LyX project."));
	} else {
		file.open(QIODevice::ReadOnly);
		QTextStream ts(&file);
		QString line;
		do {
			line = ts.readLine();
			if (line.startsWith("@b"))
				out << "<b>" << line.mid(2) << "</b>";
			else if (line.startsWith("@i"))
				out << "<i>" << line.mid(2) << "</i>";
			else
				out << line;
			out << "<br>";
		} while (!line.isNull());
	}
	out.flush();
	return res;
}


static QString copyright()
{
	return toqstr(_("LyX is Copyright (C) 1995 by Matthias Ettrich,\n1995-2006 LyX Team"));
}


static QString license()
{
	return toqstr(_("This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version."));
}


static QString disclaimer()
{
	return toqstr(_("LyX is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\nSee the GNU General Public License for more details.\nYou should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA."));
}


static QString version()
{
	QString res;
	QTextStream out(&res);
	out << toqstr(_("LyX Version "));
	out << lyx_version;
	out << " (";
	out << lyx_release_date;
	out << ")\n";
	out << toqstr(_("Library directory: "));
	out << toqstr(makeDisplayPath(package().system_support().absFilename()));
	out << "\n";
	out << toqstr(_("User directory: "));
	out << toqstr(makeDisplayPath(package().user_support().absFilename()));
	return res;
}


GuiAbout::GuiAbout(LyXView & lv)
	: GuiDialog(lv, "aboutlyx"), Controller(this)
{
	setupUi(this);
	setViewTitle(_("About LyX"));
	setController(this, false);

	connect(closePB, SIGNAL(clicked()), this, SLOT(reject()));

	copyrightTB->setPlainText(copyright());
	copyrightTB->append(QString());
	copyrightTB->append(license());
	copyrightTB->append(QString());
	copyrightTB->append(disclaimer());

	versionLA->setText(version());
	creditsTB->setHtml(credits());
}


Dialog * createGuiAbout(LyXView & lv) { return new GuiAbout(lv); }


} // namespace frontend
} // namespace lyx

#include "GuiAbout_moc.cpp"
