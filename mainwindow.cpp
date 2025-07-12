#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTableWidget>
#include <QDebug>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QInputDialog>
#include <QStringList>
#include "array.h"
#include "hashtable.hpp"
#include "avltree3.hpp"
extern Array<Patient, 1000> PatientArray;
extern Array<Appointment, 1000> AppointmentArray;

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

QString MainWindow::formatDate(const Date& date) {
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
    setupUI();

    // Подключаем сигналы
    connect(loadPatientsAction, &QAction::triggered, this, &MainWindow::loadPatientsFromFile);
    connect(loadAppointmentsAction, &QAction::triggered, this, &MainWindow::loadAppointmentsFromFile);
    connect(addPatientAction, &QAction::triggered, this, &MainWindow::addPatient);
    connect(addAppointmentAction, &QAction::triggered, this, &MainWindow::addAppointment);
    connect(deletePatientAction, &QAction::triggered, this, &MainWindow::deletePatient);
    connect(deleteAppointmentAction, &QAction::triggered, this, &MainWindow::deleteAppointment);
    connect(reportAction, &QAction::triggered, this, &MainWindow::generateReport);
    connect(debugAction, &QAction::triggered, this, &MainWindow::showDebugWindow);
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
    reportTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Вкладка: Хэш таблица
    hashTableView = new QTableWidget(this);
    hashTableView->setColumnCount(7);
    hashTableView->setHorizontalHeaderLabels({
        "Индекс", "Хэш", "Статус", "Ключ", "ФИО", "Дата рождения", "Индекс в массиве"
    });
    hashTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Вкладка: Визуализация дерева
    treeGraphicsView = new QGraphicsView(this);
    treeScene = new QGraphicsScene(this);
    treeGraphicsView->setScene(treeScene);

    // Добавляем вкладки
    tabWidget->addTab(patientTable, "Пациенты");
    tabWidget->addTab(appointmentTable, "Приемы");
    tabWidget->addTab(reportTable, "Отчёт");
    tabWidget->addTab(hashTableView, "Хэш таблица");
    tabWidget->addTab(treeGraphicsView, "Дерево");
}

void MainWindow::loadPatientsFromFile() {
    QString filename = QFileDialog::getOpenFileName(this,
                                                    "Загрузить файл пациентов", "", "Text Files (*.txt)");

    if (filename.isEmpty()) return;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл пациентов");
        return;
    }

    QTextStream in(&file);
    int loaded = 0;
    int lineNumber = 0;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        lineNumber++;

        if (line.isEmpty()) continue;

        std::string policy;
        Patient patient;

        if (parsePatientLine(line, policy, patient)) {
            try {
                hashTable.insert(policy,
                                 patient.surname + " " + patient.name + " " + patient.middlename,
                                 patient.birthDate.day, patient.birthDate.month, patient.birthDate.year);
                loaded++;
            } catch (const std::exception& e) {
                qDebug() << "Ошибка вставки пациента на строке" << lineNumber << ":" << e.what();
            }
        } else {
            qDebug() << "Ошибка парсинга строки" << lineNumber << ":" << line;
        }
    }

    file.close();
    updateAllTables();

    QMessageBox::information(this, "Загрузка завершена",
                             QString("Загружено пациентов: %1").arg(loaded));
}

void MainWindow::loadAppointmentsFromFile() {
    QString filename = QFileDialog::getOpenFileName(this,
                                                    "Загрузить файл приёмов", "", "Text Files (*.txt)");

    if (filename.isEmpty()) return;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл приёмов");
        return;
    }

    QTextStream in(&file);
    int loaded = 0;
    int lineNumber = 0;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        lineNumber++;

        if (line.isEmpty()) continue;

        std::string policy;
        Appointment appointment;

        if (parseAppointmentLine(line, policy, appointment)) {
            if (avlTree.insert(policy, appointment, AppointmentArray)) {
                appointmentPolicies.push_back(policy);
                loaded++;
            } else {
                qDebug() << "Ошибка вставки приёма на строке" << lineNumber;
            }
        } else {
            qDebug() << "Ошибка парсинга строки" << lineNumber << ":" << line;
        }
    }

    file.close();
    updateAllTables();

    QMessageBox::information(this, "Загрузка завершена",
                             QString("Загружено приёмов: %1").arg(loaded));
}

bool MainWindow::parsePatientLine(const QString& line, std::string& policy, Patient& patient) {
    // Новый формат: 1234 5678 9012 3456 Иванов Иван Иванович 12 янв 1990
    QStringList parts = line.split(" ", Qt::SkipEmptyParts);
    if (parts.size() != 10) return false;

    // Полис = первые 4 части
    QString policyStr = parts[0] + parts[1] + parts[2] + parts[3];
    policy = policyStr.toStdString();

    // ФИО
    patient.surname = parts[4].toStdString();
    patient.name = parts[5].toStdString();
    patient.middlename = parts[6].toStdString();

    // Дата
    bool ok;
    patient.birthDate.day = parts[7].toInt(&ok);
    if (!ok) return false;

    patient.birthDate.month = monthFromShortString(parts[8]);
    patient.birthDate.year = parts[9].toInt(&ok);
    if (!ok) return false;

    return true;
}


bool MainWindow::parseAppointmentLine(const QString& line, std::string& policy, Appointment& appointment) {
    // Новый формат: полис (4 части), диагноз, врач, день, месяц, год
    QStringList parts = line.split(" ", Qt::SkipEmptyParts);
    if (parts.size() != 9) return false;

    // Полис
    QString policyStr = parts[0] + parts[1] + parts[2] + parts[3];
    policy = policyStr.toStdString();

    // Диагноз и тип врача
    appointment.diagnosis = parts[4].toStdString();
    appointment.doctorType = parts[5].toStdString();

    // Дата
    bool ok;
    appointment.appointmentDate.day = parts[6].toInt(&ok);
    if (!ok) return false;

    appointment.appointmentDate.month = monthFromShortString(parts[7]);
    appointment.appointmentDate.year = parts[8].toInt(&ok);
    if (!ok) return false;

    return true;
}


void MainWindow::updatePatientTable() {
    patientTable->setRowCount(0);

    for (std::size_t i = 0; i < PatientArray.Size(); ++i) {
        const Patient& patient = PatientArray[i];
        std::string policy = hashTable.getKeyForIndex(i);

        QString fio = QString("%1 %2 %3")
                          .arg(QString::fromStdString(patient.surname))
                          .arg(QString::fromStdString(patient.name))
                          .arg(QString::fromStdString(patient.middlename));

        QString birth = formatDate(patient.birthDate);

        int row = patientTable->rowCount();
        patientTable->insertRow(row);
        patientTable->setItem(row, 0, new QTableWidgetItem(fio));
        patientTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(policy)));
        patientTable->setItem(row, 2, new QTableWidgetItem(birth));
    }
}

void MainWindow::updateAppointmentTable() {
    appointmentTable->setRowCount(0);

    for (std::size_t i = 0; i < AppointmentArray.Size() && i < appointmentPolicies.size(); ++i) {
        const Appointment& app = AppointmentArray[i];

        QString policy = QString::fromStdString(appointmentPolicies[i]);
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

void MainWindow::updateHashTableView() {
    hashTableView->setRowCount(hashTable.getSize());

    for (std::size_t i = 0; i < hashTable.getSize(); ++i) {
        const HashRecord& record = hashTable.getRecord(i);

        hashTableView->setItem(i, 0, new QTableWidgetItem(QString::number(i)));
        hashTableView->setItem(i, 1, new QTableWidgetItem(QString::number(i))); // hash функция

        QString status = (record.getStatus() == Status::Empty) ? "Empty" : "Active";
        hashTableView->setItem(i, 2, new QTableWidgetItem(status));
        hashTableView->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(record.getKey())));

        if (record.getStatus() == Status::Active) {
            const Patient& patient = PatientArray[record.getArrayIndex()];
            QString fio = QString("%1 %2 %3")
                              .arg(QString::fromStdString(patient.surname))
                              .arg(QString::fromStdString(patient.name))
                              .arg(QString::fromStdString(patient.middlename));

            hashTableView->setItem(i, 4, new QTableWidgetItem(fio));
            hashTableView->setItem(i, 5, new QTableWidgetItem(formatDate(patient.birthDate)));
            hashTableView->setItem(i, 6, new QTableWidgetItem(QString::number(record.getArrayIndex())));
        } else {
            hashTableView->setItem(i, 4, new QTableWidgetItem(""));
            hashTableView->setItem(i, 5, new QTableWidgetItem(""));
            hashTableView->setItem(i, 6, new QTableWidgetItem(""));
        }
    }
}

void MainWindow::updateTreeView() {
    // Здесь можно добавить визуализацию дерева
    // Пока оставим пустым
}

void MainWindow::updateAllTables() {
    updatePatientTable();
    updateAppointmentTable();
    updateHashTableView();
    updateTreeView();
}

void MainWindow::addPatient() {
    // Диалог добавления пациента
    QMessageBox::information(this, "Добавление", "Функция добавления пациента в разработке");
}

void MainWindow::addAppointment() {
    // Диалог добавления приёма
    QMessageBox::information(this, "Добавление", "Функция добавления приёма в разработке");
}

void MainWindow::deletePatient() {
    // Диалог удаления пациента
    QMessageBox::information(this, "Удаление", "Функция удаления пациента в разработке");
}

void MainWindow::deleteAppointment() {
    // Диалог удаления приёма
    QMessageBox::information(this, "Удаление", "Функция удаления приёма в разработке");
}

void MainWindow::generateReport() {
    reportTable->setRowCount(0);

    // Простой отчёт - показываем все приёмы
    for (std::size_t i = 0; i < AppointmentArray.Size() && i < appointmentPolicies.size(); ++i) {
        const Appointment& app = AppointmentArray[i];

        QString policy = QString::fromStdString(appointmentPolicies[i]);
        QString doctor = QString::fromStdString(app.doctorType);
        QString diagnosis = QString::fromStdString(app.diagnosis);
        QString date = formatDate(app.appointmentDate);

        int row = reportTable->rowCount();
        reportTable->insertRow(row);
        reportTable->setItem(row, 0, new QTableWidgetItem(policy));
        reportTable->setItem(row, 1, new QTableWidgetItem(doctor));
        reportTable->setItem(row, 2, new QTableWidgetItem(diagnosis));
        reportTable->setItem(row, 3, new QTableWidgetItem(date));
    }

    // Переключаемся на вкладку отчёта
    tabWidget->setCurrentIndex(2);
}

void MainWindow::showDebugWindow() {
    QString debug = QString("Пациентов в массиве: %1\n").arg(PatientArray.Size());
    debug += QString("Приёмов в массиве: %1\n").arg(AppointmentArray.Size());
    debug += QString("Записей в хэш-таблице: %1\n").arg(hashTable.getCount());

    QMessageBox::information(this, "Отладочная информация", debug);
}
