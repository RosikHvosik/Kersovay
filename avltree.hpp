/*#ifndef AVL_TREE_H
#define AVL_TREE_H

#include <string>
#include <utility>
#include "linkedlist.h"

template<typename T, typename ArrayType>
struct AVLNode {
    std::size_t key;
    linkedList indexList; // список индексов в массиве
    int height;
    AVLNode* left;
    AVLNode* right;

    AVLNode(std::size_t key_)
        : key(key_), height(1), left(nullptr), right(nullptr) {}
};

template<typename T, typename ArrayType>
class AVLTree {
private:
    using Node = AVLNode<T, ArrayType>;
    Node* root;

    Node* insert(Node* node, std::size_t key, std::size_t arrayIndex);
    Node* remove(Node* node, std::size_t key, const T& value); // опционально

    int getHeight(Node* node);
    int getBalance(Node* node);
    void updateHeight(Node* node);
    Node* rotateLeft(Node* x);
    Node* rotateRight(Node* y);
    Node* balance(Node* node);

    Node* findNode(Node* node, std::size_t key);
    void clear(Node* node);

    std::size_t stringToKey(const std::string& str) const;

public:
    AVLTree();
    ~AVLTree();

    void removeAll();

    bool insert(const std::string& keyStr, const T& value, ArrayType& array);
    bool remove(const std::string& keyStr, const T& value, ArrayType& array); // опционально

    Node* find(const std::string& keyStr);
    Node* getRoot() const;
    void clear();
    bool isEmpty() const;
    std::pair<Node*, bool> findAppointment(const std::string& keyStr,
                                            const std::string& doctor,
                                            const std::string& diagnosis,
                                            const Date& date);

    void fixIndex(std::size_t oldIdx, std::size_t newIdx);

    //void fixIndex(std::size_t oldIdx, std::size_t newIdx);
};
#endif // AVL_TREE_H
*/
