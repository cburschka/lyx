#include <config.h>

#include <fstream>
#include <cstdlib>

#ifdef __GNUG__
#pragma implementation
#endif

#include FORMS_H_LOCATION  
#include "frontends/Dialogs.h"
#include "insetbib.h"
#include "buffer.h"
#include "debug.h"
#include "BufferView.h"
#include "gettext.h"
#include "lyxtext.h"
#include "support/filetools.h"
#include "support/path.h"
#include "lyxrc.h"
#include "font.h"
#include "LyXView.h" 

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
	: InsetCommand(p)
{
	counter = 1;
	if (getContents().empty())
		setContents(key_prefix + tostr(++key_counter));
}


InsetBibKey::~InsetBibKey()
{
}


Inset * InsetBibKey::Clone(Buffer const &) const
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
void InsetBibKey::Write(Buffer const *, ostream & os) const
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
void InsetBibKey::Read(Buffer const *, LyXLex & lex)
{    
	string token;

	if (lex.EatLine()) {
		token = lex.GetString();
		scanCommand(token);
	} else
		lex.printError("InsetCommand: Parse error: `$$Token'");

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

string const InsetBibKey::getScreenLabel() const
{
	return getContents() + " [" + getBibLabel() + "]";
}


void InsetBibKey::Edit(BufferView * bv, int, int, unsigned int)
{ 
	bv->owner()->getDialogs()->showBibitem(this);
}


InsetBibtex::InsetBibtex(InsetCommandParams const & p)
	: InsetCommand(p)
{}


InsetBibtex::~InsetBibtex()
{
}


string const InsetBibtex::getScreenLabel() const
{
	return _("BibTeX Generated References");
}


int InsetBibtex::Latex(Buffer const * buffer, ostream & os,
		       bool /*fragile*/, bool/*fs*/) const
{
	// If we generate in a temp dir, we might need to give an
	// absolute path there. This is a bit complicated since we can
	// have a comma-separated list of bibliographies
	string adb, db_out;
	string db_in = getContents();
	db_in = split(db_in, adb, ',');
	while(!adb.empty()) {
		if (!buffer->niceFile &&
		    IsFileReadable(MakeAbsPath(adb, buffer->filepath)+".bib")) 
                         adb = MakeAbsPath(adb, buffer->filepath);

		db_out += adb;
		db_out += ',';
		db_in= split(db_in, adb,',');
	}
	db_out = strip(db_out, ',');
	// Idem, but simpler
	string style;
	if (!buffer->niceFile 
	    && IsFileReadable(MakeAbsPath(getOptions(), buffer->filepath)
			      + ".bst")) 
		style = MakeAbsPath(getOptions(), buffer->filepath);
	else
		style = getOptions();

	os << "\\bibliographystyle{" << style << "}\n"
	   << "\\bibliography{" << db_out << "}\n";
	return 2;
}


// This method returns a comma separated list of Bibtex entries
vector<pair<string, string> > const InsetBibtex::getKeys(Buffer const * buffer) const
{
	Path p(buffer->filepath);

	vector<pair<string,string> > keys;
	string tmp;
	string bibfiles = getContents();
	bibfiles = split(bibfiles, tmp, ',');
	while(!tmp.empty()) {
		string fil = findtexfile(ChangeExtension(tmp, "bib"),
					 "bib");
		lyxerr[Debug::LATEX] << "Bibfile: " << fil << endl;
		// If we didn't find a matching file name just fail silently
		if (!fil.empty()) {
			// This is a _very_ simple parser for Bibtex database
			// files. All it does is to look for lines starting
			// in @ and not being @preamble and @string entries.
			// It does NOT do any syntax checking!
			ifstream ifs(fil.c_str());
			string linebuf0;
			while (getline(ifs, linebuf0)) {
				string linebuf = frontStrip(strip(linebuf0));
				if (linebuf.empty() ) continue;
				if (prefixIs(linebuf, "@")) {
					linebuf = subst(linebuf, '{', '(');
					linebuf = split(linebuf, tmp, '(');
					tmp = lowercase(tmp);
					if (!prefixIs(tmp, "@string")
					    && !prefixIs(tmp, "@preamble")) {
						linebuf = split(linebuf, tmp, ',');
						tmp = frontStrip(tmp);
						if (!tmp.empty()) {
							keys.push_back(pair<string,string>(tmp,string()));
						}
					}
				} else if (!keys.empty()) {
					keys.back().second += linebuf + "\n";
				}
			}
		}
		// Get next file name
    		bibfiles = split(bibfiles, tmp, ',');
	}
  	return keys;
}


void InsetBibtex::Edit(BufferView * bv, int, int, unsigned int)
{
	bv->owner()->getDialogs()->showBibtex(this);
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
	// Does look like a hack? It is! (but will change at 0.13)
	LyXParagraph * par = bv->buffer()->paragraph;
    
	while (par) {
		if (par->bibkey) {
			int const wx = par->bibkey->width(bv, font);
			if (wx > w) w = wx;
		}
#ifndef NEW_INSETS
		par = par->next_;
#else
		par = par->next();
#endif
	}
	return w;
}


// ale070405
string const bibitemWidest(Buffer const * buffer)
{
	int w = 0;
	// Does look like a hack? It is! (but will change at 0.13)
	LyXParagraph * par = buffer->paragraph;
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
#ifndef NEW_INSETS
		par = par->next_;
#else
		par = par->next();
#endif
	}
    
	if (bkey && !bkey->getBibLabel().empty())
		return bkey->getBibLabel();
    
	return "99";
}
