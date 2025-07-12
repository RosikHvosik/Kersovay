#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include "avltree3.h"
#include "array.h"

template<typename KeyType, typename T, typename ArrayType>
class DataManager {
private:
    ArrayType array;
    AVLTree<KeyType, T, ArrayType> tree;

public:
    // Вставка записи
    bool insert(const KeyType& key, const T& value);

    // Обход записей
    void traverse(std::function<void(const T&, const KeyType&)> callback) const;

    // Доступ к массиву
    const ArrayType& getArray() const { return array; }
    ArrayType& getArray() { return array; }

    // Доступ к дереву
    const AVLTree<KeyType, T, ArrayType>& getTree() const { return tree; }
    AVLTree<KeyType, T, ArrayType>& getTree() { return tree; }
};



// === Реализация методов ===

template<typename KeyType, typename T, typename ArrayType>
bool DataManager<KeyType, T, ArrayType>::insert(const KeyType& key, const T& value) {
    if (!array.Add(value))
        return false;

    size_t index = array.Size() - 1;
    return tree.insertIndex(key, index);
}

template<typename KeyType, typename T, typename ArrayType>
void DataManager<KeyType, T, ArrayType>::traverse(std::function<void(const T&, const KeyType&)> callback) const {
    tree.traverse([&](size_t idx, const KeyType& key) {
        callback(array[idx], key);
    });
}


#endif
