#pragma once

#include <initializer_list>
#include <memory>
#include <dLink.h>

/**
 * Implementasi linked list dilakukan dengan mengalokasikan 2 uninitialized
 * elemens yang datanya tidak akan diakses, hanya untuk penanda saja
 */

//==============================================================================


template<typename Elem>
class list {
public:
    list()
        : sz{ 0 }, first{ alloc.allocate(1) }, last{ alloc.allocate(1) }
    {
        first->succ = last;
        first->prev = nullptr;
        last->prev = first;
        last->succ = nullptr;
    }

    list(std::initializer_list<Elem> lst)
        : list()	// delegate to default ctor
    {
        for (const auto& x : lst)
            push_back(x);
    }

    list(const list& l)
        : list()
    {
        for (const auto& x : l)
            push_back(x);
    }

    list(list&& l)
        : list()
    {
        // steal representation excluding fl.first and fl.last
        sz = l.sz;
        first->succ = l.first->succ;
        l.first->succ->prev = first;
        last->prev = l.last->prev;
        l.last->prev->succ = last;

        // set l's first and last to connect to each other
        l.first->succ = l.last;
        l.last->prev = l.first;
        l.sz = 0;
    }

    ~list()
    {
        clear();
        alloc.deallocate(first, 1); // deallocate first
        alloc.deallocate(last, 1);	// deallocate last
    }

    list& operator=(const list& l)
    {
        if (this == &l) return *this;  // assignment to self

        clear();
        for (const auto& x : l)
            push_back(x);

        return *this;
    }

    list& operator=(list&& l)
    {
        if (this == &l) return *this;  // assignment to self

        // clear current content excluding first and last
        clear();

        // steal representation excluding fl.first and fl.last
        sz = l.sz;
        first->succ = l.first->succ;
        l.first->succ->prev = first;
        last->prev = l.last->prev;
        l.last->prev->succ = last;

        // set l's first and last to connect to each other
        l.first->succ = l.last;
        l.last->prev = l.first;
        l.sz = 0;

        return *this;
    }

    class iterator; // member type: iterator

    iterator begin() { return iterator(first->succ, first, last); } // iterator to first element
    iterator end() { return iterator(last, first, last); } // iterator to one beyond last element
    const iterator begin() const { return iterator(first->succ, first, last); }
    const iterator end() const { return iterator(last, first, last); }

    iterator insert(iterator p, const Elem& v); // insert v into list before p
    iterator erase(iterator p); // remove p from the list

    void push_back(const Elem& v); // insert v at end
    void push_front(const Elem& v); // insert v at front
    void pop_front(); // remove the first element
    void pop_back(); // remove the last element

    Elem& front(); // the first element
    Elem& back(); // the last element
    const Elem& front() const;
    const Elem& back() const;

    void clear();       // empty list

    int size() const { return sz; }

private:
    size_t sz;
    dLink<Elem>* first;	// one elem before range
    dLink<Elem>* last;	// one elem beyond range
    std::allocator<dLink<Elem>> alloc;
};

template<typename Elem> // requires Element<Elem>() (§19.3.3)
class list<Elem>::iterator {
public:
    iterator(dLink<Elem>* p, dLink<Elem>* first, dLink<Elem>* last)
        : curr{ p }, first{ first }, last{ last } { }

    // only overloaded for prefix decrement and increment
    iterator& operator++()  // forward
    {
        if (curr == last)
            throw std::out_of_range("increment beyond end()");
        curr = curr->succ;
        return *this;
    }
    iterator& operator--() // backward
    {
        if (curr->prev == first)
            throw std::out_of_range("decrement beyond begin()");
        curr = curr->prev;
        return *this;
    }
    Elem& operator*() // get value (dereference)
    {
        if (curr == first || curr == last)  // first and last are not to be accessed
            throw std::out_of_range("dereference beyond range");
        return curr->val;
    }
    const Elem& operator*() const // get value (dereference)
    {
        if (curr == first || curr == last)  // first and last are not to be accessed
            throw std::out_of_range("dereference beyond range");
        return curr->val;
    }

    dLink<Elem>* operator->() { return curr; }

    bool operator==(const iterator& b) const { return curr == b.curr; }
    bool operator!=(const iterator& b) const { return curr != b.curr; }
    explicit operator bool() const { return curr; }

    dLink<Elem>* ptr() const { return curr; }

private:
    dLink<Elem>* curr; // current link
    // storing first last to impose iterator check
    dLink<Elem>* first;
    dLink<Elem>* last;
};

template<typename Elem>
typename list<Elem>::iterator list<Elem>::insert(list<Elem>::iterator p,
    const Elem& v)
{
    if (p.ptr() == first) throw std::out_of_range("attempting to insert before first");

    std::unique_ptr<dLink<Elem>> newLink{ alloc.allocate(1) };// allocate
    alloc.construct(newLink.get(), dLink<Elem>(v));			// construct

    newLink->succ = p.ptr();
    newLink->prev = p->prev;
    p->prev->succ = newLink.get();
    p->prev = newLink.get();

    ++sz;

    return iterator(newLink.release(), first, last);
}

template<typename Elem>
typename list<Elem>::iterator list<Elem>::erase(list<Elem>::iterator p)
{
    if (sz == 0) throw std::runtime_error("empty list");   // empty list
    // trying to erase end()
    if (p == end()) throw std::out_of_range("attempting to erase end()");
    // trying to erase first
    if (p.ptr() == first) throw std::out_of_range("attempting to erase before begin()");

    p->succ->prev = p->prev;
    p->prev->succ = p->succ;

    auto it = p->succ;		// iterator to be returned

    alloc.destroy(p.ptr());
    alloc.deallocate(p.ptr(), 1);
    --sz;

    return iterator(it, first, last);   // return value after p prior to removal
}

template<typename Elem>
void list<Elem>::push_back(const Elem& v)
{
    insert(end(), v);
}

template<typename Elem>
void list<Elem>::push_front(const Elem& v)
{
    insert(begin(), v);
}

template<typename Elem>
void list<Elem>::pop_back()
{
    iterator p{ last->prev, first, last };
    erase(p);
}

template<typename Elem>
void list<Elem>::pop_front()
{
    erase(begin());
}

template<typename Elem>
Elem& list<Elem>::front()
{
    if (sz == 0) throw std::runtime_error("empty list");
    return begin()->val;
}

template<typename Elem>
Elem& list<Elem>::back()
{
    if (sz == 0) throw std::runtime_error("empty list");
    return end()->prev->val;
}

template<typename Elem>
const Elem& list<Elem>::front() const
{
    if (sz == 0) throw std::runtime_error("empty list");
    return begin()->val;
}

template<typename Elem>
const Elem& list<Elem>::back() const
{
    if (sz == 0) throw std::runtime_error("empty list");
    return end()->prev->val;
}

template<typename Elem>
void list<Elem>::clear()
{
    dLink<Elem>* temp = nullptr; // storing p->succ, because after deleted, p->succ causes segfault
    for (dLink<Elem>* p = begin().ptr(); p != last; p = temp) {
        temp = p->succ;
        alloc.destroy(p);
        alloc.deallocate(p, 1);
    }

    // link first and last
    first->succ = last;
    last->prev = first;
    sz = 0;
}

//==============================================================================

template<typename Iterator>
void doubly_advance(Iterator& iter, int distance)
// move iterator
{
    while (distance > 0) {
        ++iter;
        --distance;
    }

    while (distance < 0) {
        --iter;
        ++distance;
    }
}
//==============================================================================
