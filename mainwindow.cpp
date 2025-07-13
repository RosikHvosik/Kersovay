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
#include <QDialog>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QFont>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QGraphicsLineItem>
#include <QGraphicsSceneMouseEvent>
#include <QToolTip>
#include <QApplication>
#include <QObject>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <climits>
#include <map>
#include <functional>
#include <QLabel>
#include "array.h"
#include "hashtable.hpp"
#include "avltree3.hpp"
#include "types.h"
// В mainwindow.cpp замените класс TreeNodeItem на этот исправленный:

// В mainwindow.cpp замените класс TreeNodeItem на этот исправленный:

class TreeNodeItem : public QGraphicsEllipseItem
{
public:
    TreeNodeItem(const QString& key, const std::vector<std::size_t>& indices,
                 HashTable* hashTable, qreal x, qreal y, qreal width = 100, qreal height = 50)
        : QGraphicsEllipseItem(x - width/2, y - height/2, width, height)
        , m_key(key), m_indices(indices), m_hashTable(hashTable)
    {
        QColor nodeColor;
        if (indices.size() == 1) {
            nodeColor = QColor(144, 238, 144);
        } else if (indices.size() <= 3) {
            nodeColor = QColor(173, 216, 230);
        } else {
            nodeColor = QColor(255, 182, 193);
        }

        setBrush(QBrush(nodeColor));
        setPen(QPen(QColor(70, 130, 180), 2));
        setFlag(QGraphicsItem::ItemIsSelectable, true);
        setFlag(QGraphicsItem::ItemIsFocusable, true);
        setAcceptHoverEvents(true);  // КРИТИЧЕСКИ ВАЖНАЯ СТРОКА!

        QString displayKey = key;
        if (displayKey.length() > 12) {
            displayKey = displayKey.left(4) + "..." + displayKey.right(4);
        }

        QString nodeText = QString("%1\n(%2)").arg(displayKey).arg(indices.size());
        m_textItem = new QGraphicsTextItem(nodeText, this);

        QRectF textRect = m_textItem->boundingRect();
        m_textItem->setPos(-textRect.width()/2, -textRect.height()/2);
        m_textItem->setDefaultTextColor(QColor(0, 0, 139));

        QFont font = m_textItem->font();
        font.setBold(true);
        font.setPointSize(8);
        m_textItem->setFont(font);

        updateTooltip();
    }

    void updateTooltip() {
        QString tooltip = QString("Полис: %1\nКоличество приёмов: %2\n\nПриёмы:\n")
                              .arg(m_key).arg(m_indices.size());

        if (m_hashTable) {
            const Patient* patient = m_hashTable->get(m_key.toStdString());
            if (patient) {
                tooltip += QString("Пациент: %1 %2 %3\n")
                               .arg(QString::fromStdString(patient->surname))
                               .arg(QString::fromStdString(patient->name))
                               .arg(QString::fromStdString(patient->middlename));
                tooltip += QString("Дата рождения: %1.%2.%3\n\n")
                               .arg(patient->birthDate.day, 2, 10, QChar('0'))
                               .arg(static_cast<int>(patient->birthDate.month), 2, 10, QChar('0'))
                               .arg(patient->birthDate.year);
            }
        }

        for (size_t i = 0; i < m_indices.size() && i < 5; ++i) {
            if (m_indices[i] < AppointmentArray.Size()) {
                const Appointment& app = AppointmentArray[m_indices[i]];
                tooltip += QString("%1. %2 - %3 (%4.%5.%6)\n")
                               .arg(i + 1)
                               .arg(QString::fromStdString(app.doctorType))
                               .arg(QString::fromStdString(app.diagnosis))
                               .arg(app.appointmentDate.day, 2, 10, QChar('0'))
                               .arg(static_cast<int>(app.appointmentDate.month), 2, 10, QChar('0'))
                               .arg(app.appointmentDate.year);
            }
        }

        if (m_indices.size() > 5) {
            tooltip += QString("... и ещё %1 приёмов").arg(m_indices.size() - 5);
        }

        setToolTip(tooltip);
    }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override {
        if (event->button() == Qt::LeftButton) {
            QString details = QString("=== ПОДРОБНАЯ ИНФОРМАЦИЯ ===\n\n");
            details += QString("Полис ОМС: %1\n").arg(m_key);
            details += QString("Количество приёмов: %1\n\n").arg(m_indices.size());

            // Показываем в диалоге
            QDialog* dialog = new QDialog();
            dialog->setWindowTitle(QString("Узел дерева: %1").arg(m_key));
            dialog->resize(500, 400);

            QVBoxLayout* layout = new QVBoxLayout(dialog);
            QTextEdit* textEdit = new QTextEdit(dialog);
            textEdit->setPlainText(details);
            textEdit->setReadOnly(true);

            QPushButton* closeBtn = new QPushButton("Закрыть", dialog);
            QObject::connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::accept);

            layout->addWidget(textEdit);
            layout->addWidget(closeBtn);

            dialog->exec();
            dialog->deleteLater();
        }
        QGraphicsEllipseItem::mousePressEvent(event);
    }

    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override {
        setBrush(QBrush(QColor(255, 223, 186)));
        setPen(QPen(QColor(255, 140, 0), 3));
        QGraphicsEllipseItem::hoverEnterEvent(event);
    }

    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override {
        QColor nodeColor;
        if (m_indices.size() == 1) {
            nodeColor = QColor(144, 238, 144);
        } else if (m_indices.size() <= 3) {
            nodeColor = QColor(173, 216, 230);
        } else {
            nodeColor = QColor(255, 182, 193);
        }
        setBrush(QBrush(nodeColor));
        setPen(QPen(QColor(70, 130, 180), 2));
        QGraphicsEllipseItem::hoverLeaveEvent(event);
    }

private:
    QString m_key;
    std::vector<std::size_t> m_indices;
    HashTable* m_hashTable;
    QGraphicsTextItem* m_textItem;
};

using StatusEnum = Status;
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

// В mainwindow.cpp исправьте конструктор MainWindow:

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupUI();
    setupTreeVisualization();  // ЭТОТ МЕТОД ДОЛЖЕН СУЩЕСТВОВАТЬ!

    connect(loadPatientsAction, &QAction::triggered, this, &MainWindow::loadPatientsFromFile);
    connect(loadAppointmentsAction, &QAction::triggered, this, &MainWindow::loadAppointmentsFromFile);
    connect(addPatientAction, &QAction::triggered, this, &MainWindow::addPatient);
    connect(addAppointmentAction, &QAction::triggered, this, &MainWindow::addAppointment);
    connect(deletePatientAction, &QAction::triggered, this, &MainWindow::deletePatient);
    connect(deleteAppointmentAction, &QAction::triggered, this, &MainWindow::deleteAppointment);
    connect(reportAction, &QAction::triggered, this, &MainWindow::generateReport);
    connect(debugAction, &QAction::triggered, this, &MainWindow::showDebugWindow);


    QAction* integrityAction = new QAction("Проверка целостности", this);
    connect(integrityAction, &QAction::triggered, this, &MainWindow::showIntegrityReport);
    toolBar->addSeparator();
    toolBar->addAction(integrityAction);

    QAction* updateTreeAction = new QAction("Обновить дерево", this);
    connect(updateTreeAction, &QAction::triggered, this, &MainWindow::updateTreeVisualization);
    toolBar->addAction(updateTreeAction);
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

void MainWindow::setupTreeVisualization() {
    treeGraphicsView->setMouseTracking(true);
    treeGraphicsView->setRenderHint(QPainter::Antialiasing);
    treeGraphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    treeGraphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    treeGraphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    treeScene->setSceneRect(-500, -100, 1000, 800);
    treeScene->setBackgroundBrush(QBrush(QColor(248, 248, 255)));
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
    // Вкладка: Отчёт
    reportTable = new QTableWidget(this);
    reportTable->setColumnCount(5);
    reportTable->setHorizontalHeaderLabels({
        "Полис ОМС", "Врач", "Диагноз", "Дата приёма", "ФИО пациента"
    });
    reportTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    fioFilterEdit = new QLineEdit(this);
    fioFilterEdit->setPlaceholderText("ФИО пациента (точно)");

    doctorFilterEdit = new QLineEdit(this);
    doctorFilterEdit->setPlaceholderText("Тип врача");

    dateFilterEdit = new QDateEdit(this);
    dateFilterEdit->setDisplayFormat("dd.MM.yyyy");
    dateFilterEdit->setCalendarPopup(true);
    dateFilterEdit->setDate(QDate::currentDate());

    QHBoxLayout* filterLayout = new QHBoxLayout();
    filterLayout->addWidget(new QLabel("ФИО:"));
    filterLayout->addWidget(fioFilterEdit);
    filterLayout->addWidget(new QLabel("Врач:"));
    filterLayout->addWidget(doctorFilterEdit);
    filterLayout->addWidget(new QLabel("Дата:"));
    filterLayout->addWidget(dateFilterEdit);

    QVBoxLayout* reportLayout = new QVBoxLayout();
    reportLayout->addLayout(filterLayout);
    reportLayout->addWidget(reportTable);

    QWidget* reportTab = new QWidget(this);
    reportTab->setLayout(reportLayout);



    // Вкладка: Хэш таблица
    hashTableView = new QTableWidget(this);
    hashTableView->setColumnCount(7);
    hashTableView->setHorizontalHeaderLabels({
        "Индекс", "Хэш", "Статус", "Ключ", "ФИО", "Дата рождения", "Индекс в массиве"
    });
    hashTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // НОВАЯ ВКЛАДКА: AVL-дерево (структура)
    avlTreeTableView = new QTableWidget(this);
    avlTreeTableView->setColumnCount(6);
    avlTreeTableView->setHorizontalHeaderLabels({
        "Полис ОМС", "Количество приёмов", "Врач", "Диагноз", "Дата приёма", "Индекс в массиве"
    });
    avlTreeTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Вкладка: Визуализация дерева (графическая)
    treeGraphicsView = new QGraphicsView(this);
    treeScene = new QGraphicsScene(this);
    treeGraphicsView->setScene(treeScene);

    // Добавляем вкладки
    tabWidget->addTab(patientTable, "Пациенты");
    tabWidget->addTab(appointmentTable, "Приемы");
    tabWidget->addTab(reportTab, "Отчёт");  // ← ✅ правильно

    tabWidget->addTab(hashTableView, "Хэш таблица");
    tabWidget->addTab(avlTreeTableView, "AVL-дерево (таблица)");
    tabWidget->addTab(treeGraphicsView, "Дерево (граф)");
}

// Также исправьте метод updateAllTables - уберите дублирование:
/*void MainWindow::updateAllTables() {
    updatePatientTable();
    updateAppointmentTable();
    updateHashTableView();
    updateAVLTreeTableView();
    updateTreeVisualization();  // Вызываем обновление визуализации
}
*/

// АЛЬТЕРНАТИВНО, если хотите добавить это в отдельный метод,
// создайте новый метод setupTreeVisualization() и вызовите его из конструктора:

/*void MainWindow::setupTreeVisualization() {
    // Настройка графического представления дерева
    treeGraphicsView->setMouseTracking(true);
    treeGraphicsView->setRenderHint(QPainter::Antialiasing);
    treeGraphicsView->setDragMode(QGraphicsView::ScrollHandDrag);  // Возможность перетаскивания
    treeGraphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    treeGraphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // Устанавливаем размер сцены
    treeScene->setSceneRect(-500, -100, 1000, 800);

    // Цвет фона сцены
    treeScene->setBackgroundBrush(QBrush(QColor(248, 248, 255))); // Очень светло-голубой фон
}
*/
// Тогда в конструкторе MainWindow добавьте вызов:
/*MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupUI();
    setupTreeVisualization();  // ДОБАВЬТЕ ЭТУ СТРОКУ

    // Подключаем сигналы
    connect(loadPatientsAction, &QAction::triggered, this, &MainWindow::loadPatientsFromFile);
    // ... остальные connections ...
}
*/
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
    searchSplitAction = new QAction("Раздельный поиск", this);
    connect(searchSplitAction, &QAction::triggered, this, &MainWindow::showSplitSearchDialog);
    toolBar->addSeparator();
    toolBar->addAction(searchSplitAction);

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
    int skipped = 0;
    int lineNumber = 0;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        lineNumber++;

        if (line.isEmpty()) continue;

        std::string policy;
        Appointment appointment;

        if (parseAppointmentLine(line, policy, appointment)) {
            // ПРОВЕРКА РЕФЕРЕНЦИАЛЬНОЙ ЦЕЛОСТНОСТИ
            if (!patientExists(policy)) {
                qDebug() << "Строка" << lineNumber << ": Пациент с полисом"
                         << QString::fromStdString(policy) << "не найден. Пропускаем приём.";
                skipped++;
                continue;
            }

            if (avlTree.insert(policy, appointment, AppointmentArray)) {
                appointmentPolicies.push_back(policy);
                loaded++;
            } else {
                qDebug() << "Ошибка вставки приёма на строке" << lineNumber;
                skipped++;
            }
        } else {
            qDebug() << "Ошибка парсинга строки" << lineNumber << ":" << line;
            skipped++;
        }
    }

    file.close();
    updateAllTables();

    QString message = QString("Загрузка завершена:\n"
                              "Загружено приёмов: %1\n"
                              "Пропущено (нет пациента): %2")
                          .arg(loaded).arg(skipped);

    QMessageBox::information(this, "Загрузка завершена", message);
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

        // Индекс в таблице
        hashTableView->setItem(i, 0, new QTableWidgetItem(QString::number(i)));

        // ИСПРАВЛЕНИЕ: Показываем реальное хэш-значение
        if (record.getStatus() == ::Status::Active) {
            std::size_t hashValue = hashTable.getHashValue(record.key);
            hashTableView->setItem(i, 1, new QTableWidgetItem(QString::number(hashValue)));
        } else {
            hashTableView->setItem(i, 1, new QTableWidgetItem("-"));
        }

        // Статус
        QString status = (record.getStatus() == ::Status::Empty) ? "Empty" : "Active";
        hashTableView->setItem(i, 2, new QTableWidgetItem(status));

        // Ключ (полис)
        hashTableView->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(record.getKey())));

        if (record.getStatus() == ::Status::Active) {
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
    updateAVLTreeTableView();
    updatePatientTable();
    updateAppointmentTable();
    updateHashTableView();
    updateAVLTreeTableView();
    updateTreeVisualization();
}

void MainWindow::addPatient() {
    // Диалог для ввода данных пациента
    bool ok;
    QString policyInput = QInputDialog::getText(this, "Добавление пациента",
                                                "Введите полис ОМС (16 цифр):",
                                                QLineEdit::Normal, "", &ok);
    if (!ok || policyInput.isEmpty()) return;

    std::string policy = policyInput.toStdString();

    // Проверяем формат полиса
    if (policy.length() != 16 || !std::all_of(policy.begin(), policy.end(), ::isdigit)) {
        QMessageBox::warning(this, "Ошибка", "Полис должен содержать ровно 16 цифр!");
        return;
    }

    // Проверяем, не существует ли уже такой полис
    if (hashTable.exists(policy)) {
        QMessageBox::warning(this, "Ошибка",
                             QString("Пациент с полисом %1 уже существует!")
                                 .arg(QString::fromStdString(policy)));
        return;
    }

    QString surname = QInputDialog::getText(this, "Добавление пациента",
                                            "Введите фамилию:",
                                            QLineEdit::Normal, "", &ok);
    if (!ok || surname.isEmpty()) return;

    QString name = QInputDialog::getText(this, "Добавление пациента",
                                         "Введите имя:",
                                         QLineEdit::Normal, "", &ok);
    if (!ok || name.isEmpty()) return;

    QString middlename = QInputDialog::getText(this, "Добавление пациента",
                                               "Введите отчество:",
                                               QLineEdit::Normal, "", &ok);
    if (!ok || middlename.isEmpty()) return;

    // Ввод даты рождения
    int day = QInputDialog::getInt(this, "Добавление пациента", "День рождения:", 1, 1, 31, 1, &ok);
    if (!ok) return;

    QStringList months = {"янв", "фев", "мар", "апр", "май", "июн",
                          "июл", "авг", "сен", "окт", "ноя", "дек"};
    QString monthStr = QInputDialog::getItem(this, "Добавление пациента", "Месяц рождения:",
                                             months, 0, false, &ok);
    if (!ok) return;

    int year = QInputDialog::getInt(this, "Добавление пациента", "Год рождения:", 1990, 1900, 2100, 1, &ok);
    if (!ok) return;

    // Формируем строку ФИО для передачи в хэш-таблицу
    QString fullName = QString("%1 %2 %3").arg(surname).arg(name).arg(middlename);

    try {
        Month birthMonth = monthFromShortString(monthStr);

        // Добавляем пациента через хэш-таблицу
        if (hashTable.insert(policy, fullName.toStdString(), day, birthMonth, year)) {
            updateAllTables();
            QMessageBox::information(this, "Успех",
                                     QString("Пациент %1 с полисом %2 добавлен!")
                                         .arg(fullName)
                                         .arg(QString::fromStdString(policy)));
        }
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Ошибка",
                             QString("Не удалось добавить пациента: %1")
                                 .arg(e.what()));
    }
}

void MainWindow::addAppointment() {
    // Диалог для ввода данных приёма
    bool ok;
    QString policyInput = QInputDialog::getText(this, "Добавление приёма",
                                                "Введите полис ОМС (16 цифр):",
                                                QLineEdit::Normal, "", &ok);
    if (!ok || policyInput.isEmpty()) return;

    std::string policy = policyInput.toStdString();

    // Проверяем формат полиса
    if (policy.length() != 16 || !std::all_of(policy.begin(), policy.end(), ::isdigit)) {
        QMessageBox::warning(this, "Ошибка", "Полис должен содержать ровно 16 цифр!");
        return;
    }

    QString doctorType = QInputDialog::getText(this, "Добавление приёма",
                                               "Введите тип врача:",
                                               QLineEdit::Normal, "", &ok);
    if (!ok || doctorType.isEmpty()) return;

    QString diagnosis = QInputDialog::getText(this, "Добавление приёма",
                                              "Введите диагноз:",
                                              QLineEdit::Normal, "", &ok);
    if (!ok || diagnosis.isEmpty()) return;

    // Ввод даты
    int day = QInputDialog::getInt(this, "Добавление приёма", "День:", 1, 1, 31, 1, &ok);
    if (!ok) return;

    QStringList months = {"янв", "фев", "мар", "апр", "май", "июн",
                          "июл", "авг", "сен", "окт", "ноя", "дек"};
    QString monthStr = QInputDialog::getItem(this, "Добавление приёма", "Месяц:",
                                             months, 0, false, &ok);
    if (!ok) return;

    int year = QInputDialog::getInt(this, "Добавление приёма", "Год:", 2024, 1900, 2100, 1, &ok);
    if (!ok) return;

    // Создаём объект приёма
    Appointment appointment;
    appointment.doctorType = doctorType.toStdString();
    appointment.diagnosis = diagnosis.toStdString();
    appointment.appointmentDate.day = day;
    appointment.appointmentDate.month = monthFromShortString(monthStr);
    appointment.appointmentDate.year = year;

    // ПРОВЕРКА РЕФЕРЕНЦИАЛЬНОЙ ЦЕЛОСТНОСТИ
    if (!validateAppointmentData(policy, appointment)) {
        return; // Валидация не прошла
    }

    // Добавляем приём
    if (avlTree.insert(policy, appointment, AppointmentArray)) {
        appointmentPolicies.push_back(policy);
        updateAllTables();

        QMessageBox::information(this, "Успех",
                                 QString("Приём для пациента с полисом %1 добавлен!")
                                     .arg(QString::fromStdString(policy)));
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось добавить приём!");
    }
}

void MainWindow::deletePatient() {
    bool ok;
    QString policyInput = QInputDialog::getText(this, "Удаление пациента",
                                                "Введите полис ОМС пациента для удаления:",
                                                QLineEdit::Normal, "", &ok);
    if (!ok || policyInput.isEmpty()) return;

    std::string policy = policyInput.toStdString();

    // Проверяем существование пациента
    if (!patientExists(policy)) {
        QMessageBox::warning(this, "Ошибка",
                             QString("Пациент с полисом %1 не найден!")
                                 .arg(QString::fromStdString(policy)));
        return;
    }

    // Подтверждение удаления
    int ret = QMessageBox::question(this, "Подтверждение удаления",
                                    QString("Вы уверены, что хотите удалить пациента с полисом %1?\n\n"
                                            "ВНИМАНИЕ: Будут удалены ВСЕ приёмы этого пациента!")
                                        .arg(QString::fromStdString(policy)),
                                    QMessageBox::Yes | QMessageBox::No);

    if (ret != QMessageBox::Yes) return;

    // КАСКАДНОЕ УДАЛЕНИЕ: сначала удаляем все приёмы пациента
    deleteAllAppointmentsForPatient(policy);

    // Затем удаляем самого пациента
    if (hashTable.remove(policy)) {
        updateAllTables();
        QMessageBox::information(this, "Успех",
                                 QString("Пациент с полисом %1 и все его приёмы удалены!")
                                     .arg(QString::fromStdString(policy)));
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось удалить пациента!");
    }
}

void MainWindow::deleteAppointment() {
    bool ok;
    QString policyInput = QInputDialog::getText(this, "Удаление приёма",
                                                "Введите полис ОМС:",
                                                QLineEdit::Normal, "", &ok);
    if (!ok || policyInput.isEmpty()) return;

    std::string policy = policyInput.toStdString();

    QString doctorType = QInputDialog::getText(this, "Удаление приёма",
                                               "Введите тип врача:",
                                               QLineEdit::Normal, "", &ok);
    if (!ok || doctorType.isEmpty()) return;

    QString diagnosis = QInputDialog::getText(this, "Удаление приёма",
                                              "Введите диагноз:",
                                              QLineEdit::Normal, "", &ok);
    if (!ok || diagnosis.isEmpty()) return;

    // Ввод даты для поиска конкретного приёма
    int day = QInputDialog::getInt(this, "Удаление приёма", "День:", 1, 1, 31, 1, &ok);
    if (!ok) return;

    QStringList months = {"янв", "фев", "мар", "апр", "май", "июн",
                          "июл", "авг", "сен", "окт", "ноя", "дек"};
    QString monthStr = QInputDialog::getItem(this, "Удаление приёма", "Месяц:",
                                             months, 0, false, &ok);
    if (!ok) return;

    int year = QInputDialog::getInt(this, "Удаление приёма", "Год:", 2024, 1900, 2100, 1, &ok);
    if (!ok) return;

    // Создаём объект приёма для поиска
    Appointment appointment;
    appointment.doctorType = doctorType.toStdString();
    appointment.diagnosis = diagnosis.toStdString();
    appointment.appointmentDate.day = day;
    appointment.appointmentDate.month = monthFromShortString(monthStr);
    appointment.appointmentDate.year = year;

    // Удаляем приём из дерева
    if (avlTree.remove(policy, appointment, AppointmentArray)) {
        // Также удаляем из вектора appointmentPolicies
        // Это сложнее, так как нужно найти соответствующий индекс
        updateAllTables();
        QMessageBox::information(this, "Успех", "Приём удалён!");
    } else {
        QMessageBox::warning(this, "Ошибка", "Приём не найден или не удалось удалить!");
    }
}

void MainWindow::generateReport() {
    reportTable->clearContents();
    reportTable->setRowCount(0);

    // Получаем значения фильтров
    QString fioText = fioFilterEdit->text().trimmed();
    QString doctorText = doctorFilterEdit->text().trimmed();
    QDate qdate = dateFilterEdit->date();

    std::string fioFilter = fioText.toStdString();
    std::string doctorFilter = doctorText.toStdString();
    Date dateFilter = {qdate.day(), static_cast<Month>(qdate.month()), qdate.year()};

    // 🔴 ПРОВЕРКА: все три поля должны быть заполнены
    if (fioFilter.empty() || doctorFilter.empty()) {
        QMessageBox::warning(this, "Недостаточно данных",
                             "Пожалуйста, заполните все три поля фильтрации:\nФИО, врач и дата.");
        return;
    }

    // 🔍 Поиск по дереву
    avlTree.traverseFiltered(
        [&](const Appointment& app) {
            if (app.doctorType != doctorFilter)
                return false;

            if (!(app.appointmentDate == dateFilter))
                return false;

            return true;
        },
        [&](const Appointment& app) {
            for (std::size_t i = 0; i < AppointmentArray.Size(); ++i) {
                if (AppointmentArray[i] == app) {
                    std::string policy = appointmentPolicies[i];

                    const Patient* patient = hashTable.get(policy);
                    if (!patient) return;

                    std::string fioActual = patient->surname + " " + patient->name + " " + patient->middlename;
                    if (fioActual != fioFilter)
                        return;

                    int row = reportTable->rowCount();
                    reportTable->insertRow(row);

                    reportTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(policy)));
                    reportTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(app.doctorType)));
                    reportTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(app.diagnosis)));
                    reportTable->setItem(row, 3, new QTableWidgetItem(formatDate(app.appointmentDate)));
                    reportTable->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(fioActual)));
                    break;
                }
            }
        },
        AppointmentArray);

    if (reportTable->columnCount() != 5) {
        reportTable->setColumnCount(5);
        reportTable->setHorizontalHeaderLabels({
            "Полис ОМС", "Врач", "Диагноз", "Дата приёма", "ФИО пациента"
        });
    }

    tabWidget->setCurrentIndex(2);

    // 🔽 СПРОСИМ, ХОЧЕТ ЛИ ПОЛЬЗОВАТЕЛЬ СОХРАНИТЬ ОТЧЁТ
    QString savePath = QFileDialog::getSaveFileName(this,
                                                    "Сохранить отчёт в файл", "", "Текстовые файлы (*.txt)");

    if (!savePath.isEmpty()) {
        QFile file(savePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            // ⚠️ НЕ вызывай setCodec — в Qt6 это удалено

            out << "=== ОТЧЁТ О ПРИЁМАХ ПАЦИЕНТОВ ===\n\n";
            out << "Фильтры: \n";
            out << "ФИО: " << fioText << "\n";
            out << "Врач: " << doctorText << "\n";
            out << "Дата: " << qdate.toString("dd.MM.yyyy") << "\n\n";

            out << "Найдено записей: " << reportTable->rowCount() << "\n\n";

            // Заголовки
            for (int col = 0; col < reportTable->columnCount(); ++col) {
                out << reportTable->horizontalHeaderItem(col)->text() << "\t";
            }
            out << "\n";

            // Данные
            for (int row = 0; row < reportTable->rowCount(); ++row) {
                for (int col = 0; col < reportTable->columnCount(); ++col) {
                    QTableWidgetItem* item = reportTable->item(row, col);
                    out << (item ? item->text() : "") << "\t";
                }
                out << "\n";
            }

            file.close();
            QMessageBox::information(this, "Отчёт сохранён",
                                     "Отчёт успешно сохранён в файл:\n" + savePath);
        } else {
            QMessageBox::warning(this, "Ошибка",
                                 "Не удалось сохранить файл отчёта.");
        }
    }

}

void MainWindow::showSplitSearchDialog() {
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("Раздельный поиск");
    dialog->resize(800, 500);

    QVBoxLayout* layout = new QVBoxLayout(dialog);

    // Поле для поиска пациента
    QLineEdit* policyEdit1 = new QLineEdit();
    policyEdit1->setPlaceholderText("Полис ОМС для пациента");

    QTableWidget* patientResult = new QTableWidget();
    patientResult->setColumnCount(3);
    patientResult->setHorizontalHeaderLabels({"ФИО", "Полис", "Дата рождения"});
    patientResult->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QPushButton* searchPatientBtn = new QPushButton("Найти пациента");
    QObject::connect(searchPatientBtn, &QPushButton::clicked, this, [=, this]() {
        patientResult->setRowCount(0);
        std::string policy = policyEdit1->text().trimmed().toStdString();
        const Patient* p = hashTable.get(policy);
        if (!p) {
            QMessageBox::warning(this, "Ошибка", "Пациент не найден.");
            return;
        }
        patientResult->insertRow(0);
        patientResult->setItem(0, 0, new QTableWidgetItem(QString::fromStdString(p->surname + " " + p->name + " " + p->middlename)));
        patientResult->setItem(0, 1, new QTableWidgetItem(QString::fromStdString(policy)));
        patientResult->setItem(0, 2, new QTableWidgetItem(formatDate(p->birthDate)));
    });

    // Поле для поиска приёмов
    QLineEdit* policyEdit2 = new QLineEdit();
    policyEdit2->setPlaceholderText("Полис ОМС для приёмов");

    QTableWidget* appointmentResult = new QTableWidget();
    appointmentResult->setColumnCount(4);
    appointmentResult->setHorizontalHeaderLabels({"Диагноз", "Врач", "Дата приёма", "Индекс"});
    appointmentResult->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QPushButton* searchAppointmentsBtn = new QPushButton("Найти приёмы");
    QObject::connect(searchAppointmentsBtn, &QPushButton::clicked, this, [=, this]() {
        appointmentResult->setRowCount(0);
        std::string policy = policyEdit2->text().trimmed().toStdString();
        avlTree.traverseByKey(policy, [&](std::size_t index) {
            if (index >= AppointmentArray.Size()) return;
            const Appointment& a = AppointmentArray[index];
            int row = appointmentResult->rowCount();
            appointmentResult->insertRow(row);
            appointmentResult->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(a.diagnosis)));
            appointmentResult->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(a.doctorType)));
            appointmentResult->setItem(row, 2, new QTableWidgetItem(formatDate(a.appointmentDate)));
            appointmentResult->setItem(row, 3, new QTableWidgetItem(QString::number(index)));
        });
    });

    // Добавим в макет
    layout->addWidget(policyEdit1);
    layout->addWidget(searchPatientBtn);
    layout->addWidget(patientResult);
    layout->addSpacing(10);
    layout->addWidget(policyEdit2);
    layout->addWidget(searchAppointmentsBtn);
    layout->addWidget(appointmentResult);

    dialog->setLayout(layout);
    dialog->exec();
}




void MainWindow::showDebugWindow() {
    // Базовая статистика
    auto hashStats = hashTable.getStatistics();
    auto treeStats = avlTree.getStatistics();

    // Анализ коллизий в хэш-таблице
    int collisions = 0;
    int maxChainLength = 0;
    int emptySlots = 0;

    std::vector<int> chainLengths(hashTable.getSize(), 0);

    for (std::size_t i = 0; i < hashTable.getSize(); ++i) {
        const HashRecord& record = hashTable.getRecord(i);
        if (record.getStatus() == ::Status::Active) {  // ИСПРАВЛЕНО
            // Вычисляем, где должен был бы находиться элемент изначально
            std::size_t idealPos = hashTable.getHashValue(record.key);
            if (idealPos != i) {
                collisions++;
            }
        } else {
            emptySlots++;
        }
    }

    // Статистика AVL-дерева
    std::vector<std::string> treeKeys = avlTree.getAllKeys();
    int totalAppointments = 0;
    int maxAppointmentsPerPolicy = 0;
    int minAppointmentsPerPolicy = INT_MAX;

    for (const auto& key : treeKeys) {
        int count = avlTree.getCountForKey(key);
        totalAppointments += count;
        maxAppointmentsPerPolicy = std::max(maxAppointmentsPerPolicy, count);
        minAppointmentsPerPolicy = std::min(minAppointmentsPerPolicy, count);
    }

    if (treeKeys.empty()) {
        minAppointmentsPerPolicy = 0;
    }

    double avgAppointmentsPerPolicy = treeKeys.empty() ? 0.0 :
                                          static_cast<double>(totalAppointments) / treeKeys.size();

    QString debug = QString(
                        "=== ПОДРОБНАЯ ДИАГНОСТИКА СИСТЕМЫ ===\n\n"

                        "МАССИВЫ:\n"
                        "• Пациенты: %1/%2 (%3% заполнено)\n"
                        "• Приёмы: %4/%5 (%6% заполнено)\n"
                        "• Свободно места для пациентов: %7\n"
                        "• Свободно места для приёмов: %8\n\n"

                        "ХЭШ-ТАБЛИЦА:\n"
                        "• Размер таблицы: %9 слотов\n"
                        "• Занято: %10 (%11%)\n"
                        "• Пусто: %12 (%13%)\n"
                        "• Коллизии: %14\n"
                        "• Коэффициент загрузки: %15%\n\n"

                        "AVL-ДЕРЕВО:\n"
                        "• Узлов в дереве: %16\n"
                        "• Общий объём приёмов: %17\n"
                        "• Уникальных полисов: %18\n"
                        "• Максимальная глубина: %19\n"
                        "• Приёмов на полис (мин/макс/сред): %20/%21/%.1f\n\n"

                        "БАЛАНСИРОВКА ДЕРЕВА:\n"
                        "• Коэффициент сбалансированности: %22\n"
                        "• Эффективность: %23%\n\n"

                        "РЕФЕРЕНЦИАЛЬНАЯ ЦЕЛОСТНОСТЬ:\n"
                        "• Статус: %24\n"
                        "• Приёмов без пациентов: %25\n"
                        "• Соответствие размеров: %26"
                        )
                        .arg(PatientArray.Size())                                    // 1
                        .arg(PatientArray.GetCapacity())                            // 2
                        .arg(PatientArray.Size() * 100.0 / PatientArray.GetCapacity(), 0, 'f', 1)  // 3
                        .arg(AppointmentArray.Size())                               // 4
                        .arg(AppointmentArray.GetCapacity())                        // 5
                        .arg(AppointmentArray.Size() * 100.0 / AppointmentArray.GetCapacity(), 0, 'f', 1)  // 6
                        .arg(PatientArray.GetCapacity() - PatientArray.Size())      // 7
                        .arg(AppointmentArray.GetCapacity() - AppointmentArray.Size())  // 8
                        .arg(hashStats.totalSlots)                                  // 9
                        .arg(hashStats.usedSlots)                                   // 10
                        .arg(hashStats.loadFactor * 100, 0, 'f', 1)               // 11
                        .arg(hashStats.emptySlots)                                  // 12
                        .arg(hashStats.emptySlots * 100.0 / hashStats.totalSlots, 0, 'f', 1)  // 13
                        .arg(collisions)                                            // 14
                        .arg(hashStats.loadFactor * 100, 0, 'f', 1)               // 15
                        .arg(treeStats.totalNodes)                                  // 16
                        .arg(treeStats.totalElements)                               // 17
                        .arg(treeStats.uniqueKeys)                                  // 18
                        .arg(treeStats.maxDepth)                                    // 19
                        .arg(minAppointmentsPerPolicy)                              // 20
                        .arg(maxAppointmentsPerPolicy)                              // 21
                        .arg(avgAppointmentsPerPolicy)                              // 22 (используется %.1f выше)
                        .arg(treeStats.maxDepth <= std::log2(treeStats.totalNodes) + 2 ? "Хорошая" : "Требует внимания")  // 22
                        .arg(treeStats.totalNodes > 0 ? 100.0 * std::log2(treeStats.totalNodes) / treeStats.maxDepth : 100, 0, 'f', 1)  // 23
                        .arg("Проверяется...")                                      // 24
                        .arg("Подсчитывается...")                                   // 25
                        .arg(PatientArray.Size() >= treeKeys.size() ? "✓" : "⚠️")  // 26
        ;

    // Создаем диалог с прокруткой для большого текста
    QDialog* debugDialog = new QDialog(this);
    debugDialog->setWindowTitle("Подробная диагностика");
    debugDialog->resize(600, 500);

    QVBoxLayout* layout = new QVBoxLayout(debugDialog);
    QTextEdit* textEdit = new QTextEdit(debugDialog);
    textEdit->setPlainText(debug);
    textEdit->setReadOnly(true);
    textEdit->setFont(QFont("Courier", 9)); // Моноширинный шрифт для лучшего выравнивания

    QPushButton* closeButton = new QPushButton("Закрыть", debugDialog);
    connect(closeButton, &QPushButton::clicked, debugDialog, &QDialog::accept);

    layout->addWidget(textEdit);
    layout->addWidget(closeButton);

    debugDialog->exec();
    debugDialog->deleteLater();
}

bool MainWindow::validateAppointmentData(const std::string& policy, const Appointment& appointment) {
    // Проверяем существование пациента
    if (!patientExists(policy)) {
        QMessageBox::warning(this, "Ошибка валидации",
                             QString("Пациент с полисом %1 не найден в системе!\n"
                                     "Сначала добавьте пациента.")
                                 .arg(QString::fromStdString(policy)));
        return false;
    }

    // Дополнительные проверки данных приёма
    if (appointment.doctorType.empty() || appointment.diagnosis.empty()) {
        QMessageBox::warning(this, "Ошибка валидации",
                             "Тип врача и диагноз не могут быть пустыми!");
        return false;
    }

    if (appointment.appointmentDate.day < 1 || appointment.appointmentDate.day > 31 ||
        appointment.appointmentDate.year < 1900 || appointment.appointmentDate.year > 2100) {
        QMessageBox::warning(this, "Ошибка валидации",
                             "Некорректная дата приёма!");
        return false;
    }

    return true;
}
void MainWindow::checkReferentialIntegrity() {
    qDebug().noquote() << "=== ПРОВЕРКА РЕФЕРЕНЦИАЛЬНОЙ ЦЕЛОСТНОСТИ ===";

    // Получаем все полисы из хэш-таблицы (пациенты)
    std::vector<std::string> patientPolicies = hashTable.getAllPolicies();

    // Получаем все полисы из AVL-дерева (приёмы)
    std::vector<std::string> appointmentPolicies = avlTree.getAllKeys();

    // Проверяем, есть ли приёмы без пациентов
    std::vector<std::string> orphanedAppointments;
    for (const auto& policy : appointmentPolicies) {
        if (!hashTable.exists(policy)) {
            orphanedAppointments.push_back(policy);
        }
    }

    qDebug().noquote() << QString("Пациентов в системе: %1").arg(patientPolicies.size());
    qDebug().noquote() << QString("Уникальных полисов с приёмами: %1").arg(appointmentPolicies.size());
    qDebug().noquote() << QString("Приёмов без пациентов (нарушение целостности): %1")
                              .arg(orphanedAppointments.size());

    if (!orphanedAppointments.empty()) {
        qDebug().noquote() << "Проблемные полисы:";
        for (const auto& policy : orphanedAppointments) {
            qDebug().noquote() << QString("  - %1").arg(QString::fromStdString(policy));
        }
    }
}


bool MainWindow::isValidPolicy(const std::string& policy) const {
    if (policy.length() != 16) return false;

    for (char c : policy) {
        if (!std::isdigit(c)) return false;
    }

    return true;
}

void MainWindow::generateIntegrityReport() {
    // Проверка целостности
    checkReferentialIntegrity();

    // Статистика хэш-таблицы
    auto hashStats = hashTable.getStatistics();

    // Статистика дерева
    auto treeStats = avlTree.getStatistics();

    // Проверка целостности дерева
    bool treeIntegrity = avlTree.validateIntegrity(AppointmentArray);

    QString report = QString(
                         "=== ОТЧЁТ О СОСТОЯНИИ СИСТЕМЫ ===\n\n"
                         "ХЭШ-ТАБЛИЦА (Пациенты):\n"
                         "• Всего слотов: %1\n"
                         "• Занято слотов: %2\n"
                         "• Свободно слотов: %3\n"
                         "• Коэффициент загрузки: %4%\n\n"

                         "AVL-ДЕРЕВО (Приёмы):\n"
                         "• Узлов в дереве: %5\n"
                         "• Всего приёмов: %6\n"
                         "• Уникальных пациентов с приёмами: %7\n"
                         "• Максимальная глубина: %8\n"
                         "• Целостность структуры: %9\n\n"

                         "МАССИВЫ:\n"
                         "• Пациентов в массиве: %10\n"
                         "• Приёмов в массиве: %11\n\n"

                         "РЕФЕРЕНЦИАЛЬНАЯ ЦЕЛОСТНОСТЬ: %12"
                         )
                         .arg(hashStats.totalSlots)
                         .arg(hashStats.usedSlots)
                         .arg(hashStats.emptySlots)
                         .arg(hashStats.loadFactor * 100, 0, 'f', 1)
                         .arg(treeStats.totalNodes)
                         .arg(treeStats.totalElements)
                         .arg(treeStats.uniqueKeys)
                         .arg(treeStats.maxDepth)
                         .arg(treeIntegrity ? "ОК" : "НАРУШЕНА")
                         .arg(PatientArray.Size())
                         .arg(AppointmentArray.Size())
                         .arg("Проверяется...");

    QMessageBox::information(this, "Отчёт о системе", report);
}

void MainWindow::updateAVLTreeTableView() {
    avlTreeTableView->setRowCount(0);

    qDebug().noquote() << "[updateAVLTreeTableView] Начинаем обновление таблицы AVL-дерева";

    // Создаем структуру для хранения уникальных записей по полисам
    std::map<std::string, std::vector<std::pair<Appointment, std::size_t>>> policyData;

    // Сначала собираем все данные
    avlTree.traverse([&policyData](const Appointment& appointment, const std::string& policy) {
        // Находим индекс записи в массиве
        std::size_t arrayIndex = 0;
        for (std::size_t i = 0; i < AppointmentArray.Size(); ++i) {
            if (AppointmentArray[i] == appointment) {
                arrayIndex = i;
                break;
            }
        }
        policyData[policy].push_back(std::make_pair(appointment, arrayIndex));
    }, AppointmentArray);

    // Теперь заполняем таблицу
    for (const auto& [policy, appointments] : policyData) {
        int appointmentCount = appointments.size();

        // Ищем информацию о пациенте
        QString patientInfo = "Неизвестен";
        const Patient* patient = hashTable.get(policy);
        if (patient) {
            patientInfo = QString("%1 %2 %3")
            .arg(QString::fromStdString(patient->surname))
                .arg(QString::fromStdString(patient->name))
                .arg(QString::fromStdString(patient->middlename));
        }

        // Цвет для группировки по полисам
        QColor rowColor;
        std::hash<std::string> hasher;
        std::size_t hash = hasher(policy);
        int colorIndex = hash % 6;

        switch (colorIndex) {
        case 0: rowColor = QColor(255, 245, 245); break; // Светло-красный
        case 1: rowColor = QColor(245, 255, 245); break; // Светло-зеленый
        case 2: rowColor = QColor(245, 245, 255); break; // Светло-синий
        case 3: rowColor = QColor(255, 255, 245); break; // Светло-желтый
        case 4: rowColor = QColor(255, 245, 255); break; // Светло-розовый
        case 5: rowColor = QColor(245, 255, 255); break; // Светло-голубой
        }

        // Добавляем строки для каждого приёма
        for (const auto& [appointment, arrayIndex] : appointments) {
            int row = avlTreeTableView->rowCount();
            avlTreeTableView->insertRow(row);

            // Создаем элементы таблицы с явным текстом и цветом
            QTableWidgetItem* policyItem = new QTableWidgetItem(QString::fromStdString(policy));
            QTableWidgetItem* countItem = new QTableWidgetItem(QString::number(appointmentCount));
            QTableWidgetItem* doctorItem = new QTableWidgetItem(QString::fromStdString(appointment.doctorType));
            QTableWidgetItem* diagnosisItem = new QTableWidgetItem(QString::fromStdString(appointment.diagnosis));
            QTableWidgetItem* dateItem = new QTableWidgetItem(formatDate(appointment.appointmentDate));
            QTableWidgetItem* indexItem = new QTableWidgetItem(QString::number(arrayIndex));

            // Устанавливаем цвет фона для всех элементов
            policyItem->setBackground(rowColor);
            countItem->setBackground(rowColor);
            doctorItem->setBackground(rowColor);
            diagnosisItem->setBackground(rowColor);
            dateItem->setBackground(rowColor);
            indexItem->setBackground(rowColor);

            // Устанавливаем цвет текста (черный для контраста)
            policyItem->setForeground(QColor(0, 0, 0));
            countItem->setForeground(QColor(0, 0, 0));
            doctorItem->setForeground(QColor(0, 0, 0));
            diagnosisItem->setForeground(QColor(0, 0, 0));
            dateItem->setForeground(QColor(0, 0, 0));
            indexItem->setForeground(QColor(0, 0, 0));

            // Добавляем элементы в таблицу
            avlTreeTableView->setItem(row, 0, policyItem);
            avlTreeTableView->setItem(row, 1, countItem);
            avlTreeTableView->setItem(row, 2, doctorItem);
            avlTreeTableView->setItem(row, 3, diagnosisItem);
            avlTreeTableView->setItem(row, 4, dateItem);
            avlTreeTableView->setItem(row, 5, indexItem);
        }
    }

    qDebug().noquote() << QString("[updateAVLTreeTableView] Таблица обновлена, строк: %1")
                              .arg(avlTreeTableView->rowCount());
}

bool MainWindow::patientExists(const std::string& policy) const {
    const Patient* patient = hashTable.get(policy);
    return patient != nullptr;
}


void MainWindow::deleteAllAppointmentsForPatient(const std::string& policy) {
    qDebug().noquote() << QString("=== КАСКАДНОЕ УДАЛЕНИЕ приёмов для полиса: %1 ===")
                              .arg(QString::fromStdString(policy));

    // Удаляем все приёмы с данным полисом из AVL-дерева
    if (avlTree.removeAllByKey(policy)) {
        qDebug().noquote() << "→ Все приёмы удалены из дерева";

        // Также нужно удалить соответствующие элементы из вектора appointmentPolicies
        auto it = appointmentPolicies.begin();
        while (it != appointmentPolicies.end()) {
            if (*it == policy) {
                it = appointmentPolicies.erase(it);
                qDebug().noquote() << "→ Удалён полис из вектора";
            } else {
                ++it;
            }
        }
    } else {
        qDebug().noquote() << "→ Приёмы с данным полисом не найдены";
    }
}


void MainWindow::updateTreeVisualization()
{
    qDebug().noquote() << "[updateTreeVisualization] Начинаем обновление визуализации дерева";
    clearTreeVisualization();
    drawTree();
}

void MainWindow::clearTreeVisualization()
{
    treeScene->clear();
    treeNodes.clear();
}

void MainWindow::drawTree()
{
    auto root = avlTree.getRoot();
    if (!root) {
        // Если дерево пустое, показываем сообщение
        QGraphicsTextItem* emptyText = treeScene->addText("Дерево пустое", QFont("Arial", 16));
        emptyText->setPos(-100, -20);
        emptyText->setDefaultTextColor(QColor(128, 128, 128));
        return;
    }

    // Вычисляем размеры дерева для правильного позиционирования
    int treeHeight = calculateTreeHeight(root);
    int treeWidth = calculateTreeWidth(root);

    // Настраиваем размер сцены
    int sceneWidth = std::max(800, treeWidth * 120);
    int sceneHeight = std::max(600, treeHeight * 100);
    treeScene->setSceneRect(-sceneWidth/2, -50, sceneWidth, sceneHeight);

    // Рисуем дерево рекурсивно
    drawNode(root, 0, 80, sceneWidth * 0.3, 1);

    qDebug().noquote() << QString("[drawTree] Нарисовано узлов: %1").arg(treeNodes.size());
}

// Вспомогательные методы для расчета размеров дерева:
int MainWindow::calculateTreeHeight(AVLNode<std::string, Appointment, Array<Appointment, 1000>>* node)
{
    if (!node) return 0;
    return 1 + std::max(calculateTreeHeight(node->left), calculateTreeHeight(node->right));
}

int MainWindow::calculateTreeWidth(AVLNode<std::string, Appointment, Array<Appointment, 1000>>* node)
{
    if (!node) return 0;
    return 1 + calculateTreeWidth(node->left) + calculateTreeWidth(node->right);
}

void MainWindow::drawNode(AVLNode<std::string, Appointment, Array<Appointment, 1000>>* node,
                          qreal x, qreal y, qreal horizontalSpacing, int level)
{
    if (!node) return;

    // Собираем индексы из связанного списка
    std::vector<std::size_t> indices;
    lNode* current = node->indexList.getHead();
    if (current) {
        do {
            indices.push_back(current->arrayIndex);
            current = current->next;
        } while (current != node->indexList.getHead());
    }

    // Создаем визуальный узел
    QString keyText = QString::fromStdString(node->key);

    // Укорачиваем полис для отображения (показываем только последние 4 цифры)
    if (keyText.length() > 8) {
        keyText = "..." + keyText.right(4);
    }

    TreeNodeItem* nodeItem = new TreeNodeItem(QString::fromStdString(node->key), indices, &hashTable, x, y);
    treeScene->addItem(nodeItem);
    treeNodes.push_back(nodeItem);

    // Рисуем связи с дочерними узлами
    qreal childSpacing = horizontalSpacing / 2;
    qreal childY = y + 100;

    if (node->left) {
        qreal leftX = x - horizontalSpacing;

        // Линия к левому потомку
        QGraphicsLineItem* leftLine = treeScene->addLine(x, y + 20, leftX, childY - 20,
                                                         QPen(QColor(100, 100, 100), 2));

        drawNode(node->left, leftX, childY, childSpacing, level + 1);
    }

    if (node->right) {
        qreal rightX = x + horizontalSpacing;

        // Линия к правому потомку
        QGraphicsLineItem* rightLine = treeScene->addLine(x, y + 20, rightX, childY - 20,
                                                          QPen(QColor(100, 100, 100), 2));

        drawNode(node->right, rightX, childY, childSpacing, level + 1);
    }
}


void MainWindow::showIntegrityReport() {
    generateIntegrityReport();
}
