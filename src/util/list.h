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

template <typename T>
class ListBase {
public:
    typedef std::forward_iterator_tag iterator_category;
    typedef T const value_type;
    typedef std::ptrdiff_t difference_type;
    typedef typename Traits::Reference<value_type>::LValue reference;
    typedef typename Traits::Reference<value_type>::RValue const_reference;
    typedef typename Traits::Reference<value_type>::Pointer pointer;

    reference operator*() const { return _cell->value; }
    pointer operator->() const { return &_cell->value; }

    operator bool() const { return _cell != NULL; }

protected:
    ListBase() : _cell(NULL) {}
    ListBase(const_reference value, ListBase const &next)
    : _cell(new Cell(value, next._cell)) {}

    bool is_equal(ListBase const &other) const {
        return _cell == other._cell;
    }

    ListBase next() const { return ListBase(_cell->next); }
    ListBase setNext(ListBase const &next) {
        _cell->next = next._cell;
        return next;
    }

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
class List<T const> : public ListBase<T const> {
public:

    typedef T const value_type;
    typedef typename Traits::Reference<value_type>::LValue reference;
    typedef typename Traits::Reference<value_type>::RValue const_reference;
    typedef typename Traits::Reference<value_type>::Pointer pointer;

    List() : ListBase<T const>() {}
    explicit List(const_reference value, List const &next=List())
    : ListBase<T const>(value, next) {}

    bool operator==(List const &other) const {
        return is_equal(other);
    }
    bool operator!=(List const &other) const {
        return !is_equal(other);
    }

    List next() const { return List(ListBase<T const>::next()); }

    List &operator++() {
        return *this = next();
    }
    List operator++(int) {
        List old(*this);
        *this = next();
        return old;
    }

private:
    explicit List(ListBase<T const> const &list) : ListBase<T const>(list) {}
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

    reference operator*() const {
        return const_cast<reference>(List<T const>::operator*());
    }
    pointer operator->() const {
        return const_cast<pointer>(List<T const>::operator->());
    }

    List next() const { return List(List<T const>::next()); }

    List &operator++() {
        return *this = next();
    }
    List operator++(int) {
        List old(*this);
        *this = next();
        return old;
    }

private:
    explicit List(List<T const> const &list) : List<T const>(list) {}
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

    bool operator==(List const &other) const {
        return is_equal(other);
    }
    bool operator!=(List const &other) const {
        return !is_equal(other);
    }

    List next() const { return List(ListBase<T &>::next()); }

    List &operator++() {
        return *this = next();
    }
    List operator++(int) {
        List old(*this);
        *this = next();
        return old;
    }

private:
    explicit List(ListBase<T &> const &list) : ListBase<T &>(list) {}
};

template <typename T>
class MutableList : public List<T> {
public:
    MutableList() {}
    explicit MutableList(typename List<T>::const_reference value,
                         MutableList<T> const &next=MutableList<T>())
    : List<T>(value, next) {}

    MutableList<T> &operator++() {
        List<T>::operator++();
        return *this;
    }
    MutableList<T> operator++(int) {
        MutableList<T> old(*this);
        List<T>::operator++();
        return old;
    }

    MutableList<T> next() const {
        return MutableList<T>(List<T>::next());
    }
    MutableList<T> setNext(MutableList<T> const &next) {
        List<T>::setNext(next);
        return next;
    }

private:
    explicit MutableList(List<T> const &list) : List<T>(list) {}
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
