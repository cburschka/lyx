
#include <config.h>

#include <cstdlib>

#ifdef __GNUG__
#pragma implementation
#endif

#include "frontends/Dialogs.h"
 
#include "insetinclude.h"
#include "buffer.h"
#include "bufferlist.h"
#include "debug.h"
#include "support/filetools.h"
#include "lyxrc.h"
#include "LyXView.h"
#include "LaTeXFeatures.h"
#include "gettext.h"
#include "support/FileInfo.h"
#include "layout.h"
#include "lyxfunc.h"

using std::ostream;
using std::endl;
using std::vector;
using std::pair;

extern BufferList bufferlist;


static inline
string unique_id() {
	static unsigned int seed = 1000;

	std::ostringstream ost;
	ost << "file" << ++seed;

	// Needed if we use lyxstring.
	return ost.str().c_str();
}


InsetInclude::InsetInclude(InsetCommandParams const & p, Buffer const & bf)
	: InsetCommand(p), master(&bf)
{
	flag = InsetInclude::INCLUDE;
	noload = false;
	include_label = unique_id();
}


InsetInclude::~InsetInclude()
{
}


Inset * InsetInclude::Clone(Buffer const & buffer) const
{ 
	InsetInclude * ii = new InsetInclude (params(), buffer); 
	ii->setNoLoad(isNoLoad());
	// By default, the newly created inset is of `include' type,
	// so we do not test this case.
	if (isInput())
		ii->setInput();
	else if (isVerb()) {
		ii->setVerb();
		ii->setVisibleSpace(isVerbVisibleSpace());
	}
	return ii;
}


void InsetInclude::Edit(BufferView * bv, int, int, unsigned int)
{
	bv->owner()->getDialogs()->showInclude(this);
}


void InsetInclude::Write(Buffer const *, ostream & os) const
{
	os << "Include " << getCommand() << "\n";
}


void InsetInclude::Read(Buffer const * buf, LyXLex & lex)
{
	InsetCommand::Read(buf, lex);
    
	if (getCmdName() == "include")
		setInclude();
	else if (getCmdName() == "input")
		setInput();
	else if (contains(getCmdName(), "verbatim")) {
		setVerb();
		if (getCmdName() == "verbatiminput*")
			setVisibleSpace(true);
	}
}


bool InsetInclude::display() const 
{
	return !isInput();
}


string const InsetInclude::getScreenLabel() const
{
	string temp;
	if (isInput())
		temp += _("Input");
	else if (isVerb()) {
		temp += _("Verbatim Input");
		if (isVerbVisibleSpace()) temp += '*';
	} else temp += _("Include");
	temp += ": ";
	
	if (getContents().empty()) {
		temp+= "???";
	} else {
		temp+= getContents();
	}
	return temp;
}


string const InsetInclude::getFileName() const
{
	return MakeAbsPath(getContents(), 
			   OnlyPath(getMasterFilename()));
}


string const InsetInclude::getMasterFilename() const
{
	return master->fileName();
}


bool InsetInclude::loadIfNeeded() const
{
	if (isNoLoad() || isVerb()) return false;
	if (!IsLyXFilename(getFileName())) return false;
	
	if (bufferlist.exists(getFileName())) return true;
	
	// the readonly flag can/will be wrong, not anymore I think.
	FileInfo finfo(getFileName());
	bool const ro = !finfo.writable();
	return bufferlist.readFile(getFileName(), ro) != 0;
}


int InsetInclude::Latex(Buffer const * buffer, ostream & os,
			bool /*fragile*/, bool /*fs*/) const
{
	string incfile(getContents());
	
	// Do nothing if no file name has been specified
	if (incfile.empty())
		return 0;
    
	if (loadIfNeeded()) {
		Buffer * tmp = bufferlist.getBuffer(getFileName());

		if (tmp->params.textclass != buffer->params.textclass) {
			lyxerr << "ERROR: Cannot handle include file `"
			       << MakeDisplayPath(getFileName())
			       << "' which has textclass `"
			       << textclasslist.NameOfClass(tmp->params.textclass)
			       << "' instead of `"
			       << textclasslist.NameOfClass(buffer->params.textclass)
			       << "'." << endl;
			return 0;
		}
		
		// write it to a file (so far the complete file)
		string writefile = ChangeExtension(getFileName(), ".tex");
		if (!buffer->tmppath.empty()
		    && !buffer->niceFile) {
			incfile = subst(incfile, '/','@');
#ifdef __EMX__
			incfile = subst(incfile, ':', '$');
#endif
			writefile = AddName(buffer->tmppath, incfile);
		} else
			writefile = getFileName();
		writefile = ChangeExtension(writefile, ".tex");
		lyxerr[Debug::LATEX] << "incfile:" << incfile << endl;
		lyxerr[Debug::LATEX] << "writefile:" << writefile << endl;
		
		tmp->markDepClean(buffer->tmppath);
		
		tmp->makeLaTeXFile(writefile,
				   OnlyPath(getMasterFilename()), 
				   buffer->niceFile, true);
	} 

	if (isVerb()) {
		os << '\\' << getCmdName() << '{' << incfile << '}';
	} else if (isInput()) {
		// \input wants file with extension (default is .tex)
		if (!IsLyXFilename(getFileName())) {
			os << '\\' << getCmdName() << '{' << incfile << '}';
		} else {
			os << '\\' << getCmdName() << '{'
			   << ChangeExtension(incfile, ".tex")
			   <<  '}';
		}
	} else {
		// \include don't want extension and demands that the
		// file really have .tex
		os << '\\' << getCmdName() << '{'
		   << ChangeExtension(incfile, string())
		   << '}';
	}

	return 0;
}


int InsetInclude::Ascii(Buffer const *, std::ostream & os, int) const
{
	if (isVerb())
		os << GetFileContents(getFileName());
	return 0;
}


int InsetInclude::Linuxdoc(Buffer const * buffer, ostream & os) const
{
	string incfile(getContents());
	
	// Do nothing if no file name has been specified
	if (incfile.empty())
		return 0;
    
	if (loadIfNeeded()) {
		Buffer * tmp = bufferlist.getBuffer(getFileName());

		// write it to a file (so far the complete file)
		string writefile = ChangeExtension(getFileName(), ".sgml");
		if (!buffer->tmppath.empty() && !buffer->niceFile) {
			incfile = subst(incfile, '/','@');
			writefile = AddName(buffer->tmppath, incfile);
		} else
			writefile = getFileName();

		if (IsLyXFilename(getFileName()))
			writefile = ChangeExtension(writefile, ".sgml");

		lyxerr[Debug::LATEX] << "incfile:" << incfile << endl;
		lyxerr[Debug::LATEX] << "writefile:" << writefile << endl;
		
		tmp->makeLinuxDocFile(writefile, buffer->niceFile, true);
	} 

	if (isVerb()) {
		os << "<inlinegraphic fileref=\"" << '&' << include_label << ';'
		   << "\" format=\"linespecific\">"
		   << "</inlinegraphic>";
	} else 
		os << '&' << include_label << ';';
	
	return 0;
}


int InsetInclude::DocBook(Buffer const * buffer, ostream & os) const
{
	string incfile(getContents());

	// Do nothing if no file name has been specified
	if (incfile.empty())
		return 0;
    
	if (loadIfNeeded()) {
		Buffer * tmp = bufferlist.getBuffer(getFileName());

		// write it to a file (so far the complete file)
		string writefile = ChangeExtension(getFileName(), ".sgml");
		if (!buffer->tmppath.empty() && !buffer->niceFile) {
			incfile = subst(incfile, '/','@');
			writefile = AddName(buffer->tmppath, incfile);
		} else
			writefile = getFileName();
		if (IsLyXFilename(getFileName()))
			writefile = ChangeExtension(writefile, ".sgml");

		lyxerr[Debug::LATEX] << "incfile:" << incfile << endl;
		lyxerr[Debug::LATEX] << "writefile:" << writefile << endl;
		
		tmp->makeDocBookFile(writefile, buffer->niceFile, true);
	} 

	if (isVerb()) {
		os << "<inlinegraphic fileref=\"" << '&' << include_label << ';'
		   << "\" format=\"linespecific\">"
		   << "</inlinegraphic>";
	} else 
		os << '&' << include_label << ';';
	
	return 0;
}


void InsetInclude::Validate(LaTeXFeatures & features) const
{

	string incfile(getContents());
	string writefile; // = ChangeExtension(getFileName(), ".sgml");

	if (!master->tmppath.empty() && !master->niceFile) {
		incfile = subst(incfile, '/','@');
		writefile = AddName(master->tmppath, incfile);
	} else
		writefile = getFileName();
		// Use the relative path.
		//writefile = incfile;

	if (IsLyXFilename(getFileName()))
		writefile = ChangeExtension(writefile, ".sgml");

	features.IncludedFiles[include_label] = writefile;

	if (isVerb())
		features.verbatim = true;

	// Here we must do the fun stuff...
	// Load the file in the include if it needs
	// to be loaded:
	if (loadIfNeeded()) {
		// a file got loaded
		Buffer * tmp = bufferlist.getBuffer(getFileName());
		tmp->validate(features);
	}
}


vector<string> const InsetInclude::getLabelList() const
{
	vector<string> l;

	if (loadIfNeeded()) {
		Buffer * tmp = bufferlist.getBuffer(getFileName());
		tmp->setParentName(""); 
		l = tmp->getLabelList();
		tmp->setParentName(getMasterFilename());
	}

	return l;
}


vector<pair<string,string> > const InsetInclude::getKeys() const
{
	vector<pair<string,string> > keys;
	
	if (loadIfNeeded()) {
		Buffer * tmp = bufferlist.getBuffer(getFileName());
		tmp->setParentName(""); 
		keys = tmp->getBibkeyList();
		tmp->setParentName(getMasterFilename());
	}
	
	return keys;
}
