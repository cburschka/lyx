/*
 * \file GraphicsConverter.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "GraphicsConverter.h"

#include "converter.h"
#include "debug.h"
#include "gettext.h"

#include "support/filetools.h"
#include "support/forkedcall.h"
#include "support/path.h"

#include <fstream>

namespace {

string const move_file(string const & from_file, string const & to_file)
{
	if (from_file == to_file)
		return string();

	ostringstream command;
	command << "fromfile=" << from_file << "\n"
		<< "tofile="   << to_file << "\n\n"
		<< "'mv' -f ${fromfile} ${tofile}\n"
		<< "if [ $? -ne 0 ]; then\n"
		<< "\t'cp' -f ${fromfile} ${tofile}\n"
		<< "\tif [ $? -ne 0 ]; then\n"
		<< "\t\texit 1\n"
		<< "\tfi\n"
		<< "\t'rm' -f ${fromfile}\n"
		<< "fi\n";

	return command.str().c_str();
}

} // namespace anon


namespace grfx {

GConverter & GConverter::get()
{
	static GConverter singleton;
	return singleton;
}


bool GConverter::isReachable(string const & from_format_name,
			     string const & to_format_name) const
{
	return converters.isReachable(from_format_name, to_format_name);
}


void GConverter::convert(string const & from_file, string const & to_file_base,
			 string const & from_format, string const & to_format,
			 SignalTypePtr on_finish)
{
	// The conversion commands are stored in a stringstream
	ostringstream script;
	script << "#!/bin/sh\n";

	bool const success = build_script(from_file, to_file_base,
					  from_format, to_format, script);

	if (!success) {
		lyxerr[Debug::GRAPHICS]
			<< "Unable to build the conversion script" << std::endl;
		on_finish->emit(string());
		return;
	}

	lyxerr[Debug::GRAPHICS] << "Conversion script:\n\n"
				<< script.str().c_str() << "\n" << std::endl;

	// Output the script to file.
	static int counter = 0;
	string const script_file = OnlyPath(to_file_base) + "lyxconvert" +
		tostr(counter++) + ".sh";

	std::ofstream fs(script_file.c_str());
	if (!fs.good()) {
		// Unable to output the conversion script to file.
		on_finish->emit(string());
		return;
	}

	fs << script.str().c_str();
	fs.close();

	// Create a dummy command for ease of understanding of the
	// list of forked processes.
	// Note that 'sh ' is absolutely essential, or execvp will fail.
	string const script_command =
		"sh " + script_file + " " +
		OnlyFilename(from_file) + " " + to_format;

	// We do not use ChangeExtension here because this is a
	// basename, which may nevertheless contain a dot
	string const to_file =
		to_file_base + '.' + formats.extension(to_format);

	// Launch the conversion process.
	ConvProcessPtr shared_ptr;
	shared_ptr.reset(new ConvProcess(script_file, script_command,
					 to_file, on_finish));
	all_processes_.push_back(shared_ptr);
}


namespace {

typedef boost::shared_ptr<ConvProcess> ConvProcessPtr;

class Find_Ptr {
public:
	Find_Ptr(ConvProcess * ptr) : ptr_(ptr) {}

	bool operator()(ConvProcessPtr const & ptr)
	{
		return ptr.get() == ptr_;
	}

private:
	ConvProcess * ptr_;
};

} // namespace anon


void GConverter::erase(ConvProcess * process)
{
	std::list<ConvProcessPtr>::iterator begin = all_processes_.begin();
	std::list<ConvProcessPtr>::iterator end   = all_processes_.end();
	std::list<ConvProcessPtr>::iterator it =
		std::find_if(begin, end, Find_Ptr(process));

	if (it == end)
		return;

	all_processes_.erase(it);
}


bool GConverter::build_script(string const & from_file,
			      string const & to_file_base,
			      string const & from_format,
			      string const & to_format,
			      ostringstream & script) const
{
	typedef Converters::EdgePath EdgePath;

	// We do not use ChangeExtension here because this is a
	// basename, which may nevertheless contain a dot
	string const to_file =
		to_file_base + '.' + formats.extension(to_format);

	if (from_format == to_format) {
		script << move_file(QuoteName(from_file), QuoteName(to_file));
		return true;
	}

	EdgePath edgepath = converters.getPath(from_format, to_format);

	if (edgepath.empty()) {
		return false;
	}

	// Create a temporary base file-name for all intermediate steps.
	// Remember to remove the temp file because we only want the name...
	static int counter = 0;
	string const tmp = "gconvert" + tostr(counter++);
	string const to_base = lyx::tempName(string(), tmp);
	lyx::unlink(to_base);

	string outfile = from_file;

	// The conversion commands may contain these tokens that need to be
	// changed to infile, infile_base, outfile respectively.
	string const token_from("$$i");
	string const token_base("$$b");
	string const token_to("$$o");

	EdgePath::const_iterator it  = edgepath.begin();
	EdgePath::const_iterator end = edgepath.end();
	for (; it != end; ++it) {
		Converter const & conv = converters.get(*it);

		// Build the conversion command
		string const infile      = outfile;
		string const infile_base = ChangeExtension(infile, string());
		outfile = ChangeExtension(to_base, conv.To->extension());

		// Store these names in the shell script
		script << "infile="      << QuoteName(infile) << '\n'
		       << "infile_base=" << QuoteName(infile_base) << '\n'
		       << "outfile="     << QuoteName(outfile) << '\n';

		string command = conv.command;
		command = subst(command, token_from, "${infile}");
		command = subst(command, token_base, "${infile_base}");
		command = subst(command, token_to,   "${outfile}");

		// Store in the shell script
		script << "\n" << command << "\n\n";

		// Test that this was successful. If not, remove
		// ${outfile} and exit the shell script
		script << "if [ $? -ne 0 ]; then\n"
		       << "\t'rm' -f ${outfile}\n"
		       << "\texit 1\n"
		       << "fi\n\n";

		// Test that the outfile exists.
		// ImageMagick's convert will often create ${outfile}.0,
		// ${outfile}.1.
		// If this occurs, move ${outfile}.0 to ${outfile}
		// and delete ${outfile}.?
		script << "if [ ! -f ${outfile} ]; then\n"
		       << "\tif [ -f ${outfile}.0 ]; then\n"
		       << "\t\t'mv' -f ${outfile}.0 ${outfile}\n"
		       << "\t\t'rm' -f ${outfile}.?\n"
		       << "\telse\n"
		       << "\t\texit 1\n"
		       << "\tfi\n"
		       << "fi\n\n";

		// Delete the infile, if it isn't the original, from_file.
		if (infile != from_file) {
			script << "'rm' -f ${infile}\n\n";
		}
	}

	// Move the final outfile to to_file
	script << move_file("${outfile}", QuoteName(to_file));

	return true;
}


ConvProcess::ConvProcess(string const & script_file,
			 string const & script_command,
			 string const & to_file, SignalTypePtr on_finish)
	: script_file_(script_file), to_file_(to_file), on_finish_(on_finish)
{
	Forkedcall::SignalTypePtr convert_ptr;
	convert_ptr.reset(new Forkedcall::SignalType);

	convert_ptr->connect(SigC::slot(this, &ConvProcess::converted));

	Forkedcall call;
	int retval = call.startscript(script_command, convert_ptr);
	if (retval > 0) {
		// Unable to even start the script, so clean-up the mess!
		converted(string(), 0, 1);
	}
}


void ConvProcess::converted(string const &/* cmd */,
			    pid_t /* pid */, int retval)
{
	// Clean-up behind ourselves
	lyx::unlink(script_file_);

	if (retval > 0) {
		lyx::unlink(to_file_);
		to_file_.erase();
	}

	if (on_finish_.get()) {
		on_finish_->emit(to_file_);
	}

	grfx::GConverter::get().erase(this);
}


} // namespace grfx
