// -*- C++ -*-
#include <sys/types.h>
#include <LString.h>

#ifdef __GNUG__
#pragma interface
#endif


/*@Doc:
  Instance starts and represents childprocesses.
  You should use this class if you need to start an external program in LyX.
  You can start a child in the background and have a callback function
  executed when the child finishes by using the DontWait starttype.
  */
class Systemcalls {
public:
	///
	enum Starttype {
		System,
		Wait,
		DontWait
	};
	
	/// Callback function gets commandline and returnvalue from child
	typedef void (*Callbackfct)(string cmd, int retval);
	
	///
	Systemcalls();
	
	/** Geberate instance and start childprocess 
	  The string "what" contains a commandline with arguments separated 
	  by spaces.
	  When the requested program finishes, the callback-function is 
	  called with the commandline and the returnvalue from the program.
	  The instance is automatically added to a timercheck if starttype is
	  DontWait (i.e. background execution). When a background child
	  finishes, the timercheck will automatically call the callback
	  function.
	  */
	Systemcalls(Starttype how, string what, Callbackfct call = 0);
	
	
	///
	~Systemcalls();
	
	/** Start childprocess. what contains a command on systemlevel. 
	 */
	int Startscript(Starttype how, string what, Callbackfct call = 0); // for reuse
	
	/** gets PID of childprocess. Used by timer */
	inline pid_t Getpid() { return pid; }
	
	/// Start callback
	inline void Callback() { if (cbk) cbk(command, retval); }
	
	/** Set return value. Used by timer */
	inline void setRetValue(int r) { retval = r; }
private:
	/// Type of execution: system, wait for child or background
	Starttype    start;
	/// Callback function
	Callbackfct  cbk;
	/// Commmand line
	string      command;
	/// Process ID of child
	pid_t        pid;
	/// Return value from child
	int retval;
	
	///
	int Startscript();
	
	///
	pid_t Fork();
	
	/// Wait for child process to finish. Updates returncode from child.
	void waitForChild();
};
