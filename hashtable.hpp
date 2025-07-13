#ifndef HASHTABLE_HPP
#define HASHTABLE_HPP

#include "globals.h"
#include <string>
#include <cmath>
#include <stdexcept>
#include <QString>
#include <QDebug>
#include "types.h"
#include<sstream>
#define MAX_SIZE 1000
#define DIGITS 4

inline std::string uint128_to_string(__uint128_t value) {
    if (value == 0) return "0";
    std::string result;
    while (value > 0) {
        result.insert(result.begin(), '0' + static_cast<char>(value % 10));
        value /= 10;
    }
    return result;
}

struct HashRecord
{
    std::size_t key{0};
    std::size_t arrayIndex{0};
    Status status{Status::Empty};

    std::string getKey() const
    {
        if (key == 0)
            return "";

        std::string keyStr = std::to_string(key);
        return keyStr;
    }

    std::size_t getArrayIndex() const { return arrayIndex; }
    Status getStatus() const { return status; }
};

class HashTable
{
private:
    HashRecord *m_table{nullptr};
    std::size_t m_size{MAX_SIZE};
    std::size_t m_count{0};

    const std::size_t upos = -1;

    std::size_t hash_function(std::size_t key) const {
        __uint128_t square = static_cast<__uint128_t>(key) * key;
        std::string squareStr = uint128_to_string(square);
        std::size_t len = squareStr.length();

        if (len < DIGITS + 2) {
            std::size_t fallbackHash = key % m_size;
            qDebug().noquote() << QString("Hash (%1): квадрат короткий → %2").arg(key).arg(fallbackHash);
            return fallbackHash;
        }

        std::size_t mid = len / 2;
        std::string midStr = squareStr.substr(mid - DIGITS / 2, DIGITS);
        std::size_t hashVal = std::stoul(midStr);
        std::size_t finalHash = hashVal % m_size;

        qDebug().noquote() << QString("Hash (%1): квадрат = %2, mid = \"%3\", итог = %4")
                                  .arg(key)
                                  .arg(QString::fromStdString(squareStr))
                                  .arg(QString::fromStdString(midStr))
                                  .arg(finalHash);

        return finalHash;
    }

    // ИСПРАВЛЕННАЯ функция поиска позиции
    std::size_t findPos(std::size_t key, bool inserting) const
    {
        std::size_t pos = hash_function(key);
        std::size_t step = 1; // Простое линейное пробирование вместо сложного

        qDebug().noquote() << QString(" Ищем позицию: Ключ = %1, Начальная позиция = %2, Шаг = %3")
                                  .arg(key)
                                  .arg(pos)
                                  .arg(step);

        std::size_t startPos = pos; // Запоминаем начальную позицию для обнаружения зацикливания

        for (std::size_t i = 0; i < m_size; ++i)
        {
            const HashRecord &record = m_table[pos];
            qDebug().noquote() << QString("  Пробируем [%1]: Позиция = %2, Статус = %3, Ключ = %4")
                                      .arg(i)
                                      .arg(pos)
                                      .arg(record.status == Status::Empty ? "Empty" :
                                               record.status == Status::Active ? "Active" : "Deleted")
                                      .arg(record.key);

            // При вставке - ищем пустое место или удаленное
            if (inserting && (record.status == Status::Empty || record.status == Status::Deleted))
            {
                qDebug().noquote() << QString("  → Нашли свободную позицию = %1").arg(pos);
                return pos;
            }

            // При поиске - ищем только активную запись с нужным ключом
            if (!inserting && record.status == Status::Active && record.key == key)
            {
                qDebug().noquote() << QString("  → Нашли на позиции = %1").arg(pos);
                return pos;
            }

            // При поиске - если дошли до пустой ячейки, элемента нет
            if (!inserting && record.status == Status::Empty)
            {
                qDebug().noquote() << QString("  → Дошли до пустой ячейки, элемент не найден");
                return upos;
            }

            pos = (pos + step) % m_size;

            // Проверяем зацикливание
            if (i > 0 && pos == startPos) {
                qDebug().noquote() << "  → Полный обход таблицы, больше нет мест";
                return upos;
            }

            qDebug().noquote() << QString("  → Коллизия, следующая позиция = %1").arg(pos);
        }

        qDebug().noquote() << "  → Таблица заполнена, нет больше мест";
        return upos;
    }

    std::size_t stringToKey(const std::string &str) const
    {
        std::size_t key = 0;
        QString debugStr = QString::fromStdString(str);

        for (char c : str)
        {
            if (std::isdigit(c))
            {
                key = key * 10 + (c - '0');
            }
        }

        qDebug().noquote() << QString("Перевели строку в ключ: \"%1\" → %2").arg(debugStr).arg(key);
        return key;
    }

public:
    HashTable()
    {
        m_table = new HashRecord[m_size]{};
        qDebug().noquote() << QString("HashTable: Размер = %1").arg(m_size);
    }

    ~HashTable()
    {
        delete[] m_table;
    }

    std::size_t getSize() const noexcept { return m_size; }
    std::size_t getCount() const noexcept { return m_count; }

    bool insert(const std::string &OMS, const std::string &fn, int day, Month month, int year)
    {
        std::string surname, name, middlename;
        std::istringstream in(fn);
        in >> surname >> name >> middlename;

        Date date{day, month, year};
        Patient patient{surname, name, middlename, date};

        qDebug().noquote() << QString("=== Вставка: \"%1\" ===").arg(QString::fromStdString(OMS));

        std::size_t numKey = stringToKey(OMS);

        // Проверяем, не переполнена ли таблица
        if (m_count >= m_size) {
            qDebug().noquote() << "Хэш-таблица переполнена";
            throw std::runtime_error("Хэш-таблица переполнена");
        }

        if (findPos(numKey, false) != upos)
        {
            qDebug().noquote() << "Нашли дубликат!";
            throw std::runtime_error("Дубликат");
        }

        if (!PatientArray.Add(patient))
        {
            qDebug().noquote() << "Массив полон";
            throw std::runtime_error("Хранилище заполнено");
        }

        std::size_t arrayIdx = PatientArray.Size() - 1;
        std::size_t pos = findPos(numKey, true);

        if (pos == upos)
        {
            qDebug().noquote() << "Не удалось найти место";
            throw std::runtime_error("Не удалось найти место");
        }

        m_table[pos] = {numKey, arrayIdx, Status::Active};
        ++m_count;

        qDebug().noquote() << QString("Успешная вставка: Позиция = %1, Индекс Массива = %2, Новый размер = %3")
                                  .arg(pos)
                                  .arg(arrayIdx)
                                  .arg(m_count);
        return true;
    }

    const HashRecord &getRecord(size_t index) const
    {
        if (index >= m_size)
            throw std::out_of_range("Индекс вне массива");
        return m_table[index];
    }

    std::size_t getHashValue(const std::string& policy) const
    {
        std::size_t key = stringToKey(policy);
        return hash_function(key);
    }

    std::size_t getHashValue(std::size_t key) const {
        return hash_function(key);
    }

    const Patient *get(const std::string &OMS) const
    {
        qDebug().noquote() << QString("=== Получение информации клиента: \"%1\" ===").arg(QString::fromStdString(OMS));

        std::size_t key = stringToKey(OMS);
        std::size_t pos = findPos(key, false);

        if (pos == upos)
        {
            qDebug().noquote() << "Не найдена запись";
            return nullptr;
        }

        qDebug().noquote() << QString("Найден на позиции = %1, Индекс массива = %2")
                                  .arg(pos)
                                  .arg(m_table[pos].arrayIndex);
        return &PatientArray[m_table[pos].arrayIndex];
    }

    bool remove(const std::string &OMS)
    {
        qDebug().noquote() << QString("=== Удаление: \"%1\" ===").arg(QString::fromStdString(OMS));

        std::size_t key = stringToKey(OMS);
        std::size_t pos = findPos(key, false);

        if (pos == upos)
        {
            qDebug().noquote() << "Ключ не найден";
            return false;
        }

        qDebug().noquote() << QString("Удаление с позиции = %1, Индекс в массиве = %2")
                                  .arg(pos)
                                  .arg(m_table[pos].arrayIndex);

        PatientArray.Remove(m_table[pos].arrayIndex, *this);
        m_table[pos].status = Status::Deleted; // Помечаем как удаленное, а не Empty
        m_table[pos].key = 0;
        --m_count;

        return true;
    }

    void fixIndex(std::size_t oldIdx, std::size_t newIdx)
    {
        qDebug().noquote() << QString("Исправили индекс: %1 → %2").arg(oldIdx).arg(newIdx);

        for (std::size_t i = 0; i < m_size; ++i)
            if (m_table[i].status == Status::Active && m_table[i].arrayIndex == oldIdx)
            {
                m_table[i].arrayIndex = newIdx;
                qDebug().noquote() << QString("  Обновили позицию = %1").arg(i);
                break;
            }
    }

    std::string getKeyForIndex(std::size_t index) const {
        for (std::size_t i = 0; i < m_size; ++i) {
            if (m_table[i].status == Status::Active && m_table[i].arrayIndex == index) {
                return std::to_string(m_table[i].key);
            }
        }
        return "";
    }

    bool exists(const std::string& OMS) const {
        qDebug().noquote() << QString("=== Проверка существования: \"%1\" ===")
                                  .arg(QString::fromStdString(OMS));

        std::size_t key = stringToKey(OMS);
        std::size_t pos = findPos(key, false);

        bool found = (pos != upos);
        qDebug().noquote() << QString("→ Результат: %1").arg(found ? "НАЙДЕН" : "НЕ НАЙДЕН");

        return found;
    }

    std::vector<std::string> getAllPolicies() const {
        std::vector<std::string> policies;

        for (std::size_t i = 0; i < m_size; ++i) {
            if (m_table[i].status == Status::Active) {
                policies.push_back(m_table[i].getKey());
            }
        }

        qDebug().noquote() << QString("Найдено активных полисов: %1").arg(policies.size());
        return policies;
    }

    struct Statistics {
        std::size_t totalSlots;
        std::size_t usedSlots;
        std::size_t emptySlots;
        double loadFactor;
    };

    Statistics getStatistics() const {
        Statistics stats;
        stats.totalSlots = m_size;
        stats.usedSlots = m_count;
        stats.emptySlots = m_size - m_count;
        stats.loadFactor = static_cast<double>(m_count) / m_size;

        return stats;
    }
};
#endif
