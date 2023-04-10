/**
 * homebrew foward_list
 */

#include <iostream>
#include <initializer_list>
#include <memory>

template<typename Elem>
struct sLink {
    sLink(const Elem& v, sLink* s = nullptr)
        : val{ v }, succ{ s } { }

    sLink* succ;     // successor (next) node
    Elem val;       // the value
};

template<typename Elem>
class forward_list {
public:
    forward_list()
        : sz{ 0 }, first{ alloc.allocate(1) }, last{ alloc.allocate(1) }
    {
        first->succ = last;
        last->succ = nullptr;
    }

    forward_list(std::initializer_list<Elem> lst)
        : forward_list()
    {
        auto it = before_begin();
        for (const auto& x : lst) {
            insert_after(it, x);
            ++it;
        }
        sz = lst.size();
    }

    forward_list(const forward_list& fl)
        : forward_list()
    {
        // iterate fl and copy the values
        iterator iter{ before_begin() };
        for (auto i = fl.first->succ; i != fl.last; i = i->succ) {
            iter = insert_after(iter, i->val);
        }
    }

    forward_list(forward_list&& fl)
        : first{ fl.first }, last{ fl.last }, sz{ fl.sz }
    {
        // give fl new representation
        fl.first = alloc.allocate(1);
        fl.last = alloc.allocate(1);
        fl.first->succ = fl.last;
        fl.last->succ = nullptr;
        fl.sz = 0;
    }

    ~forward_list()
    {
        clear();
        alloc.deallocate(first, 1);
        alloc.deallocate(last, 1);	// deallocate last
    }

    forward_list& operator=(const forward_list& fl)
    {
        if (this == &fl) return *this;  // assignment to self

        clear();
        // iterate fl and copy the values
        iterator iter{ before_begin() };
        for (auto i = fl.first->succ; i != fl.last; i = i->succ) {
            iter = insert_after(iter, i->val);
        }

        return *this;
    }

    forward_list& operator=(forward_list&& fl)
    {
        if (this == &fl) return *this;  // assignment to self

        // clear all contents including first and last
        clear();
        alloc.deallocate(first,1);
        alloc.deallocate(last,1);
        
        // steal representation
        first = fl.first;
        last = fl.last;
        sz = fl.sz;

        // give fl new representation
        fl.first = alloc.allocate(1);
        fl.last = alloc.allocate(1);
        fl.first->succ = fl.last;
        fl.last->succ = nullptr;
        fl.sz = 0;

        return *this;
    }

    class iterator;

    iterator before_begin() { return iterator(first, first, last); }
    iterator begin() { return iterator(first->succ, first, last); }
    iterator end() { return iterator(last, first, last); }
    const iterator before_begin() const { return iterator(first, first, last); }
    const iterator begin() const { return iterator(first->succ, first, last); }
    const iterator end() const { return iterator(last, first, last); }

    iterator insert_after(iterator p, const Elem& v); // insert v into forward_list after p
    iterator insert_before(iterator p, const Elem& v); // insert v into forward_list before p
    iterator erase_after(iterator p);  // remove elem after p from the forward_list
    iterator erase(iterator p);  // remove p from the forward_list

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
    sLink<Elem>* first;
    sLink<Elem>* last;
    std::allocator<sLink<Elem>> alloc;
};

template<typename Elem> // requires Element<Elem>() (§19.3.3)
class forward_list<Elem>::iterator {
public:
    iterator(sLink<Elem>* p, sLink<Elem>* first, sLink<Elem>* last)
        : curr{ p }, first{ first }, last{ last } { }

    // only overloaded for prefix increment
    iterator& operator++()  // forward
    {
        if (curr == last)
            throw std::out_of_range("increment beyond end()");
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
    sLink<Elem>* operator->() { return curr; }

    bool operator==(const iterator& b) const { return curr == b.curr; }
    bool operator!=(const iterator& b) const { return curr != b.curr; }
    explicit operator bool() const { return curr; }

    sLink<Elem>* ptr() const { return curr; }

private:
    sLink<Elem>* curr; // current link
    // storing first last to impose iterator check
    sLink<Elem>* first;
    sLink<Elem>* last;
};

// may throw access violation exception
template<typename Elem>
typename forward_list<Elem>::iterator forward_list<Elem>::insert_after(
    forward_list<Elem>::iterator p, const Elem& v)
{
    if (p == end()) throw std::out_of_range("inserting beyond end()");

    std::unique_ptr<sLink<Elem>> newLink{ alloc.allocate(1) };// allocate
    alloc.construct(newLink.get(), sLink<Elem>(v));			// construct

    newLink->succ = p->succ;
    p->succ = newLink.get();

    ++sz;

    return iterator(newLink.release(), first, last);
}

template<typename Elem>
typename forward_list<Elem>::iterator forward_list<Elem>::insert_before(
    forward_list<Elem>::iterator p, const Elem& v)
{
    if (p == before_begin()) throw std::out_of_range("inserting beyond before_begin()");

    auto it = before_begin();
    for (; it->succ != p.ptr(); ++it);  // iterate to elem before p
    return insert_after(it, v);
}

// may throw access violation exception
// invalidates erased element's iterator
template<typename Elem>
typename forward_list<Elem>::iterator forward_list<Elem>::erase_after(
    forward_list<Elem>::iterator p)
{
    if (sz == 0) throw std::runtime_error("empty list");
    if (p->succ == last) throw std::out_of_range("attempting to erase end()");
    if (p == end()) throw std::out_of_range("attempting to erase after end()");

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
typename forward_list<Elem>::iterator forward_list<Elem>::erase(
    forward_list<Elem>::iterator p)
{
    if (sz == 0) throw std::runtime_error("empty list");
    if (p == before_begin()) throw std::out_of_range("attempting to erase before_begin()");
    if (p == end()) throw std::out_of_range("attempting to erase end()");

    auto it = before_begin();
    for (; it->succ != p.ptr(); ++it);  // iterate to elem before p
    return erase_after(it);
}

template<typename Elem>
void forward_list<Elem>::push_front(const Elem& v)
{
    insert_after(before_begin(), v);
}

template<typename Elem>
void forward_list<Elem>::push_back(const Elem& v)
{
    auto it = before_begin();
    for (; it->succ != last; ++it);  // iterate to elem before last
    insert_after(it, v);
}

template<typename Elem>
void forward_list<Elem>::pop_front()
{
    erase_after(before_begin());
}

template<typename Elem>
void forward_list<Elem>::pop_back()
{
    if (sz == 0) throw std::runtime_error("empty list");

    auto it = before_begin();
    for (; it->succ->succ != last; ++it);  // iterate to 2 elems before last
    erase_after(it);
}

template<typename Elem>
Elem& forward_list<Elem>::front()
{
    if (sz == 0) throw std::runtime_error("empty forward_list");
    return begin()->val;
}

template<typename Elem>
const Elem& forward_list<Elem>::front() const
{
    if (sz == 0) throw std::runtime_error("empty forward_list");
    return begin()->val;
}

template<typename Elem>
Elem& forward_list<Elem>::back()
{
    if (sz == 0) throw std::runtime_error("empty forward_list");
    auto it = begin();
    for (; it->succ != last; ++it);  // iterate to elem before last

    return it->val;
}

template<typename Elem>
const Elem& forward_list<Elem>::back() const
{
    if (sz == 0) throw std::runtime_error("empty forward_list");
    auto it = begin();
    for (; it->succ != last; ++it);  // iterate to elem before last

    return it->val;
}

template<typename Elem>
void forward_list<Elem>::clear()
{
    sLink<Elem>* temp = nullptr; // storing p->succ, because after deleted, p->succ causes segfault
    for (sLink<Elem>* p = begin().ptr(); p != last; p = temp) {
        temp = p->succ;
        alloc.destroy(p);
        alloc.deallocate(p, 1);
    }

    first->succ = last;
    sz = 0;
}

//=========================================================================================

template<typename Iterator> // requires Forward_iterator<Iterator>
void singly_advance(Iterator& iter, int distance)
// move iterator
// no range check
// foward_list can't move back
{
    while (distance > 0) {
        ++iter;
        --distance;
    }
}
