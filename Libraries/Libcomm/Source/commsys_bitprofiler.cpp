/*!
   \file

   \par Version Control:
   - $Revision$
   - $Date$
   - $Author$
*/

#include "commsys_bitprofiler.h"

#include "fsm.h"
#include "itfunc.h"
#include "secant.h"
#include "timer.h"
#include <iostream>

namespace libcomm {

// constructor / destructor

commsys_bitprofiler::commsys_bitprofiler(libbase::randgen *src, codec *cdc, modulator *modem, puncture *punc, channel<sigspace> *chan) : \
   commsys(src, cdc, modem, punc, chan)
   {
   }

// commsys functions

void commsys_bitprofiler::cycleonce(libbase::vector<double>& result)
   {
   // Create source stream
   libbase::vector<int> source = createsource();
   // Full cycle from Encode through Demodulate
   transmitandreceive(source);
   // For every iteration
   const int skip = count()/iter;
   for(int i=0; i<iter; i++)
      {
      // Decode & count errors
      libbase::vector<int> decoded;
      cdc->decode(decoded);
      // Update the count for every bit in error
      for(int t=0; t<tau-m; t++)
         if(source(t) != decoded(t))
            result(skip*i + t)++;
      }
   }

}; // end namespace
