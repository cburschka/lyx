/**
 * \file GExternal.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Bernhard Reiter
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

#include "GExternal.h"
#include "ControlExternal.h"

#include "GtkLengthEntry.h"

#include "ghelpers.h"

#include "support/lstrings.h"
#include "support/convert.h"
#include "support/filefilterlist.h"

#include <libglademm.h>

#include <vector>
#include <string>

using lyx::support::token;
using lyx::support::FileFilterList;
using lyx::support::trim;

using std::string;


namespace lyx {
namespace frontend {

namespace {

class formatModelColumns : public Gtk::TreeModel::ColumnRecord
{
public:

	formatModelColumns() { add(name); add(extra); }

	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<Glib::ustring> extra;
};


class displayModelColumns : public Gtk::TreeModel::ColumnRecord
{
public:

	displayModelColumns() { add(name); add(type); }

	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<external::DisplayType> type;
};


class templateModelColumns : public Gtk::TreeModel::ColumnRecord
{
public:

	templateModelColumns() { add(name); add(info); add(filters);}

	Gtk::TreeModelColumn<Glib::ustring> name;
	Gtk::TreeModelColumn<Glib::ustring> info;
	Gtk::TreeModelColumn<std::vector<Gtk::FileFilter* > > filters;

};


formatModelColumns formatColumns;


/* ...ModelColumns singletons, cf. Scott Meyers, Effective C++, Item 47
 * Needed for proper initialization of the displayModelColumns::type member
 * which in turn needs to be Glib::Value<>::init'ed in time
 */
displayModelColumns& displayColumns() {
	static displayModelColumns dC;
	return dC;
}


templateModelColumns& templateColumns() {
	static templateModelColumns tC;
	return tC;
}


/// Produces a vector of Gtk::FileFilter*s out of the controller's filterstring
/// providing sensible descriptions (with alttitle as a fallback)
std::vector<Gtk::FileFilter* > get_filters(const std::string & filterstring, const std::string & alttitle)
{
	FileFilterList filterlist(filterstring);
	std::vector<Gtk::FileFilter* > filters(filterlist.size());

	for (unsigned int i=0; i < filterlist.size(); ++i) {
		FileFilterList::Filter ff = filterlist[i];

		filters[i] = new Gtk::FileFilter();
		std::string description = ff.description();

		if (description.empty())
			filters[i]->set_name(Glib::locale_to_utf8(alttitle) +
					     " " + lyx::to_utf8(_("files")));
		else
			filters[i]->set_name(Glib::locale_to_utf8(description));

		for (FileFilterList::Filter::glob_iterator git = ff.begin();
			git!=ff.end(); ++git)
			filters[i]->add_pattern(Glib::locale_to_utf8(*git));
	}
	return filters;
}


void set_display(Gtk::CheckButton * show_check, Gtk::ComboBox * display_combo,
		 Gtk::Label * display_label, GtkLengthEntry * scale_length,
		 Gtk::Label * scale_label,
		 external::DisplayType display, unsigned int scale)
{

	typedef Gtk::TreeModel::const_iterator gcit;
	Glib::RefPtr<const Gtk::TreeModel> const display_store =
		display_combo->get_model();
	bool const no_display = display == external::NoDisplay;

	if (no_display)
		display_combo->set_active(
			*(display_store->children().begin())); //Default
	else
		for (gcit it = display_store->children().begin();
		     it != display_store->children().end(); ++it) {
			if ((*it)[displayColumns().type] == display) {
				display_combo->set_active(*it);
				break;
			}
		}

	scale_length->get_spin()->set_value(scale);
	show_check->set_active(!no_display);

	display_label->set_sensitive(!no_display);
	display_combo->set_sensitive(!no_display);
	scale_label->set_sensitive(!no_display);
	scale_length->set_sensitive(!no_display);
}


void get_display(external::DisplayType & display,
		 unsigned int & scale,
		 Gtk::CheckButton const * show_check,
		 Gtk::ComboBox const * display_combo,
		 GtkLengthEntry const * scale_length)
{
	display = (*(display_combo->get_active()))[displayColumns().type];
	if (!show_check->get_active())
		display = external::NoDisplay;
	scale = scale_length->get_spin()->get_value_as_int();
}


void set_rotation(Gtk::Entry * angle_entry, Gtk::ComboBox * origin_combo,
		 external::RotationData const & data)
{
	origin_combo->set_active(int(data.origin()));
	angle_entry->set_text(Glib::locale_to_utf8(data.angle));
}


void get_rotation(external::RotationData & data,
		  Gtk::Entry const * angle_entry,
		  Gtk::ComboBox const * origin_combo)
{
	typedef external::RotationData::OriginType OriginType;

	data.origin(static_cast<OriginType>(
		origin_combo->get_active_row_number()));
	data.angle = Glib::locale_from_utf8(angle_entry->get_text());
}


void set_size(GtkLengthEntry * width_length,
	      GtkLengthEntry * height_length,
	      Gtk::CheckButton * ar_check,
	      external::ResizeData const & data)
{
	bool using_scale = data.usingScale();
	double scale = convert<double>(data.scale);
	if (data.no_resize()) {
		// Everything is zero, so default to this!
		using_scale = true;
		scale = 100.0;
	}

	if (using_scale) {
		width_length->get_spin()->set_value(scale);
		width_length->get_combo()->set_active_text(
							   lyx::to_utf8(_("Scale%")));
	} else {
		width_length->set_length(data.width);
	}

	height_length->set_length(data.height);
	if (!data.width.zero())
		height_length->get_combo()->set_active(data.width.unit());

	height_length->set_sensitive(!using_scale);

	ar_check->set_active(data.keepAspectRatio);

	bool const disable_aspectRatio = using_scale ||
		data.width.zero() || data.height.zero();
	ar_check->set_sensitive(!disable_aspectRatio);
}


void get_size(external::ResizeData & data,
	      GtkLengthEntry * width_length,
	      GtkLengthEntry * height_length,
	      Gtk::CheckButton * ar_check)
{
	if (width_length->get_combo()->get_active_text() !=
	    lyx::to_utf8(_("Scale%"))) {

		data.width = width_length->get_length();
		data.scale = string();
	} else {
		// scaling instead of a width
		data.scale = convert<string>(width_length->get_spin()->get_value());
		data.width = LyXLength();
	}

	data.height = height_length->get_length();

	data.keepAspectRatio = ar_check->get_active();
}


void set_crop(Gtk::CheckButton * clip_check,
	      Gtk::Entry * xl_entry, Gtk::Entry * yb_entry,
	      Gtk::Entry * xr_entry, Gtk::Entry * yt_entry,
	      external::ClipData const & data)
{
	clip_check->set_active(data.clip);
	graphics::BoundingBox const & bbox = data.bbox;
	xl_entry->set_text(Glib::locale_to_utf8(convert<string>(bbox.xl)));
	yb_entry->set_text(Glib::locale_to_utf8(convert<string>(bbox.yb)));
	xr_entry->set_text(Glib::locale_to_utf8(convert<string>(bbox.xr)));
	yt_entry->set_text(Glib::locale_to_utf8(convert<string>(bbox.yt)));

}


void get_crop(external::ClipData & data,
	      Gtk::CheckButton const * clip_check,
	      Gtk::Entry const * xl_entry, Gtk::Entry const * yb_entry,
	      Gtk::Entry const * xr_entry, Gtk::Entry const * yt_entry,
	      bool bb_changed)
{
	data.clip = clip_check->get_active();

	if (!bb_changed)
		return;

	data.bbox.xl = convert<int>(Glib::locale_from_utf8(xl_entry->get_text()));
	data.bbox.yb = convert<int>(Glib::locale_from_utf8(yb_entry->get_text()));
	data.bbox.xr = convert<int>(Glib::locale_from_utf8(xr_entry->get_text()));
	data.bbox.yt = convert<int>(Glib::locale_from_utf8(yt_entry->get_text()));
}


void get_extra(external::ExtraData & data,
	       Glib::RefPtr<Gtk::ListStore> format_store)
{
	Gtk::TreeModel::iterator it  = format_store->children().begin();
	Gtk::TreeModel::iterator end = format_store->children().end();
	for (; it != end; ++it)
		data.set(Glib::locale_from_utf8((*it)[formatColumns.name]),
		trim(Glib::locale_from_utf8((*it)[formatColumns.extra])));
}

} // namespace anon


GExternal::GExternal(Dialog & parent)
	: GViewCB<ControlExternal, GViewGladeB>(parent, _("External Settings"), false)
{}


void GExternal::doBuild()
{
	string const gladeName = findGladeFile("external");
	xml_ = Gnome::Glade::Xml::create(gladeName);

	xml_->get_widget("Cancel", cancelbutton_);
	setCancel(cancelbutton_);
	xml_->get_widget("Apply", applybutton_);
	setApply(applybutton_);
	xml_->get_widget("OK", okbutton_);
	setOK(okbutton_);

	xml_->get_widget("notebook", notebook_);

	templatestore_ = Gtk::ListStore::create(templateColumns());

	std::vector<string> templates(controller().getTemplates());
	int count = 0;

	std::vector<std::vector<Gtk::FileFilter> > myfilterlist;

	// Fill the templates combo
	for (std::vector<string>::const_iterator cit = templates.begin();
		cit != templates.end(); ++cit, ++count) {
		external::Template templ = controller().getTemplate(count);

		Gtk::TreeModel::iterator iter = templatestore_->append();
		(*iter)[templateColumns().name] = Glib::locale_to_utf8(*cit);
		(*iter)[templateColumns().info] =
			Glib::locale_to_utf8(templ.helpText);
		(*iter)[templateColumns().filters] = get_filters(
			controller().getTemplateFilters(*cit),*cit);
	}


	xml_->get_widget("Template", templatecombo_);
	templatecombo_->pack_start(templateColumns().name);
	templatecombo_->set_model(templatestore_);
	templatecombo_->signal_changed().connect(
		sigc::mem_fun(*this, &GExternal::template_changed));

	xml_->get_widget("TemplateView", templateview_);

	templatebuffer_ = Gtk::TextBuffer::create();
	templateview_->set_buffer(templatebuffer_);

	// *** Start "File" Page ***
	xml_->get_widget("TemplateFile", templatefcbutton_);
	templatefcbutton_->set_title(lyx::to_utf8(_("Select external file")));
	templatefcbutton_->signal_file_activated().connect(
		sigc::mem_fun(*this, &GExternal::file_changed));

	xml_->get_widget("FileLabel", filelabel_);
	filelabel_->set_mnemonic_widget(*templatefcbutton_);

	xml_->get_widget("Draft", draftcheck_);
	xml_->get_widget("EditFile", editfilebutton_);
	editfilebutton_->signal_clicked().connect(
		sigc::mem_fun(*this, &GExternal::edit_clicked));
	// *** End "File" Page ***

	// *** Start "LyX View" Page ***
	xml_->get_widget("ShowInLyX", showcheck_);
	showcheck_->signal_toggled().connect(
		sigc::mem_fun(*this, &GExternal::showcheck_toggled));

	xml_->get_widget("DisplayLabel", displaylabel_);

	displaystore_ = Gtk::ListStore::create(displayColumns());

	// Fill the display combo
	Gtk::TreeModel::iterator iter = displaystore_->append();
	(*iter)[displayColumns().name] = lyx::to_utf8(_("Default"));
	(*iter)[displayColumns().type] = external::DefaultDisplay;
	iter = displaystore_->append();
	(*iter)[displayColumns().name] = lyx::to_utf8(_("Monochrome"));
	(*iter)[displayColumns().type] = external::MonochromeDisplay;
	iter = displaystore_->append();
	(*iter)[displayColumns().name] = lyx::to_utf8(_("Grayscale"));
	(*iter)[displayColumns().type] = external::GrayscaleDisplay;
	iter = displaystore_->append();
	(*iter)[displayColumns().name] = lyx::to_utf8(_("Color"));
	(*iter)[displayColumns().type] = external::ColorDisplay;
	iter = displaystore_->append();
	(*iter)[displayColumns().name] = lyx::to_utf8(_("Preview"));
	(*iter)[displayColumns().type] = external::PreviewDisplay;

	xml_->get_widget("Display", displaycombo_);
	displaycombo_->set_model(displaystore_);
	displaycombo_->pack_start(displayColumns().name);

	xml_->get_widget_derived ("Scale", scalelength_);
	scalespin_ = scalelength_->get_spin();
	scalespin_->set_digits(0);
	scalespin_->set_range(0,100);
	scalespin_->set_increments(1,10);
	scalecombo_ = scalelength_->get_combo();
	scalecombo_->clear();
	scalecombo_->append_text(lyx::to_utf8(_("Scale%")));
	scalecombo_->set_active_text(lyx::to_utf8(_("Scale%")));

	xml_->get_widget("ScaleLabel", scalelabel_);
	scalelabel_->set_mnemonic_widget(*scalespin_);
	// *** End "LyX View" Page ***

	// *** Start "Rotate" Page ***
	xml_->get_widget("Angle", angleentry_);

	Gtk::Box * box = NULL;
	xml_->get_widget("OriginBox", box);
	box->pack_start(origincombo_, true, true, 0);
	box->show_all();

	// Fill the origins combo
	typedef std::vector<external::RotationDataType> Origins;
	Origins const & all_origins = external::all_origins();
	for (Origins::size_type i = 0; i != all_origins.size(); ++i)
		origincombo_.append_text(
					 external::origin_gui_str(i));

	xml_->get_widget("OriginLabel", originlabel_);
	originlabel_->set_mnemonic_widget(origincombo_);
	// *** End "Rotate" Page ***

	// *** Start "Scale" Page ***
	xml_->get_widget_derived ("Width", widthlength_);
	widthcombo_ = widthlength_->get_combo();
	widthcombo_->prepend_text(lyx::to_utf8(_("Scale%")));
	widthcombo_->set_active_text(lyx::to_utf8(_("Scale%")));

	xml_->get_widget("WidthLabel", widthlabel_);
	widthlabel_->set_mnemonic_widget(*(widthlength_->get_spin()));

	xml_->get_widget_derived ("Height", heightlength_);

	widthlength_->signal_changed().connect(
		sigc::mem_fun(*this, &GExternal::size_changed));
	heightlength_->signal_changed().connect(
		sigc::mem_fun(*this, &GExternal::size_changed));

	xml_->get_widget("HeightLabel", heightlabel_);
	heightlabel_->set_mnemonic_widget(*(heightlength_->get_spin()));

	xml_->get_widget ("AspectRatio", archeck_);
	// *** End "Scale" Page ***

	// *** Start "Crop" Page ***
	xml_->get_widget("Clip", clipcheck_);
	clipcheck_->signal_toggled().connect(
		sigc::mem_fun(*this, &GExternal::clipcheck_toggled));

	xml_->get_widget("GetFromFile", bbfromfilebutton_);

	bbfromfilebutton_->signal_clicked().connect(
		sigc::mem_fun(*this, &GExternal::get_bb));

	xml_->get_widget("xLabel", xlabel_);
	xml_->get_widget("yLabel", ylabel_);
	xml_->get_widget("RightTopLabel", rtlabel_);
	xml_->get_widget("LeftBottomLabel", lblabel_);

	xml_->get_widget("xLeft", xlentry_);
	xlentry_->signal_editing_done().connect(
		sigc::mem_fun(*this, &GExternal::bb_changed));

	xml_->get_widget("yBottom", ybentry_);
	ybentry_->signal_editing_done().connect(
		sigc::mem_fun(*this, &GExternal::bb_changed));

	xml_->get_widget("xRight", xrentry_);
	xrentry_->signal_editing_done().connect(
		sigc::mem_fun(*this, &GExternal::bb_changed));

	xml_->get_widget("yTop", ytentry_);
	ytentry_->signal_editing_done().connect(
		sigc::mem_fun(*this, &GExternal::bb_changed));

	// *** End "Crop" Page ***

	// *** Start "Options" Page ***
	formatstore_ = Gtk::ListStore::create(formatColumns);

	xml_->get_widget("Options", optionsview_);
	optionsview_->set_model(formatstore_);
	optionsview_->append_column(lyx::to_utf8(_("Forma_t")), formatColumns.name);
	optionsview_->append_column_editable(lyx::to_utf8(_("O_ption")), formatColumns.extra);
	// *** End "Options" Page ***
}


void GExternal::update()
{
	InsetExternalParams const & params = controller().params();

	std::string const name = params.filename.outputFilename();
	templatefcbutton_->set_filename(name);

	if (name != Glib::ustring()) {

		editfilebutton_->set_sensitive(true);
		bbfromfilebutton_->set_sensitive(true);
	}
	else {
		editfilebutton_->set_sensitive(false);
		bbfromfilebutton_->set_sensitive(false);
	}

	templatecombo_->set_active(
		controller().getTemplateNumber(params.templatename()));
	update_template();

	draftcheck_->set_active(params.draft);

	set_display(showcheck_, displaycombo_, displaylabel_, scalelength_, scalelabel_, params.display, params.lyxscale);

	set_rotation(angleentry_, &origincombo_, params.rotationdata);

	set_size(widthlength_, heightlength_, archeck_, params.resizedata);

	set_crop(clipcheck_,
		xlentry_, ybentry_,
		xrentry_, ytentry_,
		params.clipdata);

	controller().bbChanged(!params.clipdata.bbox.empty());
}


void GExternal::update_template()
{
	external::Template templ =
		controller().getTemplate(
			templatecombo_->get_active_row_number());

	// Remove file stale filters, if present
	// Keep the current file selected even after a template change
	Glib::ustring currentfilename;

	typedef std::vector<Gtk::FileFilter* >::iterator ffit;
	std::vector<Gtk::FileFilter* > templatefilters;

	if (currenttemplate_) {
		currentfilename = templatefcbutton_->get_filename();
		templatefilters =
			(*currenttemplate_)[templateColumns().filters];

		for (ffit it = templatefilters.begin();
			 it != templatefilters.end(); ++it)
			templatefcbutton_->remove_filter(**it);
	}

	currenttemplate_ = templatecombo_->get_active();

	templatebuffer_->set_text((*currenttemplate_)[templateColumns().info]);

	// Ascertain which (if any) transformations the template supports
	// and disable tabs hosting unsupported transforms.
	typedef std::vector<external::TransformID> TransformIDs;
	TransformIDs const transformIds = templ.transformIds;
	TransformIDs::const_iterator tr_begin = transformIds.begin();
	TransformIDs::const_iterator const tr_end = transformIds.end();

	bool found = find(tr_begin, tr_end, external::Rotate) != tr_end;
	Gtk::Widget * widget = notebook_->get_nth_page(2);
	widget->set_sensitive(found);
	notebook_->get_tab_label(*widget)->set_sensitive(found);

	found = find(tr_begin, tr_end, external::Resize) != tr_end;
	widget = notebook_->get_nth_page(3);
	widget->set_sensitive(found);
	notebook_->get_tab_label(*widget)->set_sensitive(found);

	found = find(tr_begin, tr_end, external::Clip) != tr_end;
	widget = notebook_->get_nth_page(4);
	widget->set_sensitive(found);
	notebook_->get_tab_label(*widget)->set_sensitive(found);

	found = find(tr_begin, tr_end, external::Extra) != tr_end;
	widget = notebook_->get_nth_page(5);
	widget->set_sensitive(found);
	notebook_->get_tab_label(*widget)->set_sensitive(found);

	// Add new filters; set the "All files" filter
	// in order to allow the previously selected file to remain selected
	templatefilters = (*currenttemplate_)[templateColumns().filters];
	for (ffit it = templatefilters.begin();
		it != templatefilters.end(); ++it)
			templatefcbutton_->add_filter(**it);

	templatefcbutton_->set_filter(*templatefilters.back()); // "All files"

	if (currentfilename != Glib::ustring())
		templatefcbutton_->set_filename(currentfilename);

	if (!found)
		return;

	// Ascertain whether the template has any formats supporting
	// the 'Extra' option
	Glib::ustring templatename =
			(*currenttemplate_)[templateColumns().name];
	formatstore_->clear();
	external::Template::Formats::const_iterator it = templ.formats.begin();
	external::Template::Formats::const_iterator end = templ.formats.end();

	bool const enabled = (it != end);

	for (; it != end; ++it) {
		if (it->second.option_transformers.find(external::Extra) ==
		    it->second.option_transformers.end())
			continue;
		string const format = it->first;
		string const opt = controller().params().extradata.get(format);

		Gtk::TreeModel::iterator iter = formatstore_->append();
		(*iter)[formatColumns.name] = Glib::locale_to_utf8(format);
		(*iter)[formatColumns.extra] = Glib::locale_to_utf8(opt);
	}

	// widget is still the 'Options' tab
	notebook_->get_tab_label(*widget)->set_sensitive(enabled);
}


void GExternal::apply()
{
	InsetExternalParams params = controller().params();

	params.filename.set(templatefcbutton_->get_filename(),
			    kernel().bufferFilepath());

	params.settemplate(
		Glib::locale_from_utf8((*(templatecombo_->get_active()))[templateColumns().name]));

	params.draft = draftcheck_->get_active();

	get_display(params.display, params.lyxscale,
		   showcheck_, displaycombo_, scalelength_);

	if (notebook_->get_nth_page(2)->is_sensitive())
		get_rotation(params.rotationdata,
			    angleentry_, &origincombo_);

	if (notebook_->get_nth_page(3)->is_sensitive())
		get_size(params.resizedata,
			 widthlength_, heightlength_, archeck_);

	if (notebook_->get_nth_page(4)->is_sensitive())
		get_crop(params.clipdata,
			clipcheck_,
			xlentry_, ybentry_,
			xrentry_, ytentry_,
			controller().bbChanged());

	if (notebook_->get_nth_page(5)->is_sensitive())
		get_extra(params.extradata, formatstore_);

	controller().setParams(params);
}


void GExternal::get_bb()
{
	xlentry_->set_text("0");
	ybentry_->set_text("0");
	xrentry_->set_text("0");
	ytentry_->set_text("0");

	string const filename = templatefcbutton_->get_filename();
	if (filename.empty())
		return;

	string const bb = controller().readBB(filename);
	if (bb.empty())
		return;

	xlentry_->set_text(Glib::locale_to_utf8(token(bb, ' ', 0)));
	ybentry_->set_text(Glib::locale_to_utf8(token(bb, ' ', 1)));
	xrentry_->set_text(Glib::locale_to_utf8(token(bb, ' ', 2)));
	ytentry_->set_text(Glib::locale_to_utf8(token(bb, ' ', 3)));

	controller().bbChanged(false);
	bc().valid(true);
}


bool GExternal::activate_ar() const
{
	if (widthlength_->get_combo()->get_active_text() ==
	    lyx::to_utf8(_("Scale%")))
		return false;

	if (widthlength_->get_spin()->get_value() < 0.05)
		return false;

	if (heightlength_->get_spin()->get_value() < 0.05)
		return false;

	return true;
}


void GExternal::bb_changed()
{
	controller().bbChanged(true);
	bc().valid(true);
}


void GExternal::edit_clicked()
{
	controller().editExternal();
}


void GExternal::size_changed()
{
	archeck_->set_sensitive(activate_ar());

	bool useHeight = widthlength_->get_combo()->get_active_text() !=
		lyx::to_utf8(_("Scale%"));

	heightlength_->set_sensitive(useHeight);
}


void GExternal::template_changed()
{
	update_template();
	bc().valid(true);
}


void GExternal::showcheck_toggled()
{
	bool checked = showcheck_->get_active();
	displaylabel_->set_sensitive(checked);
	displaycombo_->set_sensitive(checked);
	scalelabel_->set_sensitive(checked);
	scalelength_->set_sensitive(checked);

	bc().valid(true);
}


void GExternal::clipcheck_toggled()
{
	bool checked = clipcheck_->get_active();

	xlabel_->set_sensitive(checked);
	ylabel_->set_sensitive(checked);
	rtlabel_->set_sensitive(checked);
	lblabel_->set_sensitive(checked);

	xlentry_->set_sensitive(checked);
	ybentry_->set_sensitive(checked);
	xrentry_->set_sensitive(checked);
	ytentry_->set_sensitive(checked);

	bc().valid(true);
}


void GExternal::file_changed()
{
	if (templatefcbutton_->get_filename() != Glib::ustring()) {

		editfilebutton_->set_sensitive(true);
		bbfromfilebutton_->set_sensitive(true);
	}
	else {
		editfilebutton_->set_sensitive(false);
		bbfromfilebutton_->set_sensitive(false);
	}
	bc().valid(true);
}


} // namespace frontend
} // namespace lyx
