/*!
 * \file
 *
 * Copyright (c) 2010 Johann A. Briffa
 *
 * This file is part of SimCommSys.
 *
 * SimCommSys is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SimCommSys is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SimCommSys.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "exit_computer.h"

#include "modem/informed_modulator.h"
#include "codec/codec_softout.h"
#include "vector_itfunc.h"
#include "fsm.h"
#include "itfunc.h"
#include "secant.h"
#include "timer.h"
#include "histogram.h"
#include "histogram2d.h"
#include "rvstatistics.h"
#include <iostream>
#include <sstream>

namespace libcomm {

// Determine debug level:
// 1 - Normal debug output only
// 2 - Show histograms of probability tables
#ifndef NDEBUG
#  undef DEBUG
#  define DEBUG 1
#endif

// *** Templated Common Base ***

// Internal functions

/*!
 * \brief Create source sequence to be encoded
 * \return Source sequence of the required length
 *
 * The source sequence consists of uniformly random symbols followed by a
 * tail sequence if required by the given codec.
 */
template <class S>
libbase::vector<int> exit_computer<S>::createsource()
   {
   const int tau = sys->input_block_size();
   array1i_t source(tau);
   for (int t = 0; t < tau; t++)
      source(t) = src.ival(sys->num_inputs());
   return source;
   }

/*!
 * \brief Create table of Gaussian-distributed priors
 * \param[out] priors Table of Gaussian-distributed priors for given input
 * \param[in] tx Vector of transmitted symbols
 */
template <class S>
libbase::vector<libbase::vector<double> > exit_computer<S>::createpriors(
      const array1i_t& tx)
   {
   // determine sizes
   const int N = sys->getcodec()->input_block_size();
   const int q = sys->getcodec()->num_inputs();
   const int k = int(log2(q));
   assert(tx.size() == N);
   assert(q == (1<<k));
   // allocate space for results
   array1vd_t priors;
   libbase::allocate(priors, N, q);
   // allocate space for temporary binary LLRs
   array1d_t llr(k);
   // constants
   const double mu = sigma * sigma / 2;
   // determine random priors
   for (int i = 0; i < N; i++)
      {
      const int cw = tx(i);
      assert(cw >= 0 && cw < q);
      // generate random LLRs (for given codeword)
      // Note: i. LLR is interpreted as ln(Pr(0)/Pr(1))
      //       ii. vector is given lsb first
      for (int j = 0; j < k; j++)
         {
         llr(j) = src.gval(sigma);
         if ((cw & (1 << j)) == 0)
            llr(j) += mu;
         else
            llr(j) -= mu;
         }
      // determine non-binary priors from binary ones
      for (int d = 0; d < q; d++)
         {
         double p = 1;
         for (int j = 0; j < k; j++)
            {
            const double lr = exp(llr(j)); // = p0/p1 = p0/(1-p0) = (1-p1)/p1
            if ((d & (1 << j)) == 0)
               p *= lr / (1 + lr); // = p0
            else
               p *= 1 / (1 + lr); // = p1
            }
         priors(i)(d) = p;
         }
      }
   return priors;
   }

/*!
 * \brief Determine the mutual information between x and p
 * \param x The known transmitted sequence
 * \param p The probability table at the receiving end p(y|x)
 *
 * For a transmitted sequence X = {x_i} where x_i ∈ 𝔽_q and prior or posterior
 * probabilities Y = {y_i} where y_i = [y_i1, y_i2, ... y_iq] and
 * y_ij = Pr{x_i = j} or Pr{R | x_i = j} where R is the received sequence
 *
 * I(X;Y) = ∑_x p(x) ∫_y f(y|x) . log₂ f(y|x)/f(y) dy
 */
template <class S>
double exit_computer<S>::compute_mutual_information(const array1i_t& x,
      const array1vd_t& p)
   {
   // fixed parameters
   const int bins = 10;
   // determine sizes
   const int N = p.size();
   assert(N > 0);
   const int q = p(0).size();
   assert(q > 1);
   assert(x.size() == N);
   // estimate probability density of input
   libbase::histogram<int> hx(x, 0, q, q);
   const libbase::vector<double> fx = libbase::vector<double>(hx.get_count())
         / double(N);
   // estimate probability density of unconditional prior/posterior probabilities
   libbase::histogram2d hy(p, 0, 1, bins, 0, 1, bins);
   const libbase::matrix<double> fy = libbase::matrix<double>(hy.get_count())
         / double(N);
#if DEBUG>=2
   std::cerr << "DEBUG (exit): hy = " << hy.get_count();
   std::cerr << "DEBUG (exit): N = " << N << std::endl;
   std::cerr << "DEBUG (exit): fy = " << fy;
#endif
   // compute mutual information
   double I = 0;
   for (int d = 0; d < q; d++)
      {
      // extract elements where input was equal to 'd'
      const libbase::vector<bool> mask = (x == d);
      const array1vd_t pd = p.mask(mask);
      // determine number of elements
      const int Nd = pd.size();
      assert(Nd > 0);
      // estimate probability density of conditional prior/posterior probabilities
      libbase::histogram2d hyd(pd, 0, 1, bins, 0, 1, bins);
      const libbase::matrix<double> fyd = libbase::matrix<double>(hyd.get_count())
            / double(Nd);
#if DEBUG>=2
      std::cerr << "DEBUG (exit): fy(" << d << ") = " << fyd;
#endif
      // accumulate mutual information
      for (int i = 0; i < bins; i++)
         for (int j = 0; j < bins; j++)
            {
            if (fyd(i, j) > 0 && fy(i, j) > 0)
               I += fx(d) * fyd(i, j) * log2(fyd(i, j) / fy(i, j));
            }
      }
   return I;
   }

/*!
 * \brief Determine the distribution statistics for 'p' where the binary
 *        decomposition of 'x' is equal to 'value'
 * \param x The known transmitted sequence
 * \param p The probability table at the receiving end p(y|x)
 * \param value The conditional value for the transmitted sequence
 * \param sigma The standard deviation of the distribution
 * \param mu The mean of the distribution
 *
 * \todo adapt this to work with non-binary alphabets
 */
template <class S>
void exit_computer<S>::compute_statistics(const array1i_t& x,
      const array1vd_t& p, const int value, double& sigma, double& mu)
   {
   // determine sizes
   const int N = p.size();
   assert(N > 0);
   const int q = p(0).size();
   assert(q > 1);
   assert(x.size() == N);
   const int k = int(log2(q));
   assert(q == (1<<k));
   // iterate through each symbol in the table
   assertalways(k == 1);
   libbase::rvstatistics rv;
   for (int i = 0; i < N; i++)
      {
      if(x(i) == value)
         {
         // compute LLR from probabilities
         const double llr = log(p(i)(0)/p(i)(1));
         rv.insert(llr);
         }
      }
   // store results
   sigma = rv.sigma();
   mu = rv.mean();
   }

/*!
 * \brief Calculate results
 * \param x The known transmitted sequence
 * \param pin The prior probability table
 * \param pout The posterior probability table
 * \param result The vector of results
 */
template <class S>
void exit_computer<S>::compute_results(const array1i_t& x,
      const array1vd_t& pin, const array1vd_t& pout, array1d_t& result)
   {
   assert(result.size() == 10);
   // Compute results
   result(0) = compute_mutual_information(x, pin);
   result(1) = compute_mutual_information(x, pout);
   compute_statistics(x, pin, 0, result(2), result(3));
   compute_statistics(x, pin, 1, result(4), result(5));
   compute_statistics(x, pout, 0, result(6), result(7));
   compute_statistics(x, pout, 1, result(8), result(9));
   }

// Experiment handling

/*!
 * \brief Determine mutual information at input and output of inner and outer decoders
 * \param[out] result   Vector containing the set of results to be updated
 *
 * Results are organized as ...
 */
template <class S>
void exit_computer<S>::sample(array1d_t& result)
   {
   // Initialise result vector
   result.init(count());

   // Create source stream
   const array1i_t source = createsource();
   // Encode
   array1i_t encoded;
   sys->getcodec()->encode(source, encoded);
   // Map
   array1i_t mapped;
   sys->getmapper()->transform(encoded, mapped);
   // Modulate
   array1s_t transmitted;
   sys->getmodem()->modulate(sys->getmodem()->num_symbols(), mapped,
         transmitted);
   // Transmit
   const array1s_t received = sys->transmit(transmitted);

   switch(exit_type)
      {
      case exit_parallel_codec:
         {
         // Demodulate
         array1vd_t ptable_mapped;
         sys->getmodem()->demodulate(*sys->getrxchan(), received, ptable_mapped);
         // Inverse Map
         array1vd_t ptable_encoded;
         sys->getmapper()->inverse(ptable_mapped, ptable_encoded);

         // Create random priors
         array1vd_t priors = createpriors(source);
         // Translate (using given priors)
         codec_softout<libbase::vector>& c = dynamic_cast<codec_softout<
               libbase::vector>&>(*sys->getcodec());
         c.init_decoder(ptable_encoded, priors);
         // Perform soft-output decoding for as many iterations as required
         array1vd_t ri;
         array1vd_t ro;
         for (int i = 0; i < c.num_iter(); i++)
            c.softdecode(ri, ro);
         // Compute extrinsic information
         libbase::compute_extrinsic(ri, ri, priors);
         libbase::normalize_results(ri, ri);

         // compute results
         compute_results(source, priors, ri, result);
         }
         break;

      default:
         failwith("Unknown EXIT chart type");
         break;
      }
   }

// Description & Serialization

template <class S>
std::string exit_computer<S>::description() const
   {
   std::ostringstream sout;
   sout << "EXIT Chart Computer for " << sys->description() << ", ";
   // EXIT chart type
   switch(exit_type)
      {
      case exit_parallel_codec:
         sout << "parallel concatenated codec";
         break;

      case exit_serial_codec:
         sout << "serial concatenated codec";
         break;

      case exit_serial_modem:
         sout << "serial concatenated modem";
         break;

      default:
         failwith("Unknown EXIT chart type");
         break;
      }
   // system parameter
   const double p = sys->gettxchan()->get_parameter();
   assert(p == sys->getrxchan()->get_parameter());
   sout << ", system parameter = " << p;
   return sout.str();
   }

// object serialization - saving

template <class S>
std::ostream& exit_computer<S>::serialize(std::ostream& sout) const
   {
   // format version
   sout << "# Version" << std::endl;
   sout << 1 << std::endl;
   sout << "# EXIT chart type (0=parallel/codec, 1=serial/codec, 2=serial/modem)" << std::endl;
   sout << exit_type << std::endl;
   // system parameter
   const double p = sys->gettxchan()->get_parameter();
   assert(p == sys->getrxchan()->get_parameter());
   sout << "# System parameter" << std::endl;
   sout << p << std::endl;
   // underlying system
   sout << sys;
   return sout;
   }

// object serialization - loading

/*!
 * \version 1 Initial version
 */

template <class S>
std::istream& exit_computer<S>::serialize(std::istream& sin)
   {
   free();
   assertalways(sin.good());
   // get format version
   int version;
   sin >> libbase::eatcomments >> version >> libbase::verify;
   // read type of EXIT chart to compute
   int temp;
   sin >> libbase::eatcomments >> temp >> libbase::verify;
   exit_type = (exit_t) temp;
   // get system parameter
   double p;
   sin >> libbase::eatcomments >> p >> libbase::verify;
   // underlying system
   sin >> libbase::eatcomments >> sys >> libbase::verify;
   // setup
   sys->gettxchan()->set_parameter(p);
   sys->getrxchan()->set_parameter(p);
   return sin;
   }

} // end namespace

#include "gf.h"

namespace libcomm {

// Explicit Realizations
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/stringize.hpp>

using libbase::serializer;

#define USING_GF(r, x, type) \
      using libbase::type;

BOOST_PP_SEQ_FOR_EACH(USING_GF, x, GF_TYPE_SEQ)

// *** General Communication System ***

#define SYMBOL_TYPE_SEQ \
   (sigspace)(bool) \
   GF_TYPE_SEQ

/* Serialization string: exit_computer<type>
 * where:
 *      type = sigspace | bool | gf2 | gf4 ...
 */
#define INSTANTIATE(r, x, type) \
      template class exit_computer<type>; \
      template <> \
      const serializer exit_computer<type>::shelper( \
            "experiment", \
            "exit_computer<" BOOST_PP_STRINGIZE(type) ">", \
            exit_computer<type>::create);

BOOST_PP_SEQ_FOR_EACH(INSTANTIATE, x, SYMBOL_TYPE_SEQ)

} // end namespace
