/**
 * homebrew circular foward_list
 */

// JANGAN PAKAI RANGE-BASED FOR LOOP

#include <iostream>
#include <initializer_list>
#include <memory>

template<typename Elem>
struct Link {
    Link(const Elem& v, Link* s = nullptr)
        : val{ v }, succ{ s } { }

    Link* succ;     // successor (next) node
    Elem val;       // the value
};

template<typename Elem>
class cforward_list {
public:
    cforward_list()
        : sz{ 0 }, first{ alloc.allocate(1) }, last{ alloc.allocate(1) }
    {
        first->succ = last;
        last->succ = first;
    }

    cforward_list(std::initializer_list<Elem> lst)
        : cforward_list()
    {
        auto it = before_begin();
        for (const auto& x : lst) {
            insert_after(it, x);
            ++it;
        }
        sz = lst.size();
    }

    ~cforward_list()
    {
        clear();
        alloc.deallocate(first, 1);
        alloc.deallocate(last, 1);	// deallocate last
    }

    class iterator;

    iterator before_begin() { return iterator(first, first, last); }
    iterator begin() { return iterator(first->succ, first, last); }
    iterator end() { return iterator(last, first, last); }
    const iterator before_begin() const { return iterator(first, first, last); }
    const iterator begin() const { return iterator(first->succ, first, last); }
    const iterator end() const { return iterator(last, first, last); }

    iterator insert_after(iterator p, const Elem& v); // insert v into cforward_list after p
    iterator insert_before(iterator p, const Elem& v); // insert v into cforward_list before p
    iterator erase_after(iterator p);  // remove elem after p from the cforward_list
    iterator erase(iterator p);  // remove p from the cforward_list

    void push_back(const Elem& v);
    void push_front(const Elem& v);
    void pop_front();
    void pop_back();

    Elem& front();
    Elem& back();
    const Elem& front() const;
    const Elem& back() const;

    void clear();   // empty list

    int size() const { return sz; }

private:
    size_t sz;
    Link<Elem>* first;
    Link<Elem>* last;
    std::allocator<Link<Elem>> alloc;
};

template<typename Elem> // requires Element<Elem>() (§19.3.3)
class cforward_list<Elem>::iterator {
public:
    iterator(Link<Elem>* p, Link<Elem>* first, Link<Elem>* last)
        : curr{ p }, first{ first }, last{ last } { }

    // only overloaded for prefix increment
    iterator& operator++()  // forward
    {
        curr = curr->succ;
        if (curr == last)
            curr = curr->succ;
        if (curr == first)
            curr = curr->succ;
        return *this;
    }
    Elem& operator*() // get value (dereference)
    {
        if (curr == first || curr == last) // first and last are not to be accessed
            throw std::out_of_range("dereference beyond range");
        return curr->val;
    }
    const Elem& operator*() const // get value (dereference)
    {
        if (curr == first || curr == last) // first and last are not to be accessed
            throw std::out_of_range("dereference beyond range");
        return curr->val;
    }
    Link<Elem>* operator->() { return curr; }

    bool operator==(const iterator& b) const { return curr == b.curr; }
    bool operator!=(const iterator& b) const { return curr != b.curr; }
    explicit operator bool() const { return curr; }

    Link<Elem>* ptr() const { return curr; }

private:
    Link<Elem>* curr; // current link
    // storing first last to impose iterator check
    Link<Elem>* first;
    Link<Elem>* last;
};

// may throw access violation exception
template<typename Elem>
typename cforward_list<Elem>::iterator cforward_list<Elem>::insert_after(
    cforward_list<Elem>::iterator p, const Elem& v)
{
    if (p == end()) throw std::out_of_range("inserting after end()");

    std::unique_ptr<Link<Elem>> newLink{ alloc.allocate(1) };// allocate
    alloc.construct(newLink.get(), Link<Elem>(v));			// construct

    newLink->succ = p->succ;
    p->succ = newLink.get();

    ++sz;

    return iterator(newLink.release(), first, last);
}

template<typename Elem>
typename cforward_list<Elem>::iterator cforward_list<Elem>::insert_before(
    cforward_list<Elem>::iterator p, const Elem& v)
{
    if (p == before_begin()) throw std::out_of_range("inserting beyond before_begin()");

    auto it = before_begin();
    for (; it->succ != p.ptr(); ++it);  // iterate to elem before p
    return insert_after(it, v);
}

// may throw access violation exception
// invalidates erased element's iterator
template<typename Elem>
typename cforward_list<Elem>::iterator cforward_list<Elem>::erase_after(
    cforward_list<Elem>::iterator p)
{
    if (sz == 0) throw std::runtime_error("empty list");
    if (p == end()) throw std::out_of_range("attempting to erase after end()");
    if (p->succ == last) p = before_begin();

    auto temp = p->succ;    // store iterator to be erased
    p->succ = p->succ->succ;

    alloc.destroy(temp);
    alloc.deallocate(temp, 1);

    --sz;

    return iterator(p->succ, first, last);
}

// don't use before_begin as p
// invalidates erased element's iterator
template<typename Elem>
typename cforward_list<Elem>::iterator cforward_list<Elem>::erase(
    cforward_list<Elem>::iterator p)
{
    if (sz == 0) throw std::runtime_error("empty list");
    if (p == before_begin()) throw std::runtime_error("attempting to erase before_begin()");
    if (p == end()) throw std::runtime_error("attempting to erase end()");

    auto it = before_begin();
    for (; it->succ != p.ptr(); ++it);  // iterate to elem before p
    return erase_after(it);
}

template<typename Elem>
void cforward_list<Elem>::push_front(const Elem& v)
{
    insert_after(before_begin(), v);
}

template<typename Elem>
void cforward_list<Elem>::push_back(const Elem& v)
{
    auto it = before_begin();
    for (; it->succ != last; ++it);  // iterate to elem before last
    insert_after(it, v);
}

template<typename Elem>
void cforward_list<Elem>::pop_front()
{
    erase_after(before_begin());
}

template<typename Elem>
void cforward_list<Elem>::pop_back()
{
    if (sz == 0) throw std::runtime_error("empty list");

    auto it = before_begin();
    for (; it->succ->succ != last; ++it);  // iterate to 2 elems before last
    erase_after(it);
}

template<typename Elem>
Elem& cforward_list<Elem>::front()
{
    if (sz == 0) throw std::runtime_error("empty cforward_list");
    return begin()->val;
}

template<typename Elem>
const Elem& cforward_list<Elem>::front() const
{
    if (sz == 0) throw std::runtime_error("empty cforward_list");
    return begin()->val;
}

template<typename Elem>
Elem& cforward_list<Elem>::back()
{
    if (sz == 0) throw std::runtime_error("empty cforward_list");
    auto it = begin();
    for (; it->succ != last; ++it);  // iterate to elem before last

    return it->val;
}

template<typename Elem>
const Elem& cforward_list<Elem>::back() const
{
    if (sz == 0) throw std::runtime_error("empty cforward_list");
    auto it = begin();
    for (; it->succ != last; ++it);  // iterate to elem before last

    return it->val;
}

template<typename Elem>
void cforward_list<Elem>::clear()
{
    Link<Elem>* temp = nullptr; // storing p->succ, because after deleted, p->succ causes segfault
    for (Link<Elem>* p = begin().ptr(); p != last; p = temp) {
        temp = p->succ;
        alloc.destroy(p);
        alloc.deallocate(p, 1);
    }

    first->succ = last;
    sz = 0;
}

//=========================================================================================

template<typename Iterator> // requires Forward_iterator<Iterator>
void advance(Iterator& iter, int distance)
// move iterator
// no range check
// foward_list can't move back
{
    while (distance > 0) {
        ++iter;
        --distance;
    }
}
