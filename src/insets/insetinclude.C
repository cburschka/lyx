/**
 * \file insetinclude.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "insetinclude.h"
#include "buffer.h"
#include "buffer_funcs.h"
#include "bufferlist.h"
#include "BufferView.h"
#include "debug.h"
#include "funcrequest.h"
#include "gettext.h"
#include "LaTeXFeatures.h"
#include "latexrunparams.h"
#include "Lsstream.h"
#include "lyxlex.h"
#include "lyxrc.h"
#include "metricsinfo.h"
#include "dimension.h"

#include "frontends/Dialogs.h"
#include "frontends/LyXView.h"
#include "frontends/Painter.h"

#include "support/filetools.h"
#include "support/FileInfo.h"
#include "support/FileMonitor.h"
#include "support/lstrings.h" // contains
#include "support/tostr.h"

#include "graphics/PreviewedInset.h"
#include "graphics/PreviewImage.h"

#include <boost/bind.hpp>

#include <cstdlib>

using namespace lyx::support;

using std::ostream;
using std::endl;
using std::vector;
using std::pair;

extern BufferList bufferlist;


class InsetInclude::PreviewImpl : public lyx::graphics::PreviewedInset {
public:
	///
	PreviewImpl(InsetInclude & p) : PreviewedInset(p) {}

	///
	bool previewWanted() const;
	///
	string const latexString() const;
	///
	InsetInclude & parent() const {
		return *static_cast<InsetInclude*>(inset());
	}

	///
	bool monitoring() const { return monitor_.get(); }
	///
	void startMonitoring();
	///
	void stopMonitoring() { monitor_.reset(); }

private:
	/// Invoked by monitor_ should the parent file change.
	void restartLoading();
	///
	boost::scoped_ptr<FileMonitor> monitor_;
};


namespace {

string const uniqueID()
{
	static unsigned int seed = 1000;
	return "file" + tostr(++seed);
}

} // namespace anon


InsetInclude::InsetInclude(Params const & p)
	: params_(p), include_label(uniqueID()),
	  preview_(new PreviewImpl(*this)),
	  set_label_(false)
{}


InsetInclude::InsetInclude(InsetCommandParams const & p, Buffer const & b)
	: include_label(uniqueID()),
	  preview_(new PreviewImpl(*this)),
	  set_label_(false)
{
	params_.cparams = p;
	params_.masterFilename_ = b.fileName();
}


InsetInclude::InsetInclude(InsetInclude const & other)
	: Inset(other),
	  params_(other.params_),
	  include_label(other.include_label),
	  preview_(new PreviewImpl(*this)),
	  set_label_(other.set_label_)
{}


InsetInclude::~InsetInclude()
{
	InsetIncludeMailer mailer(*this);
	mailer.hideDialog();
}


dispatch_result InsetInclude::localDispatch(FuncRequest const & cmd)
{
	switch (cmd.action) {

	case LFUN_INSET_MODIFY: {
		InsetInclude::Params p;
		InsetIncludeMailer::string2params(cmd.argument, p);
		if (!p.cparams.getCmdName().empty()) {
			set(p);
			params_.masterFilename_ = cmd.view()->buffer()->fileName();
			cmd.view()->updateInset(this);
		}
		return DISPATCHED;
	}

	case LFUN_INSET_DIALOG_UPDATE:
		InsetIncludeMailer(*this).updateDialog(cmd.view());
		return DISPATCHED;

	case LFUN_MOUSE_RELEASE:
	case LFUN_INSET_EDIT:
		InsetIncludeMailer(*this).showDialog(cmd.view());
		return DISPATCHED;

	default:
		return Inset::localDispatch(cmd);
	}
}


InsetInclude::Params const & InsetInclude::params() const
{
	return params_;
}


bool InsetInclude::Params::operator==(Params const & o) const
{
	return cparams == o.cparams && flag == o.flag &&
	    masterFilename_ == o.masterFilename_;
}


bool InsetInclude::Params::operator!=(Params const & o) const
{
	return !(*this == o);
}


void InsetInclude::set(Params const & p)
{
	params_ = p;

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

	if (preview_->monitoring())
		preview_->stopMonitoring();

	if (lyx::graphics::PreviewedInset::activated() && params_.flag == INPUT)
		preview_->generatePreview();
}


InsetBase * InsetInclude::clone() const
{
	//Params p(params_);
	//p.masterFilename_ = buffer.fileName();
#warning FIXME: broken cross-doc copy/paste - must fix

	return new InsetInclude(params_);
}


void InsetInclude::write(Buffer const *, ostream & os) const
{
	os << "Include " << params_.cparams.getCommand() << '\n'
	   << "preview " << tostr(params_.cparams.preview()) << '\n';
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
	if (isVerbatim())
		return false;

	if (!IsLyXFilename(getFileName()))
		return false;

	if (bufferlist.exists(getFileName()))
		return true;

	// the readonly flag can/will be wrong, not anymore I think.
	FileInfo finfo(getFileName());
	if (!finfo.isOK())
		return false;
	return loadLyXFile(bufferlist.newBuffer(getFileName()),
			   getFileName());
}


int InsetInclude::latex(Buffer const * buffer, ostream & os,
			LatexRunParams const & runparams) const
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
			       << tmp->params.getLyXTextClass().name()
			       << "' while parent file has textclass `"
			       << buffer->params.getLyXTextClass().name()
			       << "'." << endl;
			//return 0;
		}

		// write it to a file (so far the complete file)
		string writefile = ChangeExtension(getFileName(), ".tex");

		if (!buffer->tmppath.empty() && !runparams.nice) {
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

		tmp->makeLaTeXFile(writefile, OnlyPath(getMasterFilename()),
				   runparams, true);
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


int InsetInclude::ascii(Buffer const *, ostream & os, int) const
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
		os << "<![CDATA["
		   << GetFileContents(getFileName())
		   << "]]>";
	} else
		os << '&' << include_label << ';';

	return 0;
}


int InsetInclude::docbook(Buffer const * buffer, ostream & os,
			  bool /*mixcont*/) const
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
		os << "<inlinegraphic fileref=\""
		   << '&' << include_label << ';'
		   << "\" format=\"linespecific\">";
	} else
		os << '&' << include_label << ';';

	return 0;
}


void InsetInclude::validate(LaTeXFeatures & features) const
{

	string incfile(params_.cparams.getContents());
	string writefile;

	Buffer const * const b = bufferlist.getBuffer(getMasterFilename());

	if (b && !b->tmppath.empty() && !b->niceFile && !isVerbatim()) {
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
		Buffer * const tmp = bufferlist.getBuffer(getFileName());
		if (tmp) {
			if (b)
				tmp->niceFile = b->niceFile;
			tmp->validate(features);
		}
	}
}


void InsetInclude::getLabelList(std::vector<string> & list) const
{
	if (loadIfNeeded()) {
		Buffer * tmp = bufferlist.getBuffer(getFileName());
		tmp->setParentName("");
		tmp->getLabelList(list);
		tmp->setParentName(getMasterFilename());
	}
}


void InsetInclude::fillWithBibKeys(std::vector<std::pair<string,string> > & keys) const
{
	if (loadIfNeeded()) {
		Buffer * tmp = bufferlist.getBuffer(getFileName());
		tmp->setParentName("");
		tmp->fillWithBibKeys(keys);
		tmp->setParentName(getMasterFilename());
	}
}


void InsetInclude::metrics(MetricsInfo & mi, Dimension & dim) const
{
	if (preview_->previewReady()) {
		dim.asc = preview_->pimage()->ascent();
		dim.des = preview_->pimage()->descent();
		dim.wid = preview_->pimage()->width();
	} else {
		if (!set_label_) {
			set_label_ = true;
			button_.update(getScreenLabel(mi.base.bv->buffer()),
				       editable() != NOT_EDITABLE);
		}
		button_.metrics(mi, dim);
	}
	dim_ = dim;
}


void InsetInclude::draw(PainterInfo & pi, int x, int y) const
{
	cache(pi.base.bv);
	if (!preview_->previewReady()) {
		button_.draw(pi, x, y);
		return;
	}

	if (!preview_->monitoring())
		preview_->startMonitoring();

	pi.pain.image(x, y - dim_.asc, dim_.wid, dim_.height(),
			    *(preview_->pimage()->image()));
}


BufferView * InsetInclude::view() const
{
	return button_.view();
}


//
// preview stuff
//

void InsetInclude::addPreview(lyx::graphics::PreviewLoader & ploader) const
{
	preview_->addPreview(ploader);
}


bool InsetInclude::PreviewImpl::previewWanted() const
{
	return parent().params_.flag == InsetInclude::INPUT &&
		parent().params_.cparams.preview() &&
		IsFileReadable(parent().getFileName());
}


string const InsetInclude::PreviewImpl::latexString() const
{
	if (!view() || !view()->buffer())
		return string();

	ostringstream os;
	LatexRunParams runparams;
	runparams.flavor = LatexRunParams::LATEX;
	parent().latex(view()->buffer(), os, runparams);

	return STRCONV(os.str());
}


void InsetInclude::PreviewImpl::startMonitoring()
{
	monitor_.reset(new FileMonitor(parent().getFileName(), 2000));
	monitor_->connect(boost::bind(&PreviewImpl::restartLoading, this));
	monitor_->start();
}


void InsetInclude::PreviewImpl::restartLoading()
{
	lyxerr << "restartLoading()" << std::endl;
	removePreview();
	if (view())
		view()->updateInset(&parent());
	generatePreview();
}


string const InsetIncludeMailer::name_("include");

InsetIncludeMailer::InsetIncludeMailer(InsetInclude & inset)
	: inset_(inset)
{}


string const InsetIncludeMailer::inset2string(Buffer const &) const
{
	return params2string(inset_.params());
}


void InsetIncludeMailer::string2params(string const & in,
				       InsetInclude::Params & params)
{
	params = InsetInclude::Params();

	if (in.empty())
		return;

	istringstream data(STRCONV(in));
	LyXLex lex(0,0);
	lex.setStream(data);

	if (lex.isOK()) {
		lex.next();
		string const token = lex.getString();
		if (token != name_)
			return;
	}

	// This is part of the inset proper that is usually swallowed
	// by Buffer::readInset
	if (lex.isOK()) {
		lex.next();
		string const token = lex.getString();
		if (token != "Include")
			return;
	}

	if (lex.isOK()) {
		InsetInclude inset(params);
		inset.read(0, lex);
		params = inset.params();
	}
}


string const
InsetIncludeMailer::params2string(InsetInclude::Params const & params)
{
	InsetInclude inset(params);
	inset.set(params);
	ostringstream data;
	data << name_ << ' ';
	inset.write(0, data);
	data << "\\end_inset\n";
	return STRCONV(data.str());
}
