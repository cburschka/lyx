
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


InsetInclude::InsetInclude(InsetIncludeParams const & p)
{
	include_label = unique_id();
	setFromParams(p);
	params_.buffer = p.buffer;
}


InsetInclude::InsetInclude(InsetCommandParams const & p, Buffer const & b)
{
	include_label = unique_id();
	params_.buffer = &b;
	setFromParams(p);
}


InsetInclude::~InsetInclude()
{
	hideDialog();
}


InsetInclude::InsetIncludeParams const & InsetInclude::params() const
{
	return params_;
}


void InsetInclude::setFromParams(InsetIncludeParams const & p)
{
	params_.cparams.setContents(p.cparams.getContents());
	params_.noload = p.noload;
	if (params_.flag == p.flag)
		return;

	params_.flag = p.flag;

	string command;

	switch (params_.flag) {
		case INCLUDE:
			command="include";
			break;
		case VERB:
			command="verbatiminput";
			break;
		case INPUT:
			command="input";
			break;
		case VERBAST:
			command="verbatiminput*";
			break;
	}

	params_.cparams.setCmdName(command);
}


Inset * InsetInclude::Clone(Buffer const & buffer) const
{
	InsetIncludeParams p(params_);
	p.buffer = &buffer;

	return new InsetInclude (p);
}


void InsetInclude::Edit(BufferView * bv, int, int, unsigned int)
{
	bv->owner()->getDialogs()->showInclude(this);
}


void InsetInclude::Write(Buffer const *, ostream & os) const
{
	os << "Include " << params_.cparams.getCommand() << "\n";
}


void InsetInclude::Read(Buffer const *, LyXLex & lex)
{
	params_.cparams.Read(lex);
   
	if (params_.cparams.getCmdName() == "include")
		params_.flag = INCLUDE;
	else if (params_.cparams.getCmdName() == "input")
		params_.flag = INPUT;
	/* FIXME: is this logic necessary now ? */
	else if (contains(params_.cparams.getCmdName(), "verbatim")) {
		params_.flag = VERB;
		if (params_.cparams.getCmdName() == "verbatiminput*")
			params_.flag = VERBAST;
	}
}


bool InsetInclude::display() const
{
	return !(params_.flag == INPUT);
}


string const InsetInclude::getScreenLabel() const
{
	string temp;

	switch (params_.flag) {
		case INPUT: temp += _("Input"); break;
		case VERB: temp += _("Verbatim Input"); break;
		case VERBAST: temp += _("Verbatim Input*"); break;
		case INCLUDE: temp += _("Include"); break;
	}

	temp += ": ";
	
	if (params_.cparams.getContents().empty())
		temp += "???";
	else
		temp += params_.cparams.getContents();

	return temp;
}


string const InsetInclude::getRelFileBaseName() const
{
	return OnlyFilename(ChangeExtension(params_.cparams.getContents(), string()));
}

 
string const InsetInclude::getFileName() const
{
	return MakeAbsPath(params_.cparams.getContents(),
			   OnlyPath(getMasterFilename()));
}


string const InsetInclude::getMasterFilename() const
{
	return params_.buffer->fileName();
}


bool InsetInclude::loadIfNeeded() const
{
	if (params_.noload || isVerbatim())
		return false;

	if (!IsLyXFilename(getFileName()))
		return false;
	
	if (bufferlist.exists(getFileName()))
		return true;
	
	// the readonly flag can/will be wrong, not anymore I think.
	FileInfo finfo(getFileName());
	bool const ro = !finfo.writable();
	return bufferlist.readFile(getFileName(), ro) != 0;
}


int InsetInclude::Latex(Buffer const * buffer, ostream & os,
			bool /*fragile*/, bool /*fs*/) const
{
	string incfile(params_.cparams.getContents());
	
	// Do nothing if no file name has been specified
	if (incfile.empty())
		return 0;
   
	if (loadIfNeeded()) {
		Buffer * tmp = bufferlist.getBuffer(getFileName());

		// FIXME: this should be a GUI warning
		if (tmp->params.textclass != buffer->params.textclass) {
			lyxerr << "WARNING: Included file `"
			       << MakeDisplayPath(getFileName())
			       << "' has textclass `"
			       << textclasslist.NameOfClass(tmp->params.textclass)
			       << "' while parent file has textclass `"
			       << textclasslist.NameOfClass(buffer->params.textclass)
			       << "'." << endl;
			//return 0;
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

	if (isVerbatim()) {
		os << '\\' << params_.cparams.getCmdName() << '{' << incfile << '}';
	} else if (params_.flag == INPUT) {
		// \input wants file with extension (default is .tex)
		if (!IsLyXFilename(getFileName())) {
			os << '\\' << params_.cparams.getCmdName() << '{' << incfile << '}';
		} else {
			os << '\\' << params_.cparams.getCmdName() << '{'
			   << ChangeExtension(incfile, ".tex")
			   <<  '}';
		}
	} else {
		// \include don't want extension and demands that the
		// file really have .tex
		os << '\\' << params_.cparams.getCmdName() << '{'
		   << ChangeExtension(incfile, string())
		   << '}';
	}

	return 0;
}


int InsetInclude::Ascii(Buffer const *, std::ostream & os, int) const
{
	if (isVerbatim())
		os << GetFileContents(getFileName());
	return 0;
}


int InsetInclude::Linuxdoc(Buffer const * buffer, ostream & os) const
{
	string incfile(params_.cparams.getContents());
	
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

	if (isVerbatim()) {
		os << "<inlinegraphic fileref=\"" << '&' << include_label << ';'
		   << "\" format=\"linespecific\">"
		   << "</inlinegraphic>";
	} else
		os << '&' << include_label << ';';
	
	return 0;
}


int InsetInclude::DocBook(Buffer const * buffer, ostream & os) const
{
	string incfile(params_.cparams.getContents());

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

	if (isVerbatim()) {
		os << "<inlinegraphic fileref=\"" << '&' << include_label << ';'
		   << "\" format=\"linespecific\">"
		   << "</inlinegraphic>";
	} else
		os << '&' << include_label << ';';
	
	return 0;
}


void InsetInclude::Validate(LaTeXFeatures & features) const
{

	string incfile(params_.cparams.getContents());
	string writefile;

	Buffer const & b = *params_.buffer;

	if (!b.tmppath.empty() && b.niceFile) {
		incfile = subst(incfile, '/','@');
		writefile = AddName(b.tmppath, incfile);
	} else
		writefile = getFileName();

	if (IsLyXFilename(getFileName()))
		writefile = ChangeExtension(writefile, ".sgml");

	features.IncludedFiles[include_label] = writefile;

	if (isVerbatim())
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
