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

#include "uhal/ClientImplementation.hpp"

#include "uhal/NodeTreeBuilder.hpp"
#include "uhal/Utilities.hpp"
#include "uhal/log/log.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/spirit/include/qi.hpp>

namespace uhal
{

  const char* NodeTreeBuilder::mIdAttribute = "id";
  const char* NodeTreeBuilder::mAddressAttribute = "address";
  const char* NodeTreeBuilder::mTagsAttribute = "tags";
  const char* NodeTreeBuilder::mDescriptionAttribute = "description";
  const char* NodeTreeBuilder::mPermissionsAttribute = "permission";
  const char* NodeTreeBuilder::mMaskAttribute = "mask";
  const char* NodeTreeBuilder::mModeAttribute = "mode";
  const char* NodeTreeBuilder::mSizeAttribute = "size";
  const char* NodeTreeBuilder::mClassAttribute = "class";
  const char* NodeTreeBuilder::mModuleAttribute = "module";



  NodeTreeBuilder* NodeTreeBuilder::mInstance = NULL;


  NodeTreeBuilder::NodeTreeBuilder ()
  {
    logging();
    //------------------------------------------------------------------------------------------------------------------------
    Rule<Node*> lPlainNode;
    lPlainNode.forbid ( NodeTreeBuilder::mClassAttribute )
    .forbid ( NodeTreeBuilder::mModuleAttribute )
    .forbid ( NodeTreeBuilder::mMaskAttribute )
    .optional ( NodeTreeBuilder::mIdAttribute )
    .optional ( NodeTreeBuilder::mAddressAttribute )
    .optional ( NodeTreeBuilder::mPermissionsAttribute )
    .optional ( NodeTreeBuilder::mModeAttribute )
    .optional ( NodeTreeBuilder::mSizeAttribute )
    .optional ( NodeTreeBuilder::mTagsAttribute )
    .optional ( NodeTreeBuilder::mDescriptionAttribute );
    //------------------------------------------------------------------------------------------------------------------------
    Rule<Node*> lClass;
    lClass.require ( NodeTreeBuilder::mClassAttribute )
    .forbid ( NodeTreeBuilder::mMaskAttribute )
    .forbid ( NodeTreeBuilder::mModuleAttribute )
    .optional ( NodeTreeBuilder::mIdAttribute )
    .optional ( NodeTreeBuilder::mAddressAttribute )
    .optional ( NodeTreeBuilder::mModeAttribute )
    .optional ( NodeTreeBuilder::mSizeAttribute )
    .optional ( NodeTreeBuilder::mPermissionsAttribute )
    .optional ( NodeTreeBuilder::mTagsAttribute )
    .optional ( NodeTreeBuilder::mDescriptionAttribute );
    //------------------------------------------------------------------------------------------------------------------------
    Rule<Node*> lBitMask;
    lBitMask.require ( NodeTreeBuilder::mMaskAttribute )
    .forbid ( NodeTreeBuilder::mClassAttribute )
    .forbid ( NodeTreeBuilder::mModuleAttribute )
    .optional ( NodeTreeBuilder::mAddressAttribute )		// .forbid ( NodeTreeBuilder::mAddressAttribute ) //see https://svnweb.cern.ch/trac/cactus/ticket/92
    .forbid ( NodeTreeBuilder::mModeAttribute )
    .forbid ( NodeTreeBuilder::mSizeAttribute )
    .optional ( NodeTreeBuilder::mPermissionsAttribute )
    .optional ( NodeTreeBuilder::mTagsAttribute )
    .optional ( NodeTreeBuilder::mDescriptionAttribute );
    //------------------------------------------------------------------------------------------------------------------------
    mTopLevelNodeParser.addRule ( lPlainNode , boost::bind ( &NodeTreeBuilder::plainNodeCreator , this , false , _1 ) );
    mTopLevelNodeParser.addRule ( lClass , boost::bind ( &NodeTreeBuilder::classNodeCreator , this , false , _1 ) );
    mTopLevelNodeParser.addRule ( lBitMask , boost::bind ( &NodeTreeBuilder::bitmaskNodeCreator , this , false , _1 ) );
    //------------------------------------------------------------------------------------------------------------------------
    lPlainNode.require ( NodeTreeBuilder::mIdAttribute );
    lClass.require ( NodeTreeBuilder::mIdAttribute );
    lBitMask.require ( NodeTreeBuilder::mIdAttribute );
    //------------------------------------------------------------------------------------------------------------------------
    Rule<Node*> lModule;
    lModule.require ( NodeTreeBuilder::mIdAttribute )
    .require ( NodeTreeBuilder::mModuleAttribute )
    .forbid ( NodeTreeBuilder::mMaskAttribute )
    .forbid ( NodeTreeBuilder::mClassAttribute )
    .forbid ( NodeTreeBuilder::mModeAttribute )
    .forbid ( NodeTreeBuilder::mSizeAttribute )
    .forbid ( NodeTreeBuilder::mPermissionsAttribute )
    .optional ( NodeTreeBuilder::mAddressAttribute )
    .optional ( NodeTreeBuilder::mTagsAttribute )
    .optional ( NodeTreeBuilder::mDescriptionAttribute );
    //------------------------------------------------------------------------------------------------------------------------
    mNodeParser.addRule ( lPlainNode , boost::bind ( &NodeTreeBuilder::plainNodeCreator , this , true , _1 ) );
    mNodeParser.addRule ( lClass , boost::bind ( &NodeTreeBuilder::classNodeCreator , this , true , _1 ) );
    mNodeParser.addRule ( lBitMask , boost::bind ( &NodeTreeBuilder::bitmaskNodeCreator , this , true , _1 ) );
    mNodeParser.addRule ( lModule , boost::bind ( &NodeTreeBuilder::moduleNodeCreator , this , _1 ) );
    //------------------------------------------------------------------------------------------------------------------------
  }

  NodeTreeBuilder::~NodeTreeBuilder ()
  {
    logging();
  }



  NodeTreeBuilder& NodeTreeBuilder::getInstance()
  {
    logging();

    if ( mInstance == NULL )
    {
      mInstance = new NodeTreeBuilder();
    }

    return *mInstance;
  }

  Node* NodeTreeBuilder::getNodeTree ( const std::string& aFilenameExpr , const boost::filesystem::path& aPath )
  {
    logging();
    std::vector< std::pair<std::string, std::string> >  lAddressFiles;
    uhal::utilities::ParseSemicolonDelimitedUriList<true> ( aFilenameExpr , lAddressFiles );

    if ( lAddressFiles.size() != 1 )
    {
      log ( Error() , "Exactly one address table file must be specified. The expression " , Quote ( aFilenameExpr ) , " contains " , Integer ( lAddressFiles.size() ) , " valid file expressions." );
      throw IncorrectAddressTableFileCount();
    }

    std::vector< const Node* > lNodes;

    if ( !uhal::utilities::OpenFile ( lAddressFiles[0].first , lAddressFiles[0].second , aPath.parent_path() , boost::bind ( &NodeTreeBuilder::CallBack, boost::ref ( *this ) , _1 , _2 , _3 , boost::ref ( lNodes ) ) ) )
    {
      log ( Error() , "Failed to open address table file " , Quote ( lAddressFiles[0].second ) );
      throw FailedToOpenAddressTableFile();
    }

    if ( lNodes.size() != 1 )
    {
      log ( Error() , "Exactly one address table file must be specified. The expression " , Quote ( lAddressFiles[0].second ) , " refers to " , Integer ( lNodes.size() ) , " valid files." );
      throw IncorrectAddressTableFileCount();
    }

    Node* lNode ( lNodes[0]->clone() );
    return lNode;
  }


  void NodeTreeBuilder::CallBack ( const std::string& aProtocol , const boost::filesystem::path& aPath , std::vector<uint8_t>& aFile , std::vector< const Node* >& aNodes )
  {
    logging();
    std::string lName ( aProtocol + ( aPath.string() ) );
    std::hash_map< std::string , const Node* >::iterator lNodeIt = mNodes.find ( lName );

    if ( lNodeIt != mNodes.end() )
    {
      aNodes.push_back ( lNodeIt->second );
      return;
    }

    std::string lExtension ( aPath.extension().string().substr ( 0,4 ) );
    boost::to_lower ( lExtension ); //just in case someone decides to use capitals in their file extensions.

    if ( lExtension == ".xml" )
    {
      log ( Info() , "XML file" );
      pugi::xml_document lXmlDocument;
      pugi::xml_parse_result lLoadResult = lXmlDocument.load_buffer_inplace ( & ( aFile[0] ) , aFile.size() );

      if ( !lLoadResult )
      {
        uhal::utilities::PugiXMLParseResultPrettifier ( lLoadResult , aPath , aFile );
        return;
      }

      pugi::xml_node lXmlNode = lXmlDocument.child ( "node" );

      if ( !lXmlNode )
      {
        log ( Error() , "No XML node called ", Quote ( "node" ) , " in file " , aPath.c_str() );
        return;
      }

      mFileCallStack.push_back ( aPath );
      Node* lNode ( mTopLevelNodeParser ( lXmlNode ) );
      mFileCallStack.pop_back( );
      calculateHierarchicalAddresses ( lNode , 0x00000000 );
      checkForAddressCollisions ( lNode );  // Needs further investigation - disabled for now as it causes exceptions with valid tables.
      mNodes.insert ( std::make_pair ( lName , lNode ) );
      aNodes.push_back ( lNode );
      return;
    }
    else if ( lExtension == ".txt" )
    {
      log ( Info() , "TXT file" );
      log ( Error() , "Parser problems mean that this method has been disabled." );
      log ( Error() , "At " , ThisLocation() );
      return;
      /*
      uhal::OldHalEntryGrammar lGrammar;
      uhal::OldHalSkipParser lParser;
      std::vector< utilities::OldHalEntryType > lResponse;

      std::vector<uint8_t>::iterator lBegin( aFile.begin() );
      std::vector<uint8_t>::iterator lEnd( aFile.end() );

      boost::spirit::qi::phrase_parse( lBegin , lEnd , lGrammar , lParser , lResponse );

      for( std::vector< utilities::OldHalEntryType >::iterator lIt = lResponse.begin() ; lIt != lResponse.end() ; ++lIt ){
      	//log ( Info() , "---------------------------------------------------\n" , *lIt );
      }

      //log ( Info() , "Remaining:" );
      for( ; lBegin != lEnd ; ++lBegin ){
      	//log ( Info() , *lBegin;
      }
      std::cout );
      */
    }
    else
    {
      log ( Error() , "Extension " , Quote ( lExtension ) , " not known." );
      return;
    }
  }




  Node* NodeTreeBuilder::plainNodeCreator ( const bool& aRequireId , const pugi::xml_node& aXmlNode )
  {
    logging();
    Node* lNode ( new Node() );
    setUid ( aRequireId , aXmlNode , lNode );
    setAddr ( aXmlNode , lNode );
    setTags ( aXmlNode , lNode );
    setDescription ( aXmlNode , lNode );
    setPermissions ( aXmlNode , lNode );
    //setMask( aXmlNode , lNode );
    setModeAndSize ( aXmlNode , lNode );
    addChildren ( aXmlNode , lNode );
    log ( Debug() , lNode->mUid , " built by " , __PRETTY_FUNCTION__ );
    return lNode;
  }


  Node* NodeTreeBuilder::classNodeCreator ( const bool& aRequireId , const pugi::xml_node& aXmlNode )
  {
    logging();
    std::string lClassStr;
    //get attribute from xml file as string
    uhal::utilities::GetXMLattribute<false> ( aXmlNode , NodeTreeBuilder::mClassAttribute , lClassStr );
    //parse the string into a NodeTreeClassAttribute object
    std::string::const_iterator lBegin ( lClassStr.begin() );
    std::string::const_iterator lEnd ( lClassStr.end() );
    NodeTreeClassAttribute lClass;
    boost::spirit::qi::phrase_parse ( lBegin , lEnd , mNodeTreeClassAttributeGrammar , boost::spirit::ascii::space , lClass );
    //create an object of the class type returned by the parsed string
    std::hash_map< std::string , boost::shared_ptr<CreatorInterface> >::const_iterator lIt = mCreators.find ( lClass.mClass );

    if ( lIt == mCreators.end() )
    {
      log ( Error() , "Class " , Quote ( lClass.mClass ) , " is unknown to the NodeTreeBuilder class factory. Known types are:" );

      for ( std::hash_map< std::string , boost::shared_ptr<CreatorInterface> >::const_iterator lIt = mCreators.begin() ; lIt != mCreators.end() ; ++lIt )
      {
        log ( Error() , " > " , lIt->first );
      }

      throw LabelUnknownToClassFactory();
    }

    Node* lNode ( lIt->second->create ( lClass.mArguments ) );
    setUid ( aRequireId , aXmlNode , lNode );
    setAddr ( aXmlNode , lNode );
    setTags ( aXmlNode , lNode );
    setDescription ( aXmlNode , lNode );
    setPermissions ( aXmlNode , lNode );
    //setMask( aXmlNode , lNode );
    setModeAndSize ( aXmlNode , lNode );
    addChildren ( aXmlNode , lNode );
    log ( Debug() , lNode->mUid , " built by " , __PRETTY_FUNCTION__ );
    return lNode;
  }


  Node* NodeTreeBuilder::moduleNodeCreator ( const pugi::xml_node& aXmlNode )
  {
    logging();
    std::string lModule;
    uhal::utilities::GetXMLattribute<false> ( aXmlNode , NodeTreeBuilder::mModuleAttribute , lModule );
    Node* lNode ( getNodeTree ( lModule , mFileCallStack.back( ) ) );
    setUid ( true , aXmlNode , lNode );
    setAddr ( aXmlNode , lNode );
    setTags ( aXmlNode , lNode );
    setDescription ( aXmlNode , lNode );
    //setPermissions( aXmlNode , lNode );
    //setMask( aXmlNode , lNode );
    //setModeAndSize( aXmlNode , lNode );
    //addChildren( aXmlNode , lNode );
    log ( Debug() , lNode->mUid , " built by " , __PRETTY_FUNCTION__ );
    return lNode;
  }


  Node* NodeTreeBuilder::bitmaskNodeCreator ( const bool& aRequireId , const pugi::xml_node& aXmlNode )
  {
    logging();

    if ( aXmlNode.child ( "node" ) )
    {
      log ( Error() , "Bit-masked nodes are not allowed to have child nodes" );
      throw MaskedNodeCannotHaveChild();
    }

    Node* lNode ( new Node() );
    setUid ( aRequireId , aXmlNode , lNode );
    setAddr ( aXmlNode , lNode ); //was commented out, see https://svnweb.cern.ch/trac/cactus/ticket/92
    setTags ( aXmlNode , lNode );
    setDescription ( aXmlNode , lNode );
    setPermissions ( aXmlNode , lNode );
    setMask ( aXmlNode , lNode );
    //setModeAndSize( aXmlNode , lNode );
    //addChildren( aXmlNode , lNode );
    log ( Debug() , lNode->mUid , " built by " , __PRETTY_FUNCTION__ );
    return lNode;
  }




  void NodeTreeBuilder::setUid ( const bool& aRequireId , const pugi::xml_node& aXmlNode , Node* aNode )
  {
    logging();

    if ( aRequireId )
    {
      if ( ! uhal::utilities::GetXMLattribute<true> ( aXmlNode , NodeTreeBuilder::mIdAttribute , aNode->mUid ) )
      {
        //error description is given in the function itself so no more elaboration required
        throw NodeMustHaveUID();
      }
    }
    else
    {
      uhal::utilities::GetXMLattribute<false> ( aXmlNode , NodeTreeBuilder::mIdAttribute , aNode->mUid );
    }
  }

  void NodeTreeBuilder::setAddr ( const pugi::xml_node& aXmlNode , Node* aNode )
  {
    logging();
    //Address is an optional attribute for hierarchical addressing
    uint32_t lAddr ( 0 );
    uhal::utilities::GetXMLattribute<false> ( aXmlNode , NodeTreeBuilder::mAddressAttribute , lAddr );
    aNode->mPartialAddr |= lAddr;
  }

  void NodeTreeBuilder::setTags ( const pugi::xml_node& aXmlNode , Node* aNode )
  {
    logging();
    std::string lStr;
    //Tags is an optional attribute to allow the user to add a description to a node
    uhal::utilities::GetXMLattribute<false> ( aXmlNode , NodeTreeBuilder::mTagsAttribute , lStr );

    if ( lStr.size() && aNode->mTags.size() )
    {
      aNode->mTags += "[";
      aNode->mTags += lStr;
      aNode->mTags += "]";
    }
    else if ( lStr.size() && !aNode->mTags.size() )
    {
      aNode->mTags = lStr;
    }
  }


  void NodeTreeBuilder::setDescription ( const pugi::xml_node& aXmlNode , Node* aNode )
  {
    logging();
    std::string lStr;
    //Tags is an optional attribute to allow the user to add a description to a node
    uhal::utilities::GetXMLattribute<false> ( aXmlNode , NodeTreeBuilder::mDescriptionAttribute , lStr );

    if ( lStr.size() && aNode->mTags.size() )
    {
      aNode->mDescription += "[";
      aNode->mDescription += lStr;
      aNode->mDescription += "]";
    }
    else if ( lStr.size() && !aNode->mTags.size() )
    {
      aNode->mDescription = lStr;
    }
  }

  void NodeTreeBuilder::setPermissions ( const pugi::xml_node& aXmlNode , Node* aNode )
  {
    logging();
    //Permissions is an optional attribute for specifying read/write permissions
    std::string lPermission;

    if ( uhal::utilities::GetXMLattribute<false> ( aXmlNode , "permission" , lPermission ) )
    {
      boost::spirit::qi::phrase_parse (
        lPermission.begin(),
        lPermission.end(),
        NodeTreeBuilder::mPermissionsLut,
        boost::spirit::ascii::space,
        aNode->mPermission
      );
    }
  }


  void NodeTreeBuilder::setMask ( const pugi::xml_node& aXmlNode , Node* aNode )
  {
    logging();
    //Tags is an optional attribute to allow the user to add a description to a node
    uhal::utilities::GetXMLattribute<false> ( aXmlNode , NodeTreeBuilder::mMaskAttribute , aNode->mMask );
  }


  void NodeTreeBuilder::setModeAndSize ( const pugi::xml_node& aXmlNode , Node* aNode )
  {
    logging();
    //Mode is an optional attribute for specifying whether a block is incremental, non-incremental or a single register
    std::string lMode;

    if ( uhal::utilities::GetXMLattribute<false> ( aXmlNode , NodeTreeBuilder::mModeAttribute , lMode ) )
    {
      boost::spirit::qi::phrase_parse (
        lMode.begin(),
        lMode.end(),
        NodeTreeBuilder::mModeLut,
        boost::spirit::ascii::space,
        aNode->mMode
      );

      if ( aNode->mMode == defs::INCREMENTAL )
      {
        //If a block is incremental it requires a size attribute
        if ( ! uhal::utilities::GetXMLattribute<false> ( aXmlNode , NodeTreeBuilder::mSizeAttribute , aNode->mSize ) )
        {
          log ( Error() , "Node " , Quote ( aNode->mUid ) , " has type " , Quote ( "INCREMENTAL" ) , ", which requires a " , Quote ( NodeTreeBuilder::mSizeAttribute ) , " attribute" );
          throw IncrementalNodeRequiresSizeAttribute();
        }
      }
      else if ( aNode->mMode == defs::NON_INCREMENTAL )
      {
        //If a block is non-incremental, then a size attribute is recommended
        if ( ! uhal::utilities::GetXMLattribute<false> ( aXmlNode , NodeTreeBuilder::mSizeAttribute , aNode->mSize ) )
        {
          log ( Notice() , "Node " , Quote ( aNode->mUid ) , " has type " , Quote ( "NON_INCREMENTAL" ) , " but does not have a " , Quote ( NodeTreeBuilder::mSizeAttribute ) , " attribute. This is not necessarily a problem, but if there is a limit to the size of the read/write operation from this port, then please consider adding this attribute for the sake of safety." );
        }
      }
    }
  }



  void NodeTreeBuilder::addChildren ( const pugi::xml_node& aXmlNode , Node* aNode )
  {
    logging();
    pugi::xml_node lXmlNode = aXmlNode.child ( "node" );

    if ( aNode->mMode == defs::NON_INCREMENTAL )
    {
      if ( lXmlNode )
      {
        log ( Error() , "Block access nodes are not allowed to have child nodes" );
        throw BlockAccessNodeCannotHaveChild();
      }
    }
    else
    {
      for ( ; lXmlNode; lXmlNode = lXmlNode.next_sibling ( "node" ) )
      {
        aNode->mChildren.push_back ( mNodeParser ( lXmlNode ) );
      }

      for ( std::deque< Node* >::iterator lIt = aNode->mChildren.begin(); lIt != aNode->mChildren.end(); ++lIt )
      {
        aNode->mChildrenMap.insert ( std::make_pair ( ( **lIt ).mUid , *lIt ) );

        for ( std::hash_map< std::string , Node* >::iterator lSubMapIt = ( **lIt ).mChildrenMap.begin() ; lSubMapIt != ( **lIt ).mChildrenMap.end() ; ++lSubMapIt )
        {
          aNode->mChildrenMap.insert ( std::make_pair ( ( **lIt ).mUid +'.'+ ( lSubMapIt->first ) , lSubMapIt->second ) );
        }
      }
    }
  }


  void NodeTreeBuilder::calculateHierarchicalAddresses ( Node* aNode , const uint32_t& aAddr )
  {
    logging();

    if ( aNode->mMode == defs::HIERARCHICAL )
    {
      if ( aNode->mChildren.size() == 0 )
      {
        aNode->mMode = defs::SINGLE;
      }
      else
      {
        // bool lAnyMasked( false );
        bool lAllMasked ( true );

        for ( std::deque< Node* >::iterator lIt = aNode->mChildren.begin(); lIt != aNode->mChildren.end(); ++lIt )
        {
          if ( ( **lIt ).mMask == defs::NOMASK )
          {
            lAllMasked = false;
          }

          // else
          // {
          // lAnyMasked = true;
          // }
        }

        // if( lAnyMasked && !lAllMasked )
        // {
        // log ( Error() , "Both masked and unmasked children found in branch " , Quote ( aNode->mUid ) );
        // throw // BothMaskedAndUnmaskedChildren();
        // }

        if ( lAllMasked )
        {
          aNode->mMode = defs::SINGLE;
        }
      }
    }

    if ( aNode->mMode == defs::INCREMENTAL )
    {
      uint64_t lTopAddr ( ( uint64_t ) ( aNode->mPartialAddr ) + ( uint64_t ) ( aNode->mSize-1 ) );

      //Check that the requested block size does not extend outside register space
      if ( lTopAddr >> 32 )
      {
        log ( Error() , "A block size of " , Integer ( aNode->mSize ) , " and a base address of " , Integer ( aNode->mAddr , IntFmt<hex,fixed>() ) , " exceeds bounds of address space" );
        throw ArraySizeExceedsRegisterBound();
      }

      //Test for overlap with parent
      if ( ( uint32_t ) ( lTopAddr ) & aAddr ) //should set the most significant bit of the child address and then AND this with the parent address
      {
        log ( Warning() , "The partial address of the top register in the current branch, " , Quote ( aNode->mUid ) , " , (" , Integer ( ( uint32_t ) ( lTopAddr ) , IntFmt<hex,fixed>() ) , ") overlaps with the partial address of the parent branch (" , Integer ( aAddr , IntFmt<hex,fixed>() ) , "). This is in violation of the hierarchical design principal. For now this is a warning, but in the future this may be upgraded to throw an exception." );
      }
    }
    else
    {
      //Test for overlap with parent
      if ( aNode->mPartialAddr & aAddr ) //should set the most significant bit of the child address and then AND this with the parent address
      {
        log ( Warning() , "The partial address of the top register in the current branch, " , Quote ( aNode->mUid ) , " , (" , Integer ( aNode->mPartialAddr , IntFmt<hex,fixed>() ) , ") overlaps with the partial address of the parent branch (" , Integer ( aAddr , IntFmt<hex,fixed>() ) , "). This is in violation of the hierarchical design principal. For now this is a warning, but in the future this may be upgraded to throw an exception." );
      }
    }

    aNode->mAddr = aNode->mPartialAddr | aAddr;

    for ( std::deque< Node* >::iterator lIt = aNode->mChildren.begin(); lIt != aNode->mChildren.end(); ++lIt )
    {
      calculateHierarchicalAddresses ( *lIt , aNode->mAddr );
    }
  }



  void NodeTreeBuilder::checkForAddressCollisions ( Node* aNode )
  {
    logging();
    std::hash_map< std::string , Node* >::iterator lIt, lIt2;
    Node* lNode1, *lNode2;

    for ( lIt = aNode->mChildrenMap.begin() ; lIt != aNode->mChildrenMap.end() ; ++lIt )
    {
      lNode1 = lIt->second;
      lIt2 = lIt;
      lIt2++;

      if ( lNode1->mMode == defs::INCREMENTAL )
      {
        uint32_t lBottom1 ( lNode1->mAddr );
        uint32_t lTop1 ( lNode1->mAddr + ( lNode1->mSize - 1 ) );

        for ( ; lIt2 != aNode->mChildrenMap.end() ; ++lIt2 )
        {
          lNode2 = lIt2->second;

          if ( lNode2->mMode == defs::INCREMENTAL )
          {
            //Node1 and Node2 are both incremental
            uint32_t lBottom2 ( lNode2->mAddr );
            uint32_t lTop2 ( lNode2->mAddr + ( lNode2->mSize - 1 ) );

            if ( ( ( lTop2 >= lBottom1 ) && ( lTop2 <= lTop1 ) ) || ( ( lTop1 >= lBottom2 ) && ( lTop1 <= lTop2 ) ) )
            {
              log ( Error() , "Branch " , Quote ( lIt->first ) ,
                    " has address range [" , Integer ( lBottom1 , IntFmt<hex,fixed>() ) , " - " , Integer ( lTop1 , IntFmt<hex,fixed>() ) ,
                    "] which overlaps with branch " , Quote ( lIt2->first ) ,
                    " which has address range [" , Integer ( lBottom2 , IntFmt<hex,fixed>() ) , " - " , Integer ( lTop2 , IntFmt<hex,fixed>() ) ,
                    "]."
                  );
#ifdef THROW_ON_ADDRESS_SPACE_OVERLAP
              throw AddressSpaceOverlap();
#endif
            }
          }
          else if ( lNode2->mMode != defs::HIERARCHICAL )
          {
            //Node1 is incremental and Node2 is single address
            uint32_t lAddr2 ( lNode2->mAddr );

            if ( ( lAddr2 >= lBottom1 ) && ( lAddr2 <= lTop1 ) )
            {
              log ( Error() , "Branch " , Quote ( lIt->first ) ,
                    " has address range [" , Integer ( lBottom1 , IntFmt<hex,fixed>() ) , " - " , Integer ( lTop1 , IntFmt<hex,fixed>() ) ,
                    "] which overlaps with branch " , Quote ( lIt2->first ) ,
                    " which has address " , Integer ( lAddr2 , IntFmt<hex,fixed>() ) , "]."
                  );
#ifdef THROW_ON_ADDRESS_SPACE_OVERLAP
              throw AddressSpaceOverlap();
#endif
            }
          }
        }
      }
      else if ( lNode1->mMode != defs::HIERARCHICAL )
      {
        uint32_t lAddr1 ( lNode1->mAddr );

        for ( ; lIt2 != aNode->mChildrenMap.end() ; ++lIt2 )
        {
          lNode2 = lIt2->second;

          if ( lNode2->mMode == defs::INCREMENTAL )
          {
            //Node1 is single address and Node2 is incremental
            uint32_t lBottom2 ( lNode2->mAddr );
            uint32_t lTop2 ( lNode2->mAddr + ( lNode2->mSize - 1 ) );

            if ( ( lAddr1 >= lBottom2 ) && ( lAddr1 <= lTop2 ) )
            {
              log ( Error() , "Branch " , Quote ( lIt->first ) ,
                    " has address " , Integer ( lAddr1 , IntFmt<hex,fixed>() ) ,
                    "] which overlaps with branch " , Quote ( lIt2->first ) ,
                    " which has address range [" , Integer ( lBottom2 , IntFmt<hex,fixed>() ) , " - " , Integer ( lTop2 , IntFmt<hex,fixed>() ) , "]."
                  );
#ifdef THROW_ON_ADDRESS_SPACE_OVERLAP
              throw AddressSpaceOverlap();
#endif
            }
          }
          else if ( lNode2->mMode != defs::HIERARCHICAL )
          {
            //Node1 and Node2 are both single addresses
            uint32_t lAddr2 ( lNode2->mAddr );

            if ( lAddr1 == lAddr2 )
            {
              if ( lNode1->mMask & lNode2->mMask )
              {
                bool lShouldThrow ( true );

                if ( lNode1->mMask == 0xFFFFFFFF )
                {
                  // Node 1 is a full register, Node 2 is a masked region. Check if Node 2 is a child of Node 1 and, if not, then throw
                  for ( std::deque< Node* >::iterator lIt = lNode1->mChildren.begin() ; lIt != lNode1->mChildren.end() ; ++lIt )
                  {
                    if ( *lIt == lNode2 )
                    {
                      lShouldThrow = false;
                      break;
                    }
                  }
                }

                if ( lShouldThrow && ( lNode2->mMask == 0xFFFFFFFF ) )
                {
                  // Node 2 is a full register, Node 1 is a masked region. Check if Node 1 is a child of Node 2 and, if not, then throw
                  for ( std::deque< Node* >::iterator lIt = lNode2->mChildren.begin() ; lIt != lNode2->mChildren.end() ; ++lIt )
                  {
                    if ( *lIt == lNode1 )
                    {
                      lShouldThrow = false;
                      break;
                    }
                  }
                }

                if ( lShouldThrow )
                {
                  log ( Error() , "Branch " , Quote ( lIt->first ) ,
                        " has address " , Integer ( lAddr1 , IntFmt<hex,fixed>() ) ,
                        " and mask " , Integer ( lNode1->mMask , IntFmt<hex,fixed>() ) ,
                        " which overlaps with branch " , Quote ( lIt2->first ) ,
                        " which has address " , Integer ( lAddr2 , IntFmt<hex,fixed>() ) ,
                        " and mask " , Integer ( lNode2->mMask , IntFmt<hex,fixed>() )
                      );
#ifdef THROW_ON_ADDRESS_SPACE_OVERLAP
                  throw AddressSpaceOverlap();
#endif
                }
              }
            }
          }
        }
      }
    }
  }






  NodeTreeBuilder::permissions_lut::permissions_lut()
  {
    logging();
    add
    ( "r"			, defs::READ )
    ( "w"			, defs::WRITE )
    ( "read"		, defs::READ )
    ( "write"		, defs::WRITE )
    ( "rw"			, defs::READWRITE )
    ( "wr"			, defs::READWRITE )
    ( "readwrite"	, defs::READWRITE )
    ( "writeread"	, defs::READWRITE )
    ;
  }

  const NodeTreeBuilder::permissions_lut NodeTreeBuilder::mPermissionsLut;


  NodeTreeBuilder::mode_lut::mode_lut()
  {
    logging();
    add
    ( "single"			, defs::SINGLE )
    ( "block"			, defs::INCREMENTAL )
    ( "port"			, defs::NON_INCREMENTAL )
    ( "incremental"		, defs::INCREMENTAL )
    ( "non-incremental"	, defs::NON_INCREMENTAL )
    ( "inc"				, defs::INCREMENTAL )
    ( "non-inc"			, defs::NON_INCREMENTAL )
    ;
  }

  const NodeTreeBuilder::mode_lut NodeTreeBuilder::mModeLut;

}
