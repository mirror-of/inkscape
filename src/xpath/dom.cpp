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
## I M P L E M E N T A T I O N    C L A S S    D E C L A R A T I O N S
#########################################################################*/

class DOMImplementationImpl : public DOMImplementation
{
    public:

    bool hasFeature(DOMString& feature, DOMString& version);

    DocumentType *createDocumentType( DOMString& qualifiedName, 
                                     DOMString& publicId, 
                                     DOMString& systemId)
                                     throw(DOMException);

    Document *createDocument(DOMString& namespaceURI, 
                             DOMString& qualifiedName, 
                             DocumentType *doctype)
                             throw(DOMException);

};


class NodeImpl : public Node
{
    public:

    DOMString getNodeName();

    DOMString getNodeValue() throw (DOMException);

    void setNodeValue(DOMString& val) throw (DOMException);


    unsigned short getNodeType();


    Node *getParentNode();

    NodeList *getChildNodes();

    Node *getFirstChild();

    Node *getLastChild();


    Node *getPreviousSibling();

    Node *getNextSibling();


    NamedNodeMap *getAttributes();



    Document *getOwnerDocument();


    Node *insertBefore(Node *newChild, 
                       Node *refChild)
                       throw(DOMException);


    Node *replaceChild(Node *newChild, 
                       Node *oldChild)
                       throw(DOMException);


    Node *removeChild(Node *oldChild)
                      throw(DOMException);


    Node *appendChild(Node *newChild)
                      throw(DOMException);


    bool hasChildNodes();

    Node *cloneNode(bool deep);

    void normalize();

    bool isSupported(DOMString& feature, 
                     DOMString& version);

    DOMString getNamespaceURI();

    DOMString getPrefix();

    void setPrefix(DOMString& val) throw(DOMException);

    DOMString getLocalName();

    bool hasAttributes();

};


class NodeListImpl : public NodeList
{
    public:

    Node *item(unsigned long index);

    unsigned long getLength();

};


class NamedNodeMapImpl : public NamedNodeMap
{
    public:

    Node *getNamedItem(DOMString& name);

    Node *setNamedItem(Node *arg) throw(DOMException);

    Node *removeNamedItem(DOMString& name) throw(DOMException);

    Node *item(unsigned long index);

    unsigned long getLength();

    Node *getNamedItemNS(DOMString& namespaceURI, 
                         DOMString& localName);

    Node *setNamedItemNS(Node *arg) throw(DOMException);

    Node *removeNamedItemNS(DOMString& namespaceURI, 
                            DOMString& localName)
                            throw(DOMException);
};


class CharacterDataImpl : public CharacterData
{
    public:

    DOMString getData() throw(DOMException);

    void setData(DOMString& val) throw(DOMException);

    unsigned long getLength();

    DOMString substringData(unsigned long offset, 
                            unsigned long count)
                            throw(DOMException);

    void appendData(DOMString& arg) throw(DOMException);

    void insertData(unsigned long offset, 
                    DOMString& arg)
                    throw(DOMException);

    void deleteData(unsigned long offset, 
                    unsigned long count)
                    throw(DOMException);

    void  replaceData(unsigned long offset, 
                      unsigned long count, 
                      DOMString& arg)
                      throw(DOMException);

};



class AttrImpl : public Attr
{
    public:

    DOMString getName();

    bool getSpecified();

    DOMString getValue();

    void setValue(DOMString& val) throw(DOMException);

    Element *getOwnerElement();

};


class ElementImpl : public Element
{
    public:

    DOMString getTagName();

    DOMString getAttribute(DOMString& name);

    void setAttribute(DOMString& name, 
                      DOMString& value)
                      throw(DOMException);

    void removeAttribute(DOMString& name)
                         throw(DOMException);

    Attr *getAttributeNode(DOMString& name);

    Attr *setAttributeNode(Attr *newAttr)
                          throw(DOMException);

    Attr *removeAttributeNode(Attr *oldAttr)
                             throw(DOMException);

    NodeList *getElementsByTagName(DOMString& name);


    DOMString getAttributeNS(DOMString& namespaceURI, 
                             DOMString& localName);

    void setAttributeNS(DOMString& namespaceURI, 
                        DOMString& qualifiedName, 
                        DOMString& value)
                        throw(DOMException);


    void removeAttributeNS(DOMString& namespaceURI, 
                           DOMString& localName)
                           throw(DOMException);
 

    Attr *getAttributeNodeNS(DOMString& namespaceURI, 
                            DOMString& localName);

    Attr *setAttributeNodeNS(Attr *newAttr)
                            throw(DOMException);

    NodeList *getElementsByTagNameNS(DOMString& namespaceURI, 
                                    DOMString& localName);


    bool hasAttribute(DOMString& name);

    bool hasAttributeNS(DOMString& namespaceURI, 
                        DOMString& localName);
};



class TextImpl : public Text
{
    public:

    Text *splitText(unsigned long offset) throw(DOMException);
};


class CommentImpl : public Comment
{
};


class CDATASectionImpl : public CDATASection
{
};

class DocumentTypeImpl : public DocumentType
{
    public:

    DocumentTypeImpl(DOMString& qualifiedName, 
                     DOMString& publicId, DOMString& systemId);

    DOMString getName();


    NamedNodeMap *getEntities();


    NamedNodeMap *getNotations();


    DOMString getPublicId();


    DOMString getSystemId();

    DOMString getInternalSubset();

};


class NotationImpl : public Notation
{
    public:

    DOMString getPublicId();

    DOMString getSystemId();

};

class EntityImpl : public Entity
{
    public:

    DOMString getPublicId();

    DOMString getSystemId();

    DOMString getNotationName();

};


class EntityReferenceImpl : public EntityReference
{
};


class ProcessingInstructionImpl : public ProcessingInstruction
{
    public:

    DOMString getTarget();

    DOMString getData();

    void setData(DOMString& val) throw(DOMException);
};



class DocumentFragmentImpl : public DocumentFragment
{
};



class DocumentImpl : public Document
{
    public:

    DocumentType *DocumentImpl::getDoctype();

    DOMImplementation *DocumentImpl::getImplementation();

    Element *DocumentImpl::getDocumentElement();

    Element *DocumentImpl::createElement(DOMString& tagName) throw(DOMException);

    DocumentFragment *DocumentImpl::createDocumentFragment();

    Text *DocumentImpl::createTextNode(DOMString& data);

    Comment  *DocumentImpl::createComment(DOMString& data);

    CDATASection *DocumentImpl::createCDATASection(DOMString& data)
                                     throw(DOMException);

    ProcessingInstruction *DocumentImpl::createProcessingInstruction(DOMString& target, 
                                                       DOMString& data)
                                                       throw(DOMException);


    Attr *DocumentImpl::createAttribute(DOMString& name)
                          throw(DOMException);


    EntityReference *DocumentImpl::createEntityReference(DOMString& name)
                                           throw(DOMException);


    NodeList *DocumentImpl::getElementsByTagName(DOMString& tagname);

    Node *DocumentImpl::importNode(Node *importedNode, bool deep)
                                 throw(DOMException);

    Element *DocumentImpl::createElementNS(DOMString& namespaceURI, 
                             DOMString& qualifiedName)
                             throw(DOMException);

    Attr *DocumentImpl::createAttributeNS(DOMString& namespaceURI, 
                            DOMString& qualifiedName)
                            throw(DOMException);

    NodeList *DocumentImpl::getElementsByTagNameNS(DOMString& namespaceURI, 
                                     DOMString& localName);

    Element *DocumentImpl::getElementById(DOMString& elementId);

};



/*#########################################################################
## DOMException
#########################################################################*/
static SymTableEntry exceptionCodes[] =
{

    { "INDEX_SIZE_ERR",              INDEX_SIZE_ERR              },
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


const char *DOMException::what() const throw()
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
DocumentType *DOMImplementationImpl::createDocumentType(
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
                             DocumentType *doctype)
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
Node *NodeImpl::insertBefore(Node *newChild, 
                         Node *refChild)
                         throw(DOMException)
{

}


/**
 *
 */
Node *NodeImpl::replaceChild(Node *newChild, 
                         Node *oldChild)
                         throw(DOMException)
{

}


/**
 *
 */
Node *NodeImpl::removeChild(Node *oldChild)
                        throw(DOMException)
{

}


/**
 *
 */
Node *NodeImpl::appendChild(Node *newChild)
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
Node *NamedNodeMapImpl::setNamedItem(Node *arg) throw(DOMException)
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
Node *NamedNodeMapImpl::setNamedItemNS(Node *arg) throw(DOMException)
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
Attr *ElementImpl::setAttributeNode(Attr *newAttr)
                          throw(DOMException)
{

}


/**
 *
 */
Attr *ElementImpl::removeAttributeNode(Attr *oldAttr)
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
Attr *ElementImpl::setAttributeNodeNS(Attr *newAttr)
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
Node *DocumentImpl::importNode(Node *importedNode, 
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


