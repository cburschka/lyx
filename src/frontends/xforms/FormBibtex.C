/**
 * \file FormBibtex.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author John Levon
 * \author Herbert Voss
 * \author Rob Lahaye
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "FormBibtex.h"
#include "forms/form_bibtex.h"
#include "ControlBibtex.h"

#include "xformsBC.h"
#include "Tooltips.h"
#include "xforms_helpers.h"
#include "lyx_forms.h"
#include "combox.h"

#include "tex_helpers.h"

#include "gettext.h"
#include "support/filetools.h"
#include "support/lstrings.h"

namespace support = lyx::support;

using std::vector;


namespace {

struct SameName {
	SameName(string const & name) : name_(name) {}
	bool operator()(string const & abs_name) {
		return name_ == support::OnlyFilename(abs_name);
	}
private:
	string name_;
};


string const get_absolute_filename(string const & file,
				   string const & buffer_path,
				   vector<string> const & bibfiles)
{
	if (support::AbsolutePath(file))
		return file;

	string const try1 = support::MakeAbsPath(file, buffer_path);
	if (support::IsFileReadable(try1))
		return try1;

	string const name = support::OnlyFilename(file);

	vector<string>::const_iterator it  = bibfiles.begin();
	vector<string>::const_iterator end = bibfiles.end();
	it = std::find_if(it, end, SameName(name));
	if (it == end)
		return try1;
	return *it;
}


bool find_file_in_browser(FL_OBJECT * browser,
			  string const & name,
			  string const & buffer_path,
			  vector<string> const & bibfiles)
{
	vector<string> browser_vec = getVector(browser);
	vector<string>::iterator begin = browser_vec.begin();
	vector<string>::iterator end   = browser_vec.end();
	vector<string>::iterator it    = begin;
	for (; it != end; ++it) {
		*it = get_absolute_filename(*it, buffer_path, bibfiles);
	}

	string const abs_name = get_absolute_filename(name, buffer_path, bibfiles);
	
	it = std::find(begin, end, abs_name);
	return it != end;
}


int find_style_in_combox(FL_OBJECT * combox, string const & name)
{
	int const size = fl_get_combox_maxitems(combox);
	for (int i = 1; i <= size; ++i) {
		char const * const tmp = fl_get_combox_line(combox, i);
		string const str = tmp ? support::trim(tmp) : string();
		if (str == name)
			return i;
	}
	return 0;
}

} // namespace anon


typedef FormController<ControlBibtex, FormView<FD_bibtex> > base_class;

FormBibtex::FormBibtex(Dialog & parent)
	: base_class(parent, _("BibTeX Database"))
{}


void FormBibtex::build()
{
	dialog_.reset(build_bibtex(this));

	// Manage the ok and cancel/close buttons
	bcview().setOK(dialog_->button_ok);
	bcview().setCancel(dialog_->button_close);

	// disable for read-only documents
	bcview().addReadOnly(dialog_->browser_databases);
	bcview().addReadOnly(dialog_->button_browse_db);
	bcview().addReadOnly(dialog_->button_delete_db);
	bcview().addReadOnly(dialog_->radio_absolute_path);
	bcview().addReadOnly(dialog_->radio_relative_path);
	bcview().addReadOnly(dialog_->radio_name_only);
	bcview().addReadOnly(dialog_->input_database);
	bcview().addReadOnly(dialog_->button_add_db);
	bcview().addReadOnly(dialog_->combox_style);
	bcview().addReadOnly(dialog_->button_browse);
	bcview().addReadOnly(dialog_->check_bibtotoc);

	// callback for double click in browser
	fl_set_browser_dblclick_callback(dialog_->browser_databases,
					 C_FormDialogView_InputCB, 2);

	fl_set_input_return(dialog_->input_database, FL_RETURN_ALWAYS);

	storage_.init(dialog_->radio_absolute_path, ABSOLUTE_PATH);
	storage_.init(dialog_->radio_relative_path, RELATIVE_PATH);
	storage_.init(dialog_->radio_name_only,     NAME_ONLY);

	// set up the tooltips
	string str = _("Selected BibTeX databases. Double click to edit");
	tooltips().init(dialog_->browser_databases, str);

#if FL_VERSION == 0 || (FL_REVISION == 0 && FL_FIXLEVEL == 0)
	// Work-around xforms' bug; enable tooltips for browser widgets.
	setPrehandler(dialog_->browser_databases);
#endif
	str = _("Add a BibTeX database file");
	tooltips().init(dialog_->button_browse_db, str);

	str = _("Remove the selected database");
	tooltips().init(dialog_->button_delete_db, str);

	str = _("Store the absolute path to the file");
	tooltips().init(dialog_->radio_absolute_path, str);

	str = _("Store the relative path to the file");
	tooltips().init(dialog_->radio_relative_path, str);

	str = _("Store the file name only. Use TeX magic to find it");
	tooltips().init(dialog_->radio_name_only, str);

	str = _("Input BibTeX database file");
	tooltips().init(dialog_->input_database, str);

	str = _("Add the BibTeX database file in the input field");
	tooltips().init(dialog_->button_add_db, str);

	str = _("The BibTeX style");
	tooltips().init(dialog_->combox_style, str);

	str = _("Choose a style file");
	tooltips().init(dialog_->button_browse, str);

	str = _("Add bibliography to the table of contents");
	tooltips().init(dialog_->check_bibtotoc, str);
}


void FormBibtex::update()
{
	controller().getBibFiles(bib_files_namesonly_);
	getTexFileList("bibFiles.lst", bib_files_);

	browser_deactivated();

	InsetBibtexParams const & params = controller().params();

	fl_clear_browser(dialog_->browser_databases);
	vector<support::FileName>::const_iterator fit  = params.databases.begin();
	vector<support::FileName>::const_iterator fend = params.databases.end();
	for (; fit != fend; ++fit) {
		string const db = fit->outputFilename(kernel().bufferFilepath());
		fl_add_browser_line(dialog_->browser_databases, db.c_str());
	}

	fl_set_button(dialog_->check_bibtotoc, params.bibtotoc);

	fl_clear_combox(dialog_->combox_style);

	int item_nr = 0;
	vector<string> styles;
	controller().getBibStyles(styles);
	vector<string>::const_iterator sit  = styles.begin();
	vector<string>::const_iterator send = styles.end();
	for (; sit != send; ++sit) {
		string const item = support::ChangeExtension(*sit, "");
		fl_addto_combox(dialog_->combox_style, item.c_str());
		if (item == params.style)
			item_nr = fl_get_combox_maxitems(dialog_->combox_style);
	}

	if (item_nr == 0) {
		fl_addto_combox(dialog_->combox_style, params.style.c_str());
		item_nr = fl_get_combox_maxitems(dialog_->combox_style);
	}

	fl_set_combox(dialog_->combox_style, item_nr);
}


void FormBibtex::apply()
{
	InsetBibtexParams params;

	FL_OBJECT * browser = dialog_->browser_databases;
	int const size = fl_get_browser_maxline(browser);
	for (int i = 0; i < size; ++i) {
		string const text = fl_get_browser_line(browser, i+1);
		support::FileName file;
		file.set(text, kernel().bufferFilepath());
                params.databases.push_back(file);
        }

	params.style = support::trim(fl_get_combox_text(dialog_->combox_style));
	params.bibtotoc = fl_get_button(dialog_->check_bibtotoc);

	controller().params() = params;
}


ButtonPolicy::SMInput FormBibtex::input(FL_OBJECT * ob, long ob_value)
{
	ButtonPolicy::SMInput activate = ButtonPolicy::SMI_NOOP;

	if (ob == dialog_->browser_databases && ob_value == 2) {
		activate = browser_double_click();

	} else if (ob == dialog_->browser_databases) {
		activate = browser_selected();

	} else if (ob == dialog_->button_browse_db) {
		activate = database_browse();

	} else if (ob == dialog_->button_delete_db) {
		activate = browser_delete();

	} else if (ob == dialog_->radio_absolute_path) {
		activate = set_path(ABSOLUTE_PATH);

	} else if (ob == dialog_->radio_relative_path) {
		activate = set_path(RELATIVE_PATH);

	} else if (ob == dialog_->radio_name_only) {
		activate = set_path(NAME_ONLY);

	} else if (ob == dialog_->input_database) {
		input_database();

	} else if (ob == dialog_->button_add_db) {
		activate = add_database();

	} else if (ob == dialog_->button_browse) {
		activate = style_browse();

	} else if (ob == dialog_->combox_style || ob == dialog_->check_bibtotoc) {
		activate = ButtonPolicy::SMI_VALID;
	}

	return activate;
}


ButtonPolicy::SMInput FormBibtex::add_database()
{
	FL_OBJECT * input   = dialog_->input_database;
	FL_OBJECT * browser = dialog_->browser_databases;

	string const db = getString(input);

	bool const activate =
		!db.empty() &&
		!find_file_in_browser(browser, db, kernel().bufferFilepath(),
				      bib_files_);

	if (activate)
		fl_add_browser_line(browser, db.c_str());

	fl_set_input(input, "");
	setEnabled(dialog_->button_add_db, false);

	return activate ? ButtonPolicy::SMI_VALID : ButtonPolicy::SMI_NOOP;
}


namespace {

bool inTeXSearchPath(string const & db, vector<string> const & tex_bib_files)
{
	vector<string>::const_iterator it  = tex_bib_files.begin();
	vector<string>::const_iterator end = tex_bib_files.end();
	return std::find(it, end, db) != end;
}


FormBibtex::PathStyle storageStyle(string const & db,
				   vector<string> const & tex_bib_files)
{
	if (support::AbsolutePath(db))
		return FormBibtex::ABSOLUTE_PATH;

	string const name = support::OnlyFilename(db);
	if (db == name && inTeXSearchPath(name, tex_bib_files))
		return FormBibtex::NAME_ONLY;

	return FormBibtex::RELATIVE_PATH;
}

} // namespace anon


ButtonPolicy::SMInput FormBibtex::browser_selected()
{
	FL_OBJECT * browser = dialog_->browser_databases;

	int const sel = fl_get_browser(browser);
	if (sel < 1 || sel > fl_get_browser_maxline(browser))
		return ButtonPolicy::SMI_NOOP;

	setEnabled(dialog_->button_delete_db, true);
	storage_.setEnabled(true);
	fl_set_input(dialog_->input_database, "");

	string const db = fl_get_browser_line(browser, sel);
	storage_.set(storageStyle(db, bib_files_namesonly_));

	return ButtonPolicy::SMI_NOOP;
}


ButtonPolicy::SMInput FormBibtex::browser_double_click()
{
	FL_OBJECT * browser = dialog_->browser_databases;

	int const sel = fl_get_browser(browser);
	if (sel < 1 || sel > fl_get_browser_maxline(browser))
		return ButtonPolicy::SMI_NOOP;

	string const db = fl_get_browser_line(browser, sel);
	fl_set_input(dialog_->input_database, db.c_str());
	setEnabled(dialog_->button_add_db, true);

	// Reset the browser so that the following single-click
	// callback doesn't do anything
	fl_deselect_browser(browser);

	return ButtonPolicy::SMI_NOOP;
}


ButtonPolicy::SMInput FormBibtex::browser_add_db(string const & name)
{
	FL_OBJECT * browser = dialog_->browser_databases;

	if (find_file_in_browser(browser, name, kernel().bufferFilepath(),
				 bib_files_))
		return ButtonPolicy::SMI_NOOP;

	input_clear();

	fl_add_browser_line(browser, name.c_str());
	int const line = fl_get_browser_maxline(browser);
	fl_select_browser_line(browser, line);

	browser_selected();
	return ButtonPolicy::SMI_VALID;
}


ButtonPolicy::SMInput FormBibtex::browser_delete()
{
	FL_OBJECT * browser = dialog_->browser_databases;

	int const sel = fl_get_browser(browser);
	if (sel < 1 || sel > fl_get_browser_maxline(browser))
		return ButtonPolicy::SMI_NOOP;

	fl_delete_browser_line(browser, sel);
	browser_deactivated();
	return ButtonPolicy::SMI_VALID;
}


ButtonPolicy::SMInput FormBibtex::set_path(PathStyle style)
{
	FL_OBJECT * browser = dialog_->browser_databases;

	int const sel = fl_get_browser(browser);
	if (sel < 1 || sel > fl_get_browser_maxline(browser))
		return ButtonPolicy::SMI_NOOP;

	string const db = fl_get_browser_line(browser, sel);
	string const abs_name =
		get_absolute_filename(db, kernel().bufferFilepath(), bib_files_);

	switch (style) {
	case ABSOLUTE_PATH:
		fl_replace_browser_line(browser, sel, abs_name.c_str());
		break;
	case RELATIVE_PATH: 
	{
		string const rel_name =
			support::MakeRelPath(abs_name, kernel().bufferFilepath());
		fl_replace_browser_line(browser, sel, rel_name.c_str());
		break;
	}
	case NAME_ONLY:
	{
		string const name = support::OnlyFilename(abs_name);
		fl_replace_browser_line(browser, sel, name.c_str());
		break;
	}
	}
	
	return ButtonPolicy::SMI_VALID;
}


ButtonPolicy::SMInput FormBibtex::database_browse()
{
	ButtonPolicy::SMInput activate = ButtonPolicy::SMI_NOOP;
		
	FL_OBJECT * browser = dialog_->browser_databases;
	// When browsing, take the first file only
	string const in_name = fl_get_browser_maxline(browser) > 0 ?
		fl_get_browser_line(browser, 1) : string();

	string const out_name =
		controller().Browse("", _("Select Database"),
				    _("*.bib| BibTeX Databases (*.bib)"));

	if (!out_name.empty() &&
	    out_name != in_name &&
	    !find_file_in_browser(browser, out_name,
				  kernel().bufferFilepath(),
				  bib_files_))
		activate = browser_add_db(out_name);
	return activate;
}


ButtonPolicy::SMInput FormBibtex::style_browse()
{
	FL_OBJECT * combox = dialog_->combox_style;

	string const in_name = fl_get_combox_text(combox);
	string const style =
		controller().Browse(in_name, _("Select BibTeX-Style"),
				    _("*.bst| BibTeX Styles (*.bst)"));

	int const sel = find_style_in_combox(combox, style);
	if (sel)
		fl_set_combox(combox, sel);
	else {
		fl_addto_combox(combox, style.c_str());
		fl_set_combox(combox, fl_get_combox_maxitems(combox));
	}

	return ButtonPolicy::SMI_VALID;
}


void FormBibtex::browser_deactivated()
{
	setEnabled(dialog_->button_delete_db, false);
	input_clear();
	storage_.unset();
	storage_.setEnabled(false);
}


void FormBibtex::input_clear()
{
	fl_set_input(dialog_->input_database, "");
	setEnabled(dialog_->button_add_db, false);
}


void FormBibtex::input_database()
{
	FL_OBJECT * input = dialog_->input_database;
	setEnabled(dialog_->button_add_db, !getString(input).empty());
}
