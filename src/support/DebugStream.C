// Created by Lars Gullik Bjønnes
// Copyright 1999 Lars Gullik Bjønnes (larsbj@lyx.org)
// Released into the public domain.

// Primarily developed for use in the LyX Project http://www.lyx.org/
// but should be adaptable to any project.

//#define TEST_DEBUGSTREAM
//#define MODERN_STL

//#include "DebugStream.h"
#include "debug.h"

// Since the current C++ lib in egcs does not have a standard implementation
// of basic_streambuf and basic_filebuf we don't have to include this
// header.
#ifdef MODERN_STL
#include <fstream>
#endif

ostream & operator<<(ostream & o, Debug::type t)
{
	return o << int(t);
}

/** This is a streambuffer that never prints out anything, at least
    that is the intention. You can call it a no-op streambuffer, and
    the ostream that uses it will be a no-op stream.
*/
class nullbuf : public std::streambuf {
protected:
	///
	virtual int sync() { return 0; }
	/// 
	virtual std::streamsize xsputn(char const *, std::streamsize n) {
		// fakes a purge of the buffer by returning n
		return n;
	}
	///
	virtual int overflow(int c = EOF) {
		// fakes success by returning c
		return c == EOF ? ' ' : c;
	}
};

/** A streambuf that sends the output to two different streambufs. These
    can be any kind of streambufs.
*/
class teebuf : public std::streambuf {
public:
	///
	teebuf(std::streambuf * b1, std::streambuf * b2)
		: std::streambuf(), sb1(b1), sb2(b2) {}
protected:
	///
	virtual int sync() {
#ifdef MODERN_STL
		sb2->pubsync();
		return sb1->pubsync();
#else
		sb2->sync();
		return sb1->sync();
#endif
	}
	///
	virtual std::streamsize xsputn(char const * p, std::streamsize n) {
#ifdef MODERN_STL
		sb2->sputn(p, n);
		return sb1->sputn(p, n);
#else
		sb2->xsputn(p, n);
		return sb1->xsputn(p, n);
#endif
	}
	///
	virtual int overflow(int c = EOF) {
#ifdef MODERN_STL
		sb2->sputc(c);
		return sb1->sputc(c);
#else
		sb2->overflow(c);
		return sb1->overflow(c);
#endif
	}
private:
	///
	std::streambuf * sb1;
	///
	std::streambuf * sb2;
};

///
class debugbuf : public std::streambuf {
public:
	///
	debugbuf(std::streambuf * b)
		: std::streambuf(), sb(b) {}
protected:
	///
	virtual int sync() {
#ifdef MODERN_STL
		return sb->pubsync();
#else
		return sb->sync();
#endif
	}
	///
	virtual std::streamsize xsputn(char const * p, std::streamsize n) {
#ifdef MODERN_STL
		return sb->sputn(p, n);
#else
		return sb->xsputn(p, n);
#endif
	}
	///
	virtual int overflow(int c = EOF) {
#ifdef MODERN_STL
		return sb->sputc(c);
#else
		return sb->overflow(c);
#endif
	}
private:
	///
	std::streambuf * sb;
};

/// So that public parts of DebugStream does not need to know about filebuf
struct DebugStream::debugstream_internal {
	/// Used when logging to file.
	std::filebuf fbuf;
};

/// Constructor, sets the debug level to t.
DebugStream::DebugStream(Debug::type t)
	: std::ostream(new debugbuf(std::cerr.rdbuf())),
	  dt(t), nullstream(new nullbuf), internal(0) {}

	
/// Constructor, sets the log file to f, and the debug level to t.
DebugStream::DebugStream(char const * f, Debug::type t)
	: std::ostream(new debugbuf(std::cerr.rdbuf())),
	  dt(t), nullstream(new nullbuf),
	  internal(new debugstream_internal)
{
	internal->fbuf.open(f, std::ios::out|std::ios::app);
	delete rdbuf(new teebuf(std::cerr.rdbuf(),
				&internal->fbuf));
}


DebugStream::~DebugStream()
{
	delete nullstream.rdbuf(0); // Without this we leak
	delete rdbuf(0);            // Without this we leak
	if (internal)
		delete internal;
}

/// Sets the debugstreams' logfile to f.
void DebugStream::logFile(char const * f)
{
	if (internal) {
		internal->fbuf.close();
	} else {
		internal = new debugstream_internal;
	}
	internal->fbuf.open(f, std::ios::out|std::ios::app);
	delete rdbuf(new teebuf(std::cerr.rdbuf(),
				&internal->fbuf));
}


#ifdef TEST_DEBUGSTREAM

// Example debug stream
DebugStream debugstream;

int main(int, char **)
{
	/**
	   I have been running some tests on this to see how much overhead
	   this kind of permanent debug code has. My conclusion is: not 
	   much. In all, but the most time critical code, this will have 
	   close to no impact at all.
	   
	   In the tests that I have run the use of
	   if (debugstream.debugging(DebugStream::INFO))
	   debugstream << "some debug\n";
	   has close to no overhead when the debug level is not 
	   DebugStream::INFO.
	   
	   The overhead for
	   debugstream.debug(DebugStream::INFO) << "some debug\n";
	   is also very small when the debug level is not
	   DebugStream::INFO. However the overhead for this will increase
	   if complex debugging information is output.
	   
	   The overhead when the debug level is DebugStream::INFO can be
	   significant, but since we then are running in debug mode it is 
	   of no concern.
	   
	   Why should we use this instead of the class Error that we already
	   have? First of all it uses C++ iostream and constructs, secondly
	   it will be a lot easier to output the debug info that we need
	   without a lot of manual conversions, thirdly we can now use 
	   iomanipulators and the complete iostream formatting functions.
	   pluss it will work for all types that have a operator<< 
	   defined, and can be used in functors that take a ostream & as
	   parameter. And there should be less need for temporary objects.
	   And one nice bonus is that we get a log file almost for
	   free.
	   
	   Some of the names are of course open to modifications. I will try
	   to use the names we already use in LyX.
	*/
	// Just a few simple debugs to show how it can work.
	debugstream << "Debug level set to Debug::NONE\n";
	if (debugstream.debugging()) {
		debugstream << "Something must be debugged\n";
	}
	debugstream.debug(Debug::WARN) << "more debug(WARN)\n";
	debugstream.debug(Debug::INFO) << "even more debug(INFO)\n";
	debugstream.debug(Debug::CRIT) << "even more debug(CRIT)\n";
	debugstream.level(Debug::value("INFO"));
	debugstream << "Setting debug level to Debug::INFO\n";
	if (debugstream.debugging()) {
		debugstream << "Something must be debugged\n";
	}
	debugstream.debug(Debug::WARN) << "more debug(WARN)\n";
	debugstream.debug(Debug::INFO) << "even more debug(INFO)\n";
	debugstream.debug(Debug::CRIT) << "even more debug(CRIT)\n";
	debugstream.addLevel(Debug::type(Debug::CRIT | 
					 Debug::WARN));
	debugstream << "Adding Debug::CRIT and Debug::WARN\n";
	debugstream[Debug::WARN] << "more debug(WARN)\n";
	debugstream[Debug::INFO] << "even more debug(INFO)\n";
	debugstream[Debug::CRIT] << "even more debug(CRIT)\n";
	debugstream.delLevel(Debug::INFO);
	debugstream << "Removing Debug::INFO\n";
	debugstream[Debug::WARN] << "more debug(WARN)\n";
	debugstream[Debug::INFO] << "even more debug(INFO)\n";
	debugstream[Debug::CRIT] << "even more debug(CRIT)\n";
	debugstream.logFile("logfile");
	debugstream << "Setting logfile to \"logfile\"\n";
	debugstream << "Value: " << 123 << " " << "12\n";
	int i = 0;
	int * p = new int;
	// note: the (void*) is needed on g++ 2.7.x since it does not
	// support partial specialization. In egcs this should not be
	// needed.
	debugstream << "automatic " << &i 
		    << ", free store " << p << std::endl;
	delete p;
	/*
	for (int j = 0; j < 200000; ++j) {
		DebugStream tmp;
		tmp << "Test" << std::endl;
	}
	*/
}
#endif
