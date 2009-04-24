/*!
   \file

   \section svn Version Control
   - $Revision$
   - $Date$
   - $Author$
*/

#include "sysrepacc.h"
#include <sstream>
#include <iomanip>

namespace libcomm {

// encoding and decoding functions

template <class real, class dbl>
void sysrepacc<real,dbl>::encode(const array1i_t& source, array1i_t& encoded)
   {
   array1i_t parity;
   repacc<real,dbl>::encode(source, parity);
   encoded.init(this->output_block_size());
   encoded.segment(0,source.size()).copyfrom(source);
   encoded.segment(source.size(),parity.size()).copyfrom(parity);
   }

template <class real, class dbl>
void sysrepacc<real,dbl>::translate(const libbase::vector< libbase::vector<double> >& ptable)
   {
   // Inherit sizes
   const int Ns = repacc<real,dbl>::input_block_size();
   const int Np = repacc<real,dbl>::output_block_size();
   const int q = this->num_inputs();
   assertalways(this->num_outputs() == q);
   // Divide ptable for input and output sides
   const libbase::vector< libbase::vector<double> > iptable = ptable.extract(0,Ns);
   const libbase::vector< libbase::vector<double> > optable = ptable.extract(Ns,Np);
   // Perform standard translate
   repacc<real,dbl>::translate(optable);
   // Determine intrinsic source statistics (natural)
   // from the channel
   rp = 1.0;
   for(int i=0; i<Ns; i++)
      for(int x=0; x<q; x++)     // 'x' is the input symbol
         rp(i, x) = iptable(i)(x);
   bcjr<real,dbl>::normalize(rp);
   }

// description output

template <class real, class dbl>
std::string sysrepacc<real,dbl>::description() const
   {
   std::ostringstream sout;
   sout << "Systematic " << repacc<real,dbl>::description();
   return sout.str();
   }

// object serialization - saving

template <class real, class dbl>
std::ostream& sysrepacc<real,dbl>::serialize(std::ostream& sout) const
   {
   return repacc<real,dbl>::serialize(sout);
   }

// object serialization - loading

template <class real, class dbl>
std::istream& sysrepacc<real,dbl>::serialize(std::istream& sin)
   {
   return repacc<real,dbl>::serialize(sin);
   }

}; // end namespace

// Explicit Realizations

#include "logrealfast.h"

namespace libcomm {

using libbase::logrealfast;
using libbase::serializer;

template class sysrepacc<double>;
template <>
const serializer sysrepacc<double>::shelper = serializer("codec", "sysrepacc<double>", sysrepacc<double>::create);

template class sysrepacc<logrealfast>;
template <>
const serializer sysrepacc<logrealfast>::shelper = serializer("codec", "sysrepacc<logrealfast>", sysrepacc<logrealfast>::create);

template class sysrepacc<logrealfast,logrealfast>;
template <>
const serializer sysrepacc<logrealfast,logrealfast>::shelper = serializer("codec", "sysrepacc<logrealfast,logrealfast>", sysrepacc<logrealfast,logrealfast>::create);

}; // end namespace