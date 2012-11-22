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
 * 
 * \section svn Version Control
 * - $Id$
 */

#include "commsys_stream_simulator.h"

#include "vectorutils.h"
#include "vector_itfunc.h"
#include "hard_decision.h"
#include <sstream>

namespace libcomm {

// Determine debug level:
// 1 - Normal debug output only
// 2 - Keep track of encoded/decoded frame sizes and drift
// 3 - Observe prior and posterior PDF's
// 4 - For fidelity collector, observe actual/estimated boundary drifts
#ifndef NDEBUG
#  undef DEBUG
#  define DEBUG 1
#endif

// Experiment handling

/*!
 * \brief Perform a complete encode->transmit->receive cycle
 * \param[out] result   Vector containing the set of results to be updated
 *
 * Results are organized according to the collector used, as a function of
 * the iteration count.
 *
 * \note The results collector assumes that the result vector is an accumulator,
 * so that every call adds to the existing result. This explains the need to
 * initialize the result vector to zero.
 */
template <class S, class R>
void commsys_stream_simulator<S, R>::sample(libbase::vector<double>& result)
   {
   assert(sys_enc);

   // reset if we have reached the user-set limit for stream length
   switch (mode)
      {
      case mode_reset:
      case mode_terminated:
         if (frames_decoded >= N)
            reset();
         break;
      default:
         break;
      }

   // Get access to the results collector in codeword boundary analysis mode
   fidelity_pos* rc = dynamic_cast<fidelity_pos*>(this);
   // Get access to the decoder-side commsys object in stream-oriented mode
   commsys_stream<S>& sys_dec = getsys_stream();

   // Shorthand for transmitted frame size
   const int tau = sys_dec.output_block_size();
   // Keep a copy of the last frame's offset (in case x_max changes)
   const libbase::size_type<libbase::vector> oldoffset = offset;

   // Determine the suggested look-ahead quantity
   const libbase::size_type<libbase::vector> lookahead =
         sys_dec.getmodem_stream().get_suggested_lookahead();
   // Determine start-of-frame and end-of-frame prior probabilities
   libbase::vector<double> sof_prior;
   libbase::vector<double> eof_prior;
   sys_dec.compute_priors(eof_post, lookahead, sof_prior, eof_prior, offset);

   // Extract required segment of existing stream
   sys_dec.stream_advance(received, oldoffset, estimated_drift, offset);
   // Determine required segment size
   const int length = tau + lookahead + eof_prior.size() - 1;
#if DEBUG>=2
   std::cerr << "DEBUG (commsys_stream_simulator): final segment size = " << length << std::endl;
#endif
   // Append required segment by simulating frame transmission
   // Also make sure that we have transmitted the frame corresponding to the
   // received one.
   // NOTE: it does not matter if we transmit more frames than needed for a
   // terminated stream (and we most likely will), as once we set the eof prior
   // the decoder will know at which point to stop in the received sequence.
   for (int left = length - received.size(); left > 0 || source.empty();)
      {
      // Create next source frame
      const array1i_t source_next = Base::createsource();
      // Encode -> Map -> Modulate next frame
      const array1s_t transmitted = sys_enc->encode_path(source_next);
      // Transmit next frame
      const array1s_t received_next = sys_dec.transmit(transmitted);
      // keep data for codeword boundary analysis if this is indicated
      if (rc)
         {
         // get codeword boundary positions from modem (encoder-side)
         const array1i_t boundary_pos =
               sys_enc->getmodem_stream().get_boundaries();
         // get actual drift at codeword boundary positions from channel (decoder-side)
         const array1i_t act_drift = sys_dec.gettxchan_stream().get_drift(
               boundary_pos);
         // store what we need to keep
         act_bdry_drift.push_back(act_drift);
         }
      // shorthand for received frame size
      const int rho = received_next.size();
      // store what we need to keep
      source.push_back(source_next);
      received = concatenate(received, received_next);
      actual_drift.push_back(rho - tau);
      // update counters
      frames_encoded++;
      left -= rho;
      // Tell user what we're doing
#if DEBUG>=2
      std::cerr << "DEBUG (commsys_stream_simulator): Actual received frame = " << rho << std::endl;
      std::cerr << "DEBUG (commsys_stream_simulator): Remaining length = " << left << std::endl;
      std::cerr << "DEBUG (commsys_stream_simulator): Frames encoded = " << frames_encoded << std::endl;
#endif
      }
   // If it's the last frame in a terminated stream, set eof prior accordingly.
   if (mode == mode_terminated && frames_decoded == N - 1)
      {
      // determine the actual drift at the end of the frame to be decoder
      const int drift = actual_drift.front() - drift_error;
      // make sure this is within the range that can be expressed
      assertalways(drift + offset >= 0);
      assertalways(drift + offset < eof_prior.size());
      // set eof prior to fix end of frame position
      eof_prior = 0;
      eof_prior(drift + offset) = 1;
      }
   // Shorthand for curent segment in received sequences
   const array1s_t& received_segment = received.extract(0, length);

   // Initialise result vector
   result.init(this->count());
   result = 0;
   // Initialize extrinsic information vector
   array1vd_t ptable_ext;
   // Inner code (modem) iterations
   for (int iter_modem = 0; iter_modem < sys_dec.sys_iter(); iter_modem++)
      {
      // Demodulate
      array1vd_t ptable_post;
      array1d_t sof_post;
      sys_dec.getmodem_stream().demodulate(*sys_dec.getrxchan(),
            received_segment, lookahead, sof_prior, eof_prior, ptable_ext,
            ptable_post, sof_post, eof_post, offset);
      // Normalize posterior information
      libbase::normalize_results(ptable_post, ptable_post);
      // Compute extrinsic information for passing to codec
      libbase::compute_extrinsic(ptable_ext, ptable_post, ptable_ext);
      // After-demodulation receive path
      // Inverse Map
      array1vd_t ptable_encoded;
      sys_dec.getmapper()->inverse(ptable_ext, ptable_encoded);
      // Translate
      sys_dec.getcodec()->init_decoder(ptable_encoded);
      // Inverse Map -> Translate
      //sys_dec.softreceive_path(ptable_ext);
      //sys_dec.receive_path(received_segment, lookahead, sof_prior, eof_prior, offset);

      // and perform codeword boundary analysis if this is indicated
      if (rc)
         {
         // get estimated drift pdfs
         array1vd_t post_pdftable;
         sys_dec.getmodem_stream().get_post_drift_pdf(post_pdftable);
         // get most probable estimated drift positions
         array1i_t est_drift(post_pdftable.size());
         for (int i = 0; i < post_pdftable.size(); i++)
            est_drift(i) = commsys_stream<S>::estimate_drift(post_pdftable(i),
                  offset);
         // get actual drift at codeword boundary positions to compare against
         assert(!act_bdry_drift.empty());
         const array1i_t act_drift = act_bdry_drift.front();
         // Tell user what we're doing
#if DEBUG>=4
         std::cerr << "DEBUG (commsys_stream_simulator): act bdry drift = " << act_drift << std::endl;
         std::cerr << "DEBUG (commsys_stream_simulator): est bdry drift = " << est_drift << std::endl;
#endif
         // accumulate results
         libbase::indirect_vector<double> result_segment = result.segment(
               R::count() * iter_modem, R::count());
         rc->updateresults(result_segment, act_drift, est_drift);
         }

      // Get source message to compare against
      assert(!source.empty());
      array1i_t source_this = source.front();
      // For every iteration
      array1i_t decoded;
      array1vd_t ri;
      array1vd_t ro;
      for (int iter_codec = 0; iter_codec < sys_dec.num_iter(); iter_codec++)
         {
         // Perform soft-output decoding
         sys_dec.getcodec_softout().softdecode(ri, ro);
         // Compute hard-decision for results gatherer
         hard_decision<libbase::vector, double> functor;
         functor(ri, decoded);
         // Update results if necessary
         if (!rc)
            {
            libbase::indirect_vector<double> result_segment = result.segment(
                  R::count() * (iter_modem * sys_dec.num_iter() + iter_codec),
                  R::count());
            R::updateresults(result_segment, source_this, decoded);
            }
         }
      // Normalize posterior information
      libbase::normalize_results(ro, ro);
      // Compute extrinsic information for next demodulation cycle
      libbase::compute_extrinsic(ptable_encoded, ro, ptable_encoded);
      // Pass through mapper
      sys_dec.getmapper()->transform(ptable_encoded, ptable_ext);
      //array1vd_t ro_mapped;
      //sys_dec.getmapper()->transform(ro, ro_mapped);
      // Compute extrinsic information for next demodulation cycle
      //libbase::compute_extrinsic(ptable_ext, ro_mapped, ptable_ext);
      // Keep record of what we last simulated
      this->last_event = concatenate(source_this, decoded);
      // If this was not the last iteration, mark components as clean
      if (iter_modem + 1 < sys_dec.sys_iter())
         {
         sys_dec.getmodem()->mark_as_clean();
         sys_dec.getmapper()->mark_as_clean();
         }
      }

   // Prepare comparison sequences for next frame
   source.pop_front();
   if (rc)
      act_bdry_drift.pop_front();
   // Store posterior end-of-frame drift probabilities
   //eof_post = sys_dec.get_eof_post();
   // Determine estimated drift
   estimated_drift = commsys_stream<S>::estimate_drift(eof_post, offset);
   // Centralize posterior probabilities
   eof_post = commsys_stream<S>::centralize_pdf(eof_post, estimated_drift);
   // Tell user what we're doing
#if DEBUG>=3
   std::cerr << "DEBUG (commsys_stream_simulator): eof prior = " << eof_prior << std::endl;
   std::cerr << "DEBUG (commsys_stream_simulator): eof post = " << eof_post << std::endl;
#endif
   // Determine actual cumulative drift and error in drift estimation
   assert(!actual_drift.empty());
   const int actual_drift_this = actual_drift.front();
   actual_drift.pop_front();
   drift_error += estimated_drift - actual_drift_this;
   // Tell user what we're doing
#if DEBUG>=2
   std::cerr << "DEBUG (commsys_stream_simulator): Actual frame drift = " << actual_drift_this << std::endl;
   std::cerr << "DEBUG (commsys_stream_simulator): Estimated frame drift = " << estimated_drift << std::endl;
   std::cerr << "DEBUG (commsys_stream_simulator): Acc. drift error at eof = " << drift_error << std::endl;
   std::cerr << "DEBUG (commsys_stream_simulator): Frames decoded = " << frames_decoded << std::endl;
#endif
   // update counters
   frames_decoded++;
   }

// Description & Serialization

template <class S, class R>
std::string commsys_stream_simulator<S, R>::description() const
   {
   std::ostringstream sout;
   sout << "Stream-oriented ";
   sout << Base::description();
   switch (mode)
      {
      case mode_open:
         sout << ", open-ended stream";
         break;
      case mode_reset:
         sout << ", stream reset every " << N << " frames";
         break;
      case mode_terminated:
         sout << ", stream ends after " << N << " frames";
         break;
      default:
         break;
      }
   return sout.str();
   }

// object serialization - saving

template <class S, class R>
std::ostream& commsys_stream_simulator<S, R>::serialize(
      std::ostream& sout) const
   {
   // format version
   sout << "# Version" << std::endl;
   sout << 2 << std::endl;
   sout << "# Streaming mode (0=open, 1=reset, 2=terminated)" << std::endl;
   sout << mode << std::endl;
   switch (mode)
      {
      case mode_reset:
         sout << "# Number of frames to reset" << std::endl;
         sout << N << std::endl;
         break;
      case mode_terminated:
         sout << "# Length of stream in frames" << std::endl;
         sout << N << std::endl;
         break;
      default:
         break;
      }
   // continue writing underlying system
   Base::serialize(sout);
   return sout;
   }

// object serialization - loading

/*!
 * \version 0 Initial version (un-numbered)
 *
 * \version 1 Added version numbering; added frame count to stream reset
 *
 * \version 2 Changed format to include mode, and terminating streams
 */

template <class S, class R>
std::istream& commsys_stream_simulator<S, R>::serialize(std::istream& sin)
   {
   assertalways(sin.good());
   // get format version
   int version;
   sin >> libbase::eatcomments >> version;
   // handle old-format files
   if (sin.fail())
      {
      version = 0;
      sin.clear();
      }
   // reset (valid for version 0)
   mode = mode_open;
   N = 0;
   // handle version 1: read frame count and determine mode
   if (version == 1)
      {
      sin >> libbase::eatcomments >> N >> libbase::verify;
      if (N > 0)
         mode = mode_reset;
      }
   // handle later versions
   if (version >= 2)
      {
      int temp;
      // read streaming mode
      sin >> libbase::eatcomments >> temp >> libbase::verify;
      assertalways(temp >= 0 && temp < mode_undefined);
      mode = static_cast<mode_enum>(temp);
      // read mode-dependent parameters
      switch (mode)
         {
         case mode_reset:
         case mode_terminated:
            sin >> libbase::eatcomments >> N >> libbase::verify;
            assert(N > 0);
            break;
         default:
            break;
         }
      }
   // continue reading underlying system
   Base::serialize(sin);
   reset();
   assertalways(sin.good());
   return sin;
   }

} // end namespace

#include "gf.h"
#include "result_collector/commsys/errors_hamming.h"
#include "result_collector/commsys/errors_levenshtein.h"
#include "result_collector/commsys/prof_burst.h"
#include "result_collector/commsys/prof_pos.h"
#include "result_collector/commsys/prof_sym.h"
#include "result_collector/commsys/hist_symerr.h"

namespace libcomm {

// Explicit Realizations
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/for_each_product.hpp>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/stringize.hpp>

using libbase::serializer;

#define USING_GF(r, x, type) \
      using libbase::type;

BOOST_PP_SEQ_FOR_EACH(USING_GF, x, GF_TYPE_SEQ)

// *** General Communication System ***

#define SYMBOL_TYPE_SEQ \
   (sigspace)(bool) \
   GF_TYPE_SEQ
#define COLLECTOR_TYPE_SEQ \
   (errors_hamming) \
   (errors_levenshtein) \
   (prof_burst) \
   (prof_pos) \
   (prof_sym) \
   (hist_symerr) \
   (fidelity_pos)

/* Serialization string: commsys_stream_simulator<type,collector>
 * where:
 *      type = sigspace | bool | gf2 | gf4 ...
 *      collector = errors_hamming | errors_levenshtein | ...
 */
#define INSTANTIATE(r, args) \
      template class commsys_stream_simulator<BOOST_PP_SEQ_ENUM(args)>; \
      template <> \
      const serializer commsys_stream_simulator<BOOST_PP_SEQ_ENUM(args)>::shelper( \
            "experiment", \
            "commsys_stream_simulator<" BOOST_PP_STRINGIZE(BOOST_PP_SEQ_ELEM(0,args)) "," \
            BOOST_PP_STRINGIZE(BOOST_PP_SEQ_ELEM(1,args)) ">", \
            commsys_stream_simulator<BOOST_PP_SEQ_ENUM(args)>::create); \

BOOST_PP_SEQ_FOR_EACH_PRODUCT(INSTANTIATE,
      (SYMBOL_TYPE_SEQ)(COLLECTOR_TYPE_SEQ))

} // end namespace
