/**
 * \file GraphicsConverter.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GraphicsConverter.h"

#include "converter.h"
#include "debug.h"
#include "format.h"

#include "support/filetools.h"
#include "support/forkedcallqueue.h"
#include "support/convert.h"
#include "support/lstrings.h"
#include "support/lyxlib.h"

#include <boost/bind.hpp>

#include <sstream>
#include <fstream>

namespace support = lyx::support;

using support::ChangeExtension;
using support::Forkedcall;
using support::ForkedCallQueue;
using support::LibFileSearch;
using support::LibScriptSearch;
using support::OnlyPath;
using support::OnlyFilename;
using support::QuoteName;
using support::subst;
using support::tempName;
using support::unlink;

using std::endl;
using std::ostream;
using std::ostringstream;
using std::string;


namespace lyx {
namespace graphics {

class Converter::Impl : public boost::signals::trackable {
public:
	///
	Impl(string const &, string const &, string const &, string const &);

	///
	void startConversion();

	/** This method is connected to a signal passed to the forked call
	 *  class, passing control back here when the conversion is completed.
	 *  Cleans-up the temporary files, emits the finishedConversion
	 *  signal and removes the Converter from the list of all processes.
	 */
	void converted(pid_t pid, int retval);

	/** At the end of the conversion process inform the outside world
	 *  by emitting a signal.
	 */
	typedef boost::signal<void(bool)> SignalType;
	///
	SignalType finishedConversion;

	///
	string script_command_;
	///
	string script_file_;
	///
	string to_file_;
	///
	bool valid_process_;
	///
	bool finished_;
};


bool Converter::isReachable(string const & from_format_name,
			    string const & to_format_name)
{
	return converters.isReachable(from_format_name, to_format_name);
}


Converter::Converter(string const & from_file,   string const & to_file_base,
		     string const & from_format, string const & to_format)
	: pimpl_(new Impl(from_file, to_file_base, from_format, to_format))
{}


// Empty d-tor out-of-line to keep boost::scoped_ptr happy.
Converter::~Converter()
{}


void Converter::startConversion() const
{
	pimpl_->startConversion();
}


boost::signals::connection Converter::connect(slot_type const & slot) const
{
	return pimpl_->finishedConversion.connect(slot);
}


string const & Converter::convertedFile() const
{
	static string const empty;
	return pimpl_->finished_ ? pimpl_->to_file_ : empty;
}

} // namespace graphics
} // namespace lyx


//------------------------------
// Implementation details follow
//------------------------------

namespace {

/** Build the conversion script, returning true if able to build it.
 *  The script is output to the ostringstream 'script'.
 */
bool build_script(string const & from_file, string const & to_file_base,
		  string const & from_format, string const & to_format,
		  ostream & script);

} // namespace anon


namespace lyx {
namespace graphics {

Converter::Impl::Impl(string const & from_file,   string const & to_file_base,
		      string const & from_format, string const & to_format)
	: valid_process_(false), finished_(false)
{
	lyxerr[Debug::GRAPHICS] << "Converter c-tor:\n"
		<< "\tfrom_file:      " << from_file
		<< "\n\tto_file_base: " << to_file_base
		<< "\n\tfrom_format:  " << from_format
		<< "\n\tto_format:    " << to_format << endl;

	// The converted image is to be stored in this file (we do not
	// use ChangeExtension because this is a basename which may
	// nevertheless contain a '.')
	to_file_ = to_file_base + '.' +  formats.extension(to_format);

	// The conversion commands are stored in a stringstream
	ostringstream script;
	script << "#!/bin/sh\n";
	bool const success = build_script(from_file, to_file_base,
					  from_format, to_format, script);

	if (!success) {
		script_command_ =
			"sh " + 
			QuoteName(LibFileSearch("scripts", "convertDefault.sh")) +
			' ' +
			QuoteName(from_format + ':' + from_file) +
			' ' +
			QuoteName(to_format + ':' + to_file_);

		lyxerr[Debug::GRAPHICS]
			<< "\tNo converter defined! I use convertDefault.sh\n\t"
			<< script_command_ << endl;

	} else {

		lyxerr[Debug::GRAPHICS] << "\tConversion script:"
			<< "\n--------------------------------------\n"
			<< script.str()
			<< "\n--------------------------------------\n";

		// Output the script to file.
		static int counter = 0;
		script_file_ = OnlyPath(to_file_base) + "lyxconvert" +
			convert<string>(counter++) + ".sh";

		std::ofstream fs(script_file_.c_str());
		if (!fs.good()) {
			lyxerr << "Unable to write the conversion script to \""
			       << script_file_ << '\n'
			       << "Please check your directory permissions."
			       << std::endl;
			return;
		}

		fs << script.str();
		fs.close();

		// The command needed to run the conversion process
		// We create a dummy command for ease of understanding of the
		// list of forked processes.
		// Note: 'sh ' is absolutely essential, or execvp will fail.
		script_command_ = "sh " + QuoteName(script_file_) + ' ' +
			QuoteName(OnlyFilename(from_file)) + ' ' +
			QuoteName(to_format);
	}
	// All is ready to go
	valid_process_ = true;
}


void Converter::Impl::startConversion()
{
	if (!valid_process_) {
		converted(0, 1);
		return;
	}

	Forkedcall::SignalTypePtr
		ptr = ForkedCallQueue::get().add(script_command_);

	ptr->connect(boost::bind(&Impl::converted, this, _1, _2));

}

void Converter::Impl::converted(pid_t /* pid */, int retval)
{
	if (finished_)
		// We're done already!
		return;

	finished_ = true;
	// Clean-up behind ourselves
	unlink(script_file_);

	if (retval > 0) {
		unlink(to_file_);
		to_file_.erase();
		finishedConversion(false);
	} else {
		finishedConversion(true);
	}
}

} // namespace graphics
} // namespace lyx

namespace {

string const move_file(string const & from_file, string const & to_file)
{
	if (from_file == to_file)
		return string();

	ostringstream command;
	command << "fromfile=" << from_file << "\n"
		<< "tofile="   << to_file << "\n\n"
		<< "'mv' -f \"${fromfile}\" \"${tofile}\" ||\n"
		<< "{\n"
		<< "\t'cp' -f \"${fromfile}\" \"${tofile}\" ||\n"
		<< "\t{\n"
		<< "\t\texit 1\n"
		<< "\t}\n"
		<< "\t'rm' -f \"${fromfile}\"\n"
		<< "}\n";

	return command.str();
}


bool build_script(string const & from_file,
		  string const & to_file_base,
		  string const & from_format,
		  string const & to_format,
		  ostream & script)
{
	lyxerr[Debug::GRAPHICS] << "build_script ... ";
	typedef Converters::EdgePath EdgePath;

	// we do not use ChangeExtension because this is a basename
	// which may nevertheless contain a '.'
	string const to_file = to_file_base + '.'
		+ formats.extension(to_format);

	if (from_format == to_format) {
		script << move_file(QuoteName(from_file), QuoteName(to_file));
		lyxerr[Debug::GRAPHICS] << "ready (from == to)" << endl;
		return true;
	}

	EdgePath edgepath = converters.getPath(from_format, to_format);

	if (edgepath.empty()) {
		lyxerr[Debug::GRAPHICS] << "ready (edgepath.empty())" << endl;
		return false;
	}

	// Create a temporary base file-name for all intermediate steps.
	// Remember to remove the temp file because we only want the name...
	static int counter = 0;
	string const tmp = "gconvert" + convert<string>(counter++);
	string const to_base = tempName(string(), tmp);
	unlink(to_base);

	string outfile = from_file;

	// The conversion commands may contain these tokens that need to be
	// changed to infile, infile_base, outfile respectively.
	string const token_from("$$i");
	string const token_base("$$b");
	string const token_to("$$o");

	EdgePath::const_iterator it  = edgepath.begin();
	EdgePath::const_iterator end = edgepath.end();

	for (; it != end; ++it) {
		::Converter const & conv = converters.get(*it);

		// Build the conversion command
		string const infile      = outfile;
		string const infile_base = ChangeExtension(infile, string());
		outfile = ChangeExtension(to_base, conv.To->extension());

		// Store these names in the shell script
		script << "infile="      << QuoteName(infile) << '\n'
		       << "infile_base=" << QuoteName(infile_base) << '\n'
		       << "outfile="     << QuoteName(outfile) << '\n';

		string command = conv.command;
		command = subst(command, token_from, "\"${infile}\"");
		command = subst(command, token_base, "\"${infile_base}\"");
		command = subst(command, token_to,   "\"${outfile}\"");
		command = LibScriptSearch(command);

		// Store in the shell script
		script << "\n" << command << " ||\n";

		// Test that this was successful. If not, remove
		// ${outfile} and exit the shell script
		script << "{\n"
		       << "\t'rm' -f \"${outfile}\"\n"
		       << "\texit 1\n"
		       << "}\n\n";

		// Test that the outfile exists.
		// ImageMagick's convert will often create ${outfile}.0,
		// ${outfile}.1.
		// If this occurs, move ${outfile}.0 to ${outfile}
		// and delete ${outfile}.?
		script << "if [ ! -f \"${outfile}\" ]; then\n"
		       << "\tif [ -f \"${outfile}\".0 ]; then\n"
		       << "\t\t'mv' -f \"${outfile}\".0 \"${outfile}\"\n"
		       << "\t\t'rm' -f \"${outfile}\".?\n"
		       << "\telse\n"
		       << "\t\texit 1\n"
		       << "\tfi\n"
		       << "fi\n\n";

		// Delete the infile, if it isn't the original, from_file.
		if (infile != from_file) {
			script << "'rm' -f \"${infile}\"\n\n";
		}
	}

	// Move the final outfile to to_file
	script << move_file("\"${outfile}\"", QuoteName(to_file));
	lyxerr[Debug::GRAPHICS] << "ready!" << endl;

	return true;
}

} // namespace anon
