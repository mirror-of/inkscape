/*
 * Inkscape::Util::List - managed linked list
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_UTIL_LIST_H
#define SEEN_INKSCAPE_UTIL_LIST_H

#include <cstddef>
#include <iterator>
#include "gc-managed.h"
#include "traits/reference.h"

namespace Inkscape {

namespace Util {

template <typename T> class List;
template <typename T> class MutableList;

template <typename T>
typename List<T>::reference first(List<T> const &list);

template <typename T>
List<T> const &rest(List<T> const &list);

template <typename T>
MutableList<T> &rest(MutableList<T> const &list);

template <typename T>
MutableList<T> const &set_rest(MutableList<T> const &list,
                               MutableList<T> const &rest);

template <typename T>
class ListBase {
public:
    typedef std::forward_iterator_tag iterator_category;
    typedef T value_type;
    typedef std::ptrdiff_t difference_type;
    typedef typename Traits::Reference<value_type>::LValue reference;
    typedef typename Traits::Reference<value_type>::RValue const_reference;
    typedef typename Traits::Reference<value_type>::Pointer pointer;

    operator bool() const { return _cell != NULL; }

protected:
    ListBase() : _cell(NULL) {}
    ListBase(const_reference value, ListBase const &next)
    : _cell(new Cell(value, next._cell)) {}

    bool _is_equivalent(ListBase const &other) const {
        return _cell == other._cell;
    }

    reference _first() const { return _cell->value; }
    ListBase &_rest() const {
        return reinterpret_cast<ListBase &>(_cell->next);
    }
    void _advance() { _cell = _cell->next; }

private:
    struct Cell : public GC::Managed<> {
        Cell() {}
        Cell(const_reference v, Cell *n) : value(v), next(n) {}

        T value;
        Cell *next;
    };

    Cell *_cell;

    explicit ListBase(Cell *cell) : _cell(cell) {}
};

template <typename T> class List;

template <typename T>
class List<T const> : public ListBase<T> {
public:
    typedef T const value_type;
    typedef typename Traits::Reference<value_type>::LValue reference;
    typedef typename Traits::Reference<value_type>::RValue const_reference;
    typedef typename Traits::Reference<value_type>::Pointer pointer;

    List() : ListBase<T>() {}
    explicit List(const_reference value, List const &next=List())
    : ListBase<T>(value, next) {}

    reference operator*() const { return this->_first(); }
    pointer operator->() const { return &this->_first(); }

    bool operator==(List const &other) const {
        return this->_is_equivalent(other);
    }
    bool operator!=(List const &other) const {
        return !this->_is_equivalent(other);
    }

    List &operator++() {
        this->_advance();
        return *this;
    }
    List operator++(int) {
        List old(*this);
        this->_advance();
        return old;
    }

    friend reference first<>(List const &);
    friend List const &rest<>(List const &);
};

template <typename T>
class List : public List<T const> {
public:
    typedef T value_type;
    typedef typename Traits::Reference<value_type>::LValue reference;
    typedef typename Traits::Reference<value_type>::RValue const_reference;
    typedef typename Traits::Reference<value_type>::Pointer pointer;

    List() : List<T const>() {}
    explicit List(const_reference value, List const &next=List())
    : List<T const>(value, next) {}

    reference operator*() const { return this->_first(); }
    pointer operator->() const { return &this->_first(); }

    List &operator++() {
        this->_advance();
        return *this;
    }
    List operator++(int) {
        List old(*this);
        this->_advance();
        return old;
    }

    friend reference first<>(List const &);
    friend List const &rest<>(List const &);
};

template <typename T>
class List<T &> : public ListBase<T &> {
public:
    typedef T &value_type;
    typedef typename Traits::Reference<value_type>::LValue reference;
    typedef typename Traits::Reference<value_type>::RValue const_reference;
    typedef typename Traits::Reference<value_type>::Pointer pointer;

    List() : ListBase<T &>() {}
    List(const_reference value, List const &next=List())
    : ListBase<T &>(value, next) {}

    reference operator*() const { return this->_first(); }
    pointer operator->() const { return &this->_first(); }

    bool operator==(List const &other) const {
        return this->_is_equivalent(other);
    }
    bool operator!=(List const &other) const {
        return !this->_is_equivalent(other);
    }

    List &operator++() {
        this->_advance();
        return *this;
    }
    List operator++(int) {
        List old(*this);
        this->_advance();
        return old;
    }

    friend reference first<>(List const &);
    friend List const &rest<>(List const &);
};

template <typename T>
class MutableList : public List<T> {
public:
    MutableList() {}
    explicit MutableList(typename List<T>::const_reference value,
                         MutableList const &next=MutableList())
    : List<T>(value, next) {}

    MutableList &operator++() {
        this->_advance();
        return *this;
    }
    MutableList operator++(int) {
        MutableList old(*this);
        this->_advance();
        return old;
    }

    friend MutableList &rest<>(MutableList const &);
    friend MutableList const &set_rest<>(MutableList const &,
                                         MutableList const &);
};

/** @brief Creates a (non-empty) linked list.
 * 
 * Creates a new linked list with a copy of the given value ('first');
 * the remainder of the list will be the list provided as 'rest'.
 *
 * The remainder of the list -- the "tail" -- is incorporated by
 * reference, not by value, so changes to that list will affect this one.
 *
 * The copied value is managed by the garbage collector as non-finalized
 * memory; if it has a non-trivial destructor, that destructor will not
 * be automatically called when the list is destroyed.
 *
 * cons() is synonymous with List<T>(first, rest), except that the
 * compiler will usually be able to infer T from the type of 'rest'
 *
 * If you need to create an empty list, call the List<> constructor
 * with no arguments, like so:
 *
 *  List<int>()
 *
 * @see List<>
 * @see is_empty<>
 *
 * @param first the value for the first element of the list
 * @param rest the rest of the list; may be an empty list
 *
 * @returns a new list
 *
 */
template <typename T>
inline List<T> cons(typename Traits::Reference<T>::RValue first,
                    List<T> const &rest)
{
    return List<T>(first, rest);
}

/** @brief Creates a (non-empty) linked list whose tail can be exchanged
 *         for another.
 *
 * Creates a new linked list, but one whose tail can be exchanged for
 * another later by using set_rest() or assignment through rest()
 * as an lvalue.
 *
 * This form of cons() is synonymous with MutableList<T>(first, rest),
 * except that the compiler can usually infer the T from the type of
 * 'rest'.
 *
 * As with List<>, you can create an empty list like so:
 *
 *  MutableList<int>()
 *
 * @see MutableList<>
 * @see is_empty<>
 *
 * @param first the value for the first element of the list
 * @param rest the rest of the list; may be an empty list
 *
 * @returns a new list
 */
template <typename T>
inline MutableList<T> cons(typename Traits::Reference<T>::RValue first,
                           MutableList<T> const &rest)
{
    return MutableList<T>(first, rest);
}

/** @brief Returns true if the given list is empty.
 *
 * Returns true if the given list is empty.  This is equivalent
 * to !list.
 *
 * @param list the list
 *
 * @returns true if the list is empty, false otherwise.
 */
template <typename T>
inline bool is_empty(List<T> const &list) { return !list; }

/** @brief Returns the first value in a linked list.
 *
 * Returns a reference to the first value in the list.  This
 * corresponds to the value of the first argument passed to cons().
 *
 * If the list holds mutable values (or references to them), first()
 * can be used as an lvalue.
 *
 * For example:
 * 
 *  first(list) = value;
 *
 * The results of calling this on an empty list are undefined.
 *
 * @see cons<>
 * @see is_empty<>
 *
 * @param list the list; cannot be empty
 *
 * @returns a reference to the first value in the list
 */
template <typename T>
inline typename List<T>::reference first(List<T> const &list) {
    return list._first();
}

/** @brief Returns the remainder of a linked list after the first element.
 *
 * Returns the remainder of the list after the first element (its "tail").
 *
 * This will be the same as the second argument passed to cons().
 *
 * The results of calling this on an empty list are undefined.
 *
 * @see cons<>
 * @see is_empty<>
 *
 * @param list the list; cannot be empty
 *
 * @returns the remainder of the list
 */
template <typename T>
inline List<T> const &rest(List<T> const &list) {
    return static_cast<List<T> &>(list._rest());
}

/** @brief Returns a reference to the remainder of a linked list after
 *         the first element.
 *
 * Returns a reference to the remainder of the list after the first
 * element (its "tail").  For MutableList<>, rest() can be used as
 * an lvalue, to set a new tail.
 *
 * For example:
 *
 *  rest(list) = other;
 *
 * Results of calling this on an empty list are undefined.
 *
 * @see cons<>
 * @see is_empty<>
 *
 * @param list the list; cannot be empty
 *
 * @returns a reference to the remainder of the list
 */
template <typename T>
inline MutableList<T> &rest(MutableList<T> const &list) {
    return static_cast<MutableList<T> &>(list._rest());
}

/** @brief Sets a new tail for an existing linked list.
 * 
 * Sets the tail of the given MutableList<>, corresponding to the
 * second argument of cons().
 *
 * Results of calling this on an empty list are undefined.
 *
 * @see rest<>
 * @see cons<>
 * @see is_empty<>
 *
 * @param list the list; cannot be empty
 * @param rest the new tail; corresponds to the second argument of cons()
 *
 * @returns the new tail
 */
template <typename T>
inline MutableList<T> const &set_rest(MutableList<T> const &list,
                                      MutableList<T> const &rest)
{
    return static_cast<MutableList<T> &>(list._rest()) = rest;
}

}

}

#endif
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
