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

    bool hasFeature(const DOMString& feature, const DOMString& version);

    DocumentType *createDocumentType(const DOMString& qualifiedName, 
                                     const DOMString& publicId, 
                                     const DOMString& systemId)
                                     throw(DOMException);

    Document *createDocument(const DOMString& namespaceURI, 
                             const DOMString& qualifiedName, 
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

    void setNodeValue(const DOMString& val) throw (DOMException);


    unsigned short getNodeType();


    Node *getParentNode();

    NodeList *getChildNodes();

    Node *getFirstChild();

    Node *getLastChild();


    Node *getPreviousSibling();

    Node *getNextSibling();


    NamedNodeMap *getAttributes();



    Document *getOwnerDocument();


    Node *insertBefore(const Node *newChild, 
                       const Node *refChild)
                       throw(DOMException);


    Node *replaceChild(const Node *newChild, 
                       const Node *oldChild)
                       throw(DOMException);


    Node *removeChild(const Node *oldChild)
                      throw(DOMException);


    Node *appendChild(const Node *newChild)
                      throw(DOMException);


    bool hasChildNodes();

    Node *cloneNode(bool deep);

    void normalize();

    bool isSupported(const DOMString& feature, 
                     const DOMString& version);

    DOMString getNamespaceURI();

    DOMString getPrefix();

    void setPrefix(const DOMString& val) throw(DOMException);

    DOMString getLocalName();

    bool hasAttributes();

    /* not in api */
    void setNodeName(const DOMString& val);


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
    bool insert(int position, const Node *newNode);

    bool insert(const Node *current, const Node *newNode);

    bool replace(const Node *current, const Node *newNode);

    bool remove(const Node *node);

    void append(const Node *newNode);

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

    Node *getNamedItem(const DOMString& name);

    Node *setNamedItem(const Node *arg) throw(DOMException);

    Node *removeNamedItem(const DOMString& name) throw(DOMException);

    Node *item(unsigned long index);

    unsigned long getLength();

    Node *getNamedItemNS(const DOMString& namespaceURI, 
                         const DOMString& localName);

    Node *setNamedItemNS(const Node *arg) throw(DOMException);

    Node *removeNamedItemNS(const DOMString& namespaceURI, 
                            const DOMString& localName)
                            throw(DOMException);

    private:

    struct MapComparator
        {
        bool operator()(const DOMString& a, const DOMString& b) const
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

    void setData(const DOMString& val) throw(DOMException);

    unsigned long getLength();

    DOMString substringData(unsigned long offset, 
                            unsigned long count)
                            throw(DOMException);

    void appendData(const DOMString& arg) throw(DOMException);

    void insertData(unsigned long offset, 
                    const DOMString& arg)
                    throw(DOMException);

    void deleteData(unsigned long offset, 
                    unsigned long count)
                    throw(DOMException);

    void  replaceData(unsigned long offset, 
                      unsigned long count, 
                      const DOMString& arg)
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

    void setValue(const DOMString& val) throw(DOMException);

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

    DOMString getAttribute(const DOMString& name);

    void setAttribute(const DOMString& name, 
                      const DOMString& value)
                      throw(DOMException);

    void removeAttribute(const DOMString& name)
                         throw(DOMException);

    Attr *getAttributeNode(const DOMString& name);

    Attr *setAttributeNode(Attr *newAttr)
                          throw(DOMException);

    Attr *removeAttributeNode(Attr *oldAttr)
                             throw(DOMException);

    NodeList *getElementsByTagName(const DOMString& name);


    DOMString getAttributeNS(const DOMString& namespaceURI, 
                             const DOMString& localName);

    void setAttributeNS(const DOMString& namespaceURI, 
                        const DOMString& qualifiedName, 
                        const DOMString& value)
                        throw(DOMException);


    void removeAttributeNS(const DOMString& namespaceURI, 
                           const DOMString& localName)
                           throw(DOMException);
 

    Attr *getAttributeNodeNS(const DOMString& namespaceURI, 
                            const DOMString& localName);

    Attr *setAttributeNodeNS(Attr *newAttr)
                            throw(DOMException);

    NodeList *getElementsByTagNameNS(const DOMString& namespaceURI, 
                                    const DOMString& localName);

    bool hasAttribute(const DOMString& name);

    bool hasAttributeNS(const DOMString& namespaceURI, 
                        const DOMString& localName);
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

    DocumentTypeImpl(const DOMString& qualifiedName, 
                     const DOMString& publicId,
                     const DOMString& systemId);

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

    void setData(const DOMString& val) throw(DOMException);

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

    DocumentImpl(const DOMString &namespaceURI,
                 const DOMString &qualifiedName,
                 const DocumentType *doctype);

    virtual ~DocumentImpl()
        {}

    DocumentType *DocumentImpl::getDoctype();

    DOMImplementation *DocumentImpl::getImplementation();

    Element *DocumentImpl::getDocumentElement();

    Element *DocumentImpl::createElement(const DOMString& tagName) throw(DOMException);

    DocumentFragment *DocumentImpl::createDocumentFragment();

    Text *DocumentImpl::createTextNode(const DOMString& data);

    Comment  *DocumentImpl::createComment(const DOMString& data);

    CDATASection *DocumentImpl::createCDATASection(const DOMString& data)
                                     throw(DOMException);

    ProcessingInstruction *DocumentImpl::createProcessingInstruction(const DOMString& target, 
                                                       const DOMString& data)
                                                       throw(DOMException);


    Attr *DocumentImpl::createAttribute(const DOMString& name)
                          throw(DOMException);


    EntityReference *DocumentImpl::createEntityReference(const DOMString& name)
                                           throw(DOMException);


    NodeList *DocumentImpl::getElementsByTagName(const DOMString& tagname);

    Node *DocumentImpl::importNode(const Node *importedNode, bool deep)
                                 throw(DOMException);

    Element *DocumentImpl::createElementNS(const DOMString& namespaceURI, 
                             const DOMString& qualifiedName)
                             throw(DOMException);

    Attr *DocumentImpl::createAttributeNS(const DOMString& namespaceURI, 
                            const DOMString& qualifiedName)
                            throw(DOMException);

    NodeList *DocumentImpl::getElementsByTagNameNS(const DOMString& namespaceURI, 
                                     const DOMString& localName);

    Element *DocumentImpl::getElementById(const DOMString& elementId);

};





};//namespace dom
};//namespace org
};//namespace w3c


#endif // __DOMIMPL_H__


/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/




