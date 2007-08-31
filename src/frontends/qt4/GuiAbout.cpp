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
#include "Qt2BC.h"
#include "qt_helpers.h"

#include "controllers/ButtonController.h"
#include "controllers/ControlAboutlyx.h"

#include "support/lstrings.h"

#include <sstream>

#include <QLabel>
#include <QPushButton>
#include <QTextCodec>
#include <QTextBrowser>

using lyx::support::prefixIs;

using std::getline;

using std::istringstream;
using std::ostringstream;
using std::string;

namespace lyx {
namespace frontend {


GuiAbout::GuiAbout(Dialog & parent)
	: GuiView<GuiAboutDialog>(parent, _("About LyX"))
{
}


void GuiAbout::build_dialog()
{
	dialog_.reset(new GuiAboutDialog);
	connect(dialog_.get()->closePB, SIGNAL(clicked()),
		this, SLOT(slotClose()));

	dialog_->copyrightTB->setPlainText(toqstr(controller().getCopyright()));
	dialog_->copyrightTB->append("");
	dialog_->copyrightTB->append(toqstr(controller().getLicense()));
	dialog_->copyrightTB->append("");
	dialog_->copyrightTB->append(toqstr(controller().getDisclaimer()));

	dialog_->versionLA->setText(toqstr(controller().getVersion()));

	// The code below should depend on a autoconf test. (Lgb)
#if 1
	// There are a lot of buggy stringstream implementations..., but the
	// code below will work on all of them (I hope). The drawback with
	// this solutions os the extra copying. (Lgb)

	ostringstream in;
	controller().getCredits(in);

	istringstream ss(in.str());

	string s;
	ostringstream out;

	while (getline(ss, s)) {
		if (prefixIs(s, "@b"))
			out << "<b>" << s.substr(2) << "</b>";
		else if (prefixIs(s, "@i"))
			out << "<i>" << s.substr(2) << "</i>";
		else
			out << s;
		out << "<br>";
	}
#else
	// Good stringstream implementations can handle this. It avoids
	// some copying, and should thus be faster and use less memory. (Lgb)
	// I'll make this the default for a short while to see if anyone
	// see the error...
	stringstream in;
	controller().getCredits(in);
	in.seekg(0);
	string s;
	ostringstream out;

	while (getline(in, s)) {
		if (prefixIs(s, "@b"))
			out << "<b>" << s.substr(2) << "</b>";
		else if (prefixIs(s, "@i"))
			out << "<i>" << s.substr(2) << "</i>";
		else
			out << s;
		out << "<br>";
	}
#endif

	dialog_->creditsTB->setHtml(toqstr(out.str()));

	// try to resize to a good size
	dialog_->copyrightTB->hide();
	dialog_->setMinimumSize(dialog_->copyrightTB->sizeHint());
	dialog_->copyrightTB->show();
	dialog_->setMinimumSize(dialog_->sizeHint());

	// Manage the cancel/close button
	bcview().setCancel(dialog_->closePB);
	bc().refresh();
}

} // namespace frontend
} // namespace lyx

#include "GuiAbout_moc.cpp"
