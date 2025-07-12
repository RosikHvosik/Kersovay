#ifndef PATIENTPARSER_H
#define PATIENTPARSER_H
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <QDebug>
#include "types.h"
#include "array.h"

// 🔁 Преобразует "янв" → Month::янв
Month parseMonth(const std::string& m) {
    if (m == "янв") return Month::янв;
    if (m == "фев") return Month::фев;
    if (m == "мар") return Month::мар;
    if (m == "апр") return Month::апр;
    if (m == "май") return Month::май;
    if (m == "июн") return Month::июн;
    if (m == "июл") return Month::июл;
    if (m == "авг") return Month::авг;
    if (m == "сен") return Month::сен;
    if (m == "окт") return Month::окт;
    if (m == "ноя") return Month::ноя;
    if (m == "дек") return Month::дек;
    throw std::invalid_argument("Неизвестный месяц: " + m);
}

// 🧹 Удаляет пробелы
std::string removeSpaces(const std::string& str) {
    std::string res = str;
    res.erase(std::remove(res.begin(), res.end(), ' '), res.end());
    return res;
}

// 📄 Парсит файл и заполняет массив пациентов
bool parsePatientFile(const std::string& filename, Array<Patient, 1000>& patientArray) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        qDebug() << "Не удалось открыть файл пациентов:" << QString::fromStdString(filename);
        return false;
    }

    std::string line;
    int lineNumber = 0;

    while (std::getline(file, line)) {
        ++lineNumber;
        std::istringstream iss(line);
        std::string policyRaw, surname, name, middlename, dayStr, monthStr, yearStr;

        // Ожидаемый формат:
        // 1234 5678 9012 3456, Иванов Иван Иванович, 12 фев 2003
        if (!std::getline(iss, policyRaw, ',')) continue;
        if (!std::getline(iss, surname, ' ')) continue;
        if (!std::getline(iss, name, ' ')) continue;
        if (!std::getline(iss, middlename, ',')) continue;
        if (!(iss >> dayStr >> monthStr >> yearStr)) continue;

        try {
            Patient p;
            p.name = trim(name);
            p.surname = trim(surname);
            p.middlename = trim(middlename);
            p.birthDate = {
                std::stoi(dayStr),
                parseMonth(trim(monthStr)),
                std::stoi(yearStr)
            };

            std::string cleanPolicy = removeSpaces(trim(policyRaw));
            if (cleanPolicy.size() != 16) {
                qDebug() << "Некорректный полис ОМС на строке" << lineNumber;
                continue;
            }

            // Вставка в массив
            if (!patientArray.Add(p)) {
                qDebug() << "Массив пациентов переполнен!";
                return false;
            }

            qDebug().noquote() << QString("Загружен пациент [%1 %2 %3], полис: %4")
                                      .arg(QString::fromStdString(p.surname))
                                      .arg(QString::fromStdString(p.name))
                                      .arg(QString::fromStdString(p.middlename))
                                      .arg(QString::fromStdString(cleanPolicy));
        } catch (...) {
            qDebug() << "Ошибка парсинга строки" << lineNumber << ":" << QString::fromStdString(line);
            continue;
        }
    }

    return true;
}

// Удаляет начальные/конечные пробелы
std::string trim(const std::string& str) {
    const auto begin = str.find_first_not_of(" \t");
    if (begin == std::string::npos) return "";
    const auto end = str.find_last_not_of(" \t");
    return str.substr(begin, end - begin + 1);
}


#endif // PATIENTPARSER_H
