// -*- C++ -*-

#ifndef _ERROR_H_
#define _ERROR_H_

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

///
class Error
{
public:
	///
	enum DEBUG_LEVELS {
		///
		INFO       = (1),        // 1
		///
		INIT       = (1 << 1),   // 2
		///
		KEY        = (1 << 2),   // 4
		///
		TOOLBAR    = (1 << 3),   // 8
		///
		LEX_PARSER = (1 << 4),   // 16
		///
		LYXRC      = (1 << 5),   // 32
		///
		KBMAP      = (1 << 6),   // 64
		///
		LATEX      = (1 << 7),   // 128
		///
		MATHED     = (1 << 8),   // 256 // Alejandro, please use this.
		///
		FONT       = (1 << 9),   // 512
		///
		TCLASS     = (1 << 10),  // 1024
		///
		LYXVC      = (1 << 11),  // 2048
		///
		ROFF       = (1 << 12),  // 4096
		///
		LYXSERVER  = (1 << 13),  // 8192
		///
		ALL        = (65535),    // 2^16-1
		///
		ANY        = ALL
	};
	///
	Error(int level = 0);
	///
	void setDebugLevel(int level);
	/// Not implemented yet.
	void setLogLevel(int level);
	/// Not implemented yet.
	void setLogFile(string const & filename);
	///
	void debug(string const & msg, int level = INFO); // or ANY?
	///
	bool debugging(int level = INFO) // or ANY?
	{
		return (bool)(debuglevel & level);
	}
	///
	void print(string const & msg);
protected:
private:
	///
	int debuglevel;
	///
	int loglevel;
};

/// The global instance of the Error class.
extern Error lyxerr;

#endif
