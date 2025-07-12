#ifndef APPOINTMENTPARSER_H
#define APPOINTMENTPARSER_H

#include "types.h"
#include "array.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <QString>

inline std::string removeSpaces(const std::string& str) {
    std::string result;
    for (char c : str) {
        if (c != ' ') result += c;
    }
    return result;
}

inline Month parseMonth(const std::string& monthStr) {
    if (monthStr == "янв") return Month::янв;
    if (monthStr == "фев") return Month::фев;
    if (monthStr == "мар") return Month::мар;
    if (monthStr == "апр") return Month::апр;
    if (monthStr == "май") return Month::май;
    if (monthStr == "июн") return Month::июн;
    if (monthStr == "июл") return Month::июл;
    if (monthStr == "авг") return Month::авг;
    if (monthStr == "сен") return Month::сен;
    if (monthStr == "окт") return Month::окт;
    if (monthStr == "ноя") return Month::ноя;
    if (monthStr == "дек") return Month::дек;
    throw std::invalid_argument("Unknown month: " + monthStr);
}

inline void parseAppointmentFile(const std::string& filename, Array<Appointment, 1000>& appointmentArray, std::vector<std::string>& policies) {
    std::ifstream fin(filename);
    std::string line;
    int lineCount = 0;

    while (std::getline(fin, line)) {
        ++lineCount;
        std::istringstream iss(line);

        std::string policyRaw, diagnosis, doctor, dayStr, monthStr, yearStr;

        std::getline(iss, policyRaw, ',');
        std::getline(iss, diagnosis, ',');
        std::getline(iss, doctor, ',');
        iss >> dayStr >> monthStr >> yearStr;

        std::string cleanPolicy = removeSpaces(policyRaw);

        try {
            Date date = {std::stoi(dayStr), parseMonth(monthStr), std::stoi(yearStr)};
            Appointment a = {doctor, diagnosis, date};

            if (appointmentArray.Add(a)) {
                policies.push_back(cleanPolicy);
                qDebug().noquote() << QString("Загружен приём: [%1, %2, %3 %4 %5], полис: %6")
                                          .arg(QString::fromStdString(doctor))
                                          .arg(QString::fromStdString(diagnosis))
                                          .arg(date.day)
                                          .arg(static_cast<int>(date.month))
                                          .arg(date.year)
                                          .arg(QString::fromStdString(cleanPolicy));
            }
        } catch (...) {
            qDebug().noquote() << QString("Ошибка парсинга строки %1: %2").arg(lineCount).arg(QString::fromStdString(line));
        }
    }
}


#endif // APPOINTMENTPARSER_H
