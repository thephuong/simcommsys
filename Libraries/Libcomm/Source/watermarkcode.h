#ifndef __watermarkcode_h
#define __watermarkcode_h

#include "config.h"
#include "vcs.h"

#include "codec.h"
#include "itfunc.h"
#include "serializer.h"
#include <stdlib.h>
#include <math.h>

/*
  Version 1.00 (1 Oct 2007)
  initial version; implements Watermark Codes as described by Davey in "Reliable
  Communication over Channels with Insertions, Deletions, and Substitutions", Trans. IT,
  Feb 2001.
*/

namespace libcomm {

template <class real> class watermarkcode : public codec {
   static const libbase::vcs version;
   static const libbase::serializer shelper;
   static void* create() { return new watermarkcode<real>; };
private:
   int	      tau, m;		// block length, and encoder memory order
   int	      K, N;		// # of inputs and outputs (respectively)
protected:
   void init();
   void free();
   watermarkcode();
public:
   watermarkcode(const int tau);
   ~watermarkcode() { free(); };

   codec *clone() const { return new watermarkcode(*this); };		// cloning operation
   const char* name() const { return shelper.name(); };

   void encode(libbase::vector<int>& source, libbase::vector<int>& encoded);
   void translate(const libbase::matrix<double>& ptable);
   void decode(libbase::vector<int>& decoded);

   int block_size() const { return tau; };
   int num_inputs() const { return K; };
   int num_outputs() const { return N; };
   int tail_length() const { return m; };
   int num_iter() const { return 1; };

   std::string description() const;
   std::ostream& serialize(std::ostream& sout) const;
   std::istream& serialize(std::istream& sin);
};

}; // end namespace

#endif

