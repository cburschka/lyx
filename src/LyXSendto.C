#include <config.h>

#include FORMS_H_LOCATION
#include "print_form.h"
#include "lyx_main.h"
#include "lyxrc.h"
#include "LString.h"
#include "support/filetools.h"
#include "support/path.h"
#include "buffer.h"
#include "lyx_gui_misc.h"
#include "support/syscall.h"
#include "gettext.h"
#include "lyx_cb.h"

extern FD_form_sendto * fd_form_sendto;
extern BufferView * current_view;
extern int MakeDVIOutput(Buffer * buffer);
extern bool MenuRunDvips(Buffer * buffer, bool wait);

// Whereas this feature is under the menu item File->Export->Custom,
// I kept the old name sendto in the code because I am lazy (JMarc)

void MenuSendto()
{
    static int ow = -1, oh;

    // do this only if the command is empty
    if (!fl_get_input(fd_form_sendto->input_cmd) &&
        !lyxrc->custom_export_command.empty())
        fl_set_input(fd_form_sendto->input_cmd,
                     lyxrc->custom_export_command.c_str());
    if (fd_form_sendto->form_sendto->visible) {
        fl_raise_form(fd_form_sendto->form_sendto);
    } else {  
        fl_show_form(fd_form_sendto->form_sendto,
                     FL_PLACE_MOUSE | FL_FREE_SIZE, FL_FULLBORDER,
                     _("Send Document to Command"));
	if (ow < 0) {
		ow = fd_form_sendto->form_sendto->w;
		oh = fd_form_sendto->form_sendto->h;
	}
	fl_set_form_minsize(fd_form_sendto->form_sendto, ow, oh);
    }
}

void SendtoApplyCB(FL_OBJECT *, long)
{
    if (!current_view->available())
        return;

    string command = fl_get_input(fd_form_sendto->input_cmd);
    if (command.empty())
        return;
    Buffer * buffer = current_view->buffer();
    if (fl_get_button(fd_form_sendto->radio_ftype_dvi) ||
        fl_get_button(fd_form_sendto->radio_ftype_ps)) {
        ProhibitInput();
        // Generate dvi file and check if there are errors in the .lyx file
        if (MakeDVIOutput(buffer) > 0) {
            AllowInput();
            return;
        }
        AllowInput();
    }
    string ftypeext;
    if (fl_get_button(fd_form_sendto->radio_ftype_lyx))
        ftypeext = ".lyx";
    else if (fl_get_button(fd_form_sendto->radio_ftype_latex))
        ftypeext = ".tex";
    else if (fl_get_button(fd_form_sendto->radio_ftype_dvi))
        ftypeext = ".dvi";
    else if (fl_get_button(fd_form_sendto->radio_ftype_ascii))
        ftypeext = ".txt";
    else {
        ftypeext = ".ps_tmp";
        if (!MenuRunDvips(buffer, true)) {
	    return;
	}
    }

    string fname = ChangeExtension(buffer->getLatexName(), ftypeext, true);
    if (!contains(command, "$$FName"))
        command = "( " + command + " ) <$$FName";
    command = subst(command, "$$FName", fname);
    command += " &"; // execute in background
    // push directorypath, if necessary 
    string path = OnlyPath(buffer->fileName());
    if (lyxrc->use_tempdir || (IsDirWriteable(path) < 1)){
        path = buffer->tmppath;
    }
    Path p(path);
    // save the .lyx file in tmp_dir if this filetype is requested
    if (fl_get_button(fd_form_sendto->radio_ftype_lyx))
        buffer->writeFile(fname, true);
    // if the .tex file is requested save it to the tempdir
    // as now we don't do the MakeDVIOutput anymore
    if (fl_get_button(fd_form_sendto->radio_ftype_latex))
        buffer->makeLaTeXFile(fname, path, false);
    // create the .txt file in tmp_dir if this filetype is requested
    if (fl_get_button(fd_form_sendto->radio_ftype_ascii))
        buffer->writeFileAscii(fname, lyxrc->ascii_linelen);
    Systemcalls one(Systemcalls::System, command);    
}

void SendtoCancelCB(FL_OBJECT *, long)
{
    fl_hide_form(fd_form_sendto->form_sendto);
}

void SendtoOKCB(FL_OBJECT *ob, long data)
{
    SendtoCancelCB(ob, data);
    SendtoApplyCB(ob, data);
}
