/**
 * \file FormTexinfo.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Herbert Voﬂ
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "FormTexinfo.h"
#include "forms/form_texinfo.h"

#include "Tooltips.h"
#include "xformsBC.h"

#include "support/filetools.h"

#include "lyx_forms.h"

using std::string;

namespace lyx {

using support::OnlyFilename;

namespace frontend {


typedef FormController<ControlTexinfo, FormView<FD_texinfo> > base_class;

FormTexinfo::FormTexinfo(Dialog & parent)
	: base_class(parent, _("LaTeX Information")),
	  activeStyle(ControlTexinfo::cls)
{}


void FormTexinfo::build() {
	dialog_.reset(build_texinfo(this));

	// callback for double click in browser to view the selected file
	fl_set_browser_dblclick_callback(dialog_->browser,
					 C_FormDialogView_InputCB, 2);

	string const classes_List = _("LaTeX classes|LaTeX styles|BibTeX styles");
	fl_addto_choice(dialog_->choice_classes, classes_List.c_str());

	updateStyles(activeStyle);

	// set up the tooltips
	string str = _("Shows the installed classses and styles for LaTeX/BibTeX; "
			"available only if the corresponding LyX layout file exists.");
	tooltips().init(dialog_->choice_classes, str);

	str = _("Show full path or only file name.");
	tooltips().init(dialog_->check_fullpath, str);

	str = _("Runs the script \"TexFiles.sh\" to build new file lists.");
	tooltips().init(dialog_->button_rescan, str);

	str = _("Double click to view contents of file.");
	tooltips().init(dialog_->browser, str);
#if FL_VERSION == 0 || (FL_REVISION == 0 && FL_FIXLEVEL == 0)
	// Work-around xforms' bug; enable tooltips for browser widgets.
	setPrehandler(dialog_->browser);
#endif

	str = _("Runs the script \"texhash\" which builds a new LaTeX tree. "
		"Needed if you install a new TeX class or style. You need write "
		"permissions for the TeX-dirs, often /var/lib/texmf and others.");
	tooltips().init(dialog_->button_texhash, str);
}


ButtonPolicy::SMInput FormTexinfo::input(FL_OBJECT * ob, long ob_value) {

	if (ob == dialog_->button_texhash) {
		// makes only sense if the rights are set well for
		// users (/var/lib/texmf/ls-R)
		texhash();
		// texhash requires a rescan and an update of the styles
		rescanTexStyles();
		updateStyles(activeStyle);


	} else if (ob == dialog_->browser && ob_value == 2) {
		// double click in browser: view selected file
		ContentsType::size_type const sel = fl_get_browser(ob);
		ContentsType const & data = texdata_[activeStyle];
		if (sel >= 1 && sel <= data.size())
			controller().viewFile(data[sel-1]);

		// reset the browser so that the following single-click
		// callback doesn't do anything
		fl_deselect_browser(dialog_->browser);

	} else if (ob == dialog_->button_rescan) {
		// build new *Files.lst
		rescanTexStyles();
		updateStyles(activeStyle);

	} else if (ob == dialog_->check_fullpath) {
		updateStyles(activeStyle);

	} else if (ob == dialog_->choice_classes) {
		switch (fl_get_choice(dialog_->choice_classes)) {
		case 3:
			updateStyles(ControlTexinfo::bst);
			break;
		case 2:
			updateStyles(ControlTexinfo::sty);
			break;
		case 1:
		default:
			updateStyles(ControlTexinfo::cls);
		}
	}

	return ButtonPolicy::SMI_VALID;
}


void FormTexinfo::updateStyles(ControlTexinfo::texFileSuffix whichStyle)
{
	ContentsType & data = texdata_[whichStyle];
	getTexFileList(whichStyle, data);

	bool const withFullPath = fl_get_button(dialog_->check_fullpath);

	fl_clear_browser(dialog_->browser);
	ContentsType::const_iterator it  = data.begin();
	ContentsType::const_iterator end = data.end();
	for (; it != end; ++it) {
		string const line = withFullPath ? *it : OnlyFilename(*it);
		fl_add_browser_line(dialog_->browser, line.c_str());
	}

	activeStyle = whichStyle;
}

} // namespace frontend
} // namespace lyx
