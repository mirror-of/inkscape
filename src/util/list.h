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
    typedef T const value_type;
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

    reference operator*() const { return _first(); }
    pointer operator->() const { return &_first(); }

    bool operator==(List const &other) const { return _is_equivalent(other); }
    bool operator!=(List const &other) const { return !_is_equivalent(other); }

    List &operator++() {
        _advance();
        return *this;
    }
    List operator++(int) {
        List old(*this);
        _advance();
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

    reference operator*() const { return const_cast<reference>(_first()); }
    pointer operator->() const { return const_cast<pointer>(&_first()); }

    List &operator++() {
        _advance();
        return *this;
    }
    List operator++(int) {
        List old(*this);
        _advance();
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

    reference operator*() const { return _first(); }
    pointer operator->() const { return &_first(); }

    bool operator==(List const &other) const { return _is_equivalent(other); }
    bool operator!=(List const &other) const { return !_is_equivalent(other); }

    List &operator++() {
        _advance();
        return *this;
    }
    List operator++(int) {
        List old(*this);
        _advance();
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
        _advance();
        return *this;
    }
    MutableList operator++(int) {
        MutableList old(*this);
        _advance();
        return old;
    }

    friend MutableList &rest<>(MutableList const &);
    friend MutableList const &set_rest<>(MutableList const &,
                                         MutableList const &);
};

template <typename T>
inline List<T> cons(typename Traits::Reference<T>::RValue value,
                    List<T> const &next)
{
    return List<T>(value, next);
}

template <typename T>
inline MutableList<T> cons(typename Traits::Reference<T>::RValue first,
                           MutableList<T> const &rest)
{
    return MutableList<T>(first, rest);
}

template <typename T>
inline typename List<T>::reference first(List<T> const &list) {
    return list._first();
}

template <typename T>
inline List<T> const &rest(List<T> const &list) {
    return static_cast<List<T> &>(list._rest());
}

template <typename T>
inline MutableList<T> &rest(MutableList<T> const &list) {
    return static_cast<MutableList<T> &>(list._rest());
}

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
