#ifndef TYPES_H
#define TYPES_H
#include <string>
enum class Month
{
    янв = 1,
    фев = 2,
    мар = 3,
    апр = 4,
    май = 5,
    июн = 6,
    июл = 7,
    авг = 8,
    сен = 9,
    окт = 10,
    ноя = 11,
    дек = 12,
};

struct Date
{
    int day;
    Month month;
    int year;
    auto operator<=>(const Date &other) const = default;

};

struct Patient {
    std::string name, surname, middlename;
    Date birthDate;

    bool operator==(const Patient& other) const {
        return name == other.name &&
               surname == other.surname &&
               middlename == other.middlename &&
               birthDate == other.birthDate;
    }
};

struct Appointment {
    std::string doctorType, diagnosis;
    Date appointmentDate;

    bool operator==(const Appointment& other) const {
        return doctorType == other.doctorType &&
               diagnosis == other.diagnosis &&
               appointmentDate == other.appointmentDate;
    }
};
enum class Status
{
    Empty,
    Active,
    Deleted
};
#endif
