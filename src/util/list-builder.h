/*
 * Inkscape::Util::ListBuilder - STL-style container for building lists
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2004 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_INKSCAPE_UTIL_LIST_BUILDER_H
#define SEEN_INKSCAPE_UTIL_LIST_BUILDER_H

#include <iterator>
#include "traits/reference.h"
#include "util/list.h"

namespace Inkscape {

namespace Util {

template <typename T>
class ListBuilder : public GC::Managed<> {
public:
    // default constructible
    ListBuilder() : _head(), _tail() {}

    // assignable
    ListBuilder(ListBuilder<T> const &other) { *this = other; }

    ListBuilder<T> &operator=(ListBuilder<T> &other) {
        std::copy(other.begin(), other.end(),
                  std::back_insert_iterator<ListBuilder<T> >(*this));
    }

    void swap(ListBuilder<T> const &other) {
        MutableList<T> saved_head=_head;
        MutableList<T> saved_tail=_tail;
        _head = other._head;
        _tail = other._tail;
        other._head = saved_head;
        other._tail = saved_tail;
    }

    // container
    typedef T value_type;
    typedef List<T> iterator;
    typedef List<T const> const_iterator;
    typedef typename Traits::Reference<T>::LValue reference;
    typedef typename Traits::Reference<T>::RValue const_reference;
    typedef typename Traits::Reference<T>::Pointer pointer;
    typedef std::ptrdiff_t difference_type;
    typedef std::size_t size_type;

    iterator begin() { return _head; }
    const_iterator begin() const { return _head; }

    iterator end() { return List<T>(); }
    const_iterator end() const { return List<T>(); }

    size_type size() const { return std::distance(begin(), end()); }
    size_type max_size() const { return ~0; }
    bool empty() const { return begin() == end(); }

    // equality-comparable
    bool operator==(ListBuilder<T> const &other) const {
        iterator iter(begin());
        iterator other_iter(other.begin());
        while ( iter != end() && other_iter != other.end() ) {
            if ( *iter != *other_iter ) {
                return false;
            }
            ++iter;
            ++other_iter;
        }
        return iter == end() && other_iter == other.end();
    }
    bool operator!=(ListBuilder<T> const &other) const {
        return !operator==(other);
    }

    // less-than-comparable
    bool operator<(ListBuilder<T> const &other) const {
        return std::lexicographical_compare(begin(), end(),
                                            other.begin(), other.end());
    }
    bool operator>(ListBuilder<T> const &other) const {
        return other.operator<(*this);
    }
    bool operator<=(ListBuilder<T> const &other) const {
        return !operator>(other);
    }
    bool operator>=(ListBuilder<T> const &other) const {
        return !operator<(other);
    }

    // sequence
    ListBuilder(size_type count, const_reference value) {
        while (count--) {
            push_back(value);
        }
    }
    ListBuilder(size_type count) {
        while (count--) {
            push_back(value_type());
        }
    }
    template <typename InputIterator>
    ListBuilder(InputIterator start, InputIterator end) {
        std::copy(start, end,
                  std::back_insert_iterator<ListBuilder<T> >(*this));
    }

    reference front() { return *head; }
    const_reference front() const { return *head; }

    void insert(iterator pos, const_reference value) {
        MutableList<T> temp(value);
        _splice_before(static_cast<MutableList<T> &>(pos),
                       temp, temp);
    }
    void insert(iterator pos, size_type count, const_reference value) {
        if ( count > 0 ) {
            MutableList<T> &before(static_cast<MutableList<T> &>(before));
            ListBuilder<T> temp(count, value);
            MutableList<T> tail(temp._tail);
            _splice_before(before, temp.detach(before), tail);
        }
    }
    template <typename InputIterator>
    void insert(iterator pos, InputIterator start, InputIterator end) {
        if ( start != end ) {
            MutableList<T> &before(static_cast<MutableList<T> &>(before));
            ListBuilder<T> temp(start, end);
            MutableList<T> tail(temp._tail);
            _splice_before(before, temp.detach(before), tail);
        }
    }

    void erase(iterator pos) {
        erase(pos, end());
    }
    void erase(iterator start, iterator end) {
        _splice_before(static_cast<MutableList<T> &>(start),
                       static_cast<MutableList<T> &>(end),
                       static_cast<MutableList<T> &>(end));
    }
    
    void clear() { _head = _tail = MutableList<T>(); }

    void resize(size_type new_size, const_reference value) {
        size_type count(new_size);
        MutableList<T> iter(_head);
        while ( count > 0 && iter != end() ) {
            --count;
            ++iter;
        }
        if ( iter != end() ) {
            iter.setNext(MutableList<T>());
            _tail = iter;
        } else if ( count > 0 ) {
            ListBuilder<T> temp(count, value);
            MutableList<T> new_tail(temp._tail);
            iter.setNext(temp.detach().first);
            _tail = new_tail;
        } else {
            _head = _tail = MutableList<T>();
        }
    }
    void resize(size_type new_size) {
        resize(new_size, value_type());
    }

    // front-insertion-sequence
    void push_front(const_reference value) {
        _head = cons_mutable(value, _head);
        if ( _tail == end() ) {
            _tail = _head;
        }
    }
    void pop_front(const_reference value) {
        MutableList<T> saved(_head);
        ++_head;
        if ( _head == end() ) {
            _tail = MutableList<T>();
        }
        saved.setNext(end());
    }

    // back-insertion-sequence (incomplete)
    reference back() { return *_tail; }
    const_reference back() const { return *_tail; }
    void push_back(const_reference value) {
        if (_head) {
            tail.setNext(MutableList<T>(value));
            ++_tail;
        } else {
            _head = _tail = MutableList<T>(value);
        }
    }
    // we can't implement a constant-time pop_back()

    // list-builder-specific stuff
    MutableList<T> detach(MutableList<T> next=MutableList<T>()) {
        if (empty()) {
            return next;
        } else {
            MutableList<T> result(_head);
            _tail.setNext(next);
            clear();
            return result;
        }
    }

private:
    MutableList<T> _head;
    MutableList<T> _tail;

    MutableList<T> _splice_before(MutableList<T> before,
                                  MutableList<T> start,
                                  MutableList<T> tail)
    {
        if ( before == _head ) {
            _head = start;
        } else {
            MutableList<T> iter(_head);
            while ( iter.next() != before ) {
                ++iter;
            }
            iter.setNext(start);
        }
        if ( before == MutableList<T>() ) {
            _tail = tail;
        }
        return what;
    }
};

}

}


namespace std {

template <typename T>
inline
void swap(Inkscape::Util::ListBuilder<T> &a, Inkscape::Util::ListBuilder<T> &b)
{
    a.swap(b);
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
