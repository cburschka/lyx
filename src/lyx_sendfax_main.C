
#include <config.h>

#include <cstdlib>
#include FORMS_H_LOCATION
#include "debug.h"
#include "lyx_sendfax.h"
#include "lyx_main.h"
#include "lyxrc.h"
#include "support/filetools.h"
#include "lyx_gui_misc.h" // CancelCloseBoxCB
#include "support/syscall.h"
#include "gettext.h"

/* Prototypes */

bool sendfax(string const &fname, string const &sendcmd);
bool button_send(string const &fname, string const &sendcmd = string());
void show_logfile(string logfile, bool show_if_empty);

const int LEN_PHONE = 20;
const int LEN_NAME = 25;

/* Global Variables */

string phone_book;
string global_sendcmd;
string filename;

FD_xsendfax *fd_xsendfax;
FD_phonebook  *fd_phonebook;
FD_logfile  *fd_logfile;

bool send_fax(string const &fname, string const &sendcmd)
{
    // I put FAXCMD here to see if I can get rid of some strange errors. (Lgb)
    string FAXCMD = "sendfax -n -h '$$Host' -c '$$Comment' -x '$$Enterprise' -d '$$Name'@'$$Phone' '$$FName'";
    string title = _("Fax File: ");
    string path;
    
    if (fname.empty())
        return false;
    path= OnlyPath(fname);
    if (path.empty() || path == "./")
        filename = GetCWD() + "/" + fname;
    else
        filename = fname;

    if (sendcmd.empty())
	    global_sendcmd = FAXCMD;
    else
	    global_sendcmd = sendcmd;
    
    fd_xsendfax = create_form_xsendfax();
    fd_phonebook = create_form_phonebook();
    fd_logfile = create_form_logfile();

    // Make sure the close box doesn't kill LyX when being pressed. (RvdK)
    fl_set_form_atclose(fd_xsendfax->xsendfax, CancelCloseBoxCB, 0);
    fl_set_form_atclose(fd_phonebook->phonebook, CancelCloseBoxCB, 0);
    fl_set_form_atclose(fd_logfile->logfile, CancelCloseBoxCB, 0);

    /* init Phone-Book */
    string phone_book_name;
    if (lyxrc->phone_book.empty()) {
        phone_book_name = "phonebook";
    } else
        phone_book_name = lyxrc->phone_book;
    phone_book= FileSearch(user_lyxdir, phone_book_name);
    if (phone_book.empty()) 
        phone_book = AddName(user_lyxdir, phone_book_name);

    fl_set_browser_fontsize(fd_phonebook->browser, FL_NORMAL_SIZE);
    fl_set_browser_fontstyle(fd_phonebook->browser, FL_FIXED_STYLE);
    fl_load_browser(fd_phonebook->browser, phone_book.c_str());

    title += OnlyFilename(fname);

    /* show the first form */
    fl_show_form(fd_xsendfax->xsendfax, FL_PLACE_MOUSE, FL_FULLBORDER, title.c_str());
/*
    while(true) {
        obj = fl_do_forms();
        if (obj == fd_xsendfax->Button_Cancel)
            break;
        else if (obj == fd_xsendfax->Button_Send) {
            if (button_send(fname, global_sendcmd))
                break;
        } else if (obj == fd_xsendfax->Button_Apply) {
            button_send(fname, global_sendcmd);
        } else if (obj == fd_xsendfax->Button_SPhone) {
            cb_select_phoneno(0, 0);
        }
    }
    fl_hide_form(fd_xsendfax->xsendfax);
*/
    return true;
}


bool button_send(string const &fname, string const &sendcmd)
{
    string
        name = fl_get_input(fd_xsendfax->Input_Name),
        phone = fl_get_input(fd_xsendfax->Input_Phone),
        enterprise = fl_get_input(fd_xsendfax->Input_Enterprise),
        comment = fl_get_input(fd_xsendfax->Input_Comment),
        host = GetEnv("FAX_SERVER"),
        logfile,
        cmd;

    if (phone.empty())
        return false;
    logfile = TmpFileName(OnlyPath(fname), "FAX");
    cmd = sendcmd + " >";
    cmd += logfile + " 2>";
    cmd += logfile;
    cmd = subst(cmd, "$$Host", host);
    cmd = subst(cmd, "$$Comment", comment);
    cmd = subst(cmd, "$$Enterprise", enterprise);
    cmd = subst(cmd, "$$Name", name);
    cmd = subst(cmd, "$$Phone", phone);
    cmd = subst(cmd, "$$FName", fname);
    lyxerr << "CMD: " << cmd << endl;
    Systemcalls one(Systemcalls::System, cmd);
    show_logfile(logfile, false);
    remove(logfile.c_str());
    return true;
}


void cb_hide_form(FL_OBJECT *ob, long)
{
    fl_hide_form(ob->form);
}


void cb_select_phoneno(FL_OBJECT *, long)
{
    int
        i,
        n= fl_get_browser_maxline(fd_phonebook->browser);
    char const
        *line;

    fl_hide_form(fd_phonebook->phonebook);
    line = fl_get_browser_line(fd_phonebook->browser, 1);
    if (!n || strstr(line, _("Empty Phonebook"))) {
        fl_clear_browser(fd_phonebook->browser);
        return;
    }
    i = fl_get_browser(fd_phonebook->browser);
    line = fl_get_browser_line(fd_phonebook->browser, i);
    if (!line)
        return;
    char *buf = new char [strlen(line)+1];

    strcpy(buf, line);
    for(i = LEN_PHONE; (i > 0) && (buf[i] == ' '); --i)
        ;
    buf[i + 1] = 0;
    for(i = LEN_PHONE + LEN_NAME + 1; (i > LEN_PHONE) && (buf[i] == ' '); --i)
        ;
    buf[i + 1] = 0;
    fl_set_input(fd_xsendfax->Input_Phone, buf);
    fl_set_input(fd_xsendfax->Input_Name, buf+LEN_PHONE+1);
    fl_set_input(fd_xsendfax->Input_Enterprise, buf+LEN_PHONE+LEN_NAME+2);
    delete [] buf;
}


void cb_add_phoneno(FL_OBJECT *, long )
{
    char const
        *line;
    char const
        *name = fl_get_input(fd_xsendfax->Input_Name),
        *phone = fl_get_input(fd_xsendfax->Input_Phone),
        *enterprise = fl_get_input(fd_xsendfax->Input_Enterprise);
    int
        i, n;

    if (!strlen(phone))
        return;

    char * buf = new char [50 + strlen(enterprise)];

    sprintf(buf, "%-*.*s %-*.*s %s",
            LEN_PHONE, LEN_PHONE, phone,
            LEN_NAME, LEN_NAME, name,
            enterprise);
    n = fl_get_browser_maxline(fd_phonebook->browser);
    if (n) {
        line = fl_get_browser_line(fd_phonebook->browser, 1);
        if (strstr(line, _("Empty Phonebook"))) {
            fl_clear_browser(fd_phonebook->browser);
            n = 0;
        }
    }
    for(i = 1; i <= n; ++i) {
        line = fl_get_browser_line(fd_phonebook->browser, i);
        if (!strncmp(buf, line, 46))
            break;
    }
    if (i > n) {
        fl_addto_browser(fd_phonebook->browser, buf);
        fl_set_object_label(fd_xsendfax->pb_save, _("Save (needed)"));
    }
    delete[] buf;
}


void cb_delete_phoneno(FL_OBJECT *, long )
{
    char const
        *line;
    char const
        *name = fl_get_input(fd_xsendfax->Input_Name),
        *phone = fl_get_input(fd_xsendfax->Input_Phone),
        *enterprise = fl_get_input(fd_xsendfax->Input_Enterprise);
    int
        i, n;

    if (!strlen(phone))
        return;
    char *buf = new char [50+strlen(enterprise)];
    sprintf(buf, "%-*.*s %-*.*s %s",
            LEN_PHONE, LEN_PHONE, phone,
            LEN_NAME, LEN_NAME, name,
            enterprise);
    n = fl_get_browser_maxline(fd_phonebook->browser);
    if (n) {
        line = fl_get_browser_line(fd_phonebook->browser, 1);
        if (strstr(line, _("Empty Phonebook"))) {
            fl_clear_browser(fd_phonebook->browser);
            n = 0;
        }
    }
    for(i = 1; i <= n; ++i) {
        line = fl_get_browser_line(fd_phonebook->browser, i);
        if (!strncmp(buf, line, 46))
            break;
    }
    if (i <= n) {
        fl_delete_browser_line(fd_phonebook->browser, i);
        fl_set_object_label(fd_xsendfax->pb_save, _("Save (needed)"));
    }
    delete[] buf;
}


void cb_save_phoneno(FL_OBJECT *, long )
{
    char const
        *line;
    int
        i, n;
    FILE
        *fp;

    if (!(fp = fopen(phone_book.c_str(), "w"))) {
        WriteAlert(_("Error!"), _("Cannot open phone book: "), phone_book);
        return;
    }
    n = fl_get_browser_maxline(fd_phonebook->browser);
    if (n) {
        line = fl_get_browser_line(fd_phonebook->browser, 1);
        if (strstr(line, _("Empty Phonebook"))) {
            fl_clear_browser(fd_phonebook->browser);
            n = 0;
        }
    }
    for(i = 1; i <= n; ++i) {
        line = fl_get_browser_line(fd_phonebook->browser, i);
        fprintf(fp, "%s\n", line);
    }
    fclose(fp);
    fl_set_object_label(fd_xsendfax->pb_save, _("Save"));
    fl_redraw_form(fd_xsendfax->xsendfax);
}

void show_logfile(string logfile, bool show_if_empty)
{
    if (logfile.empty())
        return;
    if (!fl_load_browser(fd_logfile->browser, logfile.c_str())) {
        if (!show_if_empty)
            return;
        fl_add_browser_line(fd_logfile->browser,
                            _("NO OR EMPTY LOGFILE!"));
    }
    if (fd_logfile->logfile->visible) {
        fl_raise_form(fd_logfile->logfile);
    } else {
        fl_show_form(fd_logfile->logfile,
                     FL_PLACE_MOUSE | FL_FREE_SIZE, FL_FULLBORDER,
                     _("Message-Window"));
    }
}

void FaxLogfileCloseCB(FL_OBJECT *, long)
{
    fl_hide_form(fd_logfile->logfile);
}

void FaxCancelCB(FL_OBJECT *, long)
{
    fl_hide_form(fd_xsendfax->xsendfax);
}

void FaxApplyCB(FL_OBJECT *, long)
{
    button_send(filename, global_sendcmd);
}

void FaxSendCB(FL_OBJECT *, long)
{
    if (button_send(filename, global_sendcmd))
        FaxCancelCB(0, 0);
}

void FaxOpenPhonebookCB(FL_OBJECT *, long)
{
    int
        n= fl_get_browser_maxline(fd_phonebook->browser);

    if (!n)
        fl_addto_browser(fd_phonebook->browser, _("@L@b@cEmpty Phonebook"));
    fl_show_form(fd_phonebook->phonebook, FL_PLACE_MOUSE, FL_FULLBORDER,
                 _("Phonebook"));
}
