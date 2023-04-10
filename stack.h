#pragma once

#include "list.h"
#include "forward_list.h"
#include "vector.h"

template<
    typename T,
    typename Container = vector<T>
> class stack {
public:
    // constructor
    stack(std::initializer_list<T> lst) : con(lst) { }
    stack() : con() { }
    stack(const stack& s) : con(s.con) { }
    stack(stack&& s) : con(std::move(s.con)) { }

    stack& operator=(const stack& s)
    {
        con = s.con;
        return *this;
    }

    stack& operator=(stack&& s)
    {
        con = std::move(s.con);
        return *this;
    }

    T& top() { return con.back(); }
    const T& top() const { return con.back(); }

    bool empty() const { return con.size(); }
    size_t size() const { return con.size(); }

    void push(const T& val) { con.push_back(val); }
    void pop() { con.pop_back(); }

    // Untuk traversal
    typename Container::iterator& begin() { return con.begin(); }
    typename Container::iterator& end() { return con.end(); }
    const typename Container::iterator& begin() const { return con.begin(); }
    const typename Container::iterator& end() const { return con.end(); }
private:
    Container con;
};

/* TESTING
using STACK = stack<std::string, forward_list<std::string>>;

stack<std::string, forward_list<std::string>> foo()
{
    stack<std::string, forward_list<std::string>> s{ "a", "b", "c", "d" };
    for (auto x : s)
        std::cout << x << "\t";
    return s;
}

int main()
try {
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    STACK s{ "a", "i", "u" };
    STACK s2(std::move(s));

    std::cout << "Contents of s\n";
    for (auto x : s)
        std::cout << x << "\t";
    std::cout << "\ns size = " << s.size() << "\n";

    std::cout << "Contents of s2\n";
    for (auto x : s2)
        std::cout << x << "\t";
    std::cout << "\ns2 size = " << s2.size() << "\n";
}
catch (std::exception& e) {
    std::cout << e.what() << std::endl;
    return 1;
}
*/