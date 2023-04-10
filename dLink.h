#pragma once

// Link base for doubly linked list
template<typename Elem>
struct dLink {
    dLink(const Elem& v, dLink* p = nullptr, dLink* s = nullptr)
        : val{ v }, prev{ p }, succ{ s } { }

    dLink* prev;     // previous node
    dLink* succ;     // successor (next) node
    Elem val;       // the value
};