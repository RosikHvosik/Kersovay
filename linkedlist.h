#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <string>
#include "types.h"
struct lNode
{
    std::size_t arrayIndex;
    lNode *next;

    lNode(std::size_t idx) : arrayIndex(idx), next(nullptr) {}
};

class linkedList
{
    lNode *head;
    lNode *last;
    int size;

public:
    linkedList();
    ~linkedList();

    bool isEmpty() const;
    void add(std::size_t arrayIndex);
    std::string show();

    int searchByAppointment(const std::string &doctor,
                            const std::string &diagnosis,
                            const Date &date);
    void removeAt(int index);

    lNode *getHead() const { return head; }
    int getSize() const;

    template <typename TreeType>
    void updateIndices(std::size_t oldIdx, std::size_t newIdx, TreeType &tree);
};

#endif
