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

#ifndef __DOM_H__
#define __DOM_H__

#include <string>
#include <exception>

namespace w3c
{
namespace org
{
namespace dom
{

/**
 *
 */
typedef std::string DOMString;

/**
 *
 */
typedef unsigned long long DOMTimeStamp;

class DocumentType;
class Document;
class NodeList;
class NamedNodeMap;
class Element;



/*#########################################################################
## DOMException
#########################################################################*/
/**
 *  This is the only non-interface class
 */
class DOMException : public std::exception
{

    public:

    DOMException()
        {}

    DOMException(short theCode)

        {
        code = theCode;
        }

    ~DOMException() throw()
       {}

    /**
     *
     */
    unsigned short code;

    /**
     * Get a string, translated from the code.
     * Like std::exception. Not in spec.
     */
    const char *what() const throw();
    


};//class DOMException




/**
 * ExceptionCode
 */
enum
{
        INDEX_SIZE_ERR                 = 1,
        DOMSTRING_SIZE_ERR             = 2,
        HIERARCHY_REQUEST_ERR          = 3,
        WRONG_DOCUMENT_ERR             = 4,
        INVALID_CHARACTER_ERR          = 5,
        NO_DATA_ALLOWED_ERR            = 6,
        NO_MODIFICATION_ALLOWED_ERR    = 7,
        NOT_FOUND_ERR                  = 8,
        NOT_SUPPORTED_ERR              = 9,
        INUSE_ATTRIBUTE_ERR            = 10,
        // Introduced in DOM Level 2:
        INVALID_STATE_ERR              = 11,
        // Introduced in DOM Level 2:
        SYNTAX_ERR                     = 12,
        // Introduced in DOM Level 2:
        INVALID_MODIFICATION_ERR       = 13,
        // Introduced in DOM Level 2:
        NAMESPACE_ERR                  = 14,
        // Introduced in DOM Level 2:
        INVALID_ACCESS_ERR             = 15
};


/*#########################################################################
## DOMImplementation
#########################################################################*/
/**
 *
 */
class DOMImplementation
{
    public:

    /**
     *
     */
    virtual bool hasFeature(DOMString& feature, DOMString& version) = 0;


    /**
     * L2
     */
    virtual DocumentType *createDocumentType(DOMString& qualifiedName, 
                                     DOMString& publicId, 
                                     DOMString& systemId)
                                     throw(DOMException) = 0;

    /**
     * L2
     */
    virtual Document *createDocument(DOMString& namespaceURI, 
                             DOMString& qualifiedName, 
                             DocumentType *doctype)
                             throw(DOMException) = 0;


};//class DOMImplementation




/*#########################################################################
## Node
#########################################################################*/

/**
 *
 */
class Node
{
    public:

    // NodeType
    enum
        {
        ELEMENT_NODE                   = 1,
        ATTRIBUTE_NODE                 = 2,
        TEXT_NODE                      = 3,
        CDATA_SECTION_NODE             = 4,
        ENTITY_REFERENCE_NODE          = 5,
        ENTITY_NODE                    = 6,
        PROCESSING_INSTRUCTION_NODE    = 7,
        COMMENT_NODE                   = 8,
        DOCUMENT_NODE                  = 9,
        DOCUMENT_TYPE_NODE             = 10,
        DOCUMENT_FRAGMENT_NODE         = 11,
        NOTATION_NODE                  = 12
        };


    /**
     *
     */
    virtual DOMString getNodeName() = 0;

    /**
     *
     */
    virtual DOMString getNodeValue() throw (DOMException) = 0;

    /**
     *
     */
    virtual void setNodeValue(DOMString& val) throw (DOMException) = 0;

    /**
     *
     */
    virtual unsigned short getNodeType() = 0;

    /**
     *
     */
    virtual Node *getParentNode() = 0;

    /**
     *
     */
    virtual NodeList *getChildNodes() = 0;

    /**
     *
     */
    virtual Node *getFirstChild() = 0;

    /**
     *
     */
    virtual Node *getLastChild() = 0;

    /**
     *
     */
    virtual Node *getPreviousSibling() = 0;

    /**
     *
     */
    virtual Node *getNextSibling() = 0;

    /**
     *
     */
    virtual NamedNodeMap *getAttributes() = 0;


    /**
     * L2
     */
    virtual Document *getOwnerDocument() = 0;

    /**
     *
     */
    virtual Node *insertBefore(Node *newChild, 
                       Node *refChild)
                       throw(DOMException) = 0;

    /**
     *
     */
    virtual Node *replaceChild(Node *newChild, 
                       Node *oldChild)
                       throw(DOMException) = 0;

    /**
     *
     */
    virtual Node *removeChild(Node *oldChild)
                      throw(DOMException) = 0;

    /**
     *
     */
    virtual Node *appendChild(Node *newChild)
                      throw(DOMException) = 0;

    /**
     *
     */
    virtual bool hasChildNodes() = 0;

    /**
     *
     */
    virtual Node *cloneNode(bool deep) = 0;

    /**
     * L2
     */
    virtual void normalize() = 0;

    /**
     * L2
     */
    virtual bool isSupported(DOMString& feature, 
                     DOMString& version) = 0;

    /**
     * L2
     */
    virtual DOMString getNamespaceURI() = 0;

    /**
     * L2
     */
    virtual DOMString getPrefix() = 0;

    /**
     *
     */
    virtual void setPrefix(DOMString& val) throw(DOMException) = 0;

    /**
     * L2
     */
    virtual DOMString getLocalName() = 0;

    /**
     * L2
     */
    virtual bool hasAttributes() = 0;

};//class Node



/*#########################################################################
## NodeList
#########################################################################*/

/**
 *
 */
class NodeList
{
    public:

    /**
     *
     */
    virtual Node *item(unsigned long index) = 0;

    /**
     *
     */
    virtual unsigned long getLength() = 0;

};//class NodeList




/*#########################################################################
## NamedNodeMap
#########################################################################*/
/**
 *
 */
class NamedNodeMap
{
    public:

    /**
     *
     */
    virtual Node *getNamedItem(DOMString& name) = 0;

    /**
     *
     */
    virtual Node *setNamedItem(Node *arg) throw(DOMException) = 0;


    /**
     *
     */
    virtual Node *removeNamedItem(DOMString& name) throw(DOMException) = 0;

    /**
     *
     */
    virtual Node *item(unsigned long index) = 0;

    /**
     *
     */
    virtual unsigned long getLength() = 0;

    /**
     * L2
     */
    virtual Node *getNamedItemNS(DOMString& namespaceURI, 
                         DOMString& localName) = 0;

    /**
     * L2
     */
    virtual Node *setNamedItemNS(Node *arg) throw(DOMException) = 0;

    /**
     * L2
     */
    virtual Node *removeNamedItemNS(DOMString& namespaceURI, 
                            DOMString& localName)
                            throw(DOMException) = 0;
};//class NamedNodeMap




/*#########################################################################
## CharacterData
#########################################################################*/

/**
 *
 */
class CharacterData : virtual public Node
{
    public:

    /**
     *
     */
    virtual DOMString getData() throw(DOMException) = 0;

    /**
     *
     */
    virtual void setData(DOMString& val) throw(DOMException) = 0;

    /**
     *
     */
    virtual unsigned long getLength() = 0;

    /**
     *
     */
    virtual DOMString substringData(unsigned long offset, 
                            unsigned long count)
                            throw(DOMException) = 0;

    /**
     *
     */
    virtual void appendData(DOMString& arg) throw(DOMException) = 0;

    /**
     *
     */
    virtual void insertData(unsigned long offset, 
                    DOMString& arg)
                    throw(DOMException) = 0;

    /**
     *
     */
    virtual void deleteData(unsigned long offset, 
                    unsigned long count)
                    throw(DOMException) = 0;

    /**
     *
     */
    virtual void  replaceData(unsigned long offset, 
                      unsigned long count, 
                      DOMString& arg)
                      throw(DOMException) = 0;


};//class CharacterData





/*#########################################################################
## Attr
#########################################################################*/

/**
 *
 */
class Attr : virtual public Node
{
    public:

    /**
     *
     */
    virtual DOMString getName() = 0;

    /**
     *
     */
    virtual bool getSpecified() = 0;

    /**
     *
     */
    virtual DOMString getValue() = 0;

    /**
     *
     */
    virtual void setValue(DOMString& val) throw(DOMException) = 0;
                                        


    /**
     * L2
     */
    virtual Element *getOwnerElement() = 0;

};//class Attr





/*#########################################################################
## Element
#########################################################################*/

/**
 *
 */
class Element : virtual public Node
{
    public:

    /**
     *
     */
    virtual DOMString getTagName() = 0;

    /**
     *
     */
    virtual DOMString getAttribute(DOMString& name) = 0;

    /**
     *
     */
    virtual void setAttribute(DOMString& name, 
                      DOMString& value)
                      throw(DOMException) = 0;

    /**
     *
     */
    virtual void removeAttribute(DOMString& name)
                         throw(DOMException) = 0;

    /**
     *
     */
    virtual Attr *getAttributeNode(DOMString& name) = 0;

    /**
     *
     */
    virtual Attr *setAttributeNode(Attr *newAttr)
                          throw(DOMException) = 0;

    /**
     *
     */
    virtual Attr *removeAttributeNode(Attr *oldAttr)
                             throw(DOMException) = 0;

    /**
     *
     */
    virtual NodeList *getElementsByTagName(DOMString& name) = 0;

    /**
     * L2
     */
    virtual DOMString getAttributeNS(DOMString& namespaceURI, 
                             DOMString& localName) = 0;

    /**
     * L2
     */
    virtual void setAttributeNS(DOMString& namespaceURI, 
                        DOMString& qualifiedName, 
                        DOMString& value)
                        throw(DOMException) = 0;

    /**
     * L2
     */
    virtual void removeAttributeNS(DOMString& namespaceURI, 
                           DOMString& localName)
                           throw(DOMException) = 0;
 
    /**
     * L2
     */
    virtual Attr *getAttributeNodeNS(DOMString& namespaceURI, 
                            DOMString& localName) = 0;

    /**
     * L2
     */
    virtual Attr *setAttributeNodeNS(Attr *newAttr)
                            throw(DOMException) = 0;

    /**
     * L2
     */
    virtual NodeList *getElementsByTagNameNS(DOMString& namespaceURI, 
                                    DOMString& localName) = 0;

    /**
     * L2
     */
    virtual bool hasAttribute(DOMString& name) = 0;

    /**
     * L2
     */
    virtual bool hasAttributeNS(DOMString& namespaceURI, 
                        DOMString& localName) = 0;
};//class Element





/*#########################################################################
## Text
#########################################################################*/

/**
 *
 */
class Text : virtual public CharacterData
{
    public:

    /**
     *
     */
    virtual Text *splitText(unsigned long offset)
                    throw(DOMException) = 0;
};//class Text




/*#########################################################################
## Comment
#########################################################################*/

/**
 *
 */
class Comment : virtual public CharacterData
{
};//class Comment





/*#########################################################################
## CDATASection
#########################################################################*/
/**
 *
 */
class CDATASection : virtual public Text
{
};//class CDATASection




/*#########################################################################
## DocumentType
#########################################################################*/

/**
 *
 */
class DocumentType : virtual public Node
{
    public:

    /**
     *
     */
    virtual DOMString getName() = 0;

    /**
     *
     */
    virtual NamedNodeMap *getEntities() = 0;

    /**
     *
     */
    virtual NamedNodeMap *getNotations() = 0;

    /**
     *
     */
    virtual DOMString getPublicId() = 0;

    /**
     * L2
     */
    virtual DOMString getSystemId() = 0;

    /**
     * L2
     */
    virtual DOMString getInternalSubset() = 0;

};//class DocumentType





/*#########################################################################
## Notation
#########################################################################*/

/**
 *
 */
class Notation : virtual public Node
{
    public:

    /**
     *
     */
    virtual DOMString getPublicId() = 0;

    /**
     *
     */
    virtual DOMString getSystemId() = 0;

};//class Notation






/*#########################################################################
## Entity
#########################################################################*/

/**
 *
 */
class Entity : virtual public Node
{
    public:

    /**
     *
     */
    virtual DOMString getPublicId() = 0;

    /**
     *
     */
    virtual DOMString getSystemId() = 0;

    /**
     *
     */
    virtual DOMString getNotationName() = 0;

};//class Entity





/*#########################################################################
## EntityReference
#########################################################################*/
/**
 *
 */
class EntityReference : virtual public Node
{
};//class EntityReference





/*#########################################################################
## ProcessingInstruction
#########################################################################*/

/**
 *
 */
class ProcessingInstruction : virtual public Node
{
    public:

    /**
     *
     */
    virtual DOMString getTarget() = 0;

    /**
     *
     */
    virtual DOMString getData() = 0;

    /**
     *
     */
   virtual  void setData(DOMString& val) throw(DOMException) = 0;

};//class ProcessingInstruction





/*#########################################################################
## DocumentFragment
#########################################################################*/
/**
 *
 */
class DocumentFragment : virtual public Node
{
};//class DocumentFragment






/*#########################################################################
## Document
#########################################################################*/

/**
 *
 */
class Document : virtual public Node
{
    public:

    /**
     *
     */
    virtual DocumentType *getDoctype() = 0;

    /**
     *
     */
    virtual DOMImplementation *getImplementation() = 0;

    /**
     *
     */
    virtual Element *getDocumentElement() = 0;

    /**
     *
     */
    virtual Element *createElement(DOMString& tagName)
                           throw(DOMException) = 0;

    /**
     *
     */
    virtual DocumentFragment *createDocumentFragment() = 0;

    /**
     *
     */
    virtual Text *createTextNode(DOMString& data) = 0;

    /**
     *
     */
    virtual Comment  *createComment(DOMString& data) = 0;

    /**
     *
     */
    virtual CDATASection *createCDATASection(DOMString& data)
                                     throw(DOMException) = 0;

    /**
     *
     */
    virtual ProcessingInstruction *createProcessingInstruction(DOMString& target, 
                                                       DOMString& data)
                                                       throw(DOMException) = 0;

    /**
     *
     */
    virtual Attr *createAttribute(DOMString& name)
                          throw(DOMException) = 0;

    /**
     *
     */
    virtual EntityReference *createEntityReference(DOMString& name)
                                           throw(DOMException) = 0;

    /**
     *
     */
    virtual NodeList *getElementsByTagName(DOMString& tagname) = 0;


    /**
     * L2
     */
    virtual Node *importNode(Node *importedNode, 
                     bool deep)
                     throw(DOMException) = 0;

    /**
     * L2
     */
    virtual Element *createElementNS(DOMString& namespaceURI, 
                             DOMString& qualifiedName)
                             throw(DOMException) = 0;

    /**
     * L2
     */
    virtual Attr *createAttributeNS(DOMString& namespaceURI, 
                            DOMString& qualifiedName)
                            throw(DOMException) = 0;

    /**
     * L2
     */
    virtual NodeList *getElementsByTagNameNS(DOMString& namespaceURI, 
                                     DOMString& localName) = 0;

    /**
     * L2
     */
    virtual Element *getElementById(DOMString& elementId) = 0;


};//class Document











};//namespace dom
};//namespace org
};//namespace w3c


#endif // __DOM_H__


/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/




