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

#include "domimpl.h"


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
bool DOMImplementationImpl::hasFeature(const DOMString& feature, 
                                       const DOMString& version)
{



    return false;
}




/**
 * L2
 */
DocumentType *DOMImplementationImpl::createDocumentType(
                                     const DOMString& qualifiedName, 
                                     const DOMString& publicId, 
                                     const DOMString& systemId)
                                     throw(DOMException)
{
   DocumentType *type = new DocumentTypeImpl(qualifiedName, 
                                  publicId, systemId);
   return type;
}



/**
 * L2
 */
Document *DOMImplementationImpl::createDocument(const DOMString& namespaceURI, 
                             const DOMString& qualifiedName, 
                             DocumentType *doctype)
                             throw(DOMException)
{
    Document *doc = new DocumentImpl(namespaceURI, qualifiedName,
                                          doctype);
    return doc;

}




/*#########################################################################
## Node
#########################################################################*/

static SymTableEntry nodeTypes[] =
{
    { "ELEMENT_NODE",                Node::ELEMENT_NODE                 },
    { "ATTRIBUTE_NODE",              Node::ATTRIBUTE_NODE               },
    { "TEXT_NODE",                   Node::TEXT_NODE                    },
    { "CDATA_SECTION_NODE",          Node::CDATA_SECTION_NODE           },
    { "ENTITY_REFERENCE_NODE",       Node::ENTITY_REFERENCE_NODE        },
    { "ENTITY_NODE",                 Node::ENTITY_NODE                  },
    { "PROCESSING_INSTRUCTION_NODE", Node::PROCESSING_INSTRUCTION_NODE  },
    { "COMMENT_NODE",                Node::COMMENT_NODE                 },
    { "DOCUMENT_NODE",               Node::DOCUMENT_NODE                },
    { "DOCUMENT_TYPE_NODE",          Node::DOCUMENT_TYPE_NODE           },
    { "DOCUMENT_FRAGMENT_NODE",      Node::DOCUMENT_FRAGMENT_NODE       },
    { "NOTATION_NODE",               Node::NOTATION_NODE                },
    { NULL,                          0                                  }
};


NodeImpl::NodeImpl()
{
    childNodes      = new NodeListImpl();
    parentNode      = NULL;
    previousSibling = NULL;
    nextSibling     = NULL;
    attributes      = new NamedNodeMapImpl();
}

NodeImpl::~NodeImpl()
{
    for (unsigned int i = 0 ; i<childNodes->getLength() ; i++)
        delete (childNodes->item(i));

    delete childNodes;

    delete attributes;
}


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
    return nodeValue;
}


/**
 *
 */
void NodeImpl::setNodeValue(const DOMString& val) throw (DOMException)
{
    nodeValue = val;
}


/**
 *
 */
unsigned short NodeImpl::getNodeType()
{
    return nodeType;
}


/**
 *
 */
Node *NodeImpl::getParentNode()
{
    return parentNode;
}


/**
 *
 */
NodeList *NodeImpl::getChildNodes()
{
    return childNodes;  
}


/**
 *
 */
Node *NodeImpl::getFirstChild()
{
    if (childNodes->getLength() > 0)
        return childNodes->item(0);
    else
        return NULL;
}


/**
 *
 */
Node *NodeImpl::getLastChild()
{
    if (childNodes->getLength() > 0)
        return childNodes->item(childNodes->getLength()-1);
    else
        return NULL;
}


/**
 *
 */
Node *NodeImpl::getPreviousSibling()
{
    return previousSibling;
}


/**
 *
 */
Node *NodeImpl::getNextSibling()
{
    return nextSibling;
}


/**
 *
 */
NamedNodeMap *NodeImpl::getAttributes()
{
    return attributes;
}



/**
 * L2
 */
Document *NodeImpl::getOwnerDocument()
{
    return ownerDocument;
}


/**
 *
 */
Node *NodeImpl::insertBefore(const Node *newChild, 
                         const Node *refChild)
                         throw(DOMException)
{
    NodeListImpl *nl = (NodeListImpl *)childNodes;
    if (!nl->insert(refChild, newChild))
        {
        throw (DOMException(NOT_FOUND_ERR));
        }
    return (Node *)newChild;
} 


/**
 *
 */
Node *NodeImpl::replaceChild(const Node *newChild, 
                         const Node *oldChild)
                         throw(DOMException)
{
    NodeListImpl *nl = (NodeListImpl *)childNodes;
    if (!nl->insert(oldChild, newChild))
        {
        throw (DOMException(NOT_FOUND_ERR));
        }
    return (Node *)newChild;
}


/**
 *
 */
Node *NodeImpl::removeChild(const Node *oldChild)
                        throw(DOMException)
{
    NodeListImpl *nl = (NodeListImpl *)childNodes;
    if (!nl->remove(oldChild))
        {
        throw DOMException(NOT_FOUND_ERR);
        }
    return (Node *)oldChild;
}


/**
 *
 */
Node *NodeImpl::appendChild(const Node *newChild)
                        throw(DOMException)
{
    NodeListImpl *nl = (NodeListImpl *)childNodes;
    nl->append(newChild);
    return (Node *)newChild;
}


/**
 *
 */
bool NodeImpl::hasChildNodes()
{
    return (childNodes->getLength() > 0);
}


/**
 *
 */
Node *NodeImpl::cloneNode(bool deep)
{
    NodeImpl *newNode  = new NodeImpl();
    newNode->nodeName  = nodeName;
    newNode->nodeValue = nodeValue;
    if (deep)
        {
        int len = childNodes->getLength();
        for (int i=0 ; i<len ; i++)
            {
            Node *newChild = childNodes->item(i)->cloneNode(true);
            newNode->appendChild(newChild);
            }
        }
    return newNode;
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
bool NodeImpl::isSupported(const DOMString& feature, 
                       const DOMString& version)
{
    return false;
}


/**
 * L2
 */
DOMString NodeImpl::getNamespaceURI()
{
    return namespaceURI;
}


/**
 * L2
 */
DOMString NodeImpl::getPrefix()
{
    return prefix;
}


/**
 *
 */
void NodeImpl::setPrefix(const DOMString& val) throw(DOMException)
{
    prefix = val;
}


/**
 * L2
 */
DOMString NodeImpl::getLocalName()
{
    return DOMString("");

}


/**
 * L2
 */
bool NodeImpl::hasAttributes()
{
    return (nodeType==ELEMENT_NODE);
}

/*not in api*/
void NodeImpl::setNodeName(const DOMString& val)
{
    nodeName = val;
}



/*#########################################################################
## NodeList
#########################################################################*/

/**
 *
 */
Node *NodeListImpl::item(unsigned long index)
{
    return nodes[index];
}

/**
 *
 */
unsigned long NodeListImpl::getLength()
{
    return (unsigned long)nodes.size();
}


/*
not part of the api
*/
bool NodeListImpl::insert(int position, const Node *newNode)
{
    if (position < 0 || position >= (int)nodes.size())
        return false;
    std::vector<Node *>::iterator iter = nodes.begin() + position;
    nodes.insert(iter, (Node *)newNode);
    return true;
}

bool NodeListImpl::insert(const Node *current, const Node *newNode)
{
    bool retVal=false;
    std::vector<Node *>::iterator iter;
    for (iter = nodes.begin() ; iter!=nodes.end(); iter++)
        {
        if (current == *iter)
            {
            nodes.insert(iter, (Node *)newNode);
            retVal = true;
            }
        }
    return retVal;
}

bool NodeListImpl::replace(const Node *current, const Node *newNode)
{
    bool retVal=false;
    std::vector<Node *>::iterator iter;
    for (iter = nodes.begin() ; iter!=nodes.end(); iter++)
        {
        if (current == *iter)
            {
            *iter = (Node *)newNode;
            retVal = true;
            }
        }
    return retVal;

}


bool NodeListImpl::remove(const Node *node)
{
    bool retVal=false;
    std::vector<Node *>::iterator iter;
    for (iter = nodes.begin() ; iter!=nodes.end(); iter++)
        {
        if (node == *iter)
            {
            nodes.erase(iter);
            retVal = true;
            }
        }
    return retVal;
}


void NodeListImpl::append(const Node *newNode)
{
    nodes.push_back((Node *)newNode);

}



/*#########################################################################
## NamedNodeMap
#########################################################################*/

/**
 *
 */
Node *NamedNodeMapImpl::getNamedItem(const DOMString& name)
{
    Node *item = table[name];
    return item;
}


/**
 *
 */
Node *NamedNodeMapImpl::setNamedItem(const Node *node) throw(DOMException)
{
    Node *theNode = (Node *)node;
    table[theNode->getNodeName()] = theNode;
    return theNode;
}



/**
 *
 */
Node *NamedNodeMapImpl::removeNamedItem(const DOMString& name) throw(DOMException)
{
    Node *node = table[name];
    table.erase(name);
    return node;
}


/**
 *
 */
Node *NamedNodeMapImpl::item(unsigned long index)
{
    if (index > table.size())
        return NULL;
    std::map<DOMString, Node *, MapComparator>::iterator iter = table.begin();
    for (unsigned long i=0 ; i<index ; i++)
        iter++;
    return iter->second;
}


/**
 *
 */
unsigned long NamedNodeMapImpl::getLength()
{
    return table.size();
}


/**
 * L2
 */
Node *NamedNodeMapImpl::getNamedItemNS(const DOMString& namespaceURI, 
                                   const DOMString& localName)
{
    return table[localName];
}


/**
 * L2
 */
Node *NamedNodeMapImpl::setNamedItemNS(const Node *node) throw(DOMException)
{
    Node *theNode = (Node *)node;
    table[theNode->getNodeName()] = theNode;
    return theNode;
    
}


/**
 * L2
 */
Node *NamedNodeMapImpl::removeNamedItemNS(const DOMString& namespaceURI, 
                                      const DOMString& localName)
                                      throw(DOMException)
{
    Node *node = table[localName];
    table.erase(localName);
    return node;
}






/*#########################################################################
## CharacterData
#########################################################################*/

/**
 *
 */
DOMString CharacterDataImpl::getData() throw(DOMException)
{
    return nodeValue;
}


/**
 *
 */
void CharacterDataImpl::setData(const DOMString& val) throw(DOMException)
{
    nodeValue = val;
}


/**
 *
 */
unsigned long CharacterDataImpl::getLength()
{
    return nodeValue.size();
}


/**
 *
 */
DOMString CharacterDataImpl::substringData(unsigned long offset, 
                                       unsigned long count)
                                       throw(DOMException)
{
     return nodeValue.substr(offset, count);
}


/**
 *
 */
void CharacterDataImpl::appendData(const DOMString& arg) throw(DOMException)
{
    nodeValue += arg;
}


/**
 *
 */
void CharacterDataImpl::insertData(unsigned long offset, 
                               const DOMString& arg)
                               throw(DOMException)
{
    nodeValue.insert(offset, arg);
}


/**
 *
 */
void CharacterDataImpl::deleteData(unsigned long offset, 
                               unsigned long count)
                               throw(DOMException)
{
    nodeValue.erase(offset, count);
}


/**
 *
 */
void  CharacterDataImpl::replaceData(unsigned long offset, 
                                 unsigned long count, 
                                 const DOMString& arg)
                                 throw(DOMException)
{
    nodeValue.replace(offset, count, arg);
}








/*#########################################################################
## Attr
#########################################################################*/

/**
 *
 */
DOMString AttrImpl::getName()
{
    return nodeName;
}


/**
 *
 */
bool AttrImpl::getSpecified()
{
    return false;
}


/**
 *
 */
DOMString AttrImpl::getValue()
{
    return nodeValue;
}


/**
 *
 */
void AttrImpl::setValue(const DOMString& val) throw(DOMException)
{
    nodeValue = val;
}




/**
 * L2
 */
Element *AttrImpl::getOwnerElement()
{
    return ownerElement;
}






/*#########################################################################
## Element
#########################################################################*/

/**
 *
 */
DOMString ElementImpl::getTagName()
{
    return nodeName;
}


/**
 *
 */
DOMString ElementImpl::getAttribute(const DOMString& name)
{
    Node *node = attributes->getNamedItem(name);
    if (!node)
        return "";
    return node->getNodeValue();
}


/**
 *
 */
void ElementImpl::setAttribute(const DOMString& name, 
                      const DOMString& value)
                      throw(DOMException)
{
    AttrImpl *attr = reinterpret_cast<AttrImpl *>(attributes->getNamedItem(name));
    if (attr)
        attr->setNodeValue(value);
    else
        {
        attr = new AttrImpl();
        attr->setNodeName(name);
        attr->setNodeValue(value);
        attr->ownerElement = this;
        attributes->setNamedItem(attr);
        }
}


/**
 *
 */
void ElementImpl::removeAttribute(const DOMString& name)
                         throw(DOMException)
{
    attributes->removeNamedItem(name);
}


/**
 *
 */
Attr *ElementImpl::getAttributeNode(const DOMString& name)
{
    AttrImpl *attr = reinterpret_cast<AttrImpl *>(attributes->getNamedItem(name));
    return attr;
}


/**
 *
 */
Attr *ElementImpl::setAttributeNode(Attr *newAttr)
                          throw(DOMException)
{
    removeAttributeNode(newAttr);
    attributes->setNamedItem(newAttr);
    return (Attr *)newAttr;
}


/**
 *
 */
Attr *ElementImpl::removeAttributeNode(Attr *oldAttr)
                             throw(DOMException)
{
    attributes->removeNamedItem(oldAttr->getName());
    return oldAttr;
}


/**
 *
 */
NodeList *ElementImpl::getElementsByTagName(const DOMString& name)
{
    NodeList *list = new NodeListImpl();
    /***START HERE***/
    return list;
}


/**
 * L2
 */
DOMString ElementImpl::getAttributeNS(const DOMString& namespaceURI, 
                             const DOMString& localName)
{
    DOMString val;
    return val;
}


/**
 * L2
 */
void ElementImpl::setAttributeNS(const DOMString& namespaceURI, 
                        const DOMString& qualifiedName, 
                        const DOMString& value)
                        throw(DOMException)
{

}


/**
 * L2
 */
void ElementImpl::removeAttributeNS(const DOMString& namespaceURI, 
                           const DOMString& localName)
                           throw(DOMException)
{

}

 
/**
 * L2
 */
Attr *ElementImpl::getAttributeNodeNS(const DOMString& namespaceURI, 
                            const DOMString& localName)
{
    Attr *attr;
    return attr;
}


/**
 * L2
 */
Attr *ElementImpl::setAttributeNodeNS(Attr *newAttr)
                            throw(DOMException)
{
    Attr *attr;
    return attr;

}


/**
 * L2
 */
NodeList *ElementImpl::getElementsByTagNameNS(const DOMString& namespaceURI, 
                                    const DOMString& localName)
{
    NodeList *list = new NodeListImpl();
    return list;
}


/**
 * L2
 */
bool ElementImpl::hasAttribute(const DOMString& name)
{

}


/**
 * L2
 */
bool ElementImpl::hasAttributeNS(const DOMString& namespaceURI, 
                        const DOMString& localName)
{

}





/*#########################################################################
## Text
#########################################################################*/

/**
 *
 */
Text *TextImpl::splitText(unsigned long offset) throw(DOMException)
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
DOMString ProcessingInstructionImpl:getTarget()
{
    return target;
}


/**
 *
 */
DOMString ProcessingInstructionImpl::getData()
{
    return piData;
}


/**
 *
 */
void ProcessingInstructionImpl::setData(const DOMString& val) throw(DOMException)
{
    piData = val;
}






/*#########################################################################
## DocumentFragment
#########################################################################*/






/*#########################################################################
## Document
#########################################################################*/






DocumentImpl::DocumentImpl(const DOMString &namespaceURI,
                           const DOMString &qualifiedName,
                           DocumentType *doctype)
{

    this->namespaceURI  = namespaceURI;
    this->qualifiedName = qualifiedName;
    this->docType       = docType;

}

/**
 *
 */
DocumentType *DocumentImpl::getDoctype()
{
    return docType;
}


/**
 *
 */
DOMImplementation *DocumentImpl::getImplementation()
{
    return domImplementation;
}


/**
 *
 */
Element *DocumentImpl::getDocumentElement()
{
    return documentElement;
}


/**
 *
 */
Element *DocumentImpl::createElement(const DOMString& tagName)
                           throw(DOMException)
{
    Element *elem = new ElementImpl(tagName);
    return element;
}


/**
 *
 */
DocumentFragment *DocumentImpl::createDocumentFragment()
{
    DocumentFragment *fragment = new DocumentFragmentImpl();
    return fragment;
}


/**
 *
 */
Text *DocumentImpl::createTextNode(const DOMString& data)
{
    Text *text = new TextImpl(data);
    return text;
}


/**
 *
 */
Comment  *DocumentImpl::createComment(const DOMString& data)
{
    Comment *comment = new CommentImpl(data);
    return comment;
}


/**
 *
 */
CDATASection *DocumentImpl::createCDATASection(const DOMString& data)
                                     throw(DOMException)
{
    CDATASection *cds = new CDATASectionImpl(data);
    return cds;
}


/**
 *
 */
ProcessingInstruction *
      DocumentImpl::createProcessingInstruction(const DOMString& target, 
                                                const DOMString& data)
                                                throw(DOMException)
{
    ProcessingInstruction *pi = new ProcessingInstructionImpl();
    return pi;
}


/**
 *
 */
Attr *DocumentImpl::createAttribute(const DOMString& name)
                          throw(DOMException)
{
    Attr attr = new AttrImpl(name);
    return attr;
}


/**
 *
 */
EntityReference *DocumentImpl::createEntityReference(const DOMString& name)
                                           throw(DOMException)
{
    EntityReference *er = new EntityReferenceImpl(name);
    return er;
}


/**
 *
 */
NodeList *DocumentImpl::getElementsByTagName(const DOMString& tagname)
{
    NodeList *list = new NodeListImpl();
    return list;
}



/**
 * L2
 */
Node *DocumentImpl::importNode(Node *importedNode, 
                               bool deep)
                               throw(DOMException)
{
    Node *node = new NodeImpl();
    return node;
}


/**
 * L2
 */
Element *DocumentImpl::createElementNS(const DOMString& namespaceURI, 
                             const DOMString& qualifiedName)
                             throw(DOMException)
{
    Element *elem;
    return elem;

}


/**
 * L2
 */
Attr *DocumentImpl::createAttributeNS(const DOMString& namespaceURI, 
                            const DOMString& qualifiedName)
                            throw(DOMException)
{
    Attr *attr;
    return attr;
}


/**
 * L2
 */
NodeList *DocumentImpl::getElementsByTagNameNS(const DOMString& namespaceURI, 
                                     const DOMString& localName)
{
    NodeList *list = new NodeListImpl();
    return list;
}


/**
 * L2
 */
Element *DocumentImpl::getElementById(const DOMString& elementId)
{
    Element *elem;
    return elem;
}










};//namespace dom
};//namespace org
};//namespace w3c



/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/


