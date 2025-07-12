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


    std::size_t findPos(std::size_t key, bool inserting) const
    {
        std::size_t pos = hash_function(key);
        std::size_t step = hash_function(key) + 1;

        qDebug().noquote() << QString(" Ищем позицию: Ключ = %1, Начальная позиция = %2, Шаг = %3")
                                  .arg(key)
                                  .arg(pos)
                                  .arg(step);

        for (std::size_t i = 0; i < m_size; ++i)
        {
            const HashRecord &record = m_table[pos];
            qDebug().noquote() << QString("  Пробируем [%1]: Позиция = %2, Статус = %3, Ключ = %4")
                                      .arg(i)
                                      .arg(pos)
                                      .arg(record.status == Status::Empty ? "Empty" : "Active")
                                      .arg(record.key);

            if (record.status == Status::Empty)
            {
                if (inserting)
                {
                    qDebug().noquote() << QString("  → Нашли пустую позицию = %1").arg(pos);
                }
                return inserting ? pos : upos;
            }

            if (record.status == Status::Active && record.key == key)
            {
                qDebug().noquote() << QString("  → Нашли на позиции = %1").arg(pos);
                return pos;
            }

            pos = (pos + step) % m_size;
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
        //распарсит ли оно тут ? т.к фио это Фамимлия Имя Отчество
        Patient patient{surname, name, middlename, date};

        //для дебагера
        qDebug().noquote() << QString("=== Вставка: \"%1\" ===").arg(QString::fromStdString(OMS));

        std::size_t numKey = stringToKey(OMS);

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

        qDebug().noquote() << QString("Успешная вставка : Позиция = %1, Индекс Массива = %2, Новый размер = %3")
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

    const Patient *get(const std::string &OMS) const
    {
        qDebug().noquote() << QString("=== Взятие информации клиента: \"%1\" ===").arg(QString::fromStdString(OMS));

        std::size_t key = stringToKey(OMS);
        std::size_t pos = findPos(key, false);

        if (pos == upos)
        {
            qDebug().noquote() << "Не найдена запись";
            return nullptr;
        }

        qDebug().noquote() << QString("Удаляем на позиции = %1, Индекс массива = %2")
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
        m_table[pos].status = Status::Empty;
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
    //добавил жтот метод для отображения

    std::string getKeyForIndex(std::size_t index) const {
        for (std::size_t i = 0; i < m_size; ++i) {
            if (m_table[i].status == Status::Active && m_table[i].arrayIndex == index) {
                return std::to_string(m_table[i].key);
            }
        }
        return "";
    }



};

#endif
