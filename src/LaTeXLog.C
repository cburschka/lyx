#include <config.h>

#include <cstdlib>
#include FORMS_H_LOCATION
#include "buffer.h"
#include "log_form.h"
#include "lyx_main.h"
#include "LString.h"
#include "support/FileInfo.h"
#include "support/filetools.h"
#include "support/path.h"
#include "lyxrc.h"
#include "BufferView.h"
#include "gettext.h"
#include "converter.h"

extern FD_LaTeXLog *fd_latex_log;
extern BufferView *current_view;

void ShowLatexLog()
{
	string filename, fname, bname, path;
	bool use_build = false;
	static int ow = -1, oh;

	filename = current_view->buffer()->getLatexName();
	if (!filename.empty()) {
		fname = OnlyFilename(ChangeExtension(filename, ".log"));
		bname = OnlyFilename(ChangeExtension(filename,
				     formats.Extension("literate") + ".out"));
		path = OnlyPath(filename);
		if (lyxrc.use_tempdir || (IsDirWriteable(path) < 1)) {
			path = current_view->buffer()->tmppath;
		}
		FileInfo f_fi(path + fname), b_fi(path + bname);
		if (b_fi.exist())
			if (!f_fi.exist()
			    || f_fi.getModificationTime() < b_fi.getModificationTime())
				use_build = true; // If no Latex log or Build log is newer, show Build log
		Path p(path); // path to LaTeX file
		if (!fl_load_browser(fd_latex_log->browser_latexlog,
				     use_build ? bname.c_str() : fname.c_str()))
			fl_add_browser_line(fd_latex_log->browser_latexlog,
					    _("Unable to show log file!"));
	} else {
		fl_add_browser_line(fd_latex_log->browser_latexlog,
			       _("NO LATEX LOG FILE!"));
	}
	if (fd_latex_log->LaTeXLog->visible) {
		fl_raise_form(fd_latex_log->LaTeXLog);
	} else {
		fl_show_form(fd_latex_log->LaTeXLog,
			     FL_PLACE_MOUSE | FL_FREE_SIZE, FL_FULLBORDER,
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
