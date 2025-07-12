#include "mainwindow.h"
#include "hashtablewidget.h"
#include "ui_mainwindow.h"
#include <QTableWidget>
#include <QDebug>
#include <QHeaderView>
#include "array.h"
#include "hashtable.hpp"
#include "avltree3.hpp"
#include <QFile>
//Array<Patient, 1000> patientArray;
//Array<Appointment, 1000> appointmentArray;
//обнавление вкладки пациенты
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
void MainWindow::loadPatientsFromFile(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Не удалось открыть файл пациентов";
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList parts = line.split("|");
        if (parts.size() != 3) continue;

        QString policyStr = parts[0].trimmed();         // ключ
        QString fioStr = parts[1].trimmed();            // Фамилия Имя Отчество
        QStringList dateParts = parts[2].split(" ");    // дата рождения

        if (dateParts.size() != 3) continue;

        int day = dateParts[0].toInt();
        Month month = monthFromShortString(dateParts[1]);
        int year = dateParts[2].toInt();

        try {
            hashTable.insert(policyStr.toStdString(), fioStr.toStdString(), day, month, year);
        } catch (const std::exception& e) {
            qDebug() << "Ошибка вставки пациента:" << e.what();
        }
    }

    file.close();
    qDebug() << "Пациенты успешно загружены";
}


void MainWindow::loadAppointmentsFromFile(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Не удалось открыть файл приёмов";
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList parts = line.split("|");
        if (parts.size() != 4) continue;

        QString policyStr = parts[0].trimmed();         // ключ
        QString diagnosis = parts[1].trimmed();
        QString doctor = parts[2].trimmed();
        QStringList dateParts = parts[3].split(" ");

        if (dateParts.size() != 3) continue;

        Appointment a;
        a.diagnosis = diagnosis.toStdString();
        a.doctorType = doctor.toStdString();
        a.appointmentDate.day = dateParts[0].toInt();
        a.appointmentDate.month = monthFromShortString(dateParts[1]);
        a.appointmentDate.year = dateParts[2].toInt();

        avlTree.insert(policyStr.toStdString(), a, appointmentArray);
    }

    file.close();
    qDebug() << "Приёмы успешно загружены";
}


Month monthFromShortString(const QString& shortMonth) {
    if (shortMonth == "янв") return Month::янв;
    if (shortMonth == "фев") return Month::фев;
    if (shortMonth == "мар") return Month::мар;
    if (shortMonth == "апр") return Month::апр;
    if (shortMonth == "май") return Month::май;
    if (shortMonth == "июн") return Month::июн;
    if (shortMonth == "июл") return Month::июл;
    if (shortMonth == "авг") return Month::авг;
    if (shortMonth == "сен") return Month::сен;
    if (shortMonth == "окт") return Month::окт;
    if (shortMonth == "ноя") return Month::ноя;
    if (shortMonth == "дек") return Month::дек;
    return Month::янв; // по умолчанию
}



void MainWindow::updateAppointmentTable(const std::vector<std::string>& policies) {
    appointmentTable->setRowCount(0);

    for (std::size_t i = 0; i < appointmentArray.Size(); ++i) {
        const Appointment& app = appointmentArray[i];

        QString policy = QString::fromStdString(policies[i]);
        QString diagnosis = QString::fromStdString(app.diagnosis);
        QString doctor = QString::fromStdString(app.doctorType);
        QString date = formatDate(app.appointmentDate);

        int row = appointmentTable->rowCount();
        appointmentTable->insertRow(row);
        appointmentTable->setItem(row, 0, new QTableWidgetItem(policy));
        appointmentTable->setItem(row, 1, new QTableWidgetItem(diagnosis));
        appointmentTable->setItem(row, 2, new QTableWidgetItem(doctor));
        appointmentTable->setItem(row, 3, new QTableWidgetItem(date));
    }
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupUI();

    connect(loadPatientsAction, &QAction::triggered, this, [this]() {
        loadPatientsFromFile("patients.txt");
    });

    connect(loadAppointmentsAction, &QAction::triggered, this, [this]() {
        loadAppointmentsFromFile("appointments.txt");
    });

}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::setupUI()
{
    createToolBar();
    createTabs();
}

void MainWindow::createToolBar()
{
    toolBar = addToolBar("Main Toolbar");

    loadPatientsAction = new QAction("Загрузить пациентов", this);
    loadAppointmentsAction = new QAction("Загрузить приёмы", this);
    addPatientAction = new QAction("Добавить пациента", this);
    addAppointmentAction = new QAction("Добавить приём", this);
    deletePatientAction = new QAction("Удалить пациента", this);
    deleteAppointmentAction = new QAction("Удалить приём", this);
    debugAction = new QAction("Окно отладки", this);
    reportAction = new QAction("Сформировать отчёт", this);

    toolBar->addAction(loadPatientsAction);
    toolBar->addAction(loadAppointmentsAction);
    toolBar->addSeparator();
    toolBar->addAction(addPatientAction);
    toolBar->addAction(addAppointmentAction);
    toolBar->addSeparator();
    toolBar->addAction(deletePatientAction);
    toolBar->addAction(deleteAppointmentAction);
    toolBar->addSeparator();
    toolBar->addAction(debugAction);
    toolBar->addAction(reportAction);
}

void MainWindow::createTabs() {
    tabWidget = new QTabWidget(this);
    setCentralWidget(tabWidget);

    // Вкладка: Пациенты
    patientTable = new QTableWidget(this);
    patientTable->setColumnCount(3);
    patientTable->setHorizontalHeaderLabels({"ФИО", "Полис ОМС", "Дата рождения"});
    patientTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Вкладка: Приемы
    appointmentTable = new QTableWidget(this);
    appointmentTable->setColumnCount(4);
    appointmentTable->setHorizontalHeaderLabels({"Полис ОМС", "Диагноз", "Врач", "Дата приёма"});
    appointmentTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Вкладка: Отчёт
    reportTable = new QTableWidget(this);
    reportTable->setColumnCount(4);
    reportTable->setHorizontalHeaderLabels({"Полис ОМС", "Врач", "Диагноз", "Дата приёма"});
    reportTable->setColumnWidth(0, 150);
    reportTable->setColumnWidth(1, 150);
    reportTable->setColumnWidth(2, 150);
    reportTable->setColumnWidth(3, 150);
    reportTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Добавляем вкладки
    tabWidget->addTab(patientTable, "Пациенты");
    tabWidget->addTab(appointmentTable, "Приемы");
    tabWidget->addTab(reportTable, "Отчёт");

    // Вкладка: Визуализация хеш-таблицы
    // Вкладка: Хэш таблица
    hashTableView = new QTableWidget(this);
    hashTableView->setColumnCount(7);
    hashTableView->setHorizontalHeaderLabels({
        "Индекс", "Хэш", "Статус", "Ключ", "ФИО", "Дата рождения", "Индекс в массиве"
    });
    hashTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    tabWidget->addTab(hashTableView, "Хэш таблица");


    // Вкладка: Визуализация дерева
    treeGraphicsView = new QGraphicsView(this);
    treeScene = new QGraphicsScene(this);
    treeGraphicsView->setScene(treeScene);
    tabWidget->addTab(treeGraphicsView, "Дерево");


}

