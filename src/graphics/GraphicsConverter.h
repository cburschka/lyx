// -*- C++ -*-
/*
 * \file GraphicsConverter.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 *
 * class grfx::GConverter enables graphics files to be converted asynchronously
 * to a loadable format. It does this by building a shell script of all
 * the conversion commands needed for the transformation. This script is then
 * sent to the forked calls controller for non-blocking execution. When it
 * is finished a signal is emitted, thus informing us to proceed with the
 * loading of the image.
 *
 * Ultimately, this class should be wrapped back into Dekel's converter class.
 */

#ifndef GRAPHICSCONVERTER_H
#define GRAPHICSCONVERTER_H

#include "LString.h"
#include "Lsstream.h"
#include <boost/smart_ptr.hpp>
#include <boost/utility.hpp>
#include <sigc++/signal_system.h>
#include <list>

#ifdef __GNUG__
#pragma interface
#endif

namespace grfx {

class GConverter : boost::noncopyable {
public:

	/// This is a singleton class. Get the instance.
	static GConverter & get();

	/// Can the conversion be performed?
	bool isReachable(string const & from_format_name,
			 string const & to_format_name) const;

	/** Convert the file and at the end return it by emitting this signal
	 *  If successful, the returned string will be the name of the
	 *  converted file (to_file_base + extension(to_format_name)).
	 *  If unsuccessful, the string will be empty.
	 */
	typedef SigC::Signal1<void, string const &> SignalType;
	///
	typedef boost::shared_ptr<SignalType> SignalTypePtr;
	///
	void convert(string const & from_file,   string const & to_file_base,
		     string const & from_format, string const & to_format,
		     SignalTypePtr on_finish);

private:
	/** Make the c-tor private so we can control how many objects
	 *  are instantiated.
	 */
	GConverter() {}

	/** Build the conversion script, returning true if able to build it.
	 *  The script is output to the ostringstream 'script'.
	 */
	bool build_script(string const & from_file, string const & to_file_base,
			  string const & from_format, string const & to_format,
			  ostringstream & script) const;

	/** Remove the ConvProcess from the list of all processes.
	 *  Called by ConvProcess::converted.
	 */
	friend class ConvProcess;
	///
	void erase(ConvProcess *);

	/// The list of all conversion processs
	typedef boost::shared_ptr<ConvProcess> ConvProcessPtr;
	///
	std::list<ConvProcessPtr> all_processes_;
};


/// Each ConvProcess represents a single conversion process.
struct ConvProcess : public SigC::Object
{
	///
	typedef GConverter::SignalTypePtr SignalTypePtr;

	/** Each ConvProcess represents a single conversion process.
	 *  It is passed :
	 *  1. The name of the script_file, which it deletes once the
	 *     conversion is comlpeted;
	 *  2. The script command itself, which it passes on to the forked
	 *     call process;
	 *  3. The name of the output file, which it returns to the calling
	 *     process on successfull completion, by emitting
	 *  4. The signal on_finish.
	 */
	ConvProcess(string const & script_file, string const & script_command,
		    string const & to_file, SignalTypePtr on_finish);

	/** This method is connected to a signal passed to the forked call
	 *  class, passing control back here when the conversion is completed.
	 *  Cleans-up the temporary files, emits the on_finish signal and
	 *  removes the ConvProcess from the list of all processes.
	 */
	void converted(string cmd, pid_t pid, int retval);

	///
	string script_file_;
	///
	string to_file_;
	///
	SignalTypePtr on_finish_;
};

} // namespace grfx

#endif // GRAPHICSCONVERTER_H
