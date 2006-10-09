/**
 * \file GInclude.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

// Too hard to make concept checks work with this file
#ifdef _GLIBCXX_CONCEPT_CHECKS
#undef _GLIBCXX_CONCEPT_CHECKS
#endif
#ifdef _GLIBCPP_CONCEPT_CHECKS
#undef _GLIBCPP_CONCEPT_CHECKS
#endif

#include "GInclude.h"
#include "ControlInclude.h"
#include "ghelpers.h"

#include <libglademm.h>

using std::string;
using std::vector;

namespace lyx {
namespace frontend {

GInclude::GInclude(Dialog & parent)
	: GViewCB<ControlInclude, GViewGladeB>(parent, _("Child Document"), false)
{}


void GInclude::doBuild()
{
	string const gladeName = findGladeFile("include");
	xml_ = Gnome::Glade::Xml::create(gladeName);

	Gtk::Button * button;
	xml_->get_widget("Cancel", button);
	setCancel(button);
	xml_->get_widget("Ok", button);
	setOK(button);

	xml_->get_widget("Browse", button);
	button->signal_clicked().connect(
		sigc::mem_fun(*this, &GInclude::onBrowseClicked));

	xml_->get_widget("Open", openbutton_);
	openbutton_->signal_clicked().connect(
		sigc::mem_fun(*this, &GInclude::onOpenClicked));

	xml_->get_widget("Include", includeradio_);
	xml_->get_widget("Input", inputradio_);
	xml_->get_widget("Verbatim", verbatimradio_);
	xml_->get_widget("File", fileentry_);
	xml_->get_widget("MarkSpaces", markspacescheck_);
	xml_->get_widget("Preview", previewcheck_);

	inputradio_->signal_toggled().connect(
		sigc::mem_fun(*this, &GInclude::onTypeToggled));
	includeradio_->signal_toggled().connect(
		sigc::mem_fun(*this, &GInclude::onTypeToggled));
	verbatimradio_->signal_toggled().connect(
		sigc::mem_fun(*this, &GInclude::onTypeToggled));
}


void GInclude::update()
{
	string const filename = controller().params().getContents();
	fileentry_->set_text(filename);

	string const cmdname = controller().params().getCmdName();

	bool const inputCommand = (cmdname == "input" || cmdname.empty());
	bool const includeCommand = cmdname == "include";
	bool const verbatimStarCommand = cmdname == "verbatiminput*";
	bool const verbatimCommand = cmdname == "verbatiminput";

	bool const preview = static_cast<bool>((controller().params().preview()));

	previewcheck_->set_sensitive(inputCommand);
	previewcheck_->set_active(inputCommand ? preview : false);

	if (inputCommand)
		inputradio_->set_active(true);

	if (includeCommand)
		includeradio_->set_active(true);

	if (verbatimCommand || verbatimStarCommand) {
		verbatimradio_->set_active(true);
		markspacescheck_->set_active(verbatimStarCommand);
		markspacescheck_->set_sensitive(true);
		openbutton_->set_sensitive(false);
	} else {
		markspacescheck_->set_active(false);
		markspacescheck_->set_sensitive(false);
		openbutton_->set_sensitive(true);
	}

	bc().valid();
}


void GInclude::apply()
{
	InsetCommandParams params = controller().params();

	params.preview(previewcheck_->get_active());
	params.setContents(fileentry_->get_text());

	if (includeradio_->get_active())
		params.setCmdName("include");
	else if (inputradio_->get_active())
		params.setCmdName("input");
	else
		if (markspacescheck_->get_active())
			params.setCmdName("verbatiminput*");
		else
			params.setCmdName("verbatiminput");

	controller().setParams(params);
}


void GInclude::onTypeToggled()
{
	previewcheck_->set_sensitive(inputradio_->get_active());
	markspacescheck_->set_sensitive(verbatimradio_->get_active());
	openbutton_->set_sensitive(!verbatimradio_->get_active());
}


void GInclude::onBrowseClicked()
{
	ControlInclude::Type type;
	if (includeradio_->get_active())
		type = ControlInclude::INCLUDE;
	else if (inputradio_->get_active())
		type = ControlInclude::INPUT;
	else
		type = ControlInclude::VERBATIM;

	fileentry_->set_text(controller().browse(fileentry_->get_text(), type));
}


void GInclude::onOpenClicked()
{
	string const in_name = fileentry_->get_text();
	if (!in_name.empty() && controller().fileExists(in_name)) {
		dialog().OKButton();
		controller().load(in_name);
	}
}


} // namespace frontend
} // namespace lyx
