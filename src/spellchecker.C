/*
 *This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-1998 The LyX Team
 *
 * ====================================================== 
 */

#include <config.h>

#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <sys/wait.h>
#include <sys/types.h>
#include <cctype>
#include FORMS_H_LOCATION

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <ctime>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <ctime>
# endif
#endif

#ifdef HAVE_SYS_SELECT_H
# ifdef HAVE_STRINGS_H
   // <strings.h> is needed at least on AIX because FD_ZERO uses bzero().
#  include <strings.h> 
# endif
#include <sys/select.h>
#endif

#include <algorithm>

#include "LString.h"
#include "sp_form.h"
#include "spellchecker.h"
#include "lyx_main.h"
#include "buffer.h"
#include "lyxrc.h"
#include "BufferView.h"
#include "gettext.h"
#include "lyx_gui_misc.h"
#include "debug.h"
#include "support/lstrings.h"

using std::reverse;
using std::endl;

// Spellchecker status
enum {
	ISP_OK = 1,
	ISP_ROOT,
	ISP_COMPOUNDWORD,
	ISP_UNKNOWN,
	ISP_MISSED,
	ISP_IGNORE
};

static bool RunSpellChecker(BufferView * bv);

static FILE * in, * out;  /* streams to communicate with ispell */
pid_t isp_pid = -1; // pid for the `ispell' process. Also used (RO) in
                    // lyx_cb.C

// the true spell checker program being used
enum ActualSpellChecker {ASC_ISPELL, ASC_ASPELL};
static ActualSpellChecker actual_spell_checker;

static int isp_fd;

static FD_form_spell_options *fd_form_spell_options = 0;
FD_form_spell_check *fd_form_spell_check = 0;

//void sigchldhandler(int sig);
void sigchldhandler(pid_t pid, int *status);

//extern void sigchldchecker(int sig);
extern void sigchldchecker(pid_t pid, int *status);

struct isp_result {
	int flag;
	int count;
	string str;
	char ** misses;
	isp_result() {
		flag = ISP_UNKNOWN;
		count = 0;
		misses = static_cast<char**>(0);
	}
	~isp_result() {
		delete[] misses;
	}
};


/***** Spellchecker options *****/

// Rewritten to use ordinary LyX xforms loop and OK, Apply and Cancel set,
// now also string. Amazing, eh? (Asger)

// Set (sane) values in form to current spellchecker options
void SpellOptionsUpdate() 
{
	// Alternate language
	if (lyxrc.isp_alt_lang.empty()) {
		lyxrc.isp_use_alt_lang = false;
	} else {
		fl_set_input(fd_form_spell_options->altlang_input,
			     lyxrc.isp_alt_lang.c_str());
	}
	if (lyxrc.isp_use_alt_lang) {
		fl_set_button(fd_form_spell_options->buflang, 0);
		fl_set_button(fd_form_spell_options->altlang, 1);
	} else {
		fl_set_button(fd_form_spell_options->buflang, 1);
		fl_set_button(fd_form_spell_options->altlang, 0);
	}  

	// Personal dictionary
	if (lyxrc.isp_pers_dict.empty()) {
		lyxrc.isp_use_pers_dict = false;
	} else {
		fl_set_input(fd_form_spell_options->perdict_input,
			     lyxrc.isp_pers_dict.c_str());
	}
	fl_set_button(fd_form_spell_options->perdict,
		      lyxrc.isp_use_pers_dict ? 1:0);

	// Escape chars
	if (lyxrc.isp_esc_chars.empty()) {
		lyxrc.isp_use_esc_chars = false;
	} else {
		fl_set_input(fd_form_spell_options->esc_chars_input,
			     lyxrc.isp_esc_chars.c_str());
	}
	fl_set_button(fd_form_spell_options->esc_chars,
		      lyxrc.isp_use_esc_chars ? 1:0);

	// Options
	fl_set_button(fd_form_spell_options->compounds,
		      lyxrc.isp_accept_compound ? 1 : 0);
	fl_set_button(fd_form_spell_options->inpenc,
		      lyxrc.isp_use_input_encoding ? 1 : 0);
}

// Update spellchecker options
void SpellOptionsApplyCB(FL_OBJECT *, long)
{
	// Build new status from form data
	lyxrc.isp_use_alt_lang = 
		fl_get_button(fd_form_spell_options->altlang);
	lyxrc.isp_use_pers_dict = 
		fl_get_button(fd_form_spell_options->perdict);
	lyxrc.isp_accept_compound = 
		fl_get_button(fd_form_spell_options->compounds);
	lyxrc.isp_use_esc_chars = 
		fl_get_button(fd_form_spell_options->esc_chars);
	lyxrc.isp_use_input_encoding = 
		fl_get_button(fd_form_spell_options->inpenc);

	// Update strings with data from input fields
	lyxrc.isp_alt_lang = 
		fl_get_input(fd_form_spell_options->altlang_input);
	lyxrc.isp_pers_dict = 
		fl_get_input(fd_form_spell_options->perdict_input);
	lyxrc.isp_esc_chars = 
		fl_get_input(fd_form_spell_options->esc_chars_input);

	// Update form
	SpellOptionsUpdate();
}


void SpellOptionsCancelCB(FL_OBJECT *, long)
{
	fl_hide_form(fd_form_spell_options->form_spell_options);
}


void SpellOptionsOKCB(FL_OBJECT * ob, long data)
{
	SpellOptionsApplyCB(ob, data);
	SpellOptionsCancelCB(ob, data);
}


// Show spellchecker options form
void SpellCheckerOptions()
{
	// Create form if nescessary
	if (fd_form_spell_options ==  0) {
		fd_form_spell_options = create_form_form_spell_options();
		// Make sure pressing the close box does not kill LyX. (RvdK)
		fl_set_form_atclose(fd_form_spell_options->form_spell_options, 
				    CancelCloseBoxCB, 0);
	}

	// Update form to current options
	SpellOptionsUpdate();

	// Focus in alternate language field
	fl_set_focus_object(fd_form_spell_options->form_spell_options,
			    fd_form_spell_options->altlang_input);

	// Show form
	if (fd_form_spell_options->form_spell_options->visible) {
		fl_raise_form(fd_form_spell_options->form_spell_options);
	} else {
		fl_show_form(fd_form_spell_options->form_spell_options,
			     FL_PLACE_MOUSE, FL_FULLBORDER,
			     _("Spellchecker Options"));
	}
}


/***** Spellchecker *****/

// Could also use a clean up. (Asger Alstrup)

static
void create_ispell_pipe(BufferParams const & params, string const & lang)
{
	static char o_buf[BUFSIZ];  // jc: it could be smaller
	int pipein[2], pipeout[2];
	char * argv[14];
	int argc;

	isp_pid = -1;

	if(pipe(pipein) == -1 || pipe(pipeout) == -1) {
		lyxerr << "LyX: Can't create pipe for spellchecker!" << endl;
		return;
	}

	if ((out = fdopen(pipein[1], "w")) == 0) {
		lyxerr << "LyX: Can't create stream for pipe for spellchecker!"
		       << endl;
		return;
	}

	if ((in = fdopen(pipeout[0], "r")) == 0) {
		lyxerr <<"LyX: Can't create stream for pipe for spellchecker!"
		       << endl;
		return;
	}

	setvbuf(out, o_buf, _IOLBF, BUFSIZ);

	isp_fd = pipeout[0];

	isp_pid = fork();

	if(isp_pid == -1) {
		lyxerr << "LyX: Can't create child process for spellchecker!"
		       << endl;
		return;
	}

	if(isp_pid == 0) {        
		/* child process */
		dup2(pipein[0], STDIN_FILENO);
		dup2(pipeout[1], STDOUT_FILENO);
		close(pipein[0]);
		close(pipein[1]);
		close(pipeout[0]);
		close(pipeout[1]);

		argc = 0;
		char * tmp = new char[lyxrc.isp_command.length() + 1];
		lyxrc.isp_command.copy(tmp, lyxrc.isp_command.length());
		tmp[lyxrc.isp_command.length()] = '\0';
		argv[argc++] = tmp;
		tmp = new char[3];
		string("-a").copy(tmp, 2); tmp[2] = '\0'; // pipe mode
		argv[argc++] = tmp;

		if (lang != "default") {
			tmp = new char[3];
			string("-d").copy(tmp, 2); tmp[2] = '\0'; // Dictionary file
			argv[argc++] = tmp;
			tmp = new char[lang.length() + 1];
			lang.copy(tmp, lang.length()); tmp[lang.length()] = '\0';
			argv[argc++] = tmp;
		}

		if (lyxrc.isp_accept_compound) {
			// Consider run-together words as legal compounds
			tmp = new char[3];
			string("-C").copy(tmp, 2); tmp[2] = '\0';
			argv[argc++] = tmp;
		} else {
			// Report run-together words with
			// missing blanks as errors
			tmp = new char[3]; 
			string("-B").copy(tmp, 2); tmp[2] = '\0';
			argv[argc++] = tmp; 
		}
		if (lyxrc.isp_use_esc_chars) {
			// Specify additional characters that
			// can be part of a word
			tmp = new char[3];
			string("-w").copy(tmp, 2); tmp[2] = '\0';
			argv[argc++] = tmp; 
			// Put the escape chars in ""s
			string tms = "\"" + lyxrc.isp_esc_chars + "\"";
			tmp = new char[tms.length() + 1];
			tms.copy(tmp, tms.length()); tmp[tms.length()] = '\0';
			argv[argc++] = tmp;
		}
		if (lyxrc.isp_use_pers_dict) {
			// Specify an alternate personal dictionary
			tmp = new char[3];
			string("-p").copy(tmp, 2);
			tmp[2]= '\0';
			argv[argc++] = tmp;
			tmp = new char[lyxrc.isp_pers_dict.length() + 1];
			lyxrc.isp_pers_dict.copy(tmp, lyxrc.isp_pers_dict.length());
			tmp[lyxrc.isp_pers_dict.length()] = '\0';
			argv[argc++] = tmp;
		}
		if (lyxrc.isp_use_input_encoding &&
        	    params.inputenc != "default") {
			tmp = new char[3];
			string("-T").copy(tmp, 2); tmp[2] = '\0';
			argv[argc++] = tmp; // Input encoding
			tmp = new char[params.inputenc.length() + 1];
			params.inputenc.copy(tmp, params.inputenc.length());
			tmp[params.inputenc.length()] = '\0';
			argv[argc++] = tmp;
		}

		argv[argc++] = 0;

		execvp(argv[0], const_cast<char * const *>(argv));

		// free the memory used by string::copy in the
		// setup of argv
		for (int i= 0; i < argc -1; ++i)
			delete[] argv[i];
		
		lyxerr << "LyX: Failed to start ispell!" << endl;
		_exit(0);
	}

	/* Parent process: Read ispells identification message */
	// Hmm...what are we using this id msg for? Nothing? (Lgb)
	// Actually I used it to tell if it's truly Ispell or if it's
	// aspell -- (kevinatk@home.com)
	char buf[2048];
	fd_set infds;
	struct timeval tv;
	int retval = 0;
	FD_ZERO(&infds);
	FD_SET(pipeout[0], &infds);
	tv.tv_sec = 15; // fifteen second timeout. Probably too much,
			// but it can't really hurt.
	tv.tv_usec = 0;

	// Configure provides us with macros which are supposed to do
	// the right typecast.
	retval = select(SELECT_TYPE_ARG1 (pipeout[0]+1), 
			SELECT_TYPE_ARG234 (&infds), 
			0, 
			0, 
			SELECT_TYPE_ARG5 (&tv));

	if (retval > 0) {
		// Ok, do the reading. We don't have to FD_ISSET since
		// there is only one fd in infds.
		fgets(buf, 2048, in);
		
		// determine if the spell checker is really Aspell
		if (strstr(buf, "Aspell"))
		  actual_spell_checker = ASC_ASPELL;
		else
		  actual_spell_checker = ASC_ISPELL;
		
	} else if (retval == 0) {
		// timeout. Give nice message to user.
		lyxerr << "Ispell read timed out, what now?" << endl;
		// This probably works but could need some thought
		isp_pid = -1;
		close(pipeout[0]); close(pipeout[1]);
		close(pipein[0]); close(pipein[1]);
		isp_fd = -1;
	} else {
		// Select returned error
		lyxerr << "Select on ispell returned error, what now?" << endl;
	}
}


// Send word to ispell and get reply
static
isp_result *ispell_check_word(char *word)
{
	//Please rewrite to use string.
	isp_result *result;
	char buf[1024], *p;
	int count, i;

	fputs(word, out); 
	fputc('\n', out);
  
	fgets(buf, 1024, in); 
  
	/* I think we have to check if ispell is still alive here because
	   the signal-handler could have disabled blocking on the fd */
	if (isp_pid == -1) return 0;

	result = new isp_result;
  
	switch (*buf) {
	case '*': // Word found
		result->flag = ISP_OK;
		break;
	case '+': // Word found through affix removal
		result->flag = ISP_ROOT;
		break;
	case '-': // Word found through compound formation
		result->flag = ISP_COMPOUNDWORD;
		break;
	case '\n': // Number or when in terse mode: no problems
		result->flag = ISP_IGNORE;
		break;
	case '#': // Not found, no near misses and guesses
		result->flag = ISP_UNKNOWN;
		break;
	case '?': // Not found, and no near misses, but guesses (guesses are ignored)
	case '&': // Not found, but we have near misses
	{
		result->flag = ISP_MISSED;
		result->str = buf;
		// nb is leaked! where should it be freed? I have to
		// admit I do not understand the intent of the code :(
		// (JMarc) 
		char * nb = new char[result->str.length() + 1];
		result->str.copy(nb, result->str.length());
		nb[result->str.length()]= '\0';
		p = strpbrk(nb+2, " ");
		sscanf(p, "%d", &count); // Get near misses count
		result->count = count;
		if (count) result->misses = new char*[count];
		p = strpbrk(nb, ":");
		p += 2;
		for (i = 0; i < count; ++i) {
			result->misses[i] = p;
			p = strpbrk(p, ",\n");
			*p = 0;
			p += 2;
		}
		break;
	}
	default: // This shouldn't happend, but you know Murphy
		result->flag = ISP_UNKNOWN;
	}

	*buf = 0;
	if (result->flag!= ISP_IGNORE) {
		while (*buf!= '\n') fgets(buf, 255, in); /* wait for ispell to finish */
	}
	return result;
}


static inline
void ispell_terminate()
{
        // Note: If you decide to optimize this out when it is not 
        // needed please note that when Aspell is used this command 
        // is also needed to save the replacement dictionary.
        // -- Kevin Atkinson (kevinatk@home.com)

	fputs("#\n", out); // Save personal dictionary

	fflush(out);
	fclose(out);
}


static inline
void ispell_terse_mode()
{
	fputs("!\n", out); // Set terse mode (silently accept correct words)
}


static inline
void ispell_insert_word(char const *word)
{
	fputc('*', out); // Insert word in personal dictionary
	fputs(word, out);
	fputc('\n', out);
}


static inline
void ispell_accept_word(char const *word) 
{
	fputc('@', out); // Accept in this session
	fputs(word, out);
	fputc('\n', out);
}

static inline
void ispell_store_replacement(char const *mis, string const & cor) {
        if(actual_spell_checker == ASC_ASPELL) {
                fputs("$$ra ", out);
                fputs(mis, out);
                fputc(',', out);
                fputs(cor.c_str(), out);
                fputc('\n', out);
        }
}


void ShowSpellChecker(BufferView * bv)
{
	FL_OBJECT * obj;
	int ret;

	// Exit if we don't have a document open
	if (!bv->available())
		return;

	if (fd_form_spell_check == 0) {
		fd_form_spell_check = create_form_form_spell_check();
		// Make sure pressing the close box does not kill LyX. (RvdK)
		fl_set_form_atclose(fd_form_spell_check->form_spell_check, IgnoreCloseBoxCB, 0);
	}

	// Clear form
	fl_set_slider_value(fd_form_spell_check->slider, 0);
	fl_set_slider_bounds(fd_form_spell_check->slider, 0, 100);
	fl_set_object_label(fd_form_spell_check->text, "");
	fl_set_input(fd_form_spell_check->input, "");
	fl_clear_browser(fd_form_spell_check->browser);

	// Show form
	if (fd_form_spell_check->form_spell_check->visible) {
		fl_raise_form(fd_form_spell_check->form_spell_check);
	} else {
		fl_show_form(fd_form_spell_check->form_spell_check,
			     FL_PLACE_MOUSE, FL_FULLBORDER,
			     _("Spellchecker"));
	}
	fl_deactivate_object(fd_form_spell_check->slider); 

	// deactivate insert, accept, replace, and stop
	fl_deactivate_object(fd_form_spell_check->insert);
	fl_deactivate_object(fd_form_spell_check->accept);
	fl_deactivate_object(fd_form_spell_check->ignore);
	fl_deactivate_object(fd_form_spell_check->replace);
	fl_deactivate_object(fd_form_spell_check->stop);
	fl_deactivate_object(fd_form_spell_check->input);
	fl_deactivate_object(fd_form_spell_check->browser);
	fl_set_object_lcol(fd_form_spell_check->insert, FL_INACTIVE);
	fl_set_object_lcol(fd_form_spell_check->accept, FL_INACTIVE);
	fl_set_object_lcol(fd_form_spell_check->ignore, FL_INACTIVE);
	fl_set_object_lcol(fd_form_spell_check->replace, FL_INACTIVE);
	fl_set_object_lcol(fd_form_spell_check->stop, FL_INACTIVE);
	fl_set_object_lcol(fd_form_spell_check->input, FL_INACTIVE);
	fl_set_object_lcol(fd_form_spell_check->browser, FL_INACTIVE);

	while (true){
		obj = fl_do_forms();
		if (obj == fd_form_spell_check->options){
			SpellCheckerOptions();
		}
		if (obj == fd_form_spell_check->start){
			// activate insert, accept, and stop
			fl_activate_object(fd_form_spell_check->insert);
			fl_activate_object(fd_form_spell_check->accept);
			fl_activate_object(fd_form_spell_check->ignore);
			fl_activate_object(fd_form_spell_check->stop);
			fl_activate_object(fd_form_spell_check->input);
			fl_activate_object(fd_form_spell_check->browser);
			fl_set_object_lcol(fd_form_spell_check->insert, FL_BLACK);
			fl_set_object_lcol(fd_form_spell_check->accept, FL_BLACK);
			fl_set_object_lcol(fd_form_spell_check->ignore, FL_BLACK);
			fl_set_object_lcol(fd_form_spell_check->stop, FL_BLACK);
			fl_set_object_lcol(fd_form_spell_check->input, FL_BLACK);
			fl_set_object_lcol(fd_form_spell_check->browser, FL_BLACK);
			// activate replace only if the file is not read-only
			if (!bv->buffer()->isReadonly()) { 
			  fl_activate_object(fd_form_spell_check->replace);
			  fl_set_object_lcol(fd_form_spell_check->replace, FL_BLACK);
			}

			// deactivate options and start
			fl_deactivate_object(fd_form_spell_check->options);
			fl_deactivate_object(fd_form_spell_check->start);
			fl_set_object_lcol(fd_form_spell_check->options, FL_INACTIVE);
			fl_set_object_lcol(fd_form_spell_check->start, FL_INACTIVE);

			ret = RunSpellChecker(bv);

			// deactivate insert, accept, replace, and stop
			fl_deactivate_object(fd_form_spell_check->insert);
			fl_deactivate_object(fd_form_spell_check->accept);
			fl_deactivate_object(fd_form_spell_check->ignore);
			fl_deactivate_object(fd_form_spell_check->replace);
			fl_deactivate_object(fd_form_spell_check->stop);
			fl_deactivate_object(fd_form_spell_check->input);
			fl_deactivate_object(fd_form_spell_check->browser);
			fl_set_object_lcol(fd_form_spell_check->insert, FL_INACTIVE);
			fl_set_object_lcol(fd_form_spell_check->accept, FL_INACTIVE);
			fl_set_object_lcol(fd_form_spell_check->ignore, FL_INACTIVE);
			fl_set_object_lcol(fd_form_spell_check->replace, FL_INACTIVE);
			fl_set_object_lcol(fd_form_spell_check->stop, FL_INACTIVE);
			fl_set_object_lcol(fd_form_spell_check->input, FL_INACTIVE);
			fl_set_object_lcol(fd_form_spell_check->browser, FL_INACTIVE);

			// activate options and start
			fl_activate_object(fd_form_spell_check->options);
			fl_activate_object(fd_form_spell_check->start);
			fl_set_object_lcol(fd_form_spell_check->options, FL_BLACK);
			fl_set_object_lcol(fd_form_spell_check->start, FL_BLACK);

			// if RunSpellChecker returns false quit spellchecker
			if (!ret) break;
		}
		if (obj == fd_form_spell_check->done) break;
	}
	fl_hide_form(fd_form_spell_check->form_spell_check);
	bv->endOfSpellCheck();
	return;
}


// Perform an ispell session
static
bool RunSpellChecker(BufferView * bv)
{
	isp_result * result;
	int i, newvalue;
	FL_OBJECT * obj;

	string tmp = (lyxrc.isp_use_alt_lang) ? lyxrc.isp_alt_lang : bv->buffer()->GetLanguage();
	bool rtl = tmp == "hebrew" || tmp == "arabic";

	int oldval = 0;  /* used for updating slider only when needed */
	float newval = 0.0;
   
	/* create ispell process */
	create_ispell_pipe(bv->buffer()->params, tmp);

	if (isp_pid == -1) {
		fl_show_message(
			_("\n\n"
			  "The ispell-process has died for some reason. *One* possible reason\n"
			  "could be that you do not have a dictionary file\n"
			  "for the language of this document installed.\n"
			  "Check /usr/lib/ispell or set another\n"
			  "dictionary in the Spellchecker Options menu."), "", "");
		fclose(out);
		return false;
	}

	// Put ispell in terse mode to improve speed
	ispell_terse_mode();

	unsigned int word_count = 0;
	while (true) {
		char * word = bv->nextWord(newval);
		if (word == 0) break;
		++word_count;
		
		// Update slider if and only if value has changed
		newvalue = int(100.0*newval);
		if(newvalue!= oldval) {
			oldval = newvalue;
			fl_set_slider_value(fd_form_spell_check->slider, oldval);
 		}

		if (word_count%1000 == 0) {
			obj =  fl_check_forms();
			if (obj == fd_form_spell_check->stop) {
				delete[] word;
				ispell_terminate();
				return true;
			}
			if (obj == fd_form_spell_check->done) {
				delete[] word;
				ispell_terminate(); 
				return false;
			}
		}

		result = ispell_check_word(word);
		if (isp_pid == -1) {
			delete result;
			delete[] word;
			break;
		}

		switch (result->flag) {
		case ISP_UNKNOWN:
		case ISP_MISSED:
		{
			bv->selectLastWord();
			if (rtl) {
				string tmp = word;
				reverse(tmp.begin(),tmp.end());
				fl_set_object_label(fd_form_spell_check->text, tmp.c_str());
			} else
				fl_set_object_label(fd_form_spell_check->text, word);
			fl_set_input(fd_form_spell_check->input, word);
			fl_clear_browser(fd_form_spell_check->browser);
			for (i = 0; i < result->count; ++i) {
				if (rtl) {
					string tmp = result->misses[i];
					reverse(tmp.begin(),tmp.end());
					fl_add_browser_line(fd_form_spell_check->browser, tmp.c_str());
				} else
					fl_add_browser_line(fd_form_spell_check->browser, result->misses[i]);
			}

			int clickline = -1;
			while (true) {
				obj = fl_do_forms();
				if (obj == fd_form_spell_check->insert) {
					ispell_insert_word(word);
					break;
				}
				if (obj == fd_form_spell_check->accept) {
					ispell_accept_word(word);
					break;
				}
				if (obj == fd_form_spell_check->ignore) {
					break;
				}
				if (obj == fd_form_spell_check->replace || 
				    obj == fd_form_spell_check->input) {
				        ispell_store_replacement(word, fl_get_input(fd_form_spell_check->input));
					bv->replaceWord(fl_get_input(fd_form_spell_check->input));
					break;
				}
				if (obj == fd_form_spell_check->browser) {
					// implements double click in the browser window.
					// sent to lyx@via by Mark Burton <mark@cbl.leeds.ac.uk>
					if (clickline == 
					    fl_get_browser(fd_form_spell_check->browser)) {
				                ispell_store_replacement(word, fl_get_input(fd_form_spell_check->input));
						bv->replaceWord(fl_get_input(fd_form_spell_check->input));
						break;
					}
					clickline = fl_get_browser(fd_form_spell_check->browser);
					/// Why not use
					/// fl_set_input(fd_form_spell_check->input, result->misses[clickline-1]); ?
					if (rtl) {
						string tmp = fl_get_browser_line(fd_form_spell_check->browser,
										 clickline);
						reverse(tmp.begin(),tmp.end());
						fl_set_input(fd_form_spell_check->input, tmp.c_str());
					} else
						fl_set_input(fd_form_spell_check->input,
							     fl_get_browser_line(fd_form_spell_check->browser,
										 clickline));
				}
				if (obj == fd_form_spell_check->stop) {
					delete result;
					delete[] word;
					ispell_terminate();
					return true;
				}
	    
				if (obj == fd_form_spell_check->done) {
					delete result;
					delete[] word;
					ispell_terminate();
					return false;
				}
			}
		}
		default:
			delete result;
			delete[] word;
		}
	}
   
	if(isp_pid!= -1) {
		ispell_terminate();
		string word_msg(tostr(word_count));
		if (word_count != 1) {
			word_msg += _(" words checked.");
		} else {
			word_msg += _(" word checked.");
		}
		fl_show_message("", _("Spellchecking completed!"),
				word_msg.c_str());
		return false;
	} else {
		fl_show_message(_("The ispell-process has died for some reason.\n"
				"Maybe it has been killed."), "", "");
		fclose(out);
		return true;
	}
}


void sigchldhandler(pid_t pid, int * status)
{ 
	if (isp_pid > 0)
		if (pid == isp_pid) {
			isp_pid= -1;
			fcntl(isp_fd, F_SETFL, O_NONBLOCK); /* set the file descriptor
							       to nonblocking so we can 
							       continue */
		}
	sigchldchecker(pid, status);
}
