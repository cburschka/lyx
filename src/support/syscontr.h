// -*- C++ -*-

#ifdef __GNUG__
#pragma interface
#endif

#include <LString.h>


class Systemcalls;

///
class SystemcallsSingletoncontroller{
public:
	///
	class Startcontroller{
	public:
		///
		Startcontroller();
		///
                ~Startcontroller();
		///
		static SystemcallsSingletoncontroller *GetController();
		///
                void ReduceRefcount() { refcount--; }
	private:
		///
		static SystemcallsSingletoncontroller *contr;
		///
                static int refcount;
        };
	///
	~SystemcallsSingletoncontroller();
	///
	void AddCall(Systemcalls const &newcall);
	///
	void Timer();
	// private: // DEC cxx does not like that (JMarc)
	///
	SystemcallsSingletoncontroller();
private:
	///
	struct ControlledCalls {
		///
		Systemcalls *call;
		///
		ControlledCalls *next; };
	///
	ControlledCalls *SysCalls;
};
