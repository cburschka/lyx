// -*- C++ -*-

#ifdef __GNUG__
#pragma interface
#endif

#include <LString.h>


class Systemcalls;

class SystemcallsSingletoncontroller {
public:
	class Startcontroller {
	public:
		Startcontroller();
                ~Startcontroller();
		static SystemcallsSingletoncontroller * getController();
                void reduceRefcount() { refcount--; }
	private:
		static SystemcallsSingletoncontroller * contr;
                static int refcount;
        };
	~SystemcallsSingletoncontroller();
	void addCall(Systemcalls const & newcall);
	void timer();
	// private: // DEC cxx does not like that (JMarc)
	SystemcallsSingletoncontroller();
private:
	struct ControlledCalls {
		Systemcalls *call;
		ControlledCalls *next; 
	};
	ControlledCalls * sysCalls;
};
