#ifndef __masterslave_h
#define __masterslave_h

#include "config.h"
#include "vector.h"
#include "socket.h"
#include "timer.h"
#include "functor.h"
#include <map>

namespace libbase {

/*!
   \brief   Socket-based Master-Slave computation.
   \author  Johann Briffa

   \par Version Control:
   - $Revision$
   - $Date$
   - $Author$

   \version 1.00 (20-21 Apr 2007)
   - new class to support socket-based master-slave relationship
   - derived from cmpi 2.40
   - supports dynamic slave list
   - meant to replace cmpi in montecarlo
   - TODO (2): serialize to network byte order always...
   - TODO (3): eventually cmpi will be modified to support this class interface model,
    and a new abstract class created to encapsulate both models

   \version 1.10 (23-25 Apr 2007)
   - Modified all send/receive functions to use network byte order, allowing
    heterogenous usage
   - Added functions to send/receive byte-wide buffers & strings
   - Changed collection of CPU usage to CPU time
   - Modified waitforevent() by adding a bool parameter, to be able to disable the
    acceptance of new connections
   - Changed disable() from a static function to a regular member, and added automatic
    calling from the object destructor
   - Fixed CPU usage information reporting, by implementing the transfer between slaves
    and master, through a new function called updatecputime()
   - Left passing of priority to enable function as default priority, but this can
    now be overridden by a command-line parameter
   - Changed usage model so that client functions are not statics, and so that users
    of this class now declare themselves as derived classes, rather than instantiating
    and object; this is tied with the requirements for RPC functions.
   - TODO: In view of above, most functions are now protected rather than public, since
    only enable/disable are required by other than the derived classes.
   - Changed function-call model so that we don't have to pass pointers; this was
    in great part necessitated by the above change, since the current model only supports
    global pointers. Instead, function calls are now done by passing a string reference,
    which is used as a key in a map list. Two new functions have been added:
    - fregister() to allow registering of functions by derived classes, and
    - fcall() to actually call them
    Since this class cannot know the exact type of the function pointers, these are held
    by functors, implemented as an abstract base class and a templated derived one.
   - Heavily refactored

   \version 1.20 (8 May 2007)
   - Ported to Windows, using Winsock2 API
   - TODO: make setting priority effective on Windows

   \version 1.21 (20 Nov 2007)
   - Added timeout facility to waitforevent(), defaulting to no-timeout.
   - Modified anyoneworking(), making it a const function.
   - Added workingslaves(), returning the number of slaves currently working.

   \version 1.22 (28 Nov 2007)
   - modifications to silence 64-bit portability warnings
    - changed getnumslaves() return type from int to size_t
    - similar changes in enable() and send() functions

   \version 1.23 (30 Nov 2007)
   - refactoring work
    - extracted method dowork()

   \version 1.24 (14 Jan 2008)
   - updated enable() to allow option for local computation only

   \version 1.25 (25 Jan 2008)
   - added function to reset cpu usage accumulation
   - added function to reset a single slave to the 'new' state

   \version 1.26 (6 May 2008)
   - updated send/receive vector to include vector size; this makes
     foreknowledge of size and pre-initialization unnecessary.
*/

class masterslave {
   // constants (tags)
   static const int tag_getname;
   static const int tag_getcputime;
   static const int tag_work;
   static const int tag_die;

// communication objects
public:
   class slave {
      friend class masterslave;
   protected:
      socket *sock;
      enum { NEW, EVENT_PENDING, IDLE, WORKING } state;
   };

// items for use by everyone (?)
private:
   std::map<std::string, functor *> fmap;
   bool initialized;
   double cputimeused;
   timer t;
protected:
   void fregister(const std::string& name, functor *f);
   void fcall(const std::string& name);
public:
   // global enable of cluster system
   void enable(int *argc, char **argv[], const int priority=10);
   // informative functions
   bool isenabled() const { return initialized; };
   double getcputime() const { return initialized ? cputimeused : t.cputime(); };
   size_t getnumslaves() const { return smap.size(); };

// items for use by slaves
private:
   libbase::socket *master;
   // helper functions
   void close(libbase::socket *s);
   void setpriority(const int priority);
   void connect(const std::string& hostname, const int16u port);
   std::string gethostname();
   int gettag();
   void sendname();
   void sendcputime();
   void dowork();
   void slaveprocess(const std::string& hostname, const int16u port, const int priority);
public:
   // slave -> master communication
   bool send(const void *buf, const size_t len);
   bool send(const int x) { return send(&x, sizeof(x)); };
   bool send(const int64u x) { return send(&x, sizeof(x)); };
   bool send(const double x) { return send(&x, sizeof(x)); };
   bool send(const vector<double>& x);
   bool send(const std::string& x);
   bool receive(void *buf, const size_t len);
   bool receive(int& x) { return receive(&x, sizeof(x)); };
   bool receive(int64u& x) { return receive(&x, sizeof(x)); };
   bool receive(double& x) { return receive(&x, sizeof(x)); };
   bool receive(std::string& x);

// items for use by master
private:
   std::map<socket *, slave *> smap;
   // helper functions
   void close(slave *s);
public:
   // creation and destruction
   masterslave();
   ~masterslave();
   // disable process
   void disable();
   // slave-interface functions
   slave *newslave();
   slave *idleslave();
   slave *pendingslave();
   int workingslaves() const;
   bool anyoneworking() const;
   void waitforevent(const bool acceptnew=true, const double timeout=0);
   void resetslave(slave *s);
   void resetslaves();
   // master -> slave communication
   bool send(slave *s, const void *buf, const size_t len);
   bool send(slave *s, const int x) { return send(s, &x, sizeof(x)); };
   bool send(slave *s, const double x) { return send(s, &x, sizeof(x)); };
   bool send(slave *s, const std::string& x);
   bool call(slave *s, const std::string& x) { return send(s, tag_work) && send(s, x); };
   void resetcputime() { cputimeused = 0; };
   bool updatecputime(slave *s);
   bool receive(slave *s, void *buf, const size_t len);
   bool receive(slave *s, int& x) { return receive(s, &x, sizeof(x)); };
   bool receive(slave *s, libbase::int64u& x) { return receive(s, &x, sizeof(x)); };
   bool receive(slave *s, double& x) { return receive(s, &x, sizeof(x)); };
   bool receive(slave *s, vector<double>& x);
   bool receive(slave *s, std::string& x);
};

}; // end namespace

#endif
