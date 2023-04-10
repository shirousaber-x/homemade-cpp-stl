#include <iostream>
#include <initializer_list>
#include <memory>

// JANGAN PAKAI RANGED-BASED FOR LOOP

template<typename Elem>
struct Link {
    Link(const Elem& v, Link* p = nullptr, Link* s = nullptr)
        : val{ v }, prev{ p }, succ{ s } { }

    Link* prev;     // previous node
    Link* succ;     // successor (next) node
    Elem val;       // the value
};

template<typename Elem>
class CirList {
public:
    CirList()
        : sz{ 0 }, first{ alloc.allocate(1) }, last{ alloc.allocate(1) }
    {
        first->succ = last;
        first->prev = last;
        last->prev = first;
        last->succ = first;
    }

    CirList(std::initializer_list<Elem> lst)
        : CirList()	// delegate to default ctor
    {
        for (const auto& x : lst)
            push_back(x);
    }

    ~CirList()
    {
        clear();
        alloc.deallocate(first, 1); // deallocate first
        alloc.deallocate(last, 1);	// deallocate last
    }

    class iterator; // member type: iterator

    iterator begin() { return iterator(first->succ, first, last); } // iterator to first element
    iterator end() { return iterator(last, first, last); } // iterator to one beyond last element
    const iterator begin() const { return iterator(first->succ, first, last); }
    const iterator end() const { return iterator(last, first, last); }

    iterator insert(iterator p, const Elem& v); // insert v into CirList before p
    iterator erase(iterator p); // remove p from the CirList

    void push_back(const Elem& v); // insert v at end
    void push_front(const Elem& v); // insert v at front
    void pop_front(); // remove the first element
    void pop_back(); // remove the last element

    Elem& front(); // the first element
    Elem& back(); // the last element
    const Elem& front() const;
    const Elem& back() const;

    void clear();       // empty CirList

    int size() const { return sz; }

private:
    size_t sz;
    Link<Elem>* first;	// one elem before range
    Link<Elem>* last;	// one elem beyond range
    std::allocator<Link<Elem>> alloc;
};

template<typename Elem> // requires Element<Elem>() (§19.3.3)
class CirList<Elem>::iterator {
public:
    iterator(Link<Elem>* p, Link<Elem>* first, Link<Elem>* last)
        : curr{ p }, first{ first }, last{ last } { }

    // only overloaded for prefix decrement and increment
    iterator& operator++()  // forward
    {
        curr = curr->succ;
        if (curr == last)
            curr = curr->succ;
        if (curr == first)
            curr = curr->succ;
        return *this;
    }
    iterator& operator--() // backward
    {
        curr = curr->prev;
        if (curr == first)
            curr = curr->prev;
        if (curr == last)
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

template<typename Elem>
typename CirList<Elem>::iterator CirList<Elem>::insert(CirList<Elem>::iterator p,
    const Elem& v)
{
    if (p.ptr() == first) throw std::out_of_range("attempting to insert before first");

    std::unique_ptr<Link<Elem>> newLink{ alloc.allocate(1) };// allocate
    alloc.construct(newLink.get(), Link<Elem>(v));			// construct

    newLink->succ = p.ptr();
    newLink->prev = p->prev;
    p->prev->succ = newLink.get();
    p->prev = newLink.get();

    ++sz;

    return iterator(newLink.release(), first, last);
}

template<typename Elem>
typename CirList<Elem>::iterator CirList<Elem>::erase(CirList<Elem>::iterator p)
{
    if (sz == 0) throw std::runtime_error("empty CirList");   // empty CirList
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
void CirList<Elem>::push_back(const Elem& v)
{
    insert(end(), v);
}

template<typename Elem>
void CirList<Elem>::push_front(const Elem& v)
{
    insert(begin(), v);
}

template<typename Elem>
void CirList<Elem>::pop_back()
{
    iterator p{ last->prev, first, last };
    erase(p);
}

template<typename Elem>
void CirList<Elem>::pop_front()
{
    erase(begin());
}

template<typename Elem>
Elem& CirList<Elem>::front()
{
    if (sz == 0) throw std::runtime_error("empty CirList");
    return begin()->val;
}

template<typename Elem>
Elem& CirList<Elem>::back()
{
    if (sz == 0) throw std::runtime_error("empty CirList");
    return end()->prev->val;
}

template<typename Elem>
const Elem& CirList<Elem>::front() const
{
    if (sz == 0) throw std::runtime_error("empty CirList");
    return begin()->val;
}

template<typename Elem>
const Elem& CirList<Elem>::back() const
{
    if (sz == 0) throw std::runtime_error("empty CirList");
    return end()->prev->val;
}

template<typename Elem>
void CirList<Elem>::clear()
{
    Link<Elem>* temp = nullptr; // storing p->succ, because after deleted, p->succ causes segfault
    for (Link<Elem>* p = begin().ptr(); p != last; p = temp) {
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

template<typename Iterator> // requires Bidirectional_iterator<Iterator>
void advance(Iterator& iter, int distance)
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
