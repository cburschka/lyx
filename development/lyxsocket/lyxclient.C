/**
 * \file lyxclient.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author João Luis M. Assirati
 *
 * Full author contact details are available in file CREDITS.
 */

#include <string>
#include <vector>
#include <map>
#include <iostream>

// getpid(), getppid()
#include <sys/types.h>
#include <unistd.h>

// select()
#include <sys/select.h>

// opendir(), closedir(), readdir()
#include <sys/types.h>
#include <dirent.h>

// stat()
#include <sys/stat.h>

// socket(), connect()
#include <sys/socket.h>
#include <sys/un.h>

// fcntl()
#include <fcntl.h>

// getenv()
#include <cstdlib>

using std::string;
using std::vector;
using std::cout;
using std::cerr;
using std::cin;
using std::endl;


namespace support {

string itoa(unsigned int i)
{
	string str;
	if(!i) {
		str = "0";
	} else {
		while((0 < i) && (i <= 9)) {
			str = static_cast<char>('0' + i % 10) + str;
			i /= 10;
		}
	}
	return str;
}


bool prefixIs(string const & a, string const & pre)
{
	char const * p_a = a.c_str();
	char const * p_pre = pre.c_str();
	while ((*p_a != '\0') && (*p_pre != '\0') && (*p_a == *p_pre)) {
		++p_a;
		++p_pre;
	}
	if (*p_pre == '\0') return true;
	return false;
}


// Parts stolen from lyx::support::DirList()
// Returns the absolute pathnames of all lyx local sockets
vector<string> lyxSockets(string const & dir, string const & pid)
{
	vector<string> dirlist;
	DIR * dirp = ::opendir(dir.c_str());
	if (!dirp) {
		cerr << "lyxclient: Could not read dir " << dir
		     << ": " << strerror(errno);
		return dirlist;
	}

	dirent * dire;
	while ((dire = ::readdir(dirp))) {
		string const fil = dire->d_name;
		if (prefixIs(fil, "lyx_tmpdir" + pid)) {
			string lyxsocket = dir + '/' + fil + "/lyxsocket";
			struct stat status;
			// not checking if it is a socket -- just if it exists
			// and has reading permissions
			if (!::stat(lyxsocket.c_str(), &status)) {
				dirlist.push_back(lyxsocket);
			}
		}
	}

	::closedir(dirp);
	return dirlist;
}

namespace socktools {

int connect(string const & name)
{
	int fd; // File descriptor for the socket
	sockaddr_un addr; // Structure that hold the socket address

	// char sun_path[108]
	string::size_type len = name.size();
	if(len > 107) {
		cerr << "lyxclient: Socket address '" << name
		     << "' too long." << endl;
		return -1;
	}
	// Synonims for AF_UNIX are AF_LOCAL and AF_FILE
	addr.sun_family = AF_UNIX;
	name.copy(addr.sun_path, 107);
	addr.sun_path[len] = '\0';

	if((fd = ::socket(PF_UNIX, SOCK_STREAM, 0))== -1) {
		cerr << "lyxclient: Could not create socket descriptor: "
		     << strerror(errno) << endl;
		return -1;
	}
	if(::connect(fd, (struct sockaddr *)&addr, SUN_LEN(&addr)) == -1) {
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
class IOWatch
{
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

IOWatch::IOWatch() {
	clear();
}
void IOWatch::clear() {
	FD_ZERO(&des);
}
void IOWatch::addfd(int fd) {
	FD_SET(fd, &des);
}
bool IOWatch::wait(double timeout) {
	timeval to;
	to.tv_sec = static_cast<long int>(timeout);
	to.tv_usec = static_cast<long int>((timeout - to.tv_sec)*1E6);
	act = des;
	return select(FD_SETSIZE, &act,
		      (fd_set *)0, (fd_set *)0, &to);
}
bool IOWatch::wait() {
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
class LyXDataSocket
{
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
class CmdLineParser
{
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
	while(opt < argc) {
		vector<char *> args;
		if(helper[argv[opt]]) {
			isset[argv[opt]] = true;
			int arg = opt + 1;
			while((arg < argc) && (!helper[argv[arg]])) {
				args.push_back(argv[arg]);
				++arg;
			}
			int taken = helper[argv[opt]](args);
			if(taken == -1) return false;
			opt += 1 + taken;
		} else {
			if(argv[opt][0] == '-') {
				if((argv[opt][1] == '-')
				   && (argv[opt][2]== '\0')) {
					++opt;
					while(opt < argc) {
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



namespace cmdline
{
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

int h(vector<char *> const & arg)
{
	usage();
	exit(0);
}

string clientName(support::itoa(::getppid()) + ">" + support::itoa(::getpid()));
int n(vector<char *> const & arg)
{
	if(arg.size() < 1) {
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
	if(arg.size() < 1) {
		cerr << "lyxclient: The option -c requires 1 argument."
		     << endl;
		return -1;
	}
	singleCommand = arg[0];
	return 1;
}

int g(vector<char *> const & arg)
{
	if(arg.size() < 2) {
		cerr << "lyxclient: The option -g requires 2 arguments."
		     << endl;
		return -1;
	}
	singleCommand = "LYXCMD:server-goto-file-row "
		+ static_cast<string>(arg[0]) + ' '
		+ static_cast<string>(arg[1]);
	return 2;
}

// 0 if LYXSOCKET is not set in the environment
char * serverAddress = getenv("LYXSOCKET");
int a(vector<char *> const & arg)
{
	if(arg.size() < 1) {
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
	if(arg.size() < 1) {
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
	if(arg.size() < 1) {
		cerr << "lyxclient: The option -p requires 1 argument."
		     << endl;
		return -1;
	}
	serverPid = arg[0];
	return 1;
}

} // namespace cmdline

using support::prefixIs;

int main(int argc, char * argv[])
{
	CmdLineParser args;
	args.helper["-h"] = cmdline::h;
	args.helper["-c"] = cmdline::c;
	args.helper["-g"] = cmdline::g;
	args.helper["-n"] = cmdline::n;
	args.helper["-a"] = cmdline::a;
	args.helper["-t"] = cmdline::t;
	args.helper["-p"] = cmdline::p;
	// Command line failure conditions:
	if((!args.parse(argc, argv))
	   || (args.isset["-c"] && args.isset["-g"])
	   || (args.isset["-a"] && args.isset["-p"])) {
		cmdline::usage();
		return 1;
	}

	LyXDataSocket * server;

	if(cmdline::serverAddress) {
		server = new LyXDataSocket(cmdline::serverAddress);
		if(!server->connected()) {
			cerr << "lyxclient: " << "Could not connect to "
			     << cmdline::serverAddress << endl;
			return 1;
		}
	} else {
		// We have to look for an address.
		// serverPid can be empty.
		vector<string> addrs = support::lyxSockets(cmdline::mainTmp, cmdline::serverPid);
		vector<string>::iterator addr = addrs.begin();
		vector<string>::iterator end = addrs.end();
		while (addr < end) {
			server = new LyXDataSocket(*addr);
			if(server->connected()) break;
			cerr << "lyxclient: " << "Could not connect to "
			     << *addr << endl;
			delete server;
			++addr;
		}
		if(addr == end) {
			cerr << "lyxclient: No suitable server found." << endl;
			return 1;
		}
		cerr << "lyxclient: " << "Connected to " << *addr << endl;
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
	if(iowatch.isset(serverfd) && server->readln(answer)) {
		if(prefixIs(answer, "BYE:")) {
			cerr << "lyxclient: Server disconnected." << endl;
			cout << answer << endl;
			return 1;
		}
	} else {
		cerr << "lyxclient: No answer from server." << endl;
		return 1;
	}

	if(args.isset["-g"] || args.isset["-c"]) {
		server->writeln(cmdline::singleCommand);
		iowatch.wait(2.0);
		if(iowatch.isset(serverfd) && server->readln(answer)) {
			cout << answer;
			if(prefixIs(answer, "ERROR:")) return 1;
			return 0;
		} else {
			cerr << "lyxclient: No answer from server." << endl;
			return 1;
		}
	}

	// Take commands from stdin
	iowatch.addfd(0); // stdin
	bool saidbye = false;
	while((!saidbye) && server->connected()) {
		iowatch.wait();
		if(iowatch.isset(0)) {
			string command;
			cin >> command;
			if(command == "BYE:") {
				server->writeln("BYE:");
				saidbye = true;
			} else {
				server->writeln("LYXCMD:" + command);
			}
		}
		if(iowatch.isset(serverfd)) {
			while(server->readln(answer))
				cout << answer << endl;
		}
	}

	return 0;
}
