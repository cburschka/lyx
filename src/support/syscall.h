// -*- C++ -*-
#include <sys/types.h>

#include "LString.h"

#ifdef __GNUG__
#pragma interface
#endif

/**
  This class can be used to start child processes.

  An instance of the class represents a child process.
  You should use this class if you need to start an external program in LyX.
  If you wish, you can have a callback function executed when the process
  finishes.
  You can chose between three kinds of child processes:
  1) System processes, which are initiated with the "system" call,
     where the main thread waits for the system call to return.
  2) Wait for child process, which are forked, but the main thread waits for
     the child to end.
  3) Don't wait, which are forked, but the main thread is not stopped.
     The process can be considered a background process.
     A timer will make sure that any callback function is called when
     the child process ends.

  Notice that any callback associated with a process is called whatever
  the kind of child process.
  */
class Systemcalls {
public:
	///
	enum Starttype {
		System, // Uses system() which uses /bin/sh
		SystemDontWait, // Uses system() which uses /bin/sh
		Wait, // Uses fork() and execvp()
		DontWait // Uses fork() and execvp()
	};
	
	/// Callback function gets commandline and return value from child
	typedef void (*Callbackfct)(string cmd, int retval);
	
	///
	Systemcalls();
	
	/** Generate instance and start child process.
	  The string "what" contains a commandline with arguments separated 
	  by spaces.
	  When the requested program finishes, the callback-function is 
	  called with the commandline and the return value from the program.
	  The instance is automatically added to a timer check if starttype 
	  is DontWait (i.e. background execution). When a background child
	  finishes, the timer check will automatically call the callback
	  function.
	  */
	Systemcalls(Starttype how, string const & what, Callbackfct call = 0);
	
	///
	~Systemcalls();
	
	/** Start childprocess. "what" contains a command at system level.
	 * This is for reuse of the Systemcalls instance.
	 */
	int startscript(Starttype how, string const & what, 
			Callbackfct call = 0);
	
	/** gets PID of childprocess. Used by timer */
	pid_t getpid() { return pid; }
	
	/// Start callback
	void callback() { if (cbk) cbk(command, retval); }

	/** Set return value. Used by timer */
	void setRetValue(int r) { retval = r; }

	/** Kill child prematurely.
	First, a SIGHUP is sent to the child.
	If that does not end the child process within "tolerance"
	seconds, the SIGKILL signal is sent to the child.
	When the child is dead, the callback is called.
	*/
	void kill(int tolerance = 5);
private:
	/// Type of execution: system, wait for child or background
	Starttype start;

	/// Callback function
	Callbackfct cbk;

	/// Commmand line
	string command;

	/// Process ID of child
	pid_t pid;

	/// Return value from child
	int retval;
	
	///
	int startscript();
	
	///
	pid_t fork();
	
	/// Wait for child process to finish. Updates returncode from child.
	void waitForChild();
};
