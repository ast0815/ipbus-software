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
	@date 2013
*/

#ifndef IPbusInspector_hpp
#define IPbusInspector_hpp

//#include "uhal/uhal.hpp"
#include "uhal/log/log.hpp"
#include "uhal/ProtocolIPbus.hpp"

// Using the uhal namespace
namespace uhal
{

  /*  namespace exception
    {
      //! Exception class to handle the case where we are unable to parse a badly formed IPbus header.
      ExceptionClass ( UnableToParseHeader , "Exception class to handle the case where we are unable to parse a badly formed IPbus header." );
      ExceptionClass ( IllegalPacketHeader , "Exception class to handle the case where an illegal packet header is received." );
    }*/


  template< uint8_t IPbus_major , uint8_t IPbus_minor >
  class HostToTargetInspector
  {
    public:
      HostToTargetInspector( ) :
        mHeader ( 0 ),
        mWordCounter ( 0 ),
        mTransactionId ( 0 ),
        mResponseGood ( 0 ),
        mPacketHeader ( 0 ),
        mPacketCounter ( 0 ),
        mPacketType ( 0 )
      {}

      virtual ~HostToTargetInspector( ) {}

    protected:
      uint32_t mHeader;
      eIPbusTransactionType mType;
      uint32_t mWordCounter;
      uint32_t mTransactionId;
      uint8_t mResponseGood;

      uint32_t mPacketHeader;
      uint32_t mPacketCounter;
      uint32_t mPacketType;

    public:

      bool analyze ( std::vector<uint32_t>::const_iterator& aIt , const std::vector<uint32_t>::const_iterator& aEnd )
      {
        uint32_t lAddress , lAddend , lAndTerm , lOrTerm ;
        std::vector<uint32_t>::const_iterator lPayloadBegin, lPayloadEnd;

        if ( IPbus_major != 1 )
        {
          mPacketHeader = *aIt++;
          mPacketCounter = ( mPacketHeader>>8 ) &0xFFFF ;
          mPacketType = mPacketHeader&0x0F ;
        }

        switch ( mPacketType )
        {
          case 0:

            if ( IPbus_major != 1 )
            {
              control_packet_header ();
            }

            do
            {
              mHeader = *aIt++;

              if ( ! IPbus< IPbus_major , IPbus_minor >::ExtractHeader (
                     mHeader ,
                     mType ,
                     mWordCounter ,
                     mTransactionId ,
                     mResponseGood )
                 )
              {
                log ( Error() , "Unable to parse send header " , Integer ( mHeader, IntFmt<hex,fixed>() ) );
                return false;
              }

              switch ( mType )
              {
                case B_O_T:
                  bot();
                  break;
                case NI_READ:
                  lAddress = *aIt++;
                  ni_read ( lAddress );
                  break;
                case READ:
                  lAddress = *aIt++;
                  read ( lAddress );
                  break;
                case NI_WRITE:
                  lAddress = *aIt++;
                  lPayloadBegin = ( aIt++ );
                  lPayloadEnd = ( aIt+= ( mWordCounter-1 ) );
                  ni_write ( lAddress , lPayloadBegin , lPayloadEnd );
                  break;
                case WRITE:
                  lAddress = *aIt++;
                  lPayloadBegin = ( aIt++ );
                  lPayloadEnd = ( aIt+= ( mWordCounter-1 ) );
                  write ( lAddress , lPayloadBegin , lPayloadEnd );
                  break;
                case RMW_SUM:
                  lAddress = *aIt++;
                  lAddend = *aIt++;
                  rmw_sum ( lAddress , lAddend );
                  break;
                case RMW_BITS:
                  lAddress = *aIt++;
                  lAndTerm = *aIt++;
                  lOrTerm = *aIt++;
                  rmw_bits ( lAddress , lAndTerm , lOrTerm );
                  break;
                default:
                  unknown_type();
                  return false;
              }
            }
            while ( aIt!=aEnd );

            break;
          case 1:
            aIt=aEnd;
            status_packet_header();
            break;
          case 2:
            aIt=aEnd;
            resend_packet_header();
            break;
          default:
            unknown_packet_header( );
            return false;
        }

        return true;
      }

    protected:
      virtual void bot()
      {
        log ( Notice() , Integer ( mHeader, IntFmt<hex,fixed>() ) , " | BOT, transaction ID " , Integer ( mTransactionId ) );
      }

      virtual void ni_read ( const uint32_t& aAddress )
      {
        log ( Notice() , Integer ( mHeader, IntFmt<hex,fixed>() ) , " | Non-incrementing read, size " , Integer ( mWordCounter ) , ", transaction ID " , Integer ( mTransactionId ) );
        log ( Notice() , Integer ( aAddress, IntFmt<hex,fixed>() ) , " |  > Address" );
      }

      virtual void read ( const uint32_t& aAddress )
      {
        log ( Notice() , Integer ( mHeader, IntFmt<hex,fixed>() ) , " | Incrementing read, size " , Integer ( mWordCounter ) , ", transaction ID " , Integer ( mTransactionId ) );
        log ( Notice() , Integer ( aAddress, IntFmt<hex,fixed>() ) , " |  > Address" );
      }

      virtual void ni_write ( const uint32_t& aAddress , std::vector<uint32_t>::const_iterator& aIt , const std::vector<uint32_t>::const_iterator& aEnd )
      {
        log ( Notice() , Integer ( mHeader, IntFmt<hex,fixed>() ) , " | Non-incrementing write, size " , Integer ( mWordCounter ) , ", transaction ID " , Integer ( mTransactionId ) );
        log ( Notice() , Integer ( aAddress, IntFmt<hex,fixed>() ) , " |  > Address" );

        while ( aIt != aEnd )
        {
          log ( Notice() , Integer ( *aIt++, IntFmt<hex,fixed>() ) , " |  > Data" );
        }
      }

      virtual void write ( const uint32_t& aAddress , std::vector<uint32_t>::const_iterator& aIt , const std::vector<uint32_t>::const_iterator& aEnd )
      {
        log ( Notice() , Integer ( mHeader, IntFmt<hex,fixed>() ) , " | Incrementing write, size " , Integer ( mWordCounter ) , ", transaction ID " , Integer ( mTransactionId ) );
        log ( Notice() , Integer ( aAddress, IntFmt<hex,fixed>() ) , " |  > Address" );

        while ( aIt != aEnd )
        {
          log ( Notice() , Integer ( *aIt++, IntFmt<hex,fixed>() ) , " |  > Data" );
        }
      }

      virtual void rmw_sum ( const uint32_t& aAddress , const uint32_t& aAddend )
      {
        log ( Notice() , Integer ( mHeader, IntFmt<hex,fixed>() ) , " | Read-modify-write sum, transaction ID " , Integer ( mTransactionId ) );
        log ( Notice() , Integer ( aAddress, IntFmt<hex,fixed>() ) , " |  > Address" );
        log ( Notice() , Integer ( aAddend, IntFmt<hex,fixed>() ) , " |  > Addend" );
      }

      virtual void rmw_bits ( const uint32_t& aAddress , const uint32_t& aAndTerm , const uint32_t& aOrTerm )
      {
        log ( Notice() , Integer ( mHeader, IntFmt<hex,fixed>() ) , " | Read-modify-write bits, transaction ID " , Integer ( mTransactionId ) );
        log ( Notice() , Integer ( aAddress, IntFmt<hex,fixed>() ) , " |  > Address" );
        log ( Notice() , Integer ( aAndTerm, IntFmt<hex,fixed>() ) , " |  > And-term" );
        log ( Notice() , Integer ( aOrTerm, IntFmt<hex,fixed>() ) , " |  > Or-term" );
      }

      virtual void unknown_type()
      {
        log ( Error() , Integer ( mHeader, IntFmt<hex,fixed>() ) , " | Unknown Transaction Header" );
      }


      virtual void control_packet_header ()
      {
        log ( Notice() , Integer ( mPacketHeader , IntFmt<hex,fixed>() ) , " | Control (Instruction) Packet Header , transaction ID " , Integer ( mTransactionId ) );
      }

      virtual void status_packet_header()
      {
        log ( Notice() , Integer ( mPacketHeader , IntFmt<hex,fixed>() ) , " | Status Packet Header" );
      }

      virtual void resend_packet_header()
      {
        log ( Notice() , Integer ( mPacketHeader , IntFmt<hex,fixed>() ) , " | Resend Request Packet Header" );
      }

      virtual void unknown_packet_header()
      {
        log ( Error() , Integer ( mPacketHeader, IntFmt<hex,fixed>() ) , " | Unknown Packet Header" );
      }

  };






  template< uint8_t IPbus_major , uint8_t IPbus_minor >
  class TargetToHostInspector
  {
    public:
      TargetToHostInspector( ) :
        mHeader ( 0 ),
        mWordCounter ( 0 ),
        mTransactionId ( 0 ),
        mResponseGood ( 0 ),
        mPacketHeader ( 0 ),
        mPacketCounter ( 0 ),
        mPacketType ( 0 )
      {}
      virtual ~TargetToHostInspector( ) {}

    protected:
      uint32_t mHeader;
      eIPbusTransactionType mType;
      uint32_t mWordCounter;
      uint32_t mTransactionId;
      uint8_t mResponseGood;

      uint32_t mPacketHeader;
      uint32_t mPacketCounter;
      uint32_t mPacketType;


    public:

      bool analyze ( std::vector<uint32_t>::const_iterator& aIt , const std::vector<uint32_t>::const_iterator& aEnd )
      {
        uint32_t lNewValue;
        std::vector<uint32_t>::const_iterator lPayloadBegin, lPayloadEnd;

        if ( IPbus_major != 1 )
        {
          mPacketHeader = *aIt++;
          mPacketCounter = ( mPacketHeader>>8 ) &0xFFFF ;
          mPacketType = mPacketHeader&0x0F ;
        }

        switch ( mPacketType )
        {
          case 0:

            if ( IPbus_major != 1 )
            {
              control_packet_header ( );
            }

            do
            {
              mHeader = *aIt++;

              if ( ! IPbus< IPbus_major , IPbus_minor >::ExtractHeader (
                     mHeader ,
                     mType ,
                     mWordCounter ,
                     mTransactionId ,
                     mResponseGood )
                 )
              {
                log ( Error() , "Unable to parse reply header " , Integer ( mHeader, IntFmt<hex,fixed>() ) );
                return false;
              }

              switch ( mType )
              {
                case B_O_T:
                  bot();
                  break;
                case NI_READ:
                  lPayloadBegin = ( aIt++ );
                  lPayloadEnd = ( aIt+= ( mWordCounter-1 ) );
                  ni_read ( lPayloadBegin , lPayloadEnd );
                  break;
                case READ:
                  lPayloadBegin = ( aIt++ );
                  lPayloadEnd = ( aIt+= ( mWordCounter-1 ) );
                  read ( lPayloadBegin , lPayloadEnd );
                  break;
                case NI_WRITE:
                  ni_write ();
                  break;
                case WRITE:
                  write ();
                  break;
                case RMW_SUM:
                  lNewValue = *aIt++;
                  rmw_sum ( lNewValue );
                  break;
                case RMW_BITS:
                  lNewValue = *aIt++;
                  rmw_bits ( lNewValue );
                  break;
                default:
                  unknown_type();
                  return false;
              }
            }
            while ( aIt!=aEnd );

            break;
          case 1:
            aIt=aEnd;
            status_packet_header( );
            break;
          default:
            unknown_packet_header( );
            return false;
        }

        return true;
      }

    protected:
      virtual void bot()
      {
        log ( Notice() , Integer ( mHeader, IntFmt<hex,fixed>() ) , " | BOT, transaction ID " , Integer ( mTransactionId ) );
      }

      virtual void ni_read ( std::vector<uint32_t>::const_iterator& aIt , const std::vector<uint32_t>::const_iterator& aEnd )
      {
        log ( Notice() , Integer ( mHeader, IntFmt<hex,fixed>() ) , " | Non-incrementing read, size " , Integer ( mWordCounter ) , ", transaction ID " , Integer ( mTransactionId ) );

        while ( aIt != aEnd )
        {
          log ( Notice() , Integer ( *aIt++, IntFmt<hex,fixed>() ) , " |  > Data" );
        }
      }

      virtual void read ( std::vector<uint32_t>::const_iterator& aIt , const std::vector<uint32_t>::const_iterator& aEnd )
      {
        log ( Notice() , Integer ( mHeader, IntFmt<hex,fixed>() ) , " | Incrementing read, size " , Integer ( mWordCounter ) , ", transaction ID " , Integer ( mTransactionId ) );

        while ( aIt != aEnd )
        {
          log ( Notice() , Integer ( *aIt++, IntFmt<hex,fixed>() ) , " |  > Data" );
        }
      }

      virtual void ni_write ( )
      {
        log ( Notice() , Integer ( mHeader, IntFmt<hex,fixed>() ) , " | Non-incrementing write, size " , Integer ( mWordCounter ) , ", transaction ID " , Integer ( mTransactionId ) );
      }

      virtual void write ( )
      {
        log ( Notice() , Integer ( mHeader, IntFmt<hex,fixed>() ) , " | Incrementing write, size " , Integer ( mWordCounter ) , ", transaction ID " , Integer ( mTransactionId ) );
      }

      virtual void rmw_sum ( const uint32_t& aNewValue )
      {
        log ( Notice() , Integer ( mHeader, IntFmt<hex,fixed>() ) , " | Read-modify-write sum, transaction ID " , Integer ( mTransactionId ) );
        log ( Notice() , Integer ( aNewValue, IntFmt<hex,fixed>() ) , " |  > Data" );
      }

      virtual void rmw_bits ( const uint32_t& aNewValue )
      {
        log ( Notice() , Integer ( mHeader, IntFmt<hex,fixed>() ) , " | Read-modify-write bits, transaction ID " , Integer ( mTransactionId ) );
        log ( Notice() , Integer ( aNewValue, IntFmt<hex,fixed>() ) , " |  > Data" );
      }

      virtual void unknown_type()
      {
        log ( Error() , Integer ( mHeader, IntFmt<hex,fixed>() ) , " | Unknown Transaction Header" );
      }

      virtual void control_packet_header ()
      {
        log ( Notice() , Integer ( mPacketHeader , IntFmt<hex,fixed>() ) , " | Control (Instruction) Packet Header , transaction ID " , Integer ( mTransactionId ) );
      }

      virtual void status_packet_header()
      {
        log ( Notice() , Integer ( mPacketHeader , IntFmt<hex,fixed>() ) , " | Status Packet Header" );
      }

      virtual void unknown_packet_header()
      {
        log ( Error() , Integer ( mPacketHeader, IntFmt<hex,fixed>() ) , " | Unknown Packet Header" );
      }

  };

}
#endif
