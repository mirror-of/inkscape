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

#ifndef __DOMIMPL_H__
#define __DOMIMPL_H__

#include "svg.h"

#include <map>
#include <vector>



namespace w3c
{
namespace org
{
namespace dom
{


/*#########################################################################
## I M P L E M E N T A T I O N    C L A S S    D E C L A R A T I O N S
#########################################################################*/

class DOMImplementationImpl : public DOMImplementation
{
    public:

    DOMImplementationImpl()
        {}

    virtual ~DOMImplementationImpl()
        {}

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


class NodeImpl : virtual public Node
{
    public:

    NodeImpl();

    virtual ~NodeImpl();

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

    /* not in api */
    void setNodeName(DOMString& val);


    protected:

    DOMString nodeName;

    DOMString nodeValue;

    NodeList *childNodes;

    unsigned short nodeType;

    Node *parentNode;

    Node *previousSibling;

    Node *nextSibling;

    NamedNodeMap *attributes;

    Document *ownerDocument;

    DOMString namespaceURI;

    DOMString prefix;

};


class NodeListImpl : public NodeList
{
    public:

    NodeListImpl()
        {}

    virtual ~NodeListImpl()
        {}

    Node *item(unsigned long index);

    unsigned long getLength();

    /*
    not part of the api.  visible to other default Impls
    */
    bool insert(int position, Node *newNode);

    bool insert(Node *current, Node *newNode);

    bool replace(Node *current, Node *newNode);

    bool remove(Node *node);

    void append(Node *newNode);

    private:

    std::vector<Node *> nodes;

};


class NamedNodeMapImpl : public NamedNodeMap
{
    public:

    NamedNodeMapImpl()
        {}

    virtual ~NamedNodeMapImpl()
        {}

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

    private:

    struct MapComparator
        {
        bool operator()(DOMString& a, DOMString& b) const
            { return a < b ; }
        };

    std::map<DOMString, Node *, MapComparator>table;

};


class CharacterDataImpl : public CharacterData , public NodeImpl
{
    public:

    CharacterDataImpl()
        {}

    virtual ~CharacterDataImpl()
        {}

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



class AttrImpl : public Attr, public NodeImpl
{
    public:

    AttrImpl()
        {}

    virtual ~AttrImpl()
        {}

    DOMString getName();

    bool getSpecified();

    DOMString getValue();

    void setValue(DOMString& val) throw(DOMException);

    Element *getOwnerElement();

    /* not in api */

    Element *ownerElement;

    protected:


};


class ElementImpl : public Element, public NodeImpl
{
    public:

    ElementImpl()
        {}

    virtual ~ElementImpl()
        {}

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



class TextImpl : public Text, public CharacterDataImpl
{
    public:

    TextImpl()
        {}

    virtual ~TextImpl()
        {}

    Text *splitText(unsigned long offset) throw(DOMException);
};


class CommentImpl : public Comment, public CharacterDataImpl
{
    public:

    CommentImpl()
        {}

    virtual ~CommentImpl()
        {}

};


class CDATASectionImpl : public CDATASection, public TextImpl
{
    public:

    CDATASectionImpl()
        {}

    virtual ~CDATASectionImpl()
        {}

};

class DocumentTypeImpl : public NodeImpl, public DocumentType
{
    public:

    DocumentTypeImpl()
        {}

    virtual ~DocumentTypeImpl()
        {}

    DocumentTypeImpl(DOMString& qualifiedName, 
                     DOMString& publicId, DOMString& systemId);

    DOMString getName();


    NamedNodeMap *getEntities();


    NamedNodeMap *getNotations();


    DOMString getPublicId();


    DOMString getSystemId();

    DOMString getInternalSubset();

};


class NotationImpl : public Notation, public NodeImpl
{
    public:

    NotationImpl()
        {}

    virtual ~NotationImpl()
        {}

    DOMString getPublicId();

    DOMString getSystemId();

};

class EntityImpl : public Entity, public NodeImpl
{
    public:

    EntityImpl()
        {}

    virtual ~EntityImpl()
        {}

    DOMString getPublicId();

    DOMString getSystemId();

    DOMString getNotationName();

};


class EntityReferenceImpl : public EntityReference, public NodeImpl
{
    public:

    EntityReferenceImpl()
        {}

    virtual ~EntityReferenceImpl()
        {}

};


class ProcessingInstructionImpl : public ProcessingInstruction, public NodeImpl
{
    public:

    ProcessingInstructionImpl()
        {}

    virtual ~ProcessingInstructionImpl()
        {}

    DOMString getTarget();

    DOMString getData();

    void setData(DOMString& val) throw(DOMException);

    private:

    DOMString piData;

    DOMString target;
};



class DocumentFragmentImpl : public DocumentFragment
{
    public:

    DocumentFragmentImpl()
        {}

    virtual ~DocumentFragmentImpl()
        {}

};



class DocumentImpl : public Document, public NodeImpl
{
    public:

    DocumentImpl()
        {}

    DocumentImpl(DOMString &namespaceURI,DOMString &qualifiedName,
                   DocumentType *doctype);

    virtual ~DocumentImpl()
        {}

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





};//namespace dom
};//namespace org
};//namespace w3c


#endif // __DOMIMPL_H__


/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/




