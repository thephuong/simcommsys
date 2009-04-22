/*!
   \file

   \section svn Version Control
   - $Revision$
   - $Date$
   - $Author$
*/

#include "repacc.h"
#include <sstream>
#include <iomanip>

namespace libcomm {

// initialization / de-allocation

template <class real, class dbl>
void repacc<real,dbl>::init()
   {
   // check sizes
   assertalways(N > 0);
   assertalways(q > 0);
   // initialize BCJR subsystem for accumulator
   assertalways(encoder);
   bcjr<real,dbl>::init(*encoder, output_block_size());
   // check that encoder is rate-1
   assertalways(num_inputs() == num_outputs());
   assertalways(inter);
   // TODO: check interleaver sizes
   assertalways(iter > 0);

   initialised = false;
   }

template <class real, class dbl>
void repacc<real,dbl>::free()
   {
   if(encoder != NULL)
      delete encoder;
   if(inter != NULL)
      delete inter;
   }

template <class real, class dbl>
void repacc<real,dbl>::reset()
   {
   if(endatzero)
      {
      bcjr<real,dbl>::setstart(0);
      bcjr<real,dbl>::setend(0);
      }
   else
      {
      bcjr<real,dbl>::setstart(0);
      bcjr<real,dbl>::setend();
      }
   }

// constructor / destructor

template <class real, class dbl>
repacc<real,dbl>::repacc()
   {
   encoder = NULL;
   inter = NULL;
   }

// memory allocator (for internal use only)

template <class real, class dbl>
void repacc<real,dbl>::allocate()
   {
   ra.init(output_block_size(), num_inputs());
   rp.init(output_block_size(), num_inputs());
   R.init(output_block_size(), encoder->num_outputs());

   // determine memory occupied and tell user
   std::ios::fmtflags flags = std::cerr.flags();
   std::cerr << "RepAcc Memory Usage: " << std::fixed << std::setprecision(1);
   std::cerr << ( ra.size() + rp.size() + R.size()
      )*sizeof(dbl)/double(1<<20) << "MB\n";
   std::cerr.setf(flags);
   // flag the state of the arrays
   initialised = true;
   }

// encoding and decoding functions

template <class real, class dbl>
void repacc<real,dbl>::seedfrom(libbase::random& r)
   {
   inter->seedfrom(r);
   }

template <class real, class dbl>
void repacc<real,dbl>::encode(const array1i_t& source, array1i_t& encoded)
   {
   assert(source.size() == input_block_size());
   // Compute repeater output, including any necessary tail
   array1i_t rep(output_block_size());
   for(int i=0; i<input_block_size(); i++)
      for(int j=0; j<q; j++)
         rep(i*q+j) = source(i);
   for(int i=input_block_size()*q; i<output_block_size(); i++)
      rep(i) = fsm::tail;

   // Declare space for the interleaved sequence
   array1i_t rep2;
   // Advance interleaver to the next block
   inter->advance();
   // Create interleaved sequence
   inter->transform(rep, rep2);

   // Initialise result vector
   encoded.init(output_block_size());
   // Reset the encoder to zero state
   encoder->reset(0);
   // Encode sequence
   for(int i=0; i<output_block_size(); i++)
      encoded(i) = encoder->step(rep2(i)) / num_inputs();
   // check that encoder finishes correctly
   if(endatzero)
      assertalways(encoder->state() == 0);
   }

/*! \copydoc codec::translate()

   Sets: ra, R

   \note The BCJR normalization method is used to normalize the channel-derived
         (intrinsic) probabilities 'r' and 'R'; in view of this, the a-priori
         probabilities are now created normalized.
*/
template <class real, class dbl>
void repacc<real,dbl>::translate(const libbase::vector< libbase::vector<double> >& ptable)
   {
   // Encoder symbol space must be the same as modulation symbol space
   assertalways(ptable.size() > 0);
   assertalways(ptable(0).size() == num_outputs());
   // Confirm input sequence to be of the correct length
   assertalways(ptable.size() == output_block_size());

   // initialise memory if necessary
   if(!initialised)
      allocate();

   // Initialise a priori probabilities (extrinsic)
   ra = 1.0;
   // Initialise a priori probabilities (intrinsic)
   rp = 1.0;

   // Determine encoder-output statistics (intrinsic) from the channel
   R = 0.0;
   for(int i=0; i<output_block_size(); i++)
      for(int x=0; x<num_outputs(); x++)
         for(int j=0; j<num_inputs(); j++)
            R(i, x*num_inputs()+j) = ptable(i)(x);
   bcjr<real,dbl>::normalize(R);

   // Reset start- and end-state probabilities
   reset();
   }

/*! \copydoc codec_softout::softdecode()

   \note Implements soft-decision decoding according to Alexandre's
         interpretation:
         - when computing final output at repetition code, use only extrinsic
           information from accumulator
         - when computing extrinsic output at rep code, factor out the input
           information at that position
*/
template <class real, class dbl>
void repacc<real,dbl>::softdecode(array1vd_t& ri)
   {
   // decode accumulator

   // Temporary variables to hold posterior probabilities and
   // interleaved versions of ra/ri
   array2d_t rif, rai, rii;
   inter->transform(ra, rai);
   bcjr<real,dbl>::fdecode(R, rai, rii);
   inter->inverse(rii, rif);
   ra = rif.divide(ra.multiply(rp));
   bcjr<real,dbl>::normalize(ra);
   // allocate space for final results and initialize
   ri.init(input_block_size());
   for(int i=0; i<input_block_size(); i++)
      ri(i).init(num_inputs());
   ri = 1.0;
   // decode repetition code (based on extrinsic information only)
   for(int i=0; i<input_block_size(); i++)
      for(int j=0; j<q; j++)
         for(int x=0; x<num_inputs(); x++)
            ri(i)(x) *= ra(i*q+j,x);
   // compute extrinsic information
   for(int i=0; i<input_block_size(); i++)
      for(int j=0; j<q; j++)
         for(int x=0; x<num_inputs(); x++)
            ra(i*q+j,x) = ri(i)(x) / ra(i*q+j,x);
   // normalize results
   bcjr<real,dbl>::normalize(ra);
   }

template <class real, class dbl>
void repacc<real,dbl>::softdecode(array1vd_t& ri, array1vd_t& ro)
   {
   assertalways("Not yet implemented");
   }

// description output

template <class real, class dbl>
std::string repacc<real,dbl>::description() const
   {
   std::ostringstream sout;
   sout << "Repeat-Accumulate Code (" << N << "," << q << ") - ";
   sout << encoder->description() << ", ";
   sout << inter->description() << ", ";
   sout << iter << " iterations, ";
   sout << (endatzero ? "Terminated" : "Unterminated");
   return sout.str();
   }

// object serialization - saving

template <class real, class dbl>
std::ostream& repacc<real,dbl>::serialize(std::ostream& sout) const
   {
   // format version
   sout << 1 << '\n';
   sout << encoder;
   sout << inter;
   sout << N << '\n';
   sout << q << '\n';
   sout << iter << '\n';
   sout << int(endatzero) << '\n';
   return sout;
   }

// object serialization - loading

template <class real, class dbl>
std::istream& repacc<real,dbl>::serialize(std::istream& sin)
   {
   assertalways(sin.good());
   free();
   // get format version
   int version;
   sin >> version;
   // get first-version items
   sin >> encoder;
   sin >> inter;
   sin >> N;
   sin >> q;
   sin >> iter;
   sin >> endatzero;
   init();
   assertalways(sin.good());
   return sin;
   }

}; // end namespace

// Explicit Realizations

#include "logrealfast.h"

namespace libcomm {

using libbase::logrealfast;
using libbase::serializer;

template class repacc<double>;
template <>
const serializer repacc<double>::shelper = serializer("codec", "repacc<double>", repacc<double>::create);

template class repacc<logrealfast>;
template <>
const serializer repacc<logrealfast>::shelper = serializer("codec", "repacc<logrealfast>", repacc<logrealfast>::create);

template class repacc<logrealfast,logrealfast>;
template <>
const serializer repacc<logrealfast,logrealfast>::shelper = serializer("codec", "repacc<logrealfast,logrealfast>", repacc<logrealfast,logrealfast>::create);

}; // end namespace
