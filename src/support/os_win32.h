// os_win32.h copyright "Ruurd A. Reitsma" <R.A.Reitsma@wbmt.tudelft.nl>

#ifndef _OS_WIN32_H_
#define _OS_WIN32_H_

//Avoid zillions of windows includes
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

// Avoid some very annoying macros from MFC 
#  undef max
#  undef min

#if defined( __SGI_STL_PORT )

  namespace std {
    // These overloads prevent ambiguity errors when signed and unsigned integers are compared.
    inline long min(long x, long y) {return std::min(x, y);}  
    inline long max(long x, long y) {return std::max(x, y);}
  }

#else // defined( __SGI_STL_PORT )
  namespace std {
    // These overloads prevent ambiguity errors when int, long and unsigned int and int are compared.
	
	inline int min(int x, int y) { return x < y ? x : y; }  
	inline int max(int x, int y) { return x > y ? x : y; }

	inline unsigned int min(unsigned int x, unsigned int y) { return x < y ? x : y; }  
	inline unsigned int max(unsigned int x, unsigned int y) { return x > y ? x : y; }

	inline long min(long x, long y) { return x < y ? x : y; }  
	inline long max(long x, long y) { return x > y ? x : y; }

	inline long min(int x, long y) { return x < y ? x : y; }  
	inline long max(int x, long y) { return x > y ? x : y; }

	inline long min(long x, int y) { return x < y ? x : y; }  
	inline long max(long x, int y) { return x > y ? x : y; }

	inline unsigned long min(unsigned long x, unsigned long y) { return x < y ? x : y; }  
	inline unsigned long max(unsigned long x, unsigned long y) { return x > y ? x : y; }

	inline double min(double x, double y) { return x < y ? x : y; }  
	inline double max(double x, double y) { return x > y ? x : y; }
  }

#endif // defined( __SGI_STL_PORT )

#ifdef __cplusplus
extern "C" {
#endif


//fcntl.h
#define FD_CLOEXEC	1	/* posix */
#define	F_DUPFD		0	/* Duplicate fildes */
#define	F_GETFD		1	/* Get fildes flags (close on exec) */
#define	F_SETFD		2	/* Set fildes flags (close on exec) */
#define	F_GETFL		3	/* Get file flags */
#define	F_SETFL		4	/* Set file flags */
#define O_NONBLOCK      0x4000
inline int fcntl (int, int, ...) {return -1;}

//signal.h
#define SIGHUP 1
#define SIGKILL 9

//sys/time.h
//struct timeval {
//  long tv_sec;
//  long tv_usec;
//};

//unistd.h
inline int fork () {return -1;}
#define pipe(a) _pipe(a,0,0)


//sys/wait.h
#define waitpid(a,b,c) cwait(b,a,c)
#define WNOHANG 1
#define WUNTRACED 2
#define WIFEXITED(a) 0
#define WEXITSTATUS(a) 0
#define WIFSIGNALED(a) 0
#define WTERMSIG(a) 0
#define WIFSTOPPED(a) 0
#define WSTOPSIG(a) 0

//sys/types.h
#define fd_set int

//sys/select.h
//#define select(a,b,c,d,e) -1
#define FD_ZERO(a)
#define FD_SET(a,b)
#define FD_ISSET(fd, set) 0

#ifndef __MINGW32__ //already defined in mingw headers

#define	_S_IFBLK	0x3000
#define	S_IFIFO		_S_IFIFO
#define	S_IFBLK		_S_IFBLK
#define	S_ISFIFO(m)	(((m) & S_IFMT) == S_IFIFO)
#define	S_ISBLK(m)	(((m) & S_IFMT) == S_IFBLK)
#define popen(a,b) _popen(a,b)
#define pclose(a) _pclose(a)

#endif //!__MINGW32

#ifdef __cplusplus
}
#endif

#endif //_OS_WIN32_H_
