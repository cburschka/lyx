/**
 * \file client.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author João Luis M. Assirati
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */


#include <config.h>

#include "debug.h"
#include "support/lstrings.h"

#include <boost/filesystem/operations.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/scoped_ptr.hpp>

// getpid(), getppid()
#include <sys/types.h>
#include <unistd.h>

// select()
#include <sys/select.h>

// socket(), connect()
#include <sys/socket.h>
#include <sys/un.h>

// fcntl()
#include <fcntl.h>

#include <cerrno>
#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include <iostream>

using lyx::support::prefixIs;

using boost::scoped_ptr;
namespace fs = boost::filesystem;

using std::string;
using std::vector;
using std::cout;
using std::cerr;
using std::cin;
using std::endl;


namespace support {

string itoa(unsigned int i)
{
	return boost::lexical_cast<string>(i);
}


// Parts stolen from lyx::support::DirList()
// Returns the absolute pathnames of all lyx local sockets
vector<fs::path> lyxSockets(string const & dir, string const & pid)
{
	vector<fs::path> dirlist;

	fs::path dirpath(dir);

	if (!fs::exists(dirpath) || !fs::is_directory(dirpath)) {
		lyxerr << dir << " does not exist or is not a directory."
		       << endl;
		return dirlist;
	}

	fs::directory_iterator beg((fs::path(dir)));
	fs::directory_iterator end;

	for (; beg != end; ++beg) {
		if (prefixIs(beg->leaf(), "lyx_tmpdir" + pid)) {
			fs::path lyxsocket = *beg / "lyxsocket";
			if (fs::exists(lyxsocket)) {
				dirlist.push_back(lyxsocket);
			}
		}
	}

	return dirlist;
}


namespace socktools {


int connect(string const & name)
{
	int fd; // File descriptor for the socket
	sockaddr_un addr; // Structure that hold the socket address

	// char sun_path[108]
	string::size_type len = name.size();
	if (len > 107) {
		cerr << "lyxclient: Socket address '" << name
		     << "' too long." << endl;
		return -1;
	}
	// Synonims for AF_UNIX are AF_LOCAL and AF_FILE
	addr.sun_family = AF_UNIX;
	name.copy(addr.sun_path, 107);
	addr.sun_path[len] = '\0';

	if ((fd = ::socket(PF_UNIX, SOCK_STREAM, 0))== -1) {
		cerr << "lyxclient: Could not create socket descriptor: "
		     << strerror(errno) << endl;
		return -1;
	}
	if (::connect(fd,
		      reinterpret_cast<struct sockaddr *>(&addr),
		      sizeof(addr)) == -1) {
		cerr << "lyxclient: Could not connect to socket " << name
		     << ": " << strerror(errno) << endl;
		::close(fd);
		return -1;
	}
	if (::fcntl(fd, F_SETFL, O_NONBLOCK) == -1) {
		cerr << "lyxclient: Could not set O_NONBLOCK for socket: "
		     << strerror(errno) << endl;
		::close(fd);
		return -1;
	}
	return fd;
}


} // namespace socktools
} // namespace support



// Class IOWatch ------------------------------------------------------------
class IOWatch {
public:
	IOWatch();
	void clear();
	void addfd(int);
	bool wait(double);
	bool wait();
	bool isset(int fd);
private:
	fd_set des;
	fd_set act;
};


IOWatch::IOWatch()
{
	clear();
}


void IOWatch::clear()
{
	FD_ZERO(&des);
}


void IOWatch::addfd(int fd)
{
	FD_SET(fd, &des);
}


bool IOWatch::wait(double timeout)
{
	timeval to;
	to.tv_sec = static_cast<long int>(timeout);
	to.tv_usec = static_cast<long int>((timeout - to.tv_sec)*1E6);
	act = des;
	return select(FD_SETSIZE, &act,
		      (fd_set *)0, (fd_set *)0, &to);
}


bool IOWatch::wait()
{
	act = des;
	return select(FD_SETSIZE, &act,
		      (fd_set *)0, (fd_set *)0, (timeval *)0);
}


bool IOWatch::isset(int fd) {
	return FD_ISSET(fd, &act);
}
// ~Class IOWatch ------------------------------------------------------------


// Class LyXDataSocket -------------------------------------------------------
// Modified LyXDataSocket class for use with the client
class LyXDataSocket {
public:
	LyXDataSocket(string const &);
	~LyXDataSocket();
	// File descriptor of the connection
	int fd() const;
	// Connection status
	bool connected() const;
	// Line buffered input from the socket
	bool readln(string &);
	// Write the string + '\n' to the socket
	void writeln(string const &);
private:
	// File descriptor for the data socket
	int fd_;
	// True if the connection is up
	bool connected_;
	// buffer for input data
	string buffer;
};


LyXDataSocket::LyXDataSocket(string const & address)
{
	if ((fd_ = support::socktools::connect(address)) == -1) {
		connected_ = false;
	} else {
		connected_ = true;
	}
}


LyXDataSocket::~LyXDataSocket()
{
	::close(fd_);
}


int LyXDataSocket::fd() const
{
	return fd_;
}


bool LyXDataSocket::connected() const
{
	return connected_;
}


// Returns true if there was a complete line to input
// A line is of the form <key>:<value>
//   A line not of this form will not be passed
// The line read is splitted and stored in 'key' and 'value'
bool LyXDataSocket::readln(string & line)
{
	int const charbuf_size = 100;
	char charbuf[charbuf_size]; // buffer for the ::read() system call
	int count;
	string::size_type pos;

	// read and store characters in buffer
	while ((count = ::read(fd_, charbuf, charbuf_size - 1)) > 0) {
		charbuf[count] = '\0'; // turn it into a c string
		buffer += charbuf;
	}

	// Error conditions. The buffer must still be
	// processed for lines read
	if (count == 0) { // EOF -- connection closed
		connected_ = false;
	} else if ((count == -1) && (errno != EAGAIN)) { // IO error
		cerr << "lyxclient: IO error." << endl;
		connected_ = false;
	}

	// Cut a line from buffer
	if ((pos = buffer.find('\n')) == string::npos)
		return false; // No complete line stored
	line = buffer.substr(0, pos);
	buffer = buffer.substr(pos + 1);
	return true;
}


// Write a line of the form <key>:<value> to the socket
void LyXDataSocket::writeln(string const & line)
{
	string linen(line + '\n');
	int size = linen.size();
	int written = ::write(fd_, linen.c_str(), size);
	if (written < size) { // Allways mean end of connection.
		if ((written == -1) && (errno == EPIPE)) {
			// The program will also receive a SIGPIPE
			// that must be catched
			cerr << "lyxclient: connection closed while writing."
			     << endl;
		} else {
			// Anything else, including errno == EAGAIN, must be
			// considered IO error. EAGAIN should never happen
			// when line is small
			cerr << "lyxclient: IO error: " << strerror(errno);
		}
		connected_ = false;
	}
}
// ~Class LyXDataSocket -------------------------------------------------------


// Class CmdLineParser -------------------------------------------------------
class CmdLineParser {
public:
	typedef int (*optfunc)(vector<char *> const & args);
	std::map<string, optfunc> helper;
	std::map<string, bool> isset;
	bool parse(int, char * []);
	vector<char *> nonopt;
};


bool CmdLineParser::parse(int argc, char * argv[])
{
	int opt = 1;
	while (opt < argc) {
		vector<char *> args;
		if (helper[argv[opt]]) {
			isset[argv[opt]] = true;
			int arg = opt + 1;
			while ((arg < argc) && (!helper[argv[arg]])) {
				args.push_back(argv[arg]);
				++arg;
			}
			int taken = helper[argv[opt]](args);
			if (taken == -1)
				return false;
			opt += 1 + taken;
		} else {
			if (argv[opt][0] == '-') {
				if ((argv[opt][1] == '-')
				   && (argv[opt][2]== '\0')) {
					++opt;
					while (opt < argc) {
						nonopt.push_back(argv[opt]);
						++opt;
					}
					return true;
				} else {
					cerr << "lyxclient: unknown option "
					     << argv[opt] << endl;
					return false;
				}
			}
			nonopt.push_back(argv[opt]);
			++opt;
		}
	}
	return true;
}
// ~Class CmdLineParser -------------------------------------------------------



namespace cmdline {

void usage()
{
	cerr << "Usage: lyxclient [options]" << endl
	     << "Options are:" << endl
	     << "  -a address    set address of the lyx socket" << endl
	     << "  -t directory  set system temporary directory" << endl
	     << "  -p pid        select a running lyx by pid" << endl
	     << "  -c command    send a single command and quit" << endl
	     << "  -g file row   send a command to go to file and row" << endl
	     << "  -n name       set client name" << endl
	     << "  -h name       display this help end exit" << endl
	     << "If -a is not used, lyxclient will use the arguments of -t and -p to look for" << endl
	     << "a running lyx. If -t is not set, 'directory' defaults to /tmp. If -p is set," << endl
	     << "lyxclient will connect only to a lyx with the specified pid. Options -c and -g" << endl
	     << "cannot be set simultaneoulsly. If no -c or -g options are given, lyxclient" << endl
	     << "will read commands from standard input and disconnect when command read is BYE:"
	     << endl;
}


int h(vector<char *> const &)
{
	usage();
	exit(0);
}


string clientName(support::itoa(::getppid()) + ">" + support::itoa(::getpid()));

int n(vector<char *> const & arg)
{
	if (arg.size() < 1) {
		cerr << "lyxclient: The option -n requires 1 argument."
		     << endl;
		return -1;
	}
	clientName = arg[0];
	return 1;
}


string singleCommand;


int c(vector<char *> const & arg)
{
	if (arg.size() < 1) {
		cerr << "lyxclient: The option -c requires 1 argument."
		     << endl;
		return -1;
	}
	singleCommand = arg[0];
	return 1;
}


int g(vector<char *> const & arg)
{
	if (arg.size() < 2) {
		cerr << "lyxclient: The option -g requires 2 arguments."
		     << endl;
		return -1;
	}
	singleCommand = "LYXCMD:server-goto-file-row "
		+ string(arg[0]) + ' '
		+ string(arg[1]);
	return 2;
}


// 0 if LYXSOCKET is not set in the environment
char * serverAddress = getenv("LYXSOCKET");


int a(vector<char *> const & arg)
{
	if (arg.size() < 1) {
		cerr << "lyxclient: The option -a requires 1 argument."
		     << endl;
		return -1;
	}
	// -a supercedes LYXSOCKET environment variable
	serverAddress = arg[0];
	return 1;
}


string mainTmp("/tmp");


int t(vector<char *> const & arg)
{
	if (arg.size() < 1) {
		cerr << "lyxclient: The option -t requires 1 argument."
		     << endl;
		return -1;
	}
	mainTmp = arg[0];
	return 1;
}


string serverPid; // Init to empty string


int p(vector<char *> const & arg)
{
	if (arg.size() < 1) {
		cerr << "lyxclient: The option -p requires 1 argument."
		     << endl;
		return -1;
	}
	serverPid = arg[0];
	return 1;
}


} // namespace cmdline





int main(int argc, char * argv[])
{
	lyxerr.rdbuf(cerr.rdbuf());

	CmdLineParser args;
	args.helper["-h"] = cmdline::h;
	args.helper["-c"] = cmdline::c;
	args.helper["-g"] = cmdline::g;
	args.helper["-n"] = cmdline::n;
	args.helper["-a"] = cmdline::a;
	args.helper["-t"] = cmdline::t;
	args.helper["-p"] = cmdline::p;

	// Command line failure conditions:
	if ((!args.parse(argc, argv))
	   || (args.isset["-c"] && args.isset["-g"])
	   || (args.isset["-a"] && args.isset["-p"])) {
		cmdline::usage();
		return 1;
	}

	scoped_ptr<LyXDataSocket> server;

	if (cmdline::serverAddress) {
		server.reset(new LyXDataSocket(cmdline::serverAddress));
		if (!server->connected()) {
			cerr << "lyxclient: " << "Could not connect to "
			     << cmdline::serverAddress << endl;
			return EXIT_FAILURE;
		}
	} else {
		// We have to look for an address.
		// serverPid can be empty.
		vector<fs::path> addrs = support::lyxSockets(cmdline::mainTmp, cmdline::serverPid);
		vector<fs::path>::const_iterator addr = addrs.begin();
		vector<fs::path>::const_iterator end = addrs.end();
		for (; addr != end; ++addr) {
			server.reset(new LyXDataSocket(addr->string()));
			if (server->connected())
				break;
			lyxerr << "lyxclient: " << "Could not connect to "
			     << addr->string() << endl;
		}
		if (addr == end) {
			lyxerr << "lyxclient: No suitable server found."
			       << endl;
			return EXIT_FAILURE;
		}
		cerr << "lyxclient: " << "Connected to " << addr->string() << endl;
	}

	int const serverfd = server->fd();

	IOWatch iowatch;
	iowatch.addfd(serverfd);

	// Used to read from server
	string answer;

	// Send greeting
	server->writeln("HELLO:" + cmdline::clientName);
	// wait at most 2 seconds until server responds
	iowatch.wait(2.0);
	if (iowatch.isset(serverfd) && server->readln(answer)) {
		if (prefixIs(answer, "BYE:")) {
			cerr << "lyxclient: Server disconnected." << endl;
			cout << answer << endl;
			return EXIT_FAILURE;
		}
	} else {
		cerr << "lyxclient: No answer from server." << endl;
		return EXIT_FAILURE;
	}

	if (args.isset["-g"] || args.isset["-c"]) {
		server->writeln(cmdline::singleCommand);
		iowatch.wait(2.0);
		if (iowatch.isset(serverfd) && server->readln(answer)) {
			cout << answer;
			if (prefixIs(answer, "ERROR:"))
				return EXIT_FAILURE;
			return EXIT_SUCCESS;
		} else {
			cerr << "lyxclient: No answer from server." << endl;
			return EXIT_FAILURE;
		}
	}

	// Take commands from stdin
	iowatch.addfd(0); // stdin
	bool saidbye = false;
	while ((!saidbye) && server->connected()) {
		iowatch.wait();
		if (iowatch.isset(0)) {
			string command;
			cin >> command;
			if (command == "BYE:") {
				server->writeln("BYE:");
				saidbye = true;
			} else {
				server->writeln("LYXCMD:" + command);
			}
		}
		if (iowatch.isset(serverfd)) {
			while(server->readln(answer))
				cout << answer << endl;
		}
	}

	return EXIT_SUCCESS;
}
