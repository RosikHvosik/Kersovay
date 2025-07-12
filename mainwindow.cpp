#include "mainwindow.h"
#include "hashtablewidget.h"
#include "ui_mainwindow.h"
#include <QTableWidget>
#include <QDebug>

#include "array.h"
#include "hashtable.h"
#include "hashtable.hpp"
#include "avltree3.hpp"

Array<Patient, 1000> PatientArray;
Array<Appointment, 1000> AppointmentArray;

QString formatDate(const Date& date) {
    QStringList monthNames = {
        "", "янв", "фев", "мар", "апр", "май", "июн",
        "июл", "авг", "сен", "окт", "ноя", "дек"
    };
    return QString("%1 %2 %3")
        .arg(date.day)
        .arg(monthNames[static_cast<int>(date.month)])
        .arg(date.year);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // === ТЕСТИРОВАНИЕ AVLTree ===
    /*AVLTree<std::string, Appointment, Array<Appointment, 1000>> tree;

    // Примерные записи
    Date d1{10, Month::янв, 2024};
    Date d2{12, Month::фев, 2024};
    Date d3{15, Month::мар, 2024};

    Appointment a1 = {"Терапевт", "Грипп", d1};
    Appointment a2 = {"Терапевт", "ОРВИ", d2};
    Appointment a3 = {"Педиатр", "Кашель", d3};

    tree.insert("1234567890123456", a1, AppointmentArray);
    tree.insert("1234567890123456", a2, AppointmentArray);
    tree.insert("6543210987654321", a3, AppointmentArray);

    qDebug() << "\n=== ОБХОД ДЕРЕВА ===";
    tree.traverse([](const Appointment& a, const std::string& key) {
        qDebug().noquote() << QString("Полис: %1 | Врач: %2 | Диагноз: %3 | Дата: %4")
                                  .arg(QString::fromStdString(key))
                                  .arg(QString::fromStdString(a.doctorType))
                                  .arg(QString::fromStdString(a.diagnosis))
                                  .arg(formatDate(a.appointmentDate));
    }, AppointmentArray);

    qDebug() << "\n=== УДАЛЕНИЕ ПРИЁМА ===";
    tree.remove("1234567890123456", a1, AppointmentArray);

    qDebug() << "\n=== ОБХОД ПОСЛЕ УДАЛЕНИЯ ===";
    tree.traverse([](const Appointment& a, const std::string& key) {
        qDebug().noquote() << QString("Полис: %1 | Врач: %2 | Диагноз: %3 | Дата: %4")
                                  .arg(QString::fromStdString(key))
                                  .arg(QString::fromStdString(a.doctorType))
                                  .arg(QString::fromStdString(a.diagnosis))
                                  .arg(formatDate(a.appointmentDate));
    }, AppointmentArray);

    qDebug() << "\n=== ОЧИСТКА ДЕРЕВА ===";
    tree.clear();
*/
    // === ТЕСТИРОВАНИЕ HashTable ===
    qDebug().noquote() << "\n=== ТЕСТ ХЕШ-ТАБЛИЦЫ ===";

    HashTable table;

    // 9 уникальных записей
    std::vector<std::pair<std::string, std::string>> entries = {
        {"0000000000999991", "Иванов Иван Иванович"},
        {"0000000000999910", "Петров Петр Петрович"},
        {"0000000000111123", "Сидоров Сидор Сидорович"},
        {"0000000000222222", "Козлов Николай Николаевич"},
        {"0000000000333333", "Орлов Андрей Александрович"},
        {"0000000000444444", "Зайцев Юрий Юрьевич"},
        {"0000000000555555", "Фёдоров Виталий Викторович"},
        {"0000000000666666", "Михайлов Владимир Владимирович"},
        {"0000000000777777", "Смирнов Алексей Алексеевич"}
    };

    // Вставка всех записей
    int i = 0;
    for (const auto& [oms, fio] : entries) {
        std::istringstream in(fio);
        std::string surname, name, middlename;
        in >> surname >> name >> middlename;

        table.insert(oms, fio, 1 + i, static_cast<Month>((i % 12) + 1), 1980 + i);
        ++i;
    }
    // Коллизия: тот же хеш, что у 999910
    std::string omsCollision = "0000000000010000";
    std::string nameCollision = "Клишин Константин Константинович";

    table.insert(omsCollision, nameCollision, 9, Month::сен, 1999);
    qDebug().noquote() << "\n=== ПОИСК КОЛЛИЗИИ ДО УДАЛЕНИЯ ===";
    const Patient* collision = table.get("0000000000010000");
    if (collision) {
        qDebug().noquote() << QString("Найден (коллизия): %1 %2 %3")
                                  .arg(QString::fromStdString(collision->surname))
                                  .arg(QString::fromStdString(collision->name))
                                  .arg(QString::fromStdString(collision->middlename));
    } else {
        qDebug().noquote() << "❌ ОШИБКА: Пациент с коллизией не найден перед удалением!";
    }



    qDebug().noquote() << "\n=== ПОИСК ===";
    const Patient* p = table.get("0000000000999910");
    if (p) {
        qDebug().noquote() << QString("Найден: %1 %2 %3")
                                  .arg(QString::fromStdString(p->surname))
                                  .arg(QString::fromStdString(p->name))
                                  .arg(QString::fromStdString(p->middlename));
    }

    qDebug().noquote() << "\n=== УДАЛЕНИЕ ===";
    table.remove("0000000000999910");

    qDebug().noquote() << "\n=== ПОИСК ПОСЛЕ УДАЛЕНИЯ ===";
    const Patient* deleted = table.get("0000000000999910");
    if (!deleted) {
        qDebug().noquote() << "Пациент удалён корректно";
    }

    qDebug().noquote() << "\n=== ПОВТОРНАЯ ВСТАВКА ===";
    table.insert("0000000000999910", "Петров Петр Петрович", 15, Month::фев, 1990);
    qDebug().noquote() << "\n=== УДАЛЕНИЕ ЭЛЕМЕНТА С КОЛЛИЗИЕЙ ===";
    table.remove("0000000000010000");

    qDebug().noquote() << "\n=== ПОИСК ПОСЛЕ УДАЛЕНИЯ КОЛЛИЗИИ ===";
    const Patient* removedCollided = table.get("0000000000010000");
    if (!removedCollided) {
        qDebug().noquote() << "Пациент с коллизией удалён корректно";
    }



}

MainWindow::~MainWindow()
{
    delete ui;
}
