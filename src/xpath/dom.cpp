/*
 * Copyright (c) 2000 World Wide Web Consortium,
 * (Massachusetts Institute of Technology, Institut National de
 * Recherche en Informatique et en Automatique, Keio University). All
 * Rights Reserved. This program is distributed under the W3C's Software
 * Intellectual Property License. This program is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.
 * See W3C License http://www.w3.org/Consortium/Legal/ for more details.
 */

// File: http://www.w3.org/TR/2000/REC-DOM-Level-2-Core-20001113/dom.idl

#ifndef ___DOM_H___
#define __DOM_H__

#include "dom.h"




namespace w3c
{
namespace org
{
namespace dom
{


typedef struct
{
    char *sval; //String value
    int ival;   //Enum value
} SymTableEntry;


/*#########################################################################
## DOMException
#########################################################################*/
static SymTableEntry exceptionCodes[] =
{

    { "INDEX_SIZE_ERR",              INDEX_SIZE_ERR
    { "DOMSTRING_SIZE_ERR",          DOMSTRING_SIZE_ERR          },
    { "HIERARCHY_REQUEST_ERR",       HIERARCHY_REQUEST_ERR       },
    { "WRONG_DOCUMENT_ERR",          WRONG_DOCUMENT_ERR          },
    { "INVALID_CHARACTER_ERR",       INVALID_CHARACTER_ERR       },
    { "NO_DATA_ALLOWED_ERR",         NO_DATA_ALLOWED_ERR         },
    { "NO_MODIFICATION_ALLOWED_ERR", NO_MODIFICATION_ALLOWED_ERR },
    { "NOT_FOUND_ERR",               NOT_FOUND_ERR               },
    { "NOT_SUPPORTED_ERR",           NOT_SUPPORTED_ERR           },
    { "INUSE_ATTRIBUTE_ERR",         INUSE_ATTRIBUTE_ERR         },
    { "INVALID_STATE_ERR",           INVALID_STATE_ERR           },
    { "SYNTAX_ERR",                  SYNTAX_ERR                  },
    { "INVALID_MODIFICATION_ERR",    INVALID_MODIFICATION_ERR    },
    { "NAMESPACE_ERR",               NAMESPACE_ERR               },
    { "INVALID_ACCESS_ERR",          INVALID_ACCESS_ERR          },
    { NULL,                          0                           }
};


const char *DomException::what const throw()
{
    SymTableEntry *entry;
    for (entry = exceptionCodes; entry->sval ; entry++)
        {
        if (code == entry->ival)
            {
            return entry->sval;
            }
        }
    return "Not defined";
}




/*#########################################################################
## DOMImplementation
#########################################################################*/

class DOMImplementationImpl : public DOMImplementation
{


};

/**
 *
 */
bool DOMImplementationImpl::hasFeature(DOMString& feature, 
                                       DOMString& version)
{



    return false;
}




/**
 * L2
 */
DocumentType *DOMImplementationImpl::*createDocumentType(
                                     DOMString& qualifiedName, 
                                     DOMString& publicId, 
                                     DOMString& systemId)
                                     throw(DOMException)
{
   DocumentTypeImpl *type = new DocumentTypeImpl(qualifiedName, 
                                  publicId, systemId);
   return type;
}



/**
 * L2
 */
Document *DOMImplementationImpl::createDocument(DOMString& namespaceURI, 
                             DOMString& qualifiedName, 
                             DocumentType &doctype)
                             throw(DOMException)
{
    DocumentImpl *doc = new DocumentImpl(namespaceURI, qualifiedName,
                                          doctype);
    return doc;

}





/*#########################################################################
## Node
#########################################################################*/

static SymTableEntry nodeTypes[] =
{
    { "ELEMENT_NODE",                ELEMENT_NODE                 },
    { "ATTRIBUTE_NODE",              ATTRIBUTE_NODE               },
    { "TEXT_NODE",                   TEXT_NODE                    },
    { "CDATA_SECTION_NODE",          CDATA_SECTION_NODE           },
    { "ENTITY_REFERENCE_NODE",       ENTITY_REFERENCE_NODE        },
    { "ENTITY_NODE",                 ENTITY_NODE                  },
    { "PROCESSING_INSTRUCTION_NODE", PROCESSING_INSTRUCTION_NODE  },
    { "COMMENT_NODE",                COMMENT_NODE                 },
    { "DOCUMENT_NODE",               DOCUMENT_NODE                },
    { "DOCUMENT_TYPE_NODE",          DOCUMENT_TYPE_NODE           },
    { "DOCUMENT_FRAGMENT_NODE",      DOCUMENT_FRAGMENT_NODE       },
    { "NOTATION_NODE",               NOTATION_NODE                },
    { NULL,                          0                            }
};


/**
 *
 */
DOMString NodeImpl::getNodeName()
{
    return nodeName;
}

/**
 *
 */
DOMString NodeImpl::getNodeValue() throw (DOMException)
{

}


/**
 *
 */
void NodeImpl::setNodeValue(DOMString& val) throw (DOMException)
{

}


/**
 *
 */
unsigned short NodeImpl::getNodeType()
{

}


/**
 *
 */
Node *NodeImpl::getParentNode()
{

}


/**
 *
 */
NodeList *NodeImpl::getChildNodes()
{

}


/**
 *
 */
Node *NodeImpl::getFirstChild()
{

}


/**
 *
 */
Node *NodeImpl::getLastChild()
{

}


/**
 *
 */
Node *NodeImpl::getPreviousSibling()
{

}


/**
 *
 */
Node *NodeImpl::getNextSibling()
{

}


/**
 *
 */
NamedNodeMap *NodeImpl::getAttributes()
{

}



/**
 * L2
 */
Document *NodeImpl::getOwnerDocument()
{

}


/**
 *
 */
Node *NodeImpl::insertBefore(Node& newChild, 
                         Node& refChild)
                         throw(DOMException)
{

}


/**
 *
 */
Node *NodeImpl::replaceChild(Node newChild, 
                         Node oldChild)
                         throw(DOMException)
{

}


/**
 *
 */
Node *NodeImpl::removeChild(Node oldChild)
                        throw(DOMException)
{

}


/**
 *
 */
Node *NodeImpl::appendChild(Node newChild)
                        throw(DOMException)
{

}


/**
 *
 */
bool NodeImpl::hasChildNodes()
{

}


/**
 *
 */
Node *NodeImpl::cloneNode(bool deep)
{

}


/**
 * L2
 */
void NodeImpl::normalize()
{

}


/**
 * L2
 */
bool NodeImpl::isSupported(DOMString& feature, 
                       DOMString& version)
{

}


/**
 * L2
 */
DOMString NodeImpl::getNamespaceURI()
{

}


/**
 * L2
 */
DOMString NodeImpl::getPrefix()
{

}


/**
 *
 */
void NodeImpl::setPrefix(DOMString& val) throw(DOMException)
{

}


/**
 * L2
 */
DOMString NodeImpl::getLocalName()
{

}


/**
 * L2
 */
bool NodeImpl::hasAttributes()
{

}



/*#########################################################################
## NodeList
#########################################################################*/

/**
 *
 */
Node *NodeListImpl::item(unsigned long index)
{
    Node *node;
    return node;
}

/**
 *
 */
unsigned long NodeListImpl::getLength()
{
    return 0;
}





/*#########################################################################
## NamedNodeMap
#########################################################################*/

/**
 *
 */
Node *NamedNodeMapImpl::getNamedItem(DOMString& name)
{

}


/**
 *
 */
Node *NamedNodeMapImpl::setNamedItem(Node &arg) throw(DOMException)
{

}



/**
 *
 */
Node *NamedNodeMapImpl::removeNamedItem(DOMString& name) throw(DOMException)
{

}


/**
 *
 */
Node *NamedNodeMapImpl::item(unsigned long index)
{

}


/**
 *
 */
unsigned long NamedNodeMapImpl::getLength()
{

}


/**
 * L2
 */
Node *NamedNodeMapImpl::getNamedItemNS(DOMString& namespaceURI, 
                                   DOMString& localName)
{

}


/**
 * L2
 */
Node *NamedNodeMapImpl::setNamedItemNS(Node arg) throw(DOMException)
{

}


/**
 * L2
 */
Node *NamedNodeMapImpl::removeNamedItemNS(DOMString& namespaceURI, 
                                      DOMString& localName)
                                      throw(DOMException)
{

}






/*#########################################################################
## CharacterData
#########################################################################*/

/**
 *
 */
DOMString CharacterDataImpl::getData() throw(DOMException)
{

}


/**
 *
 */
void CharacterDataImpl::setData(DOMString& val) throw(DOMException)
{

}


/**
 *
 */
unsigned long CharacterDataImpl::getLength()
{

}


/**
 *
 */
DOMString CharacterDataImpl::substringData(unsigned long offset, 
                                       unsigned long count)
                                       throw(DOMException)
{

}


/**
 *
 */
void CharacterDataImpl::appendData(DOMString& arg) throw(DOMException)
{

}


/**
 *
 */
void CharacterDataImpl::insertData(unsigned long offset, 
                               DOMString& arg)
                               throw(DOMException)
{

}


/**
 *
 */
void CharacterDataImpl::deleteData(unsigned long offset, 
                               unsigned long count)
                               throw(DOMException)
{

}


/**
 *
 */
void  CharacterDataImpl::replaceData(unsigned long offset, 
                                 unsigned long count, 
                                 DOMString& arg)
                                 throw(DOMException)
{

}








/*#########################################################################
## Attr
#########################################################################*/

/**
 *
 */
DOMString AttrImpl::getName()
{

}


/**
 *
 */
bool AttrImpl::getSpecified()
{

}


/**
 *
 */
DOMString AttrImpl::getValue()
{

}


/**
 *
 */
void AttrImpl::setValue(DOMString& val) throw(DOMException)
{

}




/**
 * L2
 */
Element *AttrImpl::getOwnerElement()
{

}






/*#########################################################################
## Element
#########################################################################*/

/**
 *
 */
DOMString ElementImpl::getTagName()
{

}


/**
 *
 */
DOMString ElementImpl::getAttribute(DOMString& name)
{

}


/**
 *
 */
void ElementImpl::setAttribute(DOMString& name, 
                      DOMString& value)
                      throw(DOMException)
{

}


/**
 *
 */
void ElementImpl::removeAttribute(DOMString& name)
                         throw(DOMException)
{

}


/**
 *
 */
Attr *ElementImpl::getAttributeNode(DOMString& name)
{

}


/**
 *
 */
Attr *ElementImpl::setAttributeNode(Attr& newAttr)
                          throw(DOMException)
{

}


/**
 *
 */
Attr *ElementImpl::removeAttributeNode(Attr& oldAttr)
                             throw(DOMException)
{

}


/**
 *
 */
NodeList *ElementImpl::getElementsByTagName(DOMString& name)
{

}


/**
 * L2
 */
DOMString ElementImpl::getAttributeNS(DOMString& namespaceURI, 
                             DOMString& localName)
{

}


/**
 * L2
 */
void ElementImpl::setAttributeNS(DOMString& namespaceURI, 
                        DOMString& qualifiedName, 
                        DOMString& value)
                        throw(DOMException)
{

}


/**
 * L2
 */
void ElementImpl::removeAttributeNS(DOMString& namespaceURI, 
                           DOMString& localName)
                           throw(DOMException)
{

}

 
/**
 * L2
 */
Attr *ElementImpl::getAttributeNodeNS(DOMString& namespaceURI, 
                            DOMString& localName)
{

}


/**
 * L2
 */
Attr *ElementImpl::setAttributeNodeNS(Attr& newAttr)
                            throw(DOMException)
{

}


/**
 * L2
 */
NodeList *ElementImpl::getElementsByTagNameNS(DOMString& namespaceURI, 
                                    DOMString& localName)
{

}


/**
 * L2
 */
bool ElementImpl::hasAttribute(DOMString& name)
{

}


/**
 * L2
 */
bool ElementImpl::hasAttributeNS(DOMString& namespaceURI, 
                        DOMString& localName)
{

}





/*#########################################################################
## Text
#########################################################################*/

/**
 *
 */
Text *TextImpl::splitText(unsigned long offset)
{

}




/*#########################################################################
## Comment
#########################################################################*/





/*#########################################################################
## CDATASection
#########################################################################*/




/*#########################################################################
## DocumentType
#########################################################################*/



/**
 *
 */
DOMString DocumentTypeImpl::getName()
{

}


/**
 *
 */
NamedNodeMap *DocumentTypeImpl::getEntities()
{

}


/**
 *
 */
NamedNodeMap *DocumentTypeImpl::getNotations()
{

}


/**
 *
 */
DOMString DocumentTypeImpl::getPublicId()
{

}


/**
 * L2
 */
DOMString DocumentTypeImpl::getSystemId()
{

}


/**
 * L2
 */
DOMString DocumentTypeImpl::getInternalSubset()
{

}






/*#########################################################################
## Notation
#########################################################################*/


/**
 *
 */
DOMString NotationImpl::getPublicId()
{

}


/**
 *
 */
DOMString NotationImpl::getSystemId()
{

}







/*#########################################################################
## Entity
#########################################################################*/

/**
 *
 */
DOMString EntityImpl::getPublicId()
{

}


/**
 *
 */
DOMString EntityImpl::DocumentTypeImpl::getSystemId()
{

}


/**
 *
 */
DOMString EntityImpl::DocumentTypeImpl::getNotationName()
{

}







/*#########################################################################
## EntityReference
#########################################################################*/




/*#########################################################################
## ProcessingInstruction
#########################################################################*/


/**
 *
 */
DOMString ProcessingInstructionImpl::DocumentTypeImpl::getTarget()
{

}


/**
 *
 */
DOMString ProcessingInstructionImpl::getData()
{

}


/**
 *
 */
void ProcessingInstructionImpl::setData(DOMString& val) throw(DOMException)
{

}






/*#########################################################################
## DocumentFragment
#########################################################################*/






/*#########################################################################
## Document
#########################################################################*/

/**
 *
 */
DocumentType *DocumentImpl::getDoctype()
{

}


/**
 *
 */
DOMImplementation *DocumentImpl::getImplementation()
{

}


/**
 *
 */
Element *DocumentImpl::getDocumentElement()
{

}


/**
 *
 */
Element *DocumentImpl::createElement(DOMString& tagName)
                           throw(DOMException)
{

}


/**
 *
 */
DocumentFragment *DocumentImpl::createDocumentFragment()
{

}


/**
 *
 */
Text *DocumentImpl::createTextNode(DOMString& data)
{

}


/**
 *
 */
Comment  *DocumentImpl::createComment(DOMString& data)
{

}


/**
 *
 */
CDATASection *DocumentImpl::createCDATASection(DOMString& data)
                                     throw(DOMException)
{

}


/**
 *
 */
ProcessingInstruction *DocumentImpl::createProcessingInstruction(DOMString& target, 
                                                       DOMString& data)
                                                       throw(DOMException)
{

}


/**
 *
 */
Attr *DocumentImpl::createAttribute(DOMString& name)
                          throw(DOMException)
{

}


/**
 *
 */
EntityReference *DocumentImpl::createEntityReference(DOMString& name)
                                           throw(DOMException)
{

}


/**
 *
 */
NodeList *DocumentImpl::getElementsByTagName(DOMString& tagname)
{

}



/**
 * L2
 */
Node *DocumentImpl::importNode(Node& importedNode, 
                 bool deep)
                 throw(DOMException)
{

}


/**
 * L2
 */
Element *DocumentImpl::createElementNS(DOMString& namespaceURI, 
                             DOMString& qualifiedName)
                             throw(DOMException)
{

}


/**
 * L2
 */
Attr *DocumentImpl::createAttributeNS(DOMString& namespaceURI, 
                            DOMString& qualifiedName)
                            throw(DOMException)
{

}


/**
 * L2
 */
NodeList *DocumentImpl::getElementsByTagNameNS(DOMString& namespaceURI, 
                                     DOMString& localName)
{

}


/**
 * L2
 */
Element *DocumentImpl::getElementById(DOMString& elementId)
{

}










};//namespace dom
};//namespace org
};//namespace w3c


#endif // __DOM_H__

/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/


