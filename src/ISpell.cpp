/**
 * \file ISpell.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author Angus Leeming
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ISpell.h"

#include "BufferParams.h"
#include "debug.h"
#include "Encoding.h"
#include "gettext.h"
#include "Language.h"
#include "LyXRC.h"
#include "WordLangTuple.h"

#include "support/Forkedcall.h"
#include "support/lstrings.h"
#include "support/unicode.h"

// HP-UX 11.x doesn't have this header
#ifdef HAVE_SYS_SELECT_H
# include <sys/select.h>
#endif
#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif

using boost::shared_ptr;

#ifndef CXX_GLOBAL_CSTD
using std::strcpy;
using std::strlen;
using std::strpbrk;
#endif

using std::endl;
using std::max;
using std::string;


namespace lyx {

using support::bformat;

namespace {

class LaunchIspell : public support::ForkedProcess {
	typedef support::ForkedProcess ForkedProcess;
public:
	///
	LaunchIspell(BufferParams const & p, string const & l,
		     int * in, int * out, int * err)
		: params(p), lang(l), pipein(in), pipeout(out), pipeerr(err) {}
	///
	virtual shared_ptr<ForkedProcess> clone() const {
		return shared_ptr<ForkedProcess>(new LaunchIspell(*this));
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
	command_ = lyxrc.isp_command;
	return run(DontWait);
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
			? params.language->encoding()->latexName()
			: params.inputenc;
		size_t const n = enc.length();
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


string const to_iconv_encoding(docstring const & s, string const & encoding)
{
	if (lyxrc.isp_use_input_encoding) {
		std::vector<char> const encoded =
			ucs4_to_eightbit(s.data(), s.length(), encoding);
		return string(encoded.begin(), encoded.end());
	}
	// FIXME UNICODE: we don't need to convert to UTF8, but probably to the locale encoding
	return to_utf8(s);
}


docstring const from_iconv_encoding(string const & s, string const & encoding)
{
	if (lyxrc.isp_use_input_encoding) {
		std::vector<char_type> const ucs4 =
			eightbit_to_ucs4(s.data(), s.length(), encoding);
		return docstring(ucs4.begin(), ucs4.end());
	}
	// FIXME UNICODE: s is not in UTF8, but probably the locale encoding
	return from_utf8(s);
}

} // namespace anon


ISpell::ISpell(BufferParams const & params, string const & lang)
	: in(0), out(0), inerr(0), str(0)
{
	//LYXERR(Debug::GUI) << "Created ispell" << endl;

	encoding = params.encoding().iconvName();

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
					"the right languages installed.");
		child_.reset(0);
		return;
	}

	// Parent process: Read ispells identification message

	bool err_read;
	bool error = select(err_read);

	if (!error) {
		if (!err_read) {
			// Set terse mode (silently accept correct words)
			fputs("!\n", out);
			return;
		}

		// must have read something from stderr
		// FIXME UNICODE: buf is not in UTF8, but probably the locale encoding
		error_ = from_utf8(buf);
	} else {
		// select returned error
		error_ = _("The ispell process returned an error.\nPerhaps "
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
	//LYXERR(Debug::GUI) << "Killing ispell" << endl;

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


docstring const ISpell::nextMiss()
{
	// Well, somebody is a sick fuck.

	if (str == 0 || *(e+1) == '\0')
		return docstring();
	char * b = e + 2;
	e = strpbrk(b, ",\n");
	*e = '\0';
	if (b)
		return from_iconv_encoding(b, encoding);
	return docstring();
}


bool ISpell::alive()
{
	return child_.get() && child_->running();
}


enum ISpell::Result ISpell::check(WordLangTuple const & word)
{
	// FIXME Please rewrite to use string.

	Result res;

	string const encoded = to_iconv_encoding(word.word(), encoding);
	if (encoded.empty()) {
		error_ = bformat(
			_("Could not check word `%1$s' because it could not be converted to encoding `%2$s'."),
			word.word(), from_ascii(encoding));
		return UNKNOWN_WORD;
	}
	::fputs(encoded.c_str(), out);
	::fputc('\n', out);

	bool err_read;
	bool error = select(err_read);

	if (error) {
		error_ = _("Could not communicate with the ispell spellchecker process.");
		return UNKNOWN_WORD;
	}

	if (err_read) {
		// FIXME UNICODE: buf is not in UTF8, but probably the locale encoding
		error_ = from_utf8(buf);
		return UNKNOWN_WORD;
	}

	// I think we have to check if ispell is still alive here because
	// the signal-handler could have disabled blocking on the fd
	if (!alive())
		return UNKNOWN_WORD;

	switch (*buf) {
	case '*':
		res = OK;
		break;
	case '+':
		res = ROOT;
		break;
	case '-':
		res = COMPOUND_WORD;
		break;
	case '\n':
		res = IGNORED_WORD;
		break;
	case '#': // Not found, no near misses and guesses
		res = UNKNOWN_WORD;
		break;
	case '?': // Not found, and no near misses, but guesses (guesses are ignored)
	case '&': // Not found, but we have near misses
	{
		res = SUGGESTED_WORDS;
		char * p = strpbrk(buf, ":");
		str = new char[strlen(p) + 1];
		e   = str;
		strcpy(str, p);
		break;
	}
	default: // This shouldn't happen, but you know Murphy
		res = UNKNOWN_WORD;
	}

	*buf = 0;
	if (res != IGNORED_WORD) {
		/* wait for ispell to finish */
		while (*buf!= '\n')
			fgets(buf, 255, in);
	}
	return res;
}


void ISpell::insert(WordLangTuple const & word)
{
	string const encoded = to_iconv_encoding(word.word(), encoding);
	if (encoded.empty()) {
		error_ = bformat(
			_("Could not insert word `%1$s' because it could not be converted to encoding `%2$s'."),
			word.word(), from_ascii(encoding));
		return;
	}
	::fputc('*', out); // Insert word in personal dictionary
	::fputs(encoded.c_str(), out);
	::fputc('\n', out);
}


void ISpell::accept(WordLangTuple const & word)
{
	string const encoded = to_iconv_encoding(word.word(), encoding);
	if (encoded.empty()) {
		error_ = bformat(
			_("Could not accept word `%1$s' because it could not be converted to encoding `%2$s'."),
			word.word(), from_ascii(encoding));
		return;
	}
	::fputc('@', out); // Accept in this session
	::fputs(encoded.c_str(), out);
	::fputc('\n', out);
}


docstring const ISpell::error()
{
	return error_;
}


} // namespace lyx
