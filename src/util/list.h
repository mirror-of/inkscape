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

template <typename T> class MutableList;

template <typename T>
class List : public List<T const> {
public:
    typedef T value_type;
    typedef typename Traits::Reference<value_type>::LValue reference;
    typedef typename Traits::Reference<value_type>::RValue const_reference;
    typedef typename Traits::Reference<value_type>::Pointer pointer;

    List() : List<T const>() {}
    explicit List(const_reference value,
                  List<T> const &next=List<T>())
    : List<T const>(value, next) {}

    reference operator*() const { return _cell->value; }
    pointer operator->() const { return &_cell->value; }

    List<value_type> &operator++() {
        _cell = _cell->next;
        return *this;
    }
    List<value_type> operator++(int) {
        List<value_type> old(*this);
        _cell = _cell->next;
        return old;
    }

    List<value_type> next() const {
        List<value_type> next(*this);
        return ++next;
    }
};

template <typename T>
class List<T const> {
public:
    typedef std::forward_iterator_tag iterator_category;
    typedef T const value_type;
    typedef std::ptrdiff_t difference_type;
    typedef typename Traits::Reference<value_type>::LValue reference;
    typedef typename Traits::Reference<value_type>::RValue const_reference;
    typedef typename Traits::Reference<value_type>::Pointer pointer;

    List() : _cell(NULL) {}
    explicit List(const_reference value,
                  List<value_type> const &next=List<value_type>())
    : _cell(new Cell(value, next._cell)) {}

    reference operator*() const { return _cell->value; }
    pointer operator->() const { return &_cell->value; }

    bool operator==(List<value_type> const &other) const {
        return _cell == other._cell;
    }
    bool operator!=(List<value_type> const &other) const {
        return _cell != other._cell;
    }

    List<value_type> &operator++() {
        _cell = _cell->next;
        return *this;
    }
    List<value_type> operator++(int) {
        List<value_type> old(*this);
        _cell = _cell->next;
        return old;
    }

    operator bool() const { return _cell != NULL; }

    List<value_type> next() const {
        List<value_type> next(*this);
        return ++next;
    }

protected:
    struct Cell : public GC::Managed<> {
        Cell() {}
        Cell(const_reference v, Cell *n) : value(v), next(n) {}

        T value;
        Cell *next;
    };

    Cell *_cell;
};

template <typename T>
class MutableList : public List<T> {
public:
    MutableList() {}
    explicit MutableList(typename List<T>::const_reference value,
                         MutableList<T> const &next=MutableList<T>())
    : List<T>(value, next) {}

    MutableList<T> &operator++() {
        _cell = _cell->next;
        return *this;
    }
    MutableList<T> operator++(int) {
        MutableList<T> old(*this);
        _cell = _cell->next;
        return old;
    }

    MutableList<T> next() const {
        MutableList<T> next(*this);
        return ++next;
    }
    MutableList<T> setNext(MutableList<T> next) {
        _cell->next = next._cell;
        return next;
    }
};

template <typename T>
List<T> cons(typename Traits::Reference<T>::RValue value, List<T> const &next)
{
    return List<T>(value, next);
}

template <typename T>
MutableList<T> cons(typename Traits::Reference<T>::RValue value,
                    MutableList<T> const &next)
{
    return MutableList<T>(value, next);
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
