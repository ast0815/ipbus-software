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

#ifndef _uhal_URIGrammar_hpp_
#define _uhal_URIGrammar_hpp_

#include <boost/fusion/adapted/std_pair.hpp>
//#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_grammar.hpp>

#include <string>
#include <vector>


namespace uhal
{
  //! boost::fusion requires us to typedef our template types, so typedef a container which can hold key/value pairs
  typedef std::vector< std::pair<std::string, std::string> > NameValuePairVectorType;

  //! Struct to store a URI when parsed by boost spirit
  struct URI
  {
    //! The "protocol" part of a URI of the form "protocol://host:port/patha/pathb/blah.ext?key1=val1&key2=val2&key3=val3"
    std::string mProtocol;
    //! The "host" part of a URI of the form "protocol://host:port/patha/pathb/blah.ext?key1=val1&key2=val2&key3=val3"
    std::string mHostname;
    //! The "port" part of a URI of the form "protocol://host:port/patha/pathb/blah.ext?key1=val1&key2=val2&key3=val3"
    std::string mPort;
    //! The "patha/pathb/blah" part of a URI of the form "protocol://host:port/patha/pathb/blah.ext?key1=val1&key2=val2&key3=val3"
    std::string mPath;
    //! The "ext" part of a URI of the form "protocol://host:port/patha/pathb/blah.ext?key1=val1&key2=val2&key3=val3"
    std::string mExtension;
    //! The "key1=val1&key2=val2&key3=val3" part of a URI of the form "protocol://host:port/patha/pathb/blah.ext?key1=val1&key2=val2&key3=val3"	stored as a vector of key/val pairs
    NameValuePairVectorType mArguments;
  };
}

std::ostream& operator<< ( std::ostream& aStream , const uhal::URI& aURI );


// Call to BOOST_FUSION_ADAPT_STRUCT must be at global scope
//! A boost::fusion adaptive struct used by the boost::qi parser
BOOST_FUSION_ADAPT_STRUCT (
  uhal::URI,
  ( std::string , mProtocol )
  ( std::string , mHostname )
  ( std::string , mPort )
  ( std::string , mPath )
  ( std::string , mExtension )
  ( uhal::NameValuePairVectorType, mArguments )
);


namespace grammars
{
  //! A struct wrapping a set of rules as a grammar that can parse a URI of the form "protocol://host:port/patha/pathb/blah.ext?key1=val1&key2=val2&key3=val3"
  struct URIGrammar : boost::spirit::qi::grammar<std::string::const_iterator, uhal::URI(), boost::spirit::ascii::space_type>
  {
    //! Default Constructor where we will define the boost::qi rules relating the members
    URIGrammar();
    //! Boost spirit parsing rule for parsing a URI
    boost::spirit::qi::rule< std::string::const_iterator,	uhal::URI(), 											boost::spirit::ascii::space_type > start;
    //! Boost spirit parsing rule for parsing a URI
    boost::spirit::qi::rule< std::string::const_iterator,	std::string(),											boost::spirit::ascii::space_type > protocol;
    //! Boost spirit parsing rule for parsing a URI
    boost::spirit::qi::rule< std::string::const_iterator,	std::string(),											boost::spirit::ascii::space_type > hostname;
    //! Boost spirit parsing rule for parsing a URI
    boost::spirit::qi::rule< std::string::const_iterator,	std::string(),											boost::spirit::ascii::space_type > port;
    //! Boost spirit parsing rule for parsing a URI
    boost::spirit::qi::rule< std::string::const_iterator,	std::string(),											boost::spirit::ascii::space_type > path;
    //! Boost spirit parsing rule for parsing a URI
    boost::spirit::qi::rule< std::string::const_iterator,	std::string(),											boost::spirit::ascii::space_type > extension;
    //! Boost spirit parsing rule for parsing a URI
    boost::spirit::qi::rule< std::string::const_iterator,	std::vector< std::pair<std::string, std::string> > (),	boost::spirit::ascii::space_type > data_pairs_vector; //NameValuePairVectorType
    //! Boost spirit parsing rule for parsing a URI
    boost::spirit::qi::rule< std::string::const_iterator,	std::pair<std::string, std::string>(),					boost::spirit::ascii::space_type > data_pairs;
    //! Boost spirit parsing rule for parsing a URI
    boost::spirit::qi::rule< std::string::const_iterator,	std::string(),											boost::spirit::ascii::space_type > data_pairs_1;
    //! Boost spirit parsing rule for parsing a URI
    boost::spirit::qi::rule< std::string::const_iterator,	std::string(),											boost::spirit::ascii::space_type > data_pairs_2;
  };
}

#endif
