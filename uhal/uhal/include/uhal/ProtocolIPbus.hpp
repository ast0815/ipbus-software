/*
---------------------------------------------------------------------------

    This file is part of uHAL.

    uHAL is a hardware access library and programming framework
    originally developed for upgrades of the Level-1 trigger of the CMS
    experiment at CERN.

    uHAL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    uHAL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with uHAL.  If not, see <http://www.gnu.org/licenses/>.


      Andrew Rose, Imperial College, London
      email: awr01 <AT> imperial.ac.uk

      Marc Magrans de Abril, CERN
      email: marc.magrans.de.abril <AT> cern.ch

---------------------------------------------------------------------------
*/

/**
	@file
	@author Andrew W. Rose
	@date 2012
*/

#ifndef _uhal_ProtocolIPbus_hpp_
#define _uhal_ProtocolIPbus_hpp_

#include <deque>
#include "uhal/ProtocolIPbusCore.hpp"

#include "boost/date_time/posix_time/posix_time_duration.hpp"

#include "uhal/ClientInterface.hpp"

namespace uhal
{


  //! A class which provides the version-specific functionality for IPbus
  template< uint8_t IPbus_major , uint8_t IPbus_minor , uint32_t buffer_size = 350 >
  class IPbus;


  //! A class which provides the version-specific functionality for IPbus
  template< uint8_t IPbus_minor , uint32_t buffer_size >
  class IPbus < 1 , IPbus_minor, buffer_size > : public IPbusCore
  {

    public:
      /**
      	Default constructor
      	@param aId the uinique identifier that the client will be given.
      	@param aUri a struct containing the full URI of the target.
      */
      IPbus ( const std::string& aId, const URI& aUri );

      /**
      	Destructor
      */
      virtual ~IPbus();

      /**
      	Add a preamble to an IPbus buffer
      */
      void preamble( );

      /**
      	Finalize the buffer before it is transmitted
      */
      void predispatch( );


      /**
      Abstract interface of function to calculate the IPbus header for a particular protocol version
      @param aType the type of the IPbus transaction
      @param aWordCount the word count field of the IPbus header
      @param aTransactionId the TransactionId of the IPbus header
      @return an IPbus header
      */
      static uint32_t CalculateHeader ( const eIPbusTransactionType& aType , const uint32_t& aWordCount , const uint32_t& aTransactionId );

      /**
      Abstract interface of function to parse an IPbus header for a particular protocol version
      @param aHeader an IPbus header to be parsed
      @param aType return the type of the IPbus transaction
      @param aWordCount return the word count field of the IPbus header
      @param aTransactionId return the TransactionId of the IPbus header
      @param aInfoCode return the response status of the IPbus header
      @return whether extraction succeeded
      */
      static bool ExtractHeader ( const uint32_t& aHeader , eIPbusTransactionType& aType , uint32_t& aWordCount , uint32_t& aTransactionId , uint8_t& aInfoCode );


    private:


      /**
      Abstract interface of function to calculate the IPbus header for a particular protocol version
      @param aType the type of the IPbus transaction
      @param aWordCount the word count field of the IPbus header
      @param aTransactionId the TransactionId of the IPbus header
      @return an IPbus header
      */
      uint32_t implementCalculateHeader ( const eIPbusTransactionType& aType , const uint32_t& aWordCount , const uint32_t& aTransactionId );

      /**
      Abstract interface of function to parse an IPbus header for a particular protocol version
      @param aHeader an IPbus header to be parsed
      @param aType return the type of the IPbus transaction
      @param aWordCount return the word count field of the IPbus header
      @param aTransactionId return the TransactionId of the IPbus header
      @param aInfoCode return the response status of the IPbus header
      @return whether extraction succeeded
      */
      bool implementExtractHeader ( const uint32_t& aHeader , eIPbusTransactionType& aType , uint32_t& aWordCount , uint32_t& aTransactionId , uint8_t& aInfoCode );

      // std::vector< uint32_t > mSendPadding;
      // std::vector< uint32_t > mReplyPadding;

  };



  //! A class which provides the version-specific functionality for IPbus
  template< uint8_t IPbus_minor , uint32_t buffer_size >
  class IPbus < 2 , IPbus_minor, buffer_size > : public IPbusCore
  {

    public:
      /**
      	Default constructor
      	@param aId the uinique identifier that the client will be given.
      	@param aUri a struct containing the full URI of the target.
      */
      IPbus ( const std::string& aId, const URI& aUri );

      /**
      	Destructor
      */
      virtual ~IPbus();

      /**
      	Add a preamble to an IPbus buffer
      */
      void preamble( );

      /**
      Abstract interface of function to calculate the IPbus header for a particular protocol version
      @param aType the type of the IPbus transaction
      @param aWordCount the word count field of the IPbus header
      @param aTransactionId the TransactionId of the IPbus header
      @return an IPbus header
      */
      static uint32_t CalculateHeader ( const eIPbusTransactionType& aType , const uint32_t& aWordCount , const uint32_t& aTransactionId );

      /**
      Abstract interface of function to parse an IPbus header for a particular protocol version
      @param aHeader an IPbus header to be parsed
      @param aType return the type of the IPbus transaction
      @param aWordCount return the word count field of the IPbus header
      @param aTransactionId return the TransactionId of the IPbus header
      @param aInfoCode return the response status of the IPbus header
      @return whether extraction succeeded
      */
      static bool ExtractHeader ( const uint32_t& aHeader , eIPbusTransactionType& aType , uint32_t& aWordCount , uint32_t& aTransactionId , uint8_t& aInfoCode );


    private:

      /**
      	Function which the transport protocol calls when the IPbus reply is received to check that the headers are as expected
      	@param aSendBufferStart a pointer to the start of the first word of IPbus data which was sent (i.e. with no preamble)
      	@param aSendBufferEnd a pointer to the end of the last word of IPbus data which was sent
      	@param aReplyStartIt an iterator to the start of the list of memory locations in to which the reply was written
      	@param aReplyEndIt an iterator to the end (one past last valid entry) of the list of memory locations in to which the reply was written
      	@return whether the returned IPbus packet is valid
      */
      virtual bool validate ( uint8_t* aSendBufferStart ,
                              uint8_t* aSendBufferEnd ,
                              std::deque< std::pair< uint8_t* , uint32_t > >::iterator aReplyStartIt ,
                              std::deque< std::pair< uint8_t* , uint32_t > >::iterator aReplyEndIt );

      /**
      Abstract interface of function to calculate the IPbus header for a particular protocol version
      @param aType the type of the IPbus transaction
      @param aWordCount the word count field of the IPbus header
      @param aTransactionId the TransactionId of the IPbus header
      @return an IPbus header
      */
      uint32_t implementCalculateHeader ( const eIPbusTransactionType& aType , const uint32_t& aWordCount , const uint32_t& aTransactionId );

      /**
      Abstract interface of function to parse an IPbus header for a particular protocol version
      @param aHeader an IPbus header to be parsed
      @param aType return the type of the IPbus transaction
      @param aWordCount return the word count field of the IPbus header
      @param aTransactionId return the TransactionId of the IPbus header
      @param aInfoCode return the response status of the IPbus header
      @return whether extraction succeeded
      */
      bool implementExtractHeader ( const uint32_t& aHeader , eIPbusTransactionType& aType , uint32_t& aWordCount , uint32_t& aTransactionId , uint8_t& aInfoCode );

      //! The transaction counter which will be incremented in the sent IPbus headers
      uint16_t mPacketCounter;

      std::deque< uint32_t > mSendPacketHeader;
      std::deque< uint32_t > mReceivePacketHeader;

  };


}

#include "uhal/TemplateDefinitions/ProtocolIPbus.hxx"

#endif
