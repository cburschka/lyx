#include <sigc++/marshal.h>
#ifdef SIGC_PTHREADS
#include <sigc++/thread.h>
#endif

#ifdef SIGC_CXX_NAMESPACES
namespace SigC
{
#endif 


int ignore_(int value)
  {
#ifdef SIGC_PTHREADS
static Threads::Private<int> ignore;
#else
static int ignore;
#endif
   int rc=ignore;
   ignore=value;
   return rc;
  }

int RetCode::check_ignore()
  {
   return ignore_(0);
  }

void RetCode::ignore()
  {
   ignore_(1);
  }


#ifdef SIGC_CXX_NAMESPACES
} /* namespace sigc */
#endif

