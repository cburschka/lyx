#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetbib.h"
#include "buffer.h"
#include "debug.h"
#include "BufferView.h"
#include "gettext.h"
#include "lyxtext.h"
#include "lyxrc.h"
#include "font.h"
#include "frontends/LyXView.h"
#include "lyxtextclasslist.h"

#include "frontends/Dialogs.h"

#include "support/filetools.h"
#include "support/path.h"
#include "support/os.h"
#include "support/lstrings.h"
#include "support/LAssert.h"

#include <fstream>
#include <cstdlib>

using std::ostream;
using std::ifstream;
using std::getline;
using std::endl;
using std::vector;
using std::pair;
using std::max;

int InsetBibKey::key_counter = 0;
const string key_prefix = "key-";

InsetBibKey::InsetBibKey(InsetCommandParams const & p)
	: InsetCommand(p), counter(1)
{
	if (getContents().empty())
		setContents(key_prefix + tostr(++key_counter));
}


InsetBibKey::~InsetBibKey()
{
}


Inset * InsetBibKey::clone(Buffer const &, bool) const
{
	InsetBibKey * b = new InsetBibKey(params());
	b->setCounter(counter);
	return b;
}


void InsetBibKey::setCounter(int c)
{
	counter = c;
}


// I'm sorry but this is still necessary because \bibitem is used also
// as a LyX 2.x command, and lyxlex is not enough smart to understand
// real LaTeX commands. Yes, that could be fixed, but would be a waste
// of time cause LyX3 won't use lyxlex anyway.  (ale)
void InsetBibKey::write(Buffer const *, ostream & os) const
{
	os << "\\bibitem ";
	if (! getOptions().empty()) {
		os << '['
		   << getOptions() << ']';
	}
	os << '{'
	   << getContents() << "}\n";
}


// This is necessary here because this is written without begin_inset
// This should be changed!!! (Jug)
void InsetBibKey::read(Buffer const *, LyXLex & lex)
{
	if (lex.eatLine()) {
		string const token = lex.getString();
		scanCommand(token);
	} else {
		lex.printError("InsetCommand: Parse error: `$$Token'");
	}

	if (prefixIs(getContents(), key_prefix)) {
		int key = strToInt(getContents().substr(key_prefix.length()));
		key_counter = max(key_counter, key);
	}
}

string const InsetBibKey::getBibLabel() const
{
	if (! getOptions().empty())
		return getOptions();
	return tostr(counter);
}

string const InsetBibKey::getScreenLabel(Buffer const *) const
{
	return getContents() + " [" + getBibLabel() + "]";
}


void InsetBibKey::edit(BufferView * bv, int, int, unsigned int)
{
	bv->owner()->getDialogs()->showBibitem(this);
}


void InsetBibKey::edit(BufferView * bv, bool)
{
	edit(bv, 0, 0, 0);
}


InsetBibtex::InsetBibtex(InsetCommandParams const & p, bool)
	: InsetCommand(p)
{}


InsetBibtex::~InsetBibtex()
{
}


string const InsetBibtex::getScreenLabel(Buffer const *) const
{
	return _("BibTeX Generated References");
}


int InsetBibtex::latex(Buffer const * buffer, ostream & os,
		       bool /*fragile*/, bool/*fs*/) const
{
	// changing the sequence of the commands
	// 1. \bibliographystyle{style}
	// 2. \addcontentsline{...} - if option bibtotoc set
	// 3. \bibliography{database}
	string adb;
	string db_in = getContents();
	db_in = split(db_in, adb, ',');

	// Style-Options
	string style = getOptions(); // maybe empty! and with bibtotoc
	string bibtotoc;
	if (prefixIs(style,"bibtotoc")) {
		bibtotoc = "bibtotoc";
		if (contains(style,',')) {
			style = split(style,bibtotoc,',');
		}
	}

	if (!buffer->niceFile
	    && IsFileReadable(MakeAbsPath(style, buffer->filePath()) + ".bst")) {
		style = MakeAbsPath(style, buffer->filePath());
	}

	if (!style.empty()) { // we want no \biblio...{}
		os << "\\bibliographystyle{" << style << "}\n";
	}

	// bibtotoc-Option
	if (!bibtotoc.empty()) {
		// maybe a problem when a textclass has no "art" as
		// part of its name, because it's than book.
		// For the "official" lyx-layouts it's no problem to support
		// all well
		if (!contains(textclasslist[buffer->params.textclass].name(),
			      "art")) {
			if (buffer->params.sides == LyXTextClass::OneSide) {
				// oneside
				os << "\\clearpage";
			} else {
				// twoside
				os << "\\cleardoublepage";
			}

			// bookclass
			os << "\\addcontentsline{toc}{chapter}{\\bibname}";

		} else {
			// article class
			os << "\\addcontentsline{toc}{section}{\\refname}";
		}
	}

	// database
	// If we generate in a temp dir, we might need to give an
	// absolute path there. This is a bit complicated since we can
	// have a comma-separated list of bibliographies
	string db_out;
	while (!adb.empty()) {
		if (!buffer->niceFile &&
		    IsFileReadable(MakeAbsPath(adb, buffer->filePath())+".bib"))
			 adb = os::external_path(MakeAbsPath(adb, buffer->filePath()));
		db_out += adb;
		db_out += ',';
		db_in= split(db_in, adb,',');
	}
	db_out = strip(db_out, ',');
	os   << "\\bibliography{" << db_out << "}\n";
	return 2;
}


vector<string> const InsetBibtex::getFiles(Buffer const & buffer) const
{
	// Doesn't appear to be used (Angus, 31 July 2001)
	Path p(buffer.filePath());

	vector<string> vec;

	string tmp;
	string bibfiles = getContents();
	bibfiles = split(bibfiles, tmp, ',');
	while (!tmp.empty()) {
		string file = findtexfile(ChangeExtension(tmp, "bib"), "bib");
		lyxerr[Debug::LATEX] << "Bibfile: " << file << endl;

		// If we didn't find a matching file name just fail silently
		if (!file.empty())
			vec.push_back(file);

		// Get next file name
		bibfiles = split(bibfiles, tmp, ',');
	}

	return vec;
}

// This method returns a comma separated list of Bibtex entries
vector<pair<string, string> > const InsetBibtex::getKeys(Buffer const * buffer) const
{
	vector<pair<string,string> > keys;

	lyx::Assert(buffer);
	vector<string> const files = getFiles(*buffer);
	for (vector<string>::const_iterator it = files.begin();
	     it != files.end(); ++ it) {
		// This is a _very_ simple parser for Bibtex database
		// files. All it does is to look for lines starting
		// in @ and not being @preamble and @string entries.
		// It does NOT do any syntax checking!
		ifstream ifs(it->c_str());
		string linebuf0;
		while (getline(ifs, linebuf0)) {
			string linebuf = frontStrip(strip(linebuf0));
			if (linebuf.empty()) continue;
			if (prefixIs(linebuf, "@")) {
				linebuf = subst(linebuf, '{', '(');
				string tmp;
				linebuf = split(linebuf, tmp, '(');
				tmp = lowercase(tmp);
				if (!prefixIs(tmp, "@string")
				    && !prefixIs(tmp, "@preamble")) {
					linebuf = split(linebuf, tmp, ',');
					tmp = frontStrip(tmp);
					tmp = frontStrip(tmp,'\t');
					if (!tmp.empty()) {
						keys.push_back(pair<string,string>(tmp,string()));
					}
				}
			} else if (!keys.empty()) {
				keys.back().second += linebuf + "\n";
			}
		}
	}
	return keys;
}


void InsetBibtex::edit(BufferView * bv, int, int, unsigned int)
{
	bv->owner()->getDialogs()->showBibtex(this);
}


void InsetBibtex::edit(BufferView * bv, bool)
{
	edit(bv, 0, 0, 0);
}


bool InsetBibtex::addDatabase(string const & db)
{
	string contents(getContents());
	if (!contains(contents, db)) {
		if (!contents.empty())
			contents += ",";
		setContents(contents + db);
		return true;
	}
	return false;
}


bool InsetBibtex::delDatabase(string const & db)
{
	if (contains(getContents(), db)) {
		string bd = db;
		int const n = tokenPos(getContents(), ',', bd);
		if (n > 0) {
			// Weird code, would someone care to explain this?(Lgb)
			string tmp(", ");
			tmp += bd;
			setContents(subst(getContents(), tmp, ", "));
		} else if (n == 0)
			setContents(split(getContents(), bd, ','));
		else
			return false;
	}
	return true;
}


// ale070405 This function maybe shouldn't be here. We'll fix this at 0.13.
int bibitemMaxWidth(BufferView * bv, LyXFont const & font)
{
	int w = 0;
	// Ha, now we are mainly at 1.2.0 and it is still here (Jug)
	// Does look like a hack? It is! (but will change at 0.13)
	Paragraph * par = bv->buffer()->paragraph;

	while (par) {
		if (par->bibkey) {
			int const wx = par->bibkey->width(bv, font);
			if (wx > w) w = wx;
		}
		par = par->next();
	}
	return w;
}


// ale070405
string const bibitemWidest(Buffer const * buffer)
{
	int w = 0;
	// Does look like a hack? It is! (but will change at 0.13)
	Paragraph * par = buffer->paragraph;
	InsetBibKey * bkey = 0;
	LyXFont font;

	while (par) {
		if (par->bibkey) {
			int const wx =
				lyxfont::width(par->bibkey->getBibLabel(),
					       font);
			if (wx > w) {
				w = wx;
				bkey = par->bibkey;
			}
		}
		par = par->next();
	}

	if (bkey && !bkey->getBibLabel().empty())
		return bkey->getBibLabel();

	return "99";
}
