/**
 * \file GPrint.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Huang Ying
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

#include "GPrint.h"
#include "ControlPrint.h"
#include "ghelpers.h"

#include "PrinterParams.h"

#include "support/convert.h"
#include "support/lstrings.h"

#include <libglademm.h>

using std::string;
using namespace lyx::support;

namespace lyx {
namespace frontend {

GPrint::GPrint(Dialog & parent, string const & title)
	: GViewCB<ControlPrint, GViewGladeB>(parent, title, false)
{
}


void GPrint::apply()
{
	PrinterParams pp;
	pp.target = printer_->get_active() ? PrinterParams::PRINTER : PrinterParams::FILE;
	pp.printer_name = printerEntry_->get_text();
	pp.file_name = fileEntry_->get_text();
	pp.all_pages = all_->get_active();
	pp.from_page = pp.to_page = 0;
	if (!fromEntry_->get_text().empty()) {
		pp.from_page = convert<int>(fromEntry_->get_text());
		if (!toEntry_->get_text().empty())
			pp.to_page = convert<int>(toEntry_->get_text());
	}
	pp.odd_pages = odd_->get_active();
	pp.even_pages = even_->get_active();
	pp.count_copies = number_->get_value_as_int();
	pp.sorted_copies = sorted_->get_active();
	pp.reverse_order = reverse_->get_active();
	controller().params() = pp;
}


void GPrint::update()
{
	PrinterParams & pp = controller().params();
	printer_->set_active(pp.target == PrinterParams::PRINTER);
	printerEntry_->set_text(pp.printer_name);
	fileEntry_->set_text(pp.file_name);
	all_->set_active(pp.all_pages);

	string const from = ( pp.from_page ? convert<string>(pp.from_page) : string() );
	string const to   = ( pp.to_page   ? convert<string>(pp.to_page)   : string() );
	fromEntry_->set_text(from);
	toEntry_->set_text(to);
	odd_->set_active(pp.odd_pages);
	even_->set_active(pp.even_pages);
	reverse_->set_active(pp.reverse_order);
	sorted_->set_active(pp.sorted_copies);
	number_->set_value(pp.count_copies);
	bool const enable_counter = pp.target == PrinterParams::PRINTER;
	number_->set_sensitive(enable_counter);
	sorted_->set_sensitive(enable_counter && pp.count_copies > 1);
}


void GPrint::updateUI()
{
	ButtonPolicy::SMInput activate = ButtonPolicy::SMI_VALID;
	// disable OK/Apply buttons when file output is selected, but no file name entered
	if (file_->get_active() && fileEntry_->get_text().empty())
		activate = ButtonPolicy::SMI_INVALID;
	// check 'from' and 'to' fields only when 'from/to' radio button is selected
	if (fromTo_->get_active()) {
		string from = fromEntry_->get_text();
		string to = toEntry_->get_text();
		if (from.empty() || (!to.empty() && convert<int>(from) > convert<int>(to)))
			activate = ButtonPolicy::SMI_INVALID;
	}
	bool const enableCounter = printer_->get_active();
	number_->set_sensitive(enableCounter);
	bool const enableSorted = enableCounter && number_->get_value_as_int() > 1;
	sorted_->set_sensitive(enableSorted);
	bc().input(activate);
}


void GPrint::onBrowse()
{
	string const inName = fileEntry_->get_text();
	string const outName = Glib::locale_to_utf8(controller().browse(Glib::locale_from_utf8(inName)));
	if (outName != inName && !outName.empty())
		fileEntry_->set_text(outName);
	if (!outName.empty())
		file_->set_active(true);
	updateUI();
}


void GPrint::onTargetEdit(Gtk::Entry const * who)
{
	if (who == fileEntry_)
		file_->set_active(true);
	else if (who == printerEntry_)
		printer_->set_active(true);
	updateUI();
}


void GPrint::onFromToEdit()
{
	fromTo_->set_active(true);
	updateUI();
}


void GPrint::doBuild()
{
	string const gladeName = findGladeFile("print");
	xml_ = Gnome::Glade::Xml::create(gladeName);
	xml_->get_widget("Printer", printer_);
	xml_->get_widget("File", file_);
	xml_->get_widget("All", all_);
	xml_->get_widget("FromTo", fromTo_);
	xml_->get_widget("Odd", odd_);
	xml_->get_widget("Even", even_);
	xml_->get_widget("Reverse", reverse_);
	xml_->get_widget("Number", number_);
	xml_->get_widget("Collate", sorted_);
	xml_->get_widget("FromEntry", fromEntry_);
	xml_->get_widget("ToEntry", toEntry_);
	xml_->get_widget("PrinterEntry", printerEntry_);
	xml_->get_widget("FileEntry", fileEntry_);

	Gtk::Button * ok;
	Gtk::Button * cancel;
	xml_->get_widget("PrintButton", ok);
	xml_->get_widget("CancelButton", cancel);

	setOK(ok);
	setCancel(cancel);

	Gtk::Button * browse;
	xml_->get_widget("Browse", browse);
	browse->signal_clicked().connect(sigc::mem_fun(*this, &GPrint::onBrowse));

	fileEntry_->signal_changed().connect(sigc::bind(sigc::mem_fun(*this, &GPrint::onTargetEdit), fileEntry_));
	printerEntry_->signal_changed().connect(sigc::bind(sigc::mem_fun(*this, &GPrint::onTargetEdit), printerEntry_));
	fromEntry_->signal_changed().connect(sigc::mem_fun(*this, &GPrint::onFromToEdit));
	toEntry_->signal_changed().connect(sigc::mem_fun(*this, &GPrint::onFromToEdit));
	printer_->signal_toggled().connect(sigc::mem_fun(*this, &GPrint::updateUI));
	file_->signal_toggled().connect(sigc::mem_fun(*this, &GPrint::updateUI));
	all_->signal_toggled().connect(sigc::mem_fun(*this, &GPrint::updateUI));
	fromTo_->signal_toggled().connect(sigc::mem_fun(*this, &GPrint::updateUI));
	number_->signal_changed().connect(sigc::mem_fun(*this, &GPrint::updateUI));

	controller().initialiseParams("");
	update();
	updateUI();
}

} // namespace frontend
} // namespace lyx
