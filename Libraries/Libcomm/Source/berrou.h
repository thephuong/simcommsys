#ifndef __berrou_h
#define __berrou_h

#include "config.h"
#include "vcs.h"
#include "lut_interleaver.h"
#include "serializer.h"
#include "itfunc.h"

/*
  Version 1.01 (4 Nov 2001)
  added a function which outputs details on the interleaving scheme (in accordance 
  with interleaver 1.10)

  Version 1.10 (28 Feb 2002)
  moved creation functions into the implementation file rather than inline.
  added serialization facility (in accordance with interleaver 1.20)

  Version 1.11 (6 Mar 2002)
  changed vcs version variable from a global to a static class variable.
  also changed use of iostream from global to std namespace.

  Version 1.20 (7 Mar - 8 Mar 2002)
  updated the system to conform with the completed serialization protocol for interleaver
  (in conformance with interleaver 1.32), by adding the necessary name() function, and
  also by removing the class name reading/writing in serialize(); this is now done only
  in the stream << and >> functions. serialize() assumes that the correct class is
  being read/written. We also add a static serializer member and initialize it with this
  class's name and the static constructor/loader (adding that too, together with the
  necessary private empty constructor). Also made the interleaver object a public base
  class, rather than a virtual public one, since this was affecting the transfer of
  virtual functions within the class (causing access violations).

  Version 1.21 (8 Mar 2002)
  changed the name() function to use the serializer's name(), introduced in serializer
  1.03, in order to improve consistency between what's written on saving the class
  and what the class is classified as with serializer.

  Version 1.22 (11 Mar 2002)
  changed access level of init() and default constructor to protected - this should
  facilitate the creation of derived classes. Also, changed createandload to an inline
  function create() which simply allocates a new object and returns its pointer. This
  makes the system compatible with the new serializer protocol, as defined in
  serializer 1.10. Also, added public cloning function.

  Version 1.30 (27 Mar 2002)
  changed descriptive output function to conform with interleaver 1.40.

  Version 1.40 (30 Oct 2006)
  * defined class and associated data within "libcomm" namespace.
  * removed use of "using namespace std", replacing by tighter "using" statements as needed.
*/

namespace libcomm {

class berrou : public lut_interleaver {
   static const libbase::vcs version;
   static const libbase::serializer shelper;
   static void* create() { return new berrou; };
   int M;
protected:
   void init(const int M);
   berrou() {};
public:
   berrou(const int M) { init(M); };
   ~berrou() {};
   interleaver* clone() const { return new berrou(*this); };
   const char* name() const { return shelper.name(); };

   std::string description() const;
   std::ostream& serialize(std::ostream& sout) const;
   std::istream& serialize(std::istream& sin);
};

}; // end namespace

#endif

