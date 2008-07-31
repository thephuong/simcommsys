/*!
   \file

   \par Version Control:
   - $Revision$
   - $Date$
   - $Author$
*/

#include "map_straight.h"
#include <stdlib.h>
#include <sstream>

namespace libcomm {

// Serialization Support

const libbase::serializer map_straight::shelper("mapper", "map_straight", map_straight::create);

// Vector mapper operations

void map_straight::transform(const libbase::vector<int>& in, libbase::vector<int>& out)
   {
   // Determine length of encoded sequence
   const int tau = in.size();
   // Initialize results vector
   out.init(tau*s1);
   // Modulate encoded stream (least-significant first)
   for(int t=0, k=0; t<tau; t++)
      for(int i=0, x = in(t); i<s1; i++, k++, x /= M)
         out(k) = x % M;
   }

void map_straight::inverse(const libbase::matrix<double>& pin, libbase::matrix<double>& pout)
   {
   assertalways(pin.ysize() == M);
   // Determine required length of encoded sequence, and confirm validity
   const int tau = pin.xsize() / s2;
   assertalways(pin.xsize() == tau*s2);
   // Initialize results vector
   pout.init(tau,S);
   // Get the necessary data from the channel
   for(int t=0; t<tau; t++)
      for(int x=0; x<S; x++)
         {
         pout(t,x) = 1;
         for(int i=0, thisx = x; i<s2; i++, thisx /= M)
            pout(t, x) *= pin(t*s2+i, thisx % M);
         }
   }

// Setup functions

/*! \copydoc mapper::set_parameters()

   \note Each encoder output must be represented by an integral number of
         modulation symbols
*/
void map_straight::set_parameters(const int N, const int M, const int S)
   {
   map_straight::N = N;
   map_straight::M = M;
   map_straight::S = S;
   s1 = get_rate(M, N);
   s2 = get_rate(M, S);
   }

// Description

std::string map_straight::description() const
   {
   std::ostringstream sout;
   sout << "Straight Mapper";
   return sout.str();
   }

// Serialization Support

std::ostream& map_straight::serialize(std::ostream& sout) const
   {
   return sout;
   }

std::istream& map_straight::serialize(std::istream& sin)
   {
   return sin;
   }

}; // end namespace
