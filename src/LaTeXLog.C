#include <config.h>

#include <cstdlib>
#include FORMS_H_LOCATION
#include "buffer.h"
#include "log_form.h"
#include "lyx_main.h"
#include "LString.h"
#include "support/FileInfo.h"
#include "support/filetools.h"
#include "lyxrc.h"
#include "BufferView.h"
#include "gettext.h"
#include "converter.h"

extern FD_LaTeXLog *fd_latex_log;
extern BufferView *current_view;

using std::endl;

void ShowLatexLog()
{
	string filename, fname, bname, path;
	bool use_build = false;
	static int ow = -1, oh;

	filename = current_view->buffer()->getLatexName(false);

	if (!filename.empty()) {
		path = OnlyPath(filename);
		if (lyxrc.use_tempdir || (IsDirWriteable(path) < 1)) {
			path = current_view->buffer()->tmppath;
		}
		fname = AddName(path, ChangeExtension(filename, ".log"));
		bname = AddName(path,
				ChangeExtension(filename,
						formats.Extension("literate")
						+ ".out"));
		FileInfo f_fi(fname), b_fi(bname);

		if (b_fi.exist())
			// If no Latex log or Build log is newer, show
			// Build log 
			if (!f_fi.exist()
			    || f_fi.getModificationTime() < b_fi.getModificationTime())
				use_build = true; 

		string name;

		if (use_build)
			name = bname;
		else
			name = fname;

		lyxerr[Debug::FILES] << "Log file: " << name << endl;

		if (!fl_load_browser(fd_latex_log->browser_latexlog, name.c_str()))
			fl_add_browser_line(fd_latex_log->browser_latexlog,
					    _("No LaTeX log file found"));
	} else {
		fl_add_browser_line(fd_latex_log->browser_latexlog,
			       _("No LaTeX log file found"));
	}
	if (fd_latex_log->LaTeXLog->visible) {
		fl_raise_form(fd_latex_log->LaTeXLog);
	} else {
		fl_show_form(fd_latex_log->LaTeXLog,
			     FL_PLACE_MOUSE | FL_FREE_SIZE, FL_TRANSIENT,
			     use_build ? _("Build Program Log") : _("LaTeX Log"));
		if (ow < 0) {
			ow = fd_latex_log->LaTeXLog->w;
			oh = fd_latex_log->LaTeXLog->h;
		}
		fl_set_form_minsize(fd_latex_log->LaTeXLog, ow, oh);
	}
}

void LatexLogClose(FL_OBJECT *, long)
{
    fl_hide_form(fd_latex_log->LaTeXLog);
}

void LatexLogUpdate(FL_OBJECT *, long)
{
    ShowLatexLog();
}
