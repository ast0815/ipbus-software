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

#include "uhal/Utilities.hpp"

#include "uhal/ProtocolUDP.hpp"
#include "uhal/ProtocolTCP.hpp"
#include "uhal/ProtocolIPbus.hpp"

#include "uhal/grammars/URLGrammar.hpp"

#include "uhal/ClientFactory.hpp"

namespace uhal
{

  ClientFactory& ClientFactory::getInstance()
  {
    logging();

    if ( mInstance == NULL )
    {
      mInstance = new ClientFactory();
      // // ---------------------------------------------------------------------
      // mInstance->add< uhal::IPBusUDPClient< 1 , 3 > > ( "ipbusudp" , "Direct access to hardware via UDP, using the default IPbus version which is currently IPbus 1.3" );
      // // mInstance->add< uhal::IPBusUDPClient< 1 , 2 > > ( "ipbusudp-1.2" );
      mInstance->add< UDP< IPbus< 1 , 3 > > > ( "ipbusudp-1.3" );
      // // mInstance->add< uhal::IPBusUDPClient< 1 , 4 > > ( "ipbusudp-1.4" );
      // mInstance->add< uhal::IPBusUDPClient< 2 , 0 > > ( "ipbusudp-2.0" );
      // // ---------------------------------------------------------------------
      // mInstance->add< uhal::IPBusTCPClient< 1 , 3 > > ( "ipbustcp" , "Direct access to hardware via TCP, using the default IPbus version which is currently IPbus 1.3" );
      mInstance->add< TCP< IPbus< 1 , 3 > > > ( "ipbustcp-1.3" );
      // // mInstance->add< uhal::IPBusTCPClient< 1 , 4 > > ( "ipbustcp-1.4" );
      // // mInstance->add< uhal::IPBusTCPClient< 2 , 0 > > ( "ipbustcp-2.0" );
      // // ---------------------------------------------------------------------
      // /*
      // mInstance->add< uhal::ControlHubClient<CHH_1 , 1 , 3> > ( "chtcp" , "Hardware access via the Control Hub, using Control Hub Protocol 1 (Multi target packets with 32-bit identifier) and the default IPbus version which is currently IPbus 1.3" );
      // mInstance->add< uhal::ControlHubClient<CHH_1 , 1 , 2> > ( "chtcp-1.2" );
      // mInstance->add< uhal::ControlHubClient<CHH_1 , 1 , 3> > ( "chtcp-1.3" );
      // // mInstance->add< uhal::ControlHubClient<CHH_1 , 1 , 4> > ( "chtcp-1.4" );
      // // mInstance->add< uhal::ControlHubClient<CHH_1 , 2 , 0> > ( "chtcp-2.0" );
      // // ---------------------------------------------------------------------
      // mInstance->add< uhal::ControlHubClient<CHH_2 , 1 , 3> > ( "chtcp2" , "Hardware access via the Control Hub, using Control Hub Protocol 2 (Multi target packets with 48-bit IP+port as identifier) and the default IPbus version which is currently IPbus 1.3" );
      // mInstance->add< uhal::ControlHubClient<CHH_2 , 1 , 3> > ( "chtcp2-1.3" );
      // // mInstance->add< uhal::ControlHubClient<CHH_2 , 1 , 4> > ( "chtcp2-1.4" );
      // // mInstance->add< uhal::ControlHubClient<CHH_2 , 2 , 0> > ( "chtcp2-2.0" );
      // */
      // // ---------------------------------------------------------------------
      // mInstance->add< uhal::ControlHubClient< 1 , 3 > > ( "chtcp" , "Hardware access via the Control Hub, using the default IPbus version which is currently IPbus 1.3" );
      // mInstance->add< uhal::ControlHubClient< 1 , 3 > > ( "chtcp-1.3" );
      // // mInstance->add< uhal::ControlHubClient<CHH_3 , 1 , 4> > ( "chtcp2-1.4" );
      // // mInstance->add< uhal::ControlHubClient<CHH_3 , 2 , 0> > ( "chtcp2-2.0" );
      // // ---------------------------------------------------------------------
      // // mInstance->add< uhal::DummyClient >( "dummy" );
      // // ---------------------------------------------------------------------
    }

    return *mInstance;
  }




  ClientFactory* ClientFactory::mInstance = NULL;

  ClientFactory::ClientFactory()
  {
    logging();
  }

  ClientFactory::~ClientFactory()
  {
    logging();
  }


  boost::shared_ptr<ClientInterface> ClientFactory::getClient ( const std::string& aId , const std::string& aUri )
  {
    logging();
    URI lUri;

    try
    {
      grammars::URIGrammar lGrammar;
      std::string::const_iterator lBegin ( aUri.begin() );
      std::string::const_iterator lEnd ( aUri.end() );
      boost::spirit::qi::phrase_parse ( lBegin , lEnd , lGrammar , boost::spirit::ascii::space , lUri );
    }
    catch ( const std::exception& aExc )
    {
      log ( Error() , "Failed to parse URI " , Quote ( aUri ) );
      throw aExc;
    }

    log ( Info() , "URI " , Quote ( aUri ) , " parsed as:\n" , lUri );
    std::hash_map< std::string , boost::shared_ptr<CreatorInterface> >::const_iterator lIt = mCreators.find ( lUri.mProtocol );

    if ( lIt == mCreators.end() )
    {
      std::stringstream lStr;

      for ( std::map< std::string , std::string >::const_iterator lIt = mProductDescriptions.begin() ; lIt != mProductDescriptions.end() ; ++lIt )
      {
        lStr << "\n > " << lIt->first << "\t: " << lIt->second;
      }

      log ( Error() , "Protocol " , Quote ( lUri.mProtocol ) , " does not exists in map of creators. Options are:" , lStr.str() );
      throw exception::ProtocolDoesNotExist();
    }

    return lIt->second->create ( aId , lUri );
  }


}

