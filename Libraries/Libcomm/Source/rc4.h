#ifndef __rc4_h
#define __rc4_h

#include "config.h"
#include "vcs.h"
#include "vector.h"

#include <string>

/*
  Version 1.00 (03 Jul 2003)
  initial version - class that implements RSA RC4 Algorithm, as specified in
  Schneier, "Applied Cryptography", 1996, pp.397-398.

  Version 1.01 (04 Jul 2003)
  changed vector tables to int8u instead of int, to ensure validity of values.

  Version 1.02 (5 Jul 2003)
  * added self-testing on creation of the first object.
  * modified counters to be int8u instead of int - also renamed them x & y
  * removed superfluous mod 256 (& 0xff) operations

  Version 1.03 (17 Jul 2006)
  in encrypt, changed the loop variable to type size_t, to avoid the warning about
  comparisons between signed and unsigned types.

  Version 1.10 (6 Nov 2006)
  * defined class and associated data within "libcomm" namespace.
  * removed use of "using namespace std", replacing by tighter "using" statements as needed.
*/

namespace libcomm {

class rc4 {
   static const libbase::vcs version;
   // static variables
   static bool tested;
   // working spaces
   libbase::vector<libbase::int8u> S;
   libbase::int8u x,y;
public:
   // basic constructor/destructor
	rc4();
	virtual ~rc4();
   // public functions
   void init(std::string key);
   std::string encrypt(const std::string plaintext);
   libbase::int8u encrypt(const libbase::int8u plaintext);
protected:
   // private functions
   bool verify(const std::string key, const std::string plaintext, const std::string ciphertext);
};

}; // end namespace

#endif
