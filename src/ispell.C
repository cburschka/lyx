/**
 * \file ispell.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author unknown
 * \author John Levon <levon@movementarian.org>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "LString.h"
#include "lyxrc.h"
#include "language.h"
#include "debug.h"
#include "encoding.h"
#include "ispell.h"
#include "WordLangTuple.h"
#include "gettext.h"

#include "support/forkedcall.h"
#include "support/lstrings.h"

#include <sys/select.h>

#ifndef CXX_GLOBAL_CSTD
using std::strcpy;
using std::strlen;
using std::strpbrk;
using std::strstr;
#endif

using std::endl;
using std::max;

namespace {

class LaunchIspell : public ForkedProcess {
public:
	///
	LaunchIspell(BufferParams const & p, string const & l,
		     int * in, int * out, int * err)
		: params(p), lang(l), pipein(in), pipeout(out), pipeerr(err) {}
	///
	virtual ForkedProcess * clone() const {
		return new LaunchIspell(*this);
	}
	///
	int start();
private:
	///
	virtual int generateChild();

	///
	BufferParams const & params;
	string const & lang;
	int * const pipein;
	int * const pipeout;
	int * const pipeerr;
};


int LaunchIspell::start()
{
	command_ = "ispell";
	return runNonBlocking();
}


int LaunchIspell::generateChild()
{
	pid_t isp_pid = fork();

	if (isp_pid != 0) {
		// failed (-1) or parent process (>0)
		return isp_pid;
	}

	// child process
	dup2(pipein[0], STDIN_FILENO);
	dup2(pipeout[1], STDOUT_FILENO);
	dup2(pipeerr[1], STDERR_FILENO);
	close(pipein[0]);
	close(pipein[1]);
	close(pipeout[0]);
	close(pipeout[1]);
	close(pipeerr[0]);
	close(pipeerr[1]);

	char * argv[14];
	int argc = 0;

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
		string tms = '"' + lyxrc.isp_esc_chars + '"';
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


} // namespace anon


ISpell::ISpell(BufferParams const & params, string const & lang)
	: in(0), out(0), inerr(0), str(0)
{
	lyxerr[Debug::GUI] << "Created ispell" << endl;

	// static due to the setvbuf. Ugly.
	static char o_buf[BUFSIZ];
	
	// We need to throw an exception not do this
	pipein[0] = pipein[1] = pipeout[0] = pipeout[1]
		= pipeerr[0] = pipeerr[1] = -1;

	// This is what happens when goto gets banned.

	if (pipe(pipein) == -1) {
		error_ = _("Can't create pipe for spellchecker.");
		return;
	}

	if (pipe(pipeout) == -1) {
		close(pipein[0]);
		close(pipein[1]);
		error_ = _("Can't create pipe for spellchecker.");
		return;
	}

	if (pipe(pipeerr) == -1) {
		close(pipein[0]);
		close(pipein[1]);
		close(pipeout[0]);
		close(pipeout[1]);
		error_ = _("Can't create pipe for spellchecker.");
		return;
	}

	if ((out = fdopen(pipein[1], "w")) == 0) {
		error_ = _("Can't open pipe for spellchecker.");
		return;
	}

	if ((in = fdopen(pipeout[0], "r")) == 0) {
		error_ = _("Can't open pipe for spellchecker.");
		return;
	}

	if ((inerr = fdopen(pipeerr[0], "r")) == 0) {
		error_ = _("Can't open pipe for spellchecker.");
		return;
	}

	setvbuf(out, o_buf, _IOLBF, BUFSIZ);

	LaunchIspell * li = new LaunchIspell(params, lang, pipein, pipeout, pipeerr);
	child_.reset(li);
	if (li->start() == -1) {
		error_ = _("Could not create an ispell process.\nYou may not have "
			" the right languages installed.");	
		child_.reset(0);
		return;
	}

	/* Parent process: Read ispells identification message */

	bool err_read;
	bool error = select(err_read);

	if (!error) {
		if (!err_read) {
			// Set terse mode (silently accept correct words)
			fputs("!\n", out);
			return;
		}

		/* must have read something from stderr */
		error_ = buf;
	} else {
		// select returned error
		error_ = _("The spell process returned an error.\nPerhaps "
				"it has been configured wrongly ?");
	}

	close(pipein[0]);
	close(pipein[1]);
	close(pipeout[0]);
	close(pipeout[1]);
	close(pipeerr[0]);
	close(pipeerr[1]);
	child_->kill();
	child_.reset(0);
}


ISpell::~ISpell()
{
	lyxerr[Debug::GUI] << "Killing ispell" << endl;

	if (in)
		fclose(in);

	if (inerr)
		fclose(inerr);

	if (out) {
		fputs("#\n", out); // Save personal dictionary

		fflush(out);
		fclose(out);
	}

	close(pipein[0]);
	close(pipein[1]);
	close(pipeout[0]);
	close(pipeout[1]);
	close(pipeerr[0]);
	close(pipeerr[1]);
	delete [] str;
}


bool ISpell::select(bool & err_read)
{
	fd_set infds;
	struct timeval tv;
	int retval = 0;
	FD_ZERO(&infds);
	FD_SET(pipeout[0], &infds);
	FD_SET(pipeerr[0], &infds);
	tv.tv_sec = 2;
	tv.tv_usec = 0;

	retval = ::select(SELECT_TYPE_ARG1 (max(pipeout[0], pipeerr[0]) + 1),
			SELECT_TYPE_ARG234 (&infds),
			0,
			0,
			SELECT_TYPE_ARG5 (&tv));

	// error
	if (retval <= 0)
		return true;

	if (FD_ISSET(pipeerr[0], &infds)) {
		fgets(buf, BUFSIZ, inerr);
		err_read = true;
		return false;
	}

	fgets(buf, BUFSIZ, in);
	err_read = false;
	return false;
}


string const ISpell::nextMiss()
{
	// Well, somebody is a sick fuck.

	if (str == 0 || *(e+1) == '\0')
		return "";
	char * b = e + 2;
	e = strpbrk(b, ",\n");
	*e = '\0';
	if (b)
		return b;
	return "";
}


bool ISpell::alive()
{
	return child_.get() && child_->running();
}


enum ISpell::Result ISpell::check(WordLangTuple const & word)
{
	// FIXME Please rewrite to use string.

	Result res;

	::fputs(word.word().c_str(), out);
	::fputc('\n', out);

	bool err_read;
	bool error = select(err_read);

	if (error) {
		error_ = _("Could not communicate with the spell-checker program");
		return UNKNOWN;
	}

	if (err_read) {
		error_ = buf;
		return UNKNOWN;
	}

	// I think we have to check if ispell is still alive here because
	// the signal-handler could have disabled blocking on the fd
	if (!alive())
		return UNKNOWN;

	switch (*buf) {
	case '*':
		res = OK;
		break;
	case '+':
		res = ROOT;
		break;
	case '-':
		res = COMPOUNDWORD;
		break;
	case '\n':
		res = IGNORE;
		break;
	case '#': // Not found, no near misses and guesses
		res = UNKNOWN;
		break;
	case '?': // Not found, and no near misses, but guesses (guesses are ignored)
	case '&': // Not found, but we have near misses
	{
		res = MISSED;
		char * p = strpbrk(buf, ":");
		str = new char[strlen(p) + 1];
		e   = str;
		strcpy(str, p);
		break;
	}
	default: // This shouldn't happen, but you know Murphy
		res = UNKNOWN;
	}

	*buf = 0;
	if (res != IGNORE) {
		/* wait for ispell to finish */
		while (*buf!= '\n')
			fgets(buf, 255, in);
	}
	return res;
}


void ISpell::insert(WordLangTuple const & word)
{
	::fputc('*', out); // Insert word in personal dictionary
	::fputs(word.word().c_str(), out);
	::fputc('\n', out);
}


void ISpell::accept(WordLangTuple const & word)
{
	::fputc('@', out); // Accept in this session
	::fputs(word.word().c_str(), out);
	::fputc('\n', out);
}


string const ISpell::error()
{
	return error_;
}
