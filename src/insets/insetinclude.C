
#include <config.h>

#include <cstdlib>

#ifdef __GNUG__
#pragma implementation
#endif

#include "frontends/Dialogs.h"

#include "insetinclude.h"
#include "buffer.h"
#include "bufferlist.h"
#include "BufferView.h"
#include "debug.h"
#include "support/filetools.h"
#include "lyxrc.h"
#include "LyXView.h"
#include "LaTeXFeatures.h"
#include "gettext.h"
#include "support/FileInfo.h"
#include "support/lstrings.h"
#include "layout.h"

using std::ostream;
using std::endl;
using std::vector;
using std::pair;

extern BufferList bufferlist;

namespace {

string const unique_id()
{
	static unsigned int seed = 1000;

	ostringstream ost;
	ost << "file" << ++seed;

	// Needed if we use lyxstring.
	return ost.str().c_str();
}

} // namespace anon


InsetInclude::InsetInclude(Params const & p)
	: params_(p), include_label(unique_id())
{}


InsetInclude::InsetInclude(InsetCommandParams const & p, Buffer const & b)
{
	params_.cparams = p;
	params_.masterFilename_ = b.fileName();
	include_label = unique_id();
}


InsetInclude::~InsetInclude()
{
	hideDialog();
}


InsetInclude::Params const & InsetInclude::params() const
{
	return params_;
}


bool InsetInclude::Params::operator==(Params const & o) const
{
	if (cparams == o.cparams && flag == o.flag &&
	    noload == o.noload && masterFilename_ == o.masterFilename_)
		return true;
	
	return false;
}


bool InsetInclude::Params::operator!=(Params const & o) const
{
	return !(*this == o);
}


void InsetInclude::set(Params const & p)
{
	params_ = p;

	// Just to be safe...
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


Inset * InsetInclude::clone(Buffer const & buffer, bool) const
{
	Params p(params_);
	p.masterFilename_ = buffer.fileName();

	return new InsetInclude(p);
}


void InsetInclude::edit(BufferView * bv, int, int, unsigned int)
{
	bv->owner()->getDialogs()->showInclude(this);
}


void InsetInclude::edit(BufferView * bv, bool)
{
	edit(bv, 0, 0, 0);
}


void InsetInclude::write(Buffer const *, ostream & os) const
{
	os << "Include " << params_.cparams.getCommand() << "\n";
}


void InsetInclude::read(Buffer const *, LyXLex & lex)
{
	params_.cparams.read(lex);
   
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


string const InsetInclude::getScreenLabel(Buffer const *) const
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
	return params_.masterFilename_;
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


int InsetInclude::latex(Buffer const * buffer, ostream & os,
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


int InsetInclude::ascii(Buffer const *, std::ostream & os, int) const
{
	if (isVerbatim())
		os << GetFileContents(getFileName());
	return 0;
}


int InsetInclude::linuxdoc(Buffer const * buffer, ostream & os) const
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


int InsetInclude::docbook(Buffer const * buffer, ostream & os) const
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


void InsetInclude::validate(LaTeXFeatures & features) const
{

	string incfile(params_.cparams.getContents());
	string writefile;

	Buffer const * const b = bufferlist.getBuffer(getMasterFilename());

	if (b && !b->tmppath.empty() && b->niceFile) {
		incfile = subst(incfile, '/','@');
		writefile = AddName(b->tmppath, incfile);
	} else
		writefile = getFileName();

	if (IsLyXFilename(getFileName()))
		writefile = ChangeExtension(writefile, ".sgml");

	features.includeFile(include_label, writefile);

	if (isVerbatim())
		features.require("verbatim");

	// Here we must do the fun stuff...
	// Load the file in the include if it needs
	// to be loaded:
	if (loadIfNeeded()) {
		// a file got loaded
		Buffer const * const tmp = bufferlist.getBuffer(getFileName());
		if (tmp)
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
