#ifndef AVLTREE3_HPP
#define AVLTREE3_HPP

#include "linkedlist.h"
#include <functional>
#include <utility>
#include <QVariant>
#include <iostream>
#include <algorithm>

template<typename KeyType, typename T, typename ArrayType>
struct AVLNode {
    KeyType key;
    linkedList indexList;
    int height;
    AVLNode* left;
    AVLNode* right;

    AVLNode(const KeyType& k) : key(k), height(1), left(nullptr), right(nullptr) {}
};

template<typename KeyType, typename T, typename ArrayType>
class AVLTree {
public:
    using Node = AVLNode<KeyType, T, ArrayType>;

    struct TreeStatistics {
        int totalNodes;
        int totalElements;
        int maxDepth;
        int uniqueKeys;
    };

    AVLTree();
    ~AVLTree();

    bool insert(const KeyType& key, const T& value, ArrayType& array);
    bool insertIndex(const KeyType& key, std::size_t index);
    bool remove(const KeyType& key, const T& value, ArrayType& array);
    bool removeAllByKey(const KeyType& key);
    void fixIndex(std::size_t oldIdx, std::size_t newIdx);

    void traverse(std::function<void(const T&, const KeyType&)> callback, const ArrayType& array) const;
    void traverseFiltered(std::function<bool(const T&)> filter,
                          std::function<void(const T&)> onAccept,
                          const ArrayType& array) const;
    void traverseIndex(std::function<void(std::size_t, const KeyType&)> callback) const;
    void traverseByKey(const KeyType& key, std::function<void(std::size_t)> callback) const;


    void clear();
    Node* getRoot() const;
    bool isEmpty() const;

    bool keyExists(const KeyType& key) const;
    int getCountForKey(const KeyType& key) const;
    std::vector<KeyType> getAllKeys() const;
    TreeStatistics getStatistics() const;
    bool validateIntegrity(const ArrayType& array) const;

private:
    Node* root;

    Node* insert(Node* node, const KeyType& key, std::size_t index);
    Node* removeNode(Node* node, const KeyType& key);
    Node* balance(Node* node);
    int getHeight(Node* node) const;
    void updateHeight(Node* node);
    int getBalance(Node* node) const;
    Node* rotateLeft(Node* x);
    Node* rotateRight(Node* y);
    void clear(Node* node);
    void traverse(Node* node, std::function<void(const T&, const KeyType&)> callback, const ArrayType& array) const;
    void traverseFiltered(Node* node, std::function<bool(const T&)> filter,
                          std::function<void(const T&)> onAccept,
                          const ArrayType& array) const;
    void traverseIndex(Node* node, std::function<void(std::size_t, const KeyType&)> callback) const;

    Node* findNode(Node* node, const KeyType& key) const;
};

// РЕАЛИЗАЦИЯ КОНСТРУКТОРА И ДЕСТРУКТОРА
template<typename KeyType, typename T, typename ArrayType>
AVLTree<KeyType, T, ArrayType>::AVLTree() : root(nullptr) {}

template<typename KeyType, typename T, typename ArrayType>
AVLTree<KeyType, T, ArrayType>::~AVLTree() {
    clear();
}

// РЕАЛИЗАЦИЯ МЕТОДОВ ОЧИСТКИ
template<typename KeyType, typename T, typename ArrayType>
void AVLTree<KeyType, T, ArrayType>::clear() {
    qDebug().noquote() << "[clear] Очистка дерева начата";
    clear(root);
    root = nullptr;
    qDebug().noquote() << "[clear] Дерево очищено (root = nullptr)";
}

template<typename KeyType, typename T, typename ArrayType>
void AVLTree<KeyType, T, ArrayType>::clear(Node* node) {
    if (!node) return;

    clear(node->left);
    clear(node->right);

    qDebug().noquote() << "[clear] Удалён узел с ключом:"
                       << QString::fromStdString(node->key);

    delete node;
}

// РЕАЛИЗАЦИЯ МЕТОДОВ БАЛАНСИРОВКИ
template<typename KeyType, typename T, typename ArrayType>
int AVLTree<KeyType, T, ArrayType>::getHeight(Node* node) const {
    return node ? node->height : 0;
}

template<typename KeyType, typename T, typename ArrayType>
void AVLTree<KeyType, T, ArrayType>::updateHeight(Node* node) {
    if (node)
        node->height = 1 + std::max(getHeight(node->left), getHeight(node->right));
}

template<typename KeyType, typename T, typename ArrayType>
int AVLTree<KeyType, T, ArrayType>::getBalance(Node* node) const {
    return node ? getHeight(node->left) - getHeight(node->right) : 0;
}

template<typename KeyType, typename T, typename ArrayType>
typename AVLTree<KeyType, T, ArrayType>::Node*
AVLTree<KeyType, T, ArrayType>::rotateLeft(Node* x) {
    Node* y = x->right;
    x->right = y->left;
    y->left = x;

    updateHeight(x);
    updateHeight(y);

    qDebug().noquote() << "[rotateLeft] Поворот влево вокруг ключа:"
                       << QVariant::fromValue(x->key).toString();

    return y;
}

template<typename KeyType, typename T, typename ArrayType>
typename AVLTree<KeyType, T, ArrayType>::Node*
AVLTree<KeyType, T, ArrayType>::rotateRight(Node* y) {
    Node* x = y->left;
    y->left = x->right;
    x->right = y;

    updateHeight(y);
    updateHeight(x);

    qDebug().noquote() << "[rotateRight] Поворот вправо вокруг ключа:"
                       << QVariant::fromValue(y->key).toString();

    return x;
}

template<typename KeyType, typename T, typename ArrayType>
typename AVLTree<KeyType, T, ArrayType>::Node*
AVLTree<KeyType, T, ArrayType>::balance(Node* node) {
    updateHeight(node);
    int bf = getBalance(node);

    if (bf > 1) {
        qDebug().noquote() << "[balance] Левый перекос (bf =" << bf << ") у ключа:"
                           << QVariant::fromValue(node->key).toString();

        if (getBalance(node->left) < 0) {
            qDebug().noquote() << "  → двойной поворот (лево-вправо)";
            node->left = rotateLeft(node->left);
        } else {
            qDebug().noquote() << "  → одинарный поворот вправо";
        }
        return rotateRight(node);
    }

    if (bf < -1) {
        qDebug().noquote() << "[balance] Правый перекос (bf =" << bf << ") у ключа:"
                           << QVariant::fromValue(node->key).toString();

        if (getBalance(node->right) > 0) {
            qDebug().noquote() << "  → двойной поворот (право-влево)";
            node->right = rotateRight(node->right);
        } else {
            qDebug().noquote() << "  → одинарный поворот влево";
        }
        return rotateLeft(node);
    }

    return node;
}

// РЕАЛИЗАЦИЯ МЕТОДОВ ВСТАВКИ
template<typename KeyType, typename T, typename ArrayType>
typename AVLTree<KeyType, T, ArrayType>::Node*
AVLTree<KeyType, T, ArrayType>::insert(Node* node, const KeyType& key, std::size_t index) {
    if (!node) {
        qDebug().noquote() << "[insert] Создан новый узел с ключом:"
                           << QVariant::fromValue(key).toString()
                           << ", индекс в массиве:" << index;

        Node* newNode = new Node(key);
        newNode->indexList.add(index);
        return newNode;
    }

    if (key < node->key) {
        qDebug().noquote() << "[insert] Переход влево от ключа:"
                           << QVariant::fromValue(node->key).toString();
        node->left = insert(node->left, key, index);
    } else if (key > node->key) {
        qDebug().noquote() << "[insert] Переход вправо от ключа:"
                           << QVariant::fromValue(node->key).toString();
        node->right = insert(node->right, key, index);
    } else {
        qDebug().noquote() << "[insert] Добавление индекса к существующему ключу:"
                           << QVariant::fromValue(key).toString()
                           << ", индекс:" << index;
        node->indexList.add(index);
    }

    return balance(node);
}

template<typename KeyType, typename T, typename ArrayType>
bool AVLTree<KeyType, T, ArrayType>::insert(const KeyType& key, const T& value, ArrayType& array) {
    qDebug().noquote() << "[insert] Попытка добавить элемент с ключом:"
                       << QVariant::fromValue(key).toString();

    if (!array.Add(value)) {
        qDebug().noquote() << "→ Массив переполнен, вставка невозможна";
        return false;
    }

    std::size_t index = array.Size() - 1;
    qDebug().noquote() << "→ Добавлено в массив, индекс:" << index;

    root = insert(root, key, index);
    return true;
}

template<typename KeyType, typename T, typename ArrayType>
bool AVLTree<KeyType, T, ArrayType>::insertIndex(const KeyType& key, std::size_t index) {
    qDebug().noquote() << "[insertIndex] Вставка индекса:" << index
                       << "в дерево с ключом:" << QVariant::fromValue(key).toString();

    root = insert(root, key, index);
    return true;
}

// РЕАЛИЗАЦИЯ МЕТОДОВ ПОИСКА
template<typename KeyType, typename T, typename ArrayType>
typename AVLTree<KeyType, T, ArrayType>::Node*
AVLTree<KeyType, T, ArrayType>::findNode(Node* node, const KeyType& key) const {
    if (!node) {
        qDebug().noquote() << "[findNode] Ключ" << QVariant::fromValue(key).toString() << "не найден (nullptr)";
        return nullptr;
    }

    if (key == node->key) {
        qDebug().noquote() << "[findNode] Найден узел с ключом:" << QVariant::fromValue(key).toString();
        return node;
    }

    qDebug().noquote() << "[findNode] Ищу" << QVariant::fromValue(key).toString()
                       << (key < node->key ? "влево от" : "вправо от")
                       << QVariant::fromValue(node->key).toString();

    return key < node->key ? findNode(node->left, key) : findNode(node->right, key);
}

// РЕАЛИЗАЦИЯ МЕТОДОВ УДАЛЕНИЯ
template<typename KeyType, typename T, typename ArrayType>
bool AVLTree<KeyType, T, ArrayType>::remove(const KeyType& key, const T& value, ArrayType& array) {
    qDebug().noquote() << QString("=== УДАЛЕНИЕ AVL: ключ = \"%1\" ===").arg(QVariant::fromValue(key).toString());

    Node* node = findNode(root, key);
    if (!node) {
        qDebug().noquote() << "→ узел с таким ключом не найден";
        return false;
    }

    lNode* current = node->indexList.getHead();
    int indexInList = 0;
    std::size_t arrayIndex = 0;
    bool found = false;

    do {
        if (!current) break;
        const T& entry = array[current->arrayIndex];
        if (entry == value) {
            arrayIndex = current->arrayIndex;
            found = true;
            break;
        }
        current = current->next;
        ++indexInList;
    } while (current != node->indexList.getHead());

    if (!found) {
        qDebug().noquote() << "→ элемент не найден в списке индексов";
        return false;
    }

    qDebug().noquote() << QString("→ удаление по индексу %1 из массива").arg(arrayIndex);
    array.Remove(arrayIndex, *this);

    qDebug().noquote() << QString("→ удаление из списка узла, позиция в списке = %1").arg(indexInList);
    node->indexList.removeAt(indexInList);

    if (node->indexList.isEmpty()) {
        qDebug().noquote() << "→ список индексов пуст — удаляем узел из дерева";
        root = removeNode(root, key);
    }

    return true;
}

template<typename KeyType, typename T, typename ArrayType>
typename AVLTree<KeyType, T, ArrayType>::Node*
AVLTree<KeyType, T, ArrayType>::removeNode(Node* node, const KeyType& key) {
    if (!node) return nullptr;

    if (key < node->key) {
        node->left = removeNode(node->left, key);
    } else if (key > node->key) {
        node->right = removeNode(node->right, key);
    } else {
        qDebug().noquote() << QString("→ удаляем узел с ключом: %1").arg(QVariant::fromValue(key).toString());

        if (!node->left) {
            Node* right = node->right;
            qDebug().noquote() << "→ нет левого поддерева — возвращаем правого потомка";
            delete node;
            return right;
        }
        else if (!node->right) {
            Node* left = node->left;
            qDebug().noquote() << "→ нет правого поддерева — возвращаем левого потомка";
            delete node;
            return left;
        } else {
            qDebug().noquote() << "→ оба поддерева существуют — ищем наименьший в правом поддереве";
            Node* minRight = node->right;
            while (minRight->left)
                minRight = minRight->left;

            qDebug().noquote() << QString("→ найден наименьший в правом поддереве: %1").arg(QVariant::fromValue(minRight->key).toString());

            node->key = minRight->key;
            node->indexList = minRight->indexList;
            node->right = removeNode(node->right, minRight->key);
        }
    }

    return balance(node);
}

template<typename KeyType, typename T, typename ArrayType>
bool AVLTree<KeyType, T, ArrayType>::removeAllByKey(const KeyType& key) {
    qDebug().noquote() << QString("=== УДАЛЕНИЕ ВСЕХ ПО КЛЮЧУ: %1 ===")
                              .arg(QVariant::fromValue(key).toString());

    Node* node = findNode(root, key);
    if (!node) {
        qDebug().noquote() << "→ узел с таким ключом не найден";
        return false;
    }

    root = removeNode(root, key);
    qDebug().noquote() << "→ узел удалён из дерева";
    return true;
}

// РЕАЛИЗАЦИЯ ВСПОМОГАТЕЛЬНЫХ МЕТОДОВ
template<typename KeyType, typename T, typename ArrayType>
void AVLTree<KeyType, T, ArrayType>::fixIndex(std::size_t oldIdx, std::size_t newIdx) {
    qDebug().noquote() << QString("AVL fixIndex: %1 → %2").arg(oldIdx).arg(newIdx);

    std::function<void(Node*)> fix = [&](Node* node) {
        if (!node) return;

        lNode* current = node->indexList.getHead();
        if (current) {
            do {
                if (current->arrayIndex == oldIdx) {
                    current->arrayIndex = newIdx;
                    qDebug().noquote() << QString("  обновлён индекс в узле key=%1")
                                              .arg(QVariant::fromValue(node->key).toString());
                    break; // если предполагается одно вхождение
                }
                current = current->next;
            } while (current != node->indexList.getHead());
        }

        fix(node->left);
        fix(node->right);
    };

    fix(root);
}

template<typename KeyType, typename T, typename ArrayType>
typename AVLTree<KeyType, T, ArrayType>::Node*
AVLTree<KeyType, T, ArrayType>::getRoot() const {
    if (root) {
        qDebug().noquote() << "[getRoot] Корень дерева — ключ:"
                           << QVariant::fromValue(root->key).toString();
    } else {
        qDebug().noquote() << "[getRoot] Дерево пусто (root == nullptr)";
    }
    return root;
}

template<typename KeyType, typename T, typename ArrayType>
void AVLTree<KeyType, T, ArrayType>::traverse(
    std::function<void(const T&, const KeyType&)> callback,
    const ArrayType& array) const
{
    qDebug().noquote() << "[traverse] Начат обход дерева справа налево";
    traverse(root, callback, array);
}

template<typename KeyType, typename T, typename ArrayType>
void AVLTree<KeyType, T, ArrayType>::traverse(
    Node* node,
    std::function<void(const T&, const KeyType&)> callback,
    const ArrayType& array) const
{
    if (!node) return;

    // ОБХОД СПРАВА НАЛЕВО: сначала правое поддерево
    traverse(node->right, callback, array);

    qDebug().noquote() << QString("[traverse] Обработка узла с ключом: %1")
                              .arg(QVariant::fromValue(node->key).toString());

    lNode* current = node->indexList.getHead();
    int count = 0;
    if (current) {
        do {
            std::size_t idx = current->arrayIndex;
            qDebug().noquote() << QString("  → Вызов callback для индекса %1").arg(idx);
            callback(array[idx], node->key);
            current = current->next;
            ++count;
        } while (current != node->indexList.getHead());
    }

    if (count == 0) {
        qDebug().noquote() << "  → Список индексов пуст";
    }

    // Затем левое поддерево
    traverse(node->left, callback, array);
}

template<typename KeyType, typename T, typename ArrayType>
void AVLTree<KeyType, T, ArrayType>::traverseFiltered(
    std::function<bool(const T&)> filter,
    std::function<void(const T&)> onAccept,
    const ArrayType& array) const
{
    qDebug().noquote() << "[traverseFiltered] Начат обход с фильтрацией (справа налево)";
    traverseFiltered(root, filter, onAccept, array);
}

template<typename KeyType, typename T, typename ArrayType>
void AVLTree<KeyType, T, ArrayType>::traverseFiltered(
    Node* node,
    std::function<bool(const T&)> filter,
    std::function<void(const T&)> onAccept,
    const ArrayType& array) const
{
    if (!node) return;

    // ОБХОД СПРАВА НАЛЕВО
    traverseFiltered(node->right, filter, onAccept, array);

    qDebug().noquote() << QString("[traverseFiltered] Узел с ключом: %1")
                              .arg(QVariant::fromValue(node->key).toString());

    lNode* current = node->indexList.getHead();
    int passed = 0;
    if (current) {
        do {
            const T& item = array[current->arrayIndex];
            if (filter(item)) {
                ++passed;
                qDebug().noquote() << QString("  → элемент по индексу %1 прошёл фильтр").arg(current->arrayIndex);
                onAccept(item);
            } else {
                qDebug().noquote() << QString("  → элемент по индексу %1 не прошёл фильтр").arg(current->arrayIndex);
            }
            current = current->next;
        } while (current != node->indexList.getHead());
    }

    if (passed == 0)
        qDebug().noquote() << "  → ничего не подошло по фильтру";

    traverseFiltered(node->left, filter, onAccept, array);
}

template<typename KeyType, typename T, typename ArrayType>
void AVLTree<KeyType, T, ArrayType>::traverseIndex(
    std::function<void(std::size_t, const KeyType&)> callback) const
{
    qDebug().noquote() << "[traverseIndex] Начат обход индексов (справа налево)";
    traverseIndex(root, callback);
}

template<typename KeyType, typename T, typename ArrayType>
void AVLTree<KeyType, T, ArrayType>::traverseIndex(
    Node* node,
    std::function<void(std::size_t, const KeyType&)> callback) const
{
    if (!node) return;

    // ОБХОД СПРАВА НАЛЕВО
    traverseIndex(node->right, callback);

    qDebug().noquote() << QString("[traverseIndex] Узел с ключом: %1")
                              .arg(QVariant::fromValue(node->key).toString());

    lNode* current = node->indexList.getHead();
    int count = 0;
    if (current) {
        do {
            qDebug().noquote() << QString("  → индекс: %1").arg(current->arrayIndex);
            callback(current->arrayIndex, node->key);
            current = current->next;
            ++count;
        } while (current != node->indexList.getHead());
    }

    if (count == 0) {
        qDebug().noquote() << "  → список индексов пуст";
    }

    traverseIndex(node->left, callback);
}

// РЕАЛИЗАЦИЯ НОВЫХ МЕТОДОВ ДЛЯ РЕФЕРЕНЦИАЛЬНОЙ ЦЕЛОСТНОСТИ
template<typename KeyType, typename T, typename ArrayType>
bool AVLTree<KeyType, T, ArrayType>::keyExists(const KeyType& key) const {
    qDebug().noquote() << QString("[keyExists] Проверка ключа: %1")
                              .arg(QVariant::fromValue(key).toString());

    Node* node = findNode(root, key);
    bool exists = (node != nullptr && !node->indexList.isEmpty());

    qDebug().noquote() << QString("→ Результат: %1").arg(exists ? "НАЙДЕН" : "НЕ НАЙДЕН");
    return exists;
}

template<typename KeyType, typename T, typename ArrayType>
int AVLTree<KeyType, T, ArrayType>::getCountForKey(const KeyType& key) const {
    Node* node = findNode(root, key);
    if (!node) return 0;

    return node->indexList.getSize();
}

template<typename KeyType, typename T, typename ArrayType>
std::vector<KeyType> AVLTree<KeyType, T, ArrayType>::getAllKeys() const {
    std::vector<KeyType> keys;

    std::function<void(Node*)> collectKeys = [&](Node* node) {
        if (!node) return;

        collectKeys(node->left);
        if (!node->indexList.isEmpty()) {
            keys.push_back(node->key);
        }
        collectKeys(node->right);
    };

    collectKeys(root);

    qDebug().noquote() << QString("Собрано уникальных ключей: %1").arg(keys.size());
    return keys;
}

template<typename KeyType, typename T, typename ArrayType>
typename AVLTree<KeyType, T, ArrayType>::TreeStatistics
AVLTree<KeyType, T, ArrayType>::getStatistics() const {
    TreeStatistics stats = {0, 0, 0, 0};

    std::function<int(Node*, int)> calculateStats = [&](Node* node, int depth) -> int {
        if (!node) return depth - 1;

        stats.totalNodes++;
        stats.totalElements += node->indexList.getSize();
        if (!node->indexList.isEmpty()) {
            stats.uniqueKeys++;
        }

        int leftDepth = calculateStats(node->left, depth + 1);
        int rightDepth = calculateStats(node->right, depth + 1);

        return std::max(leftDepth, rightDepth);
    };

    stats.maxDepth = calculateStats(root, 0);

    qDebug().noquote() << QString("Статистика дерева: узлов=%1, элементов=%2, глубина=%3, ключей=%4")
                              .arg(stats.totalNodes)
                              .arg(stats.totalElements)
                              .arg(stats.maxDepth)
                              .arg(stats.uniqueKeys);

    return stats;
}

template<typename KeyType, typename T, typename ArrayType>
bool AVLTree<KeyType, T, ArrayType>::validateIntegrity(const ArrayType& array) const {
    bool isValid = true;

    std::function<void(Node*)> validate = [&](Node* node) {
        if (!node) return;

        // Проверяем, что все индексы в списке действительны
        lNode* current = node->indexList.getHead();
        if (current) {
            do {
                if (current->arrayIndex >= array.Size()) {
                    qDebug().noquote() << QString("ОШИБКА: индекс %1 больше размера массива %2")
                                              .arg(current->arrayIndex)
                                              .arg(array.Size());
                    isValid = false;
                }
                current = current->next;
            } while (current != node->indexList.getHead());
        }

        validate(node->left);
        validate(node->right);
    };

    validate(root);

    qDebug().noquote() << QString("Проверка целостности: %1")
                              .arg(isValid ? "ПРОЙДЕНА" : "ПРОВАЛЕНА");
    return isValid;
}
template<typename KeyType, typename T, typename ArrayType>
void AVLTree<KeyType, T, ArrayType>::traverseByKey(const KeyType& key, std::function<void(std::size_t)> callback) const {
    Node* node = findNode(root, key);
    if (!node) {
        qDebug().noquote() << QString("[traverseByKey] Ключ %1 не найден").arg(QString::fromStdString(key));
        return;
    }

    lNode* current = node->indexList.getHead();
    if (current) {
        do {
            callback(current->arrayIndex);
            current = current->next;
        } while (current != node->indexList.getHead());
    }
}


#endif // AVLTREE3_HPP
