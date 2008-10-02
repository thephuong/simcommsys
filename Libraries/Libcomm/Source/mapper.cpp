/*!
   \file

   \par Version Control:
   - $Revision$
   - $Date$
   - $Author$
*/

#include "mapper.h"

namespace libcomm {

// Setup functions

void mapper::set_parameters(const int N, const int M, const int S)
   {
   mapper::N = N;
   mapper::M = M;
   mapper::S = S;
   setup();
   }

// Vector mapper operations

void mapper::transform(const libbase::vector<int>& in, libbase::vector<int>& out) const
   {
   advance_always();
   dotransform(in, out);
   }

void mapper::inverse(const libbase::matrix<double>& pin, libbase::matrix<double>& pout) const
   {
   advance_if_dirty();
   doinverse(pin, pout);
   mark_as_dirty();
   }

}; // end namespace
