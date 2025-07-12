/*#include "avltree.hpp"
#include <QDebug>
#include <cctype>     // для std::isdigit
#include <algorithm>  // для std::max
#include "array.h"

// Конструктор
template<typename T, typename ArrayType>
AVLTree<T, ArrayType>::AVLTree() : root(nullptr) {}

// Деструктор
template<typename T, typename ArrayType>
AVLTree<T, ArrayType>::~AVLTree() {
    clear();
}
template<typename T, typename ArrayType>
bool AVLTree<T, ArrayType>::insert(const std::string& keyStr, const T& value, ArrayType& array) {
    std::size_t key = stringToKey(keyStr);

    // ищем узел с таким ключом
    Node* existingNode = findNode(root, key);
    if (existingNode) {
        if (!array.Add(value)) {
            qDebug().noquote() << "→ массив полон";
            return false;
        }

        std::size_t index = array.Size() - 1;
        existingNode->indexList.add(index);
        qDebug().noquote() << QString("→ добавлено в существующий узел, индекс = %1").arg(index);
        return true;
    }

    // узел не найден — создаём новый
    if (!array.Add(value)) {
        qDebug().noquote() << "→ массив полон";
        return false;
    }

    std::size_t index = array.Size() - 1;
    Node* newNode = new Node(key);
    newNode->indexList.add(index);

    root = insert(root, key, index); // внутренняя вставка + балансировка

    qDebug().noquote() << QString("→ создан новый узел, индекс = %1").arg(index);
    return true;
}
template<typename T, typename ArrayType>
typename AVLTree<T, ArrayType>::Node* AVLTree<T, ArrayType>::insert(Node* node, std::size_t key, std::size_t arrayIndex) {
    if (!node) {
        Node* newNode = new Node(key);
        newNode->indexList.add(arrayIndex);
        return newNode;
    }

    if (key < node->key)
        node->left = insert(node->left, key, arrayIndex);
    else if (key > node->key)
        node->right = insert(node->right, key, arrayIndex);
    else {
        // дубликат ключа не должен быть сюда передан (обрабатывается в публичной части)
        return node;
    }

    return balance(node);
}
template<typename T, typename ArrayType>
bool AVLTree<T, ArrayType>::remove(const std::string& keyStr, const T& value, ArrayType& array) {
    std::size_t key = stringToKey(keyStr);
    Node* node = findNode(root, key);
    if (!node) {
        qDebug().noquote() << "→ узел не найден";
        return false;
    }

    // поиск индекса элемента по значению
    int foundIndex = -1;
    lNode* current = node->indexList.getHead();
    int i = 0;
    do {
        if (!current) break;
        if (array[current->arrayIndex].doctorType == value.doctorType &&
            array[current->arrayIndex].diagnosis == value.diagnosis &&
            array[current->arrayIndex].appointmentDate == value.appointmentDate) {
            foundIndex = i;
            break;
        }
        current = current->next;
        ++i;
    } while (current != node->indexList.getHead());

    if (foundIndex == -1) {
        qDebug().noquote() << "→ элемент не найден в списке";
        return false;
    }

    std::size_t arrayIndex = current->arrayIndex;
    array.Remove(arrayIndex, *this);
    node->indexList.removeAt(foundIndex);

    qDebug().noquote() << QString("→ элемент удалён, индекс=%1").arg(arrayIndex);

    if (node->indexList.isEmpty()) {
        qDebug().noquote() << "→ удаляем узел (список пуст)";
        root = remove(root, key, value);
    }

    return true;
}
template<typename T, typename ArrayType>
typename AVLTree<T, ArrayType>::Node* AVLTree<T, ArrayType>::remove(Node* node, std::size_t key, const T&) {
    if (!node)
        return nullptr;

    if (key < node->key)
        node->left = remove(node->left, key, T{});
    else if (key > node->key)
        node->right = remove(node->right, key, T{});
    else {
        if (!node->left) {
            Node* rightChild = node->right;
            delete node;
            return rightChild;
        }
        else if (!node->right) {
            Node* leftChild = node->left;
            delete node;
            return leftChild;
        }

        // замена на наименьший в правом поддереве
        Node* minRight = node->right;
        while (minRight->left)
            minRight = minRight->left;

        node->key = minRight->key;
        node->indexList = minRight->indexList;
        node->right = remove(node->right, minRight->key, T{});
    }

    return balance(node);
}

template<typename T, typename ArrayType>
typename AVLTree<T, ArrayType>::Node* AVLTree<T, ArrayType>::find(const std::string& keyStr) {
    std::size_t key = stringToKey(keyStr);
    return findNode(root, key);
}


template<typename T, typename ArrayType>
typename AVLTree<T, ArrayType>::Node* AVLTree<T, ArrayType>::findNode(Node* node, std::size_t key) {
    if (!node)
        return nullptr;

    if (key == node->key)
        return node;

    if (key < node->key)
        return findNode(node->left, key);
    else
        return findNode(node->right, key);
}


template<typename T, typename ArrayType>
std::pair<typename AVLTree<T, ArrayType>::Node*, bool>
AVLTree<T, ArrayType>::findAppointment(const std::string& keyStr,
                                       const std::string& doctor,
                                       const std::string& diagnosis,
                                       const Date& date)
{
    std::size_t key = stringToKey(keyStr);
    Node* node = findNode(root, key);
    if (!node)
        return {nullptr, false};

    lNode* current = node->indexList.getHead();
    if (!current)
        return {node, false};

    do {
        const T& record = AppointmentArray[current->arrayIndex];
        if (record.doctorType == doctor &&
            record.diagnosis == diagnosis &&
            record.appointmentDate == date)
        {
            return {node, true};
        }
        current = current->next;
    } while (current != node->indexList.getHead());

    return {node, false};
}


template<typename T, typename ArrayType>
void AVLTree<T, ArrayType>::fixIndex(std::size_t oldIdx, std::size_t newIdx) {
    qDebug().noquote() << QString("AVL fixIndex: %1 → %2").arg(oldIdx).arg(newIdx);

    std::function<void(Node*)> updateNode = [&](Node* node) {
        if (!node)
            return;

        lNode* current = node->indexList.getHead();
        if (current) {
            do {
                if (current->arrayIndex == oldIdx) {
                    current->arrayIndex = newIdx;
                    qDebug().noquote() << QString("  обновлён индекс в узле key=%1").arg(node->key);
                    break; // если гарантированно только одно вхождение
                }
                current = current->next;
            } while (current != node->indexList.getHead());
        }

        updateNode(node->left);
        updateNode(node->right);
    };

    updateNode(root);
}

template<typename T, typename ArrayType>
typename AVLTree<T, ArrayType>::Node* AVLTree<T, ArrayType>::getRoot() const {
    return root;
}

template<typename T, typename ArrayType>
bool AVLTree<T, ArrayType>::isEmpty() const {
    return root == nullptr;
}


template<typename T, typename ArrayType>
void AVLTree<T, ArrayType>::clear() {
    clear(root);
    root = nullptr;
}

template<typename T, typename ArrayType>
void AVLTree<T, ArrayType>::removeAll() {
    clear(); // просто вызывает уже реализованный метод
}

template<typename T, typename ArrayType>
typename AVLTree<T, ArrayType>::Node* AVLTree<T, ArrayType>::rotateLeft(Node* x) {
    Node* y = x->right;
    x->right = y->left;
    y->left = x;

    updateHeight(x);
    updateHeight(y);

    return y;
}


template<typename T, typename ArrayType>
typename AVLTree<T, ArrayType>::Node* AVLTree<T, ArrayType>::rotateRight(Node* y) {
    Node* x = y->left;
    y->left = x->right;
    x->right = y;

    updateHeight(y);
    updateHeight(x);

    return x;
}

template<typename T, typename ArrayType>
void AVLTree<T, ArrayType>::updateHeight(Node* node) {
    if (node)
        node->height = 1 + std::max(getHeight(node->left), getHeight(node->right));
}


template<typename T, typename ArrayType>
int AVLTree<T, ArrayType>::getHeight(Node* node) {
    return node ? node->height : 0;
}


template<typename T, typename ArrayType>
typename AVLTree<T, ArrayType>::Node* AVLTree<T, ArrayType>::balance(Node* node) {
    updateHeight(node);
    int balanceFactor = getBalance(node);

    if (balanceFactor > 1) {
        if (getBalance(node->left) < 0)
            node->left = rotateLeft(node->left);
        return rotateRight(node);
    }
    else if (balanceFactor < -1) {
        if (getBalance(node->right) > 0)
            node->right = rotateRight(node->right);
        return rotateLeft(node);
    }
    return node;
}


template<typename T, typename ArrayType>
int AVLTree<T, ArrayType>::getBalance(Node* node) {
    return node ? getHeight(node->left) - getHeight(node->right) : 0;
}
*/
