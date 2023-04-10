#pragma once

#include <algorithm>
#include <memory>
#include <sstream>
 // homebrew vector

template<typename T, typename A = std::allocator<T>> // read "for all types T" (just like in math)
class vector {
public:
    using size_type = size_t;
    using value_type = T;
    using iterator = T*;
    using const_iterator = const T*;

    iterator begin() { return elem; };
    const_iterator begin() const { return elem; };
    iterator end() { return elem + sz; };
    const_iterator end() const { return elem + sz; };

    size_type size() const { return sz; };

    T& front() { return *elem; }
    T& back() { return *(elem + sz - 1); }

    vector()
        : sz{ 0 }, elem{ nullptr }, space{ 0 }
    {
    }

    vector(size_type s, T val)
        : sz{ s }, space{ s }, elem{ alloc.allocate(s) }
    {
        for (size_type i = 0; i < s; ++i)
            alloc.construct(&elem[i], val);      // initialize elements
    }

    vector(std::initializer_list<T> lst)
        : sz{ lst.size() }, space{ lst.size() }, elem{ alloc.allocate(lst.size()) }  // uninitialized memory for elements
    {
        auto it = lst.begin();
        for (size_type i = 0; i < lst.size(); ++i) {
            alloc.construct(&elem[i], *it);
            ++it;
        }
    }

    vector(const vector& arg)
    // allocate elements, then initialize them by copying
        : sz{ arg.sz }, space{ arg.sz }, elem{ alloc.allocate(arg.sz) }
    {
        auto it = arg.begin();
        for (size_type i = 0; i < arg.size(); ++i) {
            alloc.construct(&elem[i], *it);
            ++it;
        }
    }

    vector& operator=(const vector& a)
    {
        if (this == &a) return *this;       // self_assignment, no work needed

        std::unique_ptr<T[]> p{ alloc.allocate(a.sz) };       // allocate new space                    

        for (size_type i = 0; i < a.sz; ++i)     // copy elements
            alloc.construct(&p.get()[i], a.elem[i]);

        for (size_type i = 0; i < sz; ++i)  // deallocate old space
            alloc.destroy(&elem[i]);  

        alloc.deallocate(elem, space);
        elem = p.release();                           // now we can reset elem
        space = a.sz;
        sz = a.sz;
        return *this;
    }

    vector(vector&& a)
        : sz{ a.sz }, space{ a.space }, elem{ a.elem }    // copy a's elem and sz
    {
        a.sz = 0;
        a.space = 0;
        a.elem = nullptr;
    }

    vector& operator=(vector&& a)
    {
        if (this == &a) return *this;  // self assignment

        for (size_type i = 0; i < sz; ++i) alloc.destroy(&elem[i]);
        alloc.deallocate(space);      // deallocate old space
        elem = a.elem;                // copy a's elem and sz
        sz = a.sz;
        space = a.space;
        a.elem = nullptr;             // make a the empty vector
        a.sz = 0;
        a.space = 0;
        return *this;
    }

    ~vector()
    {
        for (size_type i = 0; i < sz; ++i)
            alloc.destroy(&elem[i]);
        alloc.deallocate(elem, space);
    }

    T& operator[](size_type n)
    {
        return elem[n];
    }

    const T& operator[](size_type n) const
    {
        return elem[n];
    }

    T& at(size_type n)
    {
        std::stringstream ss;
        ss << "Size = " << sz << ", Access = " << n;
        if (n < 0 || sz <= n)
            throw std::out_of_range(ss.str());
    }

    const T& at(size_type n) const
    {
        std::stringstream ss;
        ss << "Size = " << sz << ", Access = " << n;
        if (n < 0 || sz <= n)
            throw std::out_of_range(ss.str());
    }

    size_type capacity() const
    {
        return space;
    }

    void reserve(size_type newalloc)
    {
        if (newalloc <= space) return;      // never decrease allocation
        std::unique_ptr<T[]> p{ alloc.allocate(newalloc) };    // allocate new space
        for (size_type i = 0; i < sz; ++i) alloc.construct(&p[i], elem[i]);   // copy
        for (size_type i = 0; i < sz; ++i) alloc.destroy(&elem[i]);           // destroy
        alloc.deallocate(elem, space);      // deallocate old space
        elem = p.release();
        space = newalloc;
    }

    void resize(size_type newsize, T val)
    // make the vector have newsize elements
    // intitialize each new element with the default value
    {
        reserve(newsize);
        for (size_type i = sz; i < newsize; ++i) alloc.construct(&elem[i], val);  // construct
        for (size_type i = newsize; i < sz; ++i) alloc.destroy(&elem[i]);         // destroy
        sz = newsize;
    }

    void push_back(const T& val)
    // increase vector size by one; intialize the new element with d
    {
        if (space == 0)
            reserve(8);                 // start with space for 8 elements
        else if (sz == space)
            reserve(2 * space);         // get more space
        alloc.construct(&elem[sz], val);// add val at end
        ++sz;                           // increase the size (sz is the number of elements)
    }

    iterator erase(iterator p)
    {
        if (p == end()) return p;
        for (auto pos = p + 1; pos != end(); ++pos)
            *(pos - 1) = *pos;          // copy element "one position to the left"
        alloc.destroy(end() - 1);         // destroy surplus copy of last element
        --sz;
        return p;
    }

    iterator insert(iterator p, const T& val)
    {
        size_type index = p - begin();    // yielding amount of blocks of memory depending on type
        if (size() == capacity())
            reserve(size() == 0 ? 8 : 2 * size());      // make sure we have space

        // first copy last element into uninitializzed space:
        alloc.construct(elem + sz, back());

        ++sz;
        iterator pp = begin() + index;      // the place to put val
        for (auto pos = &back(); pos != pp; --pos)
            *pos = *(pos - 1);              // copy elements one position to the right
        *(begin() + index) = val;           // "insert" val
        return pp;
    }

private:
    A alloc;            // use allocate to handle memory for elements
    size_type sz;       // the size
    value_type* elem;   // pointer to the first element (of type T)
    size_type space;    // number of elements plus "free space" / "slots"
                         // for new elements ("the current allocation")
};

// Deklarasi
/*
template<typename T, typename A = std::allocator<T>> // read "for all types T" (just like in math)
class vector {
public:
    using size_type = size_t;
    using value_type = T;
    using iterator = T*;
    using const_iterator = const T*;

    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;

    iterator erase(iterator p);
    iterator insert(iterator p, const T& val);          // insert before

    size_type size() const;

    T& front();
    T& back();

    vector();                                           // default ctor
    explicit vector(size_type s, T val = T{});          // specifying size
    vector(std::initializer_list<T> lst);               // init-list ctor

    vector(const vector<T, A>& arg);                    // copy ctor
    vector<T, A>& operator=(const vector<T, A>& a);     // copy assignment

    vector(vector<T, A>&& a);                           // move constructor
    vector<T, A>& operator=(vector<T, A>&& a);          // move assignment

    ~vector();                                          // dtor

    T& operator[](size_type n);                         // overload subscript
    const T& operator[](size_type n) const;             // const counterpart of subscript

    T& at(size_type n);                                 // checked access
    const T& at(size_type n) const;                     // checked access

    void reserve(size_type newalloc);                   // configuring free space
    size_type capacity() const;
    void resize(size_type newsize, T val = T{});
    void push_back(const T& d);

private:
    A alloc;            // use allocate to handle memory for elements
    size_type sz;       // the size
    value_type* elem;   // pointer to the first element (of type T)
    size_type space;    // number of elements plus "free space" / "slots"
                         // for new elements ("the current allocation")
};
*/