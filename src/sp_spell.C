/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file sp_pspell.C
 * \author Kevin Atkinson
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

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
   // BUT we cannot include both string.h and strings.h on Irix 6.5 :(
#  ifdef _AIX
#   include <strings.h>
#  endif
# endif
#include <sys/select.h>
#endif

#include "LString.h"
#include "support/lstrings.h"
#include "lyxrc.h"
#include "language.h"
#include "debug.h"
#include "encoding.h"
#include "sp_ispell.h"

using std::endl;

namespace {
	/// pid for the `ispell' process. 
	pid_t isp_pid = -1; 
}

///
// ------------------- start special pspell code/class --------------------
///
#ifdef USE_PSPELL

#include "support/LAssert.h"

#define USE_ORIGINAL_MANAGER_FUNCS 1
# include <pspell/pspell.h>

#include "sp_pspell.h"


PSpell::PSpell() 
	: sc(0), els(0), spell_error_object(0), flag(ISP_UNKNOWN),
	  alive_(false)
{
}


PSpell::PSpell(BufferParams const & params, string const & lang)
	: sc(0), els(0), spell_error_object(0), flag(ISP_UNKNOWN),
	  alive_(false)
{
	initialize(params, lang);
}


PSpell::~PSpell() 
{
	cleanUp();
	close();
	if (els)
		delete_pspell_string_emulation(els);
}
	

void PSpell::initialize(BufferParams const &, string const & lang)
{
	PspellConfig * config = new_pspell_config();
	config->replace("language-tag", lang.c_str());
	spell_error_object = new_pspell_manager(config);
	if (pspell_error_number(spell_error_object) != 0) {
		error_ = pspell_error_message(spell_error_object);
	} else {
		error_ = 0;
		sc = to_pspell_manager(spell_error_object);
		spell_error_object = 0;
		alive_ = true;
	}
}


void PSpell::cleanUp() 
{
	if (spell_error_object) {
		delete_pspell_can_have_error(spell_error_object);
		spell_error_object = 0;
	}
}


enum PSpell::spellStatus PSpell::check(string const & word)
{
	if (!sc)
		return flag;

	int word_ok = pspell_manager_check(sc, word.c_str());
	lyx::Assert(word_ok != -1);

	if (word_ok) {
		flag = ISP_OK;
	} else {
		PspellWordList const * sugs =
			pspell_manager_suggest(sc, word.c_str());
		lyx::Assert(sugs != 0);
		els = pspell_word_list_elements(sugs);
		if (pspell_word_list_empty(sugs)) 
			flag = ISP_UNKNOWN;
		else 
			flag = ISP_MISSED;
	}
	return flag;
}


void PSpell::close()
{
	if (sc)
		pspell_manager_save_all_word_lists(sc);
}


void PSpell::insert(string const & word)
{
	if (sc)
		pspell_manager_add_to_personal(sc, word.c_str());
}


void PSpell::accept(string const & word) 
{
	if (sc)
		pspell_manager_add_to_session(sc, word.c_str());
}


void PSpell::store(string const & mis, string const & cor)
{
	if (sc)
		pspell_manager_store_replacement(sc, mis.c_str(), cor.c_str());
}


char const * PSpell::nextMiss()
{
	if (els)
		return pspell_string_emulation_next(els);
	return "";
}


char const * PSpell::error()
{
	return error_;
}


#endif

///
// ------------------- start special ispell code/class --------------------
///

ISpell::ISpell()
	: str(0), flag(ISP_UNKNOWN)
{}


ISpell::ISpell(BufferParams const & params, string const & lang)
	: str(0), flag(ISP_UNKNOWN)
{
	initialize(params, lang);
}


ISpell::~ISpell()
{
	delete[] str;
}


char const * ISpell::nextMiss()
{
	if (str == 0 || *(e+1) == '\0') return 0;
	b = e + 2;
	e = strpbrk(b, ",\n");
	*e = '\0';
	return b;
}


void ISpell::initialize(BufferParams const & params, string const & lang)
{
	static char o_buf[BUFSIZ];  // jc: it could be smaller
	int pipein[2];
	int pipeout[2];
	char * argv[14];
	int argc;

	isp_pid = -1;

	if (pipe(pipein) == -1 || pipe(pipeout) == -1) {
		lyxerr << "LyX: Can't create pipe for spellchecker!" << endl;
		goto END;
	}

	if ((out = fdopen(pipein[1], "w")) == 0) {
		lyxerr << "LyX: Can't create stream for pipe for spellchecker!"
		       << endl;
		goto END;
	}

	if ((in = fdopen(pipeout[0], "r")) == 0) {
		lyxerr <<"LyX: Can't create stream for pipe for spellchecker!"
		       << endl;
		goto END;
	}

	setvbuf(out, o_buf, _IOLBF, BUFSIZ);

	isp_fd = pipeout[0];

	isp_pid = fork();

	if (isp_pid == -1) {
		lyxerr << "LyX: Can't create child process for spellchecker!"
		       << endl;
		goto END;
	}
	
	if (isp_pid == 0) {        
		/* child process */
		dup2(pipein[0], STDIN_FILENO);
		dup2(pipeout[1], STDOUT_FILENO);
		::close(pipein[0]);
		::close(pipein[1]);
		::close(pipeout[0]);
		::close(pipeout[1]);

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
			string enc = (params.inputenc == "auto")
				? params.language->encoding()->LatexName()
				: params.inputenc;
			string::size_type n = enc.length();
			tmp = new char[3];
			string("-T").copy(tmp, 2);
			tmp[2] = '\0';
			argv[argc++] = tmp; // Input encoding
			tmp = new char[n + 1];
			enc.copy(tmp, n);
			tmp[n] = '\0';
			argv[argc++] = tmp;
		}

		argv[argc++] = 0;

		execvp(argv[0], const_cast<char * const *>(argv));

		// free the memory used by string::copy in the
		// setup of argv
		for (int i = 0; i < argc - 1; ++i)
			delete[] argv[i];
		
		lyxerr << "LyX: Failed to start ispell!" << endl;
		_exit(0);
	}
	{
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

			fputs("!\n", out); // Set terse mode (silently accept correct words)

		
		} else if (retval == 0) {
			// timeout. Give nice message to user.
			lyxerr << "Ispell read timed out, what now?" << endl;
			// This probably works but could need some thought
			isp_pid = -1;
			::close(pipeout[0]);
			::close(pipeout[1]);
			::close(pipein[0]);
			::close(pipein[1]);
			isp_fd = -1;
		} else {
			// Select returned error
			lyxerr << "Select on ispell returned error, what now?" << endl;
		}
	}
  END:
	if (isp_pid == -1) {
		error_ = 
			"\n\n"
			"The spellcheck-process has died for some reason.\n"
			"*One* possible reason could be that you do not have\n"
			"a dictionary file for the language of this document\n"
			"installed.\n"
			"Check your spellchecker or set another dictionary\n"
			"in the Spellchecker Options menu.\n\n";
	} else {
		error_ = 0;
	}
}


/* FIXME: this is a minimalist solution until the above
 * code is able to work with forkedcall.h. We only need
 * to reap the zombies here.
 */
void reapSpellchecker(void)
{
	if (isp_pid == -1) 
		return;

	waitpid(isp_pid, 0, WNOHANG);
}

 
bool ISpell::alive()
{
	return isp_pid != -1;
}


void ISpell::cleanUp() 
{
	::fclose(out);
}


enum ISpell::spellStatus ISpell::check(string const & word)
{
	//Please rewrite to use string.

	::fputs(word.c_str(), out);
	::fputc('\n', out);
  
	char buf[1024];
	::fgets(buf, 1024, in); 
  
	// I think we have to check if ispell is still alive here because
	// the signal-handler could have disabled blocking on the fd 
	if (!alive()) return ISP_UNKNOWN;

	switch (*buf) {
	case '*': // Word found
		flag = ISP_OK;
		break;
	case '+': // Word found through affix removal
		flag = ISP_ROOT;
		break;
	case '-': // Word found through compound formation
		flag = ISP_COMPOUNDWORD;
		break;
	case '\n': // Number or when in terse mode: no problems
		flag = ISP_IGNORE;
		break;
	case '#': // Not found, no near misses and guesses
		flag = ISP_UNKNOWN;
		break;
	case '?': // Not found, and no near misses, but guesses (guesses are ignored)
	case '&': // Not found, but we have near misses
	{
		flag = ISP_MISSED;
		char * p = strpbrk(buf, ":");
		str = new char[strlen(p) + 1];
		e   = str;
		strcpy(str, p);
		break;
	}
	default: // This shouldn't happend, but you know Murphy
		flag = ISP_UNKNOWN;
	}

	*buf = 0;
	if (flag!= ISP_IGNORE) {
		while (*buf!= '\n') fgets(buf, 255, in); /* wait for ispell to finish */
	}
	return flag;
}


void ISpell::close()
{
	// Note: If you decide to optimize this out when it is not 
	// needed please note that when Aspell is used this command 
	// is also needed to save the replacement dictionary.
	// -- Kevin Atkinson (kevinatk@home.com)

	fputs("#\n", out); // Save personal dictionary

	fflush(out);
	fclose(out);
}


void ISpell::insert(string const & word)
{
	::fputc('*', out); // Insert word in personal dictionary
	::fputs(word.c_str(), out);
	::fputc('\n', out);
}


void ISpell::accept(string const & word) 
{
	::fputc('@', out); // Accept in this session
	::fputs(word.c_str(), out);
	::fputc('\n', out);
}


void ISpell::store(string const & mis, string const & cor)
{
	if (actual_spell_checker == ASC_ASPELL) {
		::fputs("$$ra ", out);
		::fputs(mis.c_str(), out);
		::fputc(',', out);
		::fputs(cor.c_str(), out);
		::fputc('\n', out);
	}
}


char const * ISpell::error()
{
	return error_;
}
