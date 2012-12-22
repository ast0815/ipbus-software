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

// /*
// @file
// @author Andrew W. Rose
// @date 2012
// */

#include "uhal/IPbusPacketInfo.hpp"

// Using the uhal namespace
namespace uhal
{
  logging();
  std::string toString ( const eIPbusProtocolVersion& aIPbusProtocolVersion )
  {
    switch ( aIPbusProtocolVersion )
    {
      case IPbus_1_2:
        return "IPbus version 1.2";
      case IPbus_1_3:
        return "IPbus version 1.3";
      case IPbus_1_4:
        return "IPbus version 1.4";
      case IPbus_2_0:
        return "IPbus version 2.0";
    }

    return "IPbus version unknown!";
  }

}

