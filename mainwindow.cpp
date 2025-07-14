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



extern Array<Patient, 1000> PatientArray;
extern Array<Appointment, 1000> AppointmentArray;

// Глобальная переменная для доступа к MainWindow из DateTreeNodeItem
static MainWindow* g_mainWindow = nullptr;

class DateTreeNodeItem : public QGraphicsEllipseItem
{
public:
    DateTreeNodeItem(const QString& dateDisplay, const QString& dateKey,
                     const std::vector<std::size_t>& indices,
                     HashTable* hashTable, qreal x, qreal y, qreal width = 140, qreal height = 70)
        : QGraphicsEllipseItem(x - width/2, y - height/2, width, height)
        , m_dateDisplay(dateDisplay), m_dateKey(dateKey), m_indices(indices), m_hashTable(hashTable)
    {
        // Цвет узла зависит от количества приёмов
        QColor nodeColor;
        if (indices.size() == 1) {
            nodeColor = QColor(144, 238, 144);      // Светло-зеленый
        } else if (indices.size() <= 3) {
            nodeColor = QColor(173, 216, 230);      // Светло-голубой
        } else if (indices.size() <= 5) {
            nodeColor = QColor(255, 218, 185);      // Персиковый
        } else {
            nodeColor = QColor(255, 182, 193);      // Светло-розовый
        }

        setBrush(QBrush(nodeColor));
        setPen(QPen(QColor(70, 130, 180), 2));
        setFlag(QGraphicsItem::ItemIsSelectable, true);
        setAcceptHoverEvents(true);

        QString nodeText = QString("%1\n(%2 приёмов)")
                               .arg(dateDisplay)
                               .arg(indices.size());
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
        QString tooltip = QString("🗓️ ДАТА: %1\n").arg(m_dateDisplay);
        tooltip += QString(" Ключ: %1\n").arg(m_dateKey);
        tooltip += QString(" Приёмов: %2\n").arg(m_indices.size());
        tooltip += QString(" Индексы: [");

        // Показываем первые несколько индексов
        for (size_t i = 0; i < m_indices.size() && i < 10; ++i) {
            if (i > 0) tooltip += ", ";
            tooltip += QString::number(m_indices[i]);
        }
        if (m_indices.size() > 10) {
            tooltip += QString(", ... ещё %1").arg(m_indices.size() - 10);
        }
        tooltip += "]\n\n";

        tooltip += "🏥 ПРИЁМЫ:\n";
        for (size_t i = 0; i < m_indices.size() && i < 7; ++i) {
            std::size_t idx = m_indices[i];
            if (idx < AppointmentArray.Size()) {
                const Appointment& app = AppointmentArray[idx];

                // ИСПРАВЛЕНИЕ: Получаем полис через глобальную переменную
                QString policy = "НЕТ_ПОЛИСА";
                if (g_mainWindow && idx < g_mainWindow->appointmentPolicies.size()) {
                    policy = QString::fromStdString(g_mainWindow->appointmentPolicies[idx]);

                    // Сокращаем полис для отображения
                    if (policy.length() > 8) {
                        policy = "..." + policy.right(4);
                    }
                }

                tooltip += QString("%1. [%2] %3 → %4 (полис: %5)\n")
                               .arg(i + 1)
                               .arg(idx)
                               .arg(QString::fromStdString(app.doctorType))
                               .arg(QString::fromStdString(app.diagnosis))
                               .arg(policy);
            } else {
                tooltip += QString("%1. [%2]  ОШИБКА: индекс вне массива!\n")
                               .arg(i + 1).arg(idx);
            }
        }

        if (m_indices.size() > 7) {
            tooltip += QString("... и ещё %1 приёмов").arg(m_indices.size() - 7);
        }

        setToolTip(tooltip);
    }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override {
        if (event->button() == Qt::LeftButton) {
            // Создаем детальное окно с отладочной информацией
            QString details = QString("️ ПОДРОБНАЯ ИНФОРМАЦИЯ О ДАТЕ %1\n\n").arg(m_dateDisplay);
            details += QString("Ключ в дереве: %1\n").arg(m_dateKey);
            details += QString("Общее количество приёмов: %1\n").arg(m_indices.size());
            details += QString("Все индексы: [");

            for (size_t i = 0; i < m_indices.size(); ++i) {
                if (i > 0) details += ", ";
                details += QString::number(m_indices[i]);
            }
            details += "]\n\n";

            details += "ВСЕ ПРИЁМЫ НА ЭТУ ДАТУ:\n";
            details += QString(80, '=') + "\n";


            for (size_t i = 0; i < m_indices.size(); ++i) {
                std::size_t idx = m_indices[i];
                details += QString("\n%1. ПРИЁМ [индекс %2]:\n").arg(i + 1).arg(idx);

                if (idx < AppointmentArray.Size()) {
                    const Appointment& app = AppointmentArray[idx];

                    details += QString(" Врач: %1\n").arg(QString::fromStdString(app.doctorType));
                    details += QString(" Диагноз: %1\n").arg(QString::fromStdString(app.diagnosis));
                    details += QString(" Дата: %1.%2.%3\n")
                                   .arg(app.appointmentDate.day, 2, 10, QChar('0'))
                                   .arg(static_cast<int>(app.appointmentDate.month), 2, 10, QChar('0'))
                                   .arg(app.appointmentDate.year);

                    // ИСПРАВЛЕНИЕ: Ищем полис через глобальную переменную
                    if (g_mainWindow && idx < g_mainWindow->appointmentPolicies.size()) {
                        QString policy = QString::fromStdString(g_mainWindow->appointmentPolicies[idx]);
                        details += QString("Полис ОМС: %1\n").arg(policy);

                        // Ищем пациента
                        if (m_hashTable) {
                            const Patient* patient = m_hashTable->get(policy.toStdString());
                            if (patient) {
                                details += QString("   👤 Пациент: %1 %2 %3\n")
                                               .arg(QString::fromStdString(patient->surname))
                                               .arg(QString::fromStdString(patient->name))
                                               .arg(QString::fromStdString(patient->middlename));
                            } else {
                                details += "   Пациент не найден в справочнике!\n";
                            }
                        }
                    } else {
                        details += "   Полис не найден в индексах!\n";
                    }
                } else {
                    details += "    КРИТИЧЕСКАЯ ОШИБКА: индекс вне массива приёмов!\n";
                }

                details += QString(40, '-')
 + "\n";
            }

            // Показываем в диалоге
            QDialog* dialog = new QDialog();
            dialog->setWindowTitle(QString("Узел дерева дат: %1").arg(m_dateDisplay));
            dialog->resize(700, 500);

            QVBoxLayout* layout = new QVBoxLayout(dialog);
            QTextEdit* textEdit = new QTextEdit(dialog);
            textEdit->setPlainText(details);
            textEdit->setReadOnly(true);
            textEdit->setFont(QFont("Courier", 9));

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

        // Обновляем tooltip при наведении (на случай изменения данных)
        updateTooltip();

        QGraphicsEllipseItem::hoverEnterEvent(event);
    }

    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override {
        QColor nodeColor;
        if (m_indices.size() == 1) {
            nodeColor = QColor(144, 238, 144);
        } else if (m_indices.size() <= 3) {
            nodeColor = QColor(173, 216, 230);
        } else if (m_indices.size() <= 5) {
            nodeColor = QColor(255, 218, 185);
        } else {
            nodeColor = QColor(255, 182, 193);
        }
        setBrush(QBrush(nodeColor));
        setPen(QPen(QColor(70, 130, 180), 2));
        QGraphicsEllipseItem::hoverLeaveEvent(event);
    }

private:
    QString m_dateDisplay;
    QString m_dateKey;
    std::vector<std::size_t> m_indices;
    HashTable* m_hashTable;
    QGraphicsTextItem* m_textItem;
};
// В mainwindow.cpp замените класс TreeNodeItem на этот исправленный:
bool isValidStringField(const std::string& input) {
    if (input.length() < 2)
        return false;

    for (char c : input) {
        if (!std::isalpha(static_cast<unsigned char>(c)))
            return false;
    }

    return true;
}
// В mainwindow.cpp замените класс TreeNodeItem на этот исправленный:
bool isValidDate(const Date& date) {
    int maxDays = 31;

    switch (date.month) {
    case Month::апр:
    case Month::июн:
    case Month::сен:
    case Month::ноя:
        maxDays = 30;
        break;
    case Month::фев:
        maxDays = 29; // без високосных годов
        break;
    default:
        maxDays = 31;
    }

    return date.day >= 1 && date.day <= maxDays &&
           date.year >= 1900 && date.year <= 2100;
}
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
        setAcceptHoverEvents(true);

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
    setupTreeVisualization();

    // ИСПРАВЛЕНИЕ: Устанавливаем дерево дат по умолчанию
    currentTreeType = CurrentTreeType::DateTree;

    // Подключаем сигналы
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

    // ИСПРАВЛЕНИЕ: Правильное состояние кнопок
    showPolicyTreeAction->setEnabled(true);
    showDateTreeAction->setEnabled(false);  // Дерево дат активно по умолчанию

    // ДОБАВЛЯЕМ: Показываем сообщение о пустом дереве при запуске
    showEmptyTreeMessage("Загрузите приёмы для построения дерева дат");
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
    treeGraphicsView->setDragMode(QGraphicsView::ScrollHandDrag);  // перетаскивание мышкой
    treeGraphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    treeGraphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // Увеличенная сцена: 10 000 x 5 000
    treeScene->setSceneRect(-5000, -2500, 10000, 5000);

    treeScene->setBackgroundBrush(QBrush(QColor(248, 248, 255))); // светло-серый фон
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



void MainWindow::showPolicyTree() {
    qDebug().noquote() << "[showPolicyTree] Переключение на дерево по ОМС";

    currentTreeType = CurrentTreeType::PolicyTree;

    // Переключаемся на вкладку дерева
    tabWidget->setCurrentIndex(5);

    // Обновляем дерево
    updateCurrentTree();

    // Обновляем состояние кнопок
    showPolicyTreeAction->setEnabled(false);
    showDateTreeAction->setEnabled(true);
}

void MainWindow::showDateTree() {
    qDebug().noquote() << "[showDateTree] Переключение на дерево по датам";

    currentTreeType = CurrentTreeType::DateTree;

    // Переключаемся на вкладку дерева
    tabWidget->setCurrentIndex(5);

    // Обновляем дерево
    updateCurrentTree();

    // Обновляем состояние кнопок
    showPolicyTreeAction->setEnabled(true);
    showDateTreeAction->setEnabled(false);
}

void MainWindow::updateCurrentTree() {
    qDebug().noquote() << "[updateCurrentTree] Обновление текущего дерева";

    // Очищаем сцену
    clearTreeVisualization();

    if (currentTreeType == CurrentTreeType::PolicyTree) {
        // Дерево по ОМС
        auto root = avlTree.getRoot();
        if (root) {
            drawTreeByPolicy(root);
            qDebug().noquote() << "→ Основное дерево (ОМС) обновлено";
        } else {
            showEmptyTreeMessage("Основное дерево (по ОМС) пустое\nЗагрузите приёмы");
        }
    } else {
        // Дерево по датам - ВСЕГДА перестраиваем перед отображением
        if (AppointmentArray.Size() > 0) {
            buildDateTreeForReport();
            auto root = dateTree.getRoot();
            if (root) {
                drawTreeByDate(root);
                qDebug().noquote() << "→ Дерево дат обновлено";
            } else {
                showEmptyTreeMessage("Ошибка построения дерева дат");
            }
        } else {
            showEmptyTreeMessage("Дерево дат пустое\nЗагрузите приёмы для построения дерева");
        }
    }

    treeGraphicsView->centerOn(0, 0);
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
    searchSplitAction = new QAction("Раздельный поиск", this);

    // Кнопки деревьев
    showPolicyTreeAction = new QAction("Дерево ОМС", this);
    showPolicyTreeAction->setToolTip("Показать основное дерево (по полисам ОМС)");

    showDateTreeAction = new QAction("Дерево дат", this);
    showDateTreeAction->setToolTip("Показать дерево отчетов (по датам приёмов)");

    updateTreeAction = new QAction("Обновить дерево", this);
    updateTreeAction->setToolTip("Обновить текущее дерево");

    // НОВАЯ КНОПКА: Отладка дерева дат
    QAction* debugDateTreeAction = new QAction("Отладка дерева дат", this);
    debugDateTreeAction->setToolTip("Показать структуру узлов дерева дат");

    // Добавляем кнопки
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
    toolBar->addSeparator();
    toolBar->addAction(searchSplitAction);

    // Кнопки деревьев
    toolBar->addSeparator();
    toolBar->addAction(showPolicyTreeAction);
    toolBar->addAction(showDateTreeAction);
    toolBar->addAction(updateTreeAction);
    toolBar->addAction(debugDateTreeAction);  // НОВАЯ КНОПКА

    // Подключаем сигналы
    connect(loadPatientsAction, &QAction::triggered, this, &MainWindow::loadPatientsFromFile);
    connect(loadAppointmentsAction, &QAction::triggered, this, &MainWindow::loadAppointmentsFromFile);
    connect(addPatientAction, &QAction::triggered, this, &MainWindow::addPatient);
    connect(addAppointmentAction, &QAction::triggered, this, &MainWindow::addAppointment);
    connect(deletePatientAction, &QAction::triggered, this, &MainWindow::deletePatient);
    connect(deleteAppointmentAction, &QAction::triggered, this, &MainWindow::deleteAppointment);
    connect(reportAction, &QAction::triggered, this, &MainWindow::generateReport);
    connect(debugAction, &QAction::triggered, this, &MainWindow::showDebugWindow);
    connect(searchSplitAction, &QAction::triggered, this, &MainWindow::showSplitSearchDialog);

    // Сигналы деревьев
    connect(showPolicyTreeAction, &QAction::triggered, this, &MainWindow::showPolicyTree);
    connect(showDateTreeAction, &QAction::triggered, this, &MainWindow::showDateTree);
    connect(updateTreeAction, &QAction::triggered, this, &MainWindow::updateCurrentTree);
    connect(debugDateTreeAction, &QAction::triggered, this, &MainWindow::debugDateTreeNodes);  // НОВЫЙ СИГНАЛ
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

    // ИСПРАВЛЕНИЕ: Сначала обновляем таблицы (БЕЗ дерева)
    updateAllTables();

    // Затем строим дерево дат ОДИН раз
    if (loaded > 0) {
        buildDateTreeForReport();

        // Переключаемся на дерево дат
        currentTreeType = CurrentTreeType::DateTree;
        showPolicyTreeAction->setEnabled(true);
        showDateTreeAction->setEnabled(false);
        tabWidget->setCurrentIndex(5);

        // Обновляем дерево ОДИН раз
        updateCurrentTree();
    }

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
    QStringList parts = line.split(" ", Qt::SkipEmptyParts);

    // Минимум должно быть 8 частей: 4 (полис) + 1 (диагноз) + 1 (врач) + 1 (день) + 1 (месяц) + 1 (год)
    if (parts.size() < 8) {
        qDebug() << "Слишком мало частей в строке:" << parts.size() << "минимум 8";
        qDebug() << "Строка:" << line;
        return false;
    }

    // Полис (первые 4 части)
    QString policyStr = parts[0] + parts[1] + parts[2] + parts[3];
    policy = policyStr.toStdString();

    // Дата и месяц всегда в конце
    bool ok;
    QString yearStr = parts.last();        // последний элемент - год
    QString monthStr = parts[parts.size()-2]; // предпоследний - месяц
    QString dayStr = parts[parts.size()-3];   // третий с конца - день

    int year = yearStr.toInt(&ok);
    if (!ok) {
        qDebug() << "Ошибка парсинга года:" << yearStr;
        return false;
    }

    int day = dayStr.toInt(&ok);
    if (!ok) {
        qDebug() << "Ошибка парсинга дня:" << dayStr;
        return false;
    }

    Month month = monthFromShortString(monthStr);

    // Врач - четвертый элемент с конца
    QString doctorStr = parts[parts.size()-4];

    // Диагноз - все что между полисом и врачом
    QStringList diagnosisParts;
    for (int i = 4; i < parts.size() - 4; ++i) {
        diagnosisParts << parts[i];
    }
    QString diagnosisStr = diagnosisParts.join(" "); // Объединяем обратно пробелами

    // Заполняем структуру
    appointment.doctorType = doctorStr.toStdString();
    appointment.diagnosis = diagnosisStr.toStdString();
    appointment.appointmentDate = {day, month, year};

    qDebug() << "Парсинг успешен:"
             << "полис=" << QString::fromStdString(policy)
             << "диагноз=" << diagnosisStr
             << "врач=" << doctorStr
             << "дата=" << day << monthStr << year;

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
        QString status;
        switch (record.getStatus()) {
        case Status::Empty:   status = "Empty"; break;
        case Status::Active:  status = "Active"; break;
        case Status::Deleted: status = "Deleted"; break;
        }
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
    qDebug().noquote() << "[updateAllTables] Начинаем обновление всех таблиц";

    updatePatientTable();
    updateAppointmentTable();
    updateHashTableView();
    updateAVLTreeTableView();

    // ИСПРАВЛЕНИЕ: НЕ вызываем updateCurrentTree здесь!
    // Это должно вызываться только явно, чтобы избежать дублирования
    qDebug().noquote() << "[updateAllTables] Таблицы обновлены (без дерева)";
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

    if (!isValidStringField(surname.toStdString()) ||
        !isValidStringField(name.toStdString()) ||
        !isValidStringField(middlename.toStdString())) {
        QMessageBox::warning(this, "Ошибка валидации",
                             "Фамилия, имя и отчество должны содержать минимум 2 буквы и не содержать цифр или других символов.");
        return;
    }


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

    if (avlTree.insert(policy, appointment, AppointmentArray)) {
        appointmentPolicies.push_back(policy);

        // ИСПРАВЛЕНИЕ: Сначала таблицы (БЕЗ дерева)
        updateAllTables();

        // Затем дерево дат ОДИН раз
        buildDateTreeForReport();

        // Обновляем только текущее дерево ОДИН раз
        updateCurrentTree();

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
        // ИСПРАВЛЕНИЕ: Сначала таблицы (БЕЗ дерева)
        updateAllTables();

        // Затем дерево дат ОДИН раз
        buildDateTreeForReport();

        // Обновляем только текущее дерево ОДИН раз
        updateCurrentTree();

        QMessageBox::information(this, "Успех", "Приём удалён!");
    } else {
        QMessageBox::warning(this, "Ошибка", "Приём не найден или не удалось удалить!");
    }
}

void MainWindow::debugDateTreeNodes() {
    qDebug().noquote() << "\n=== ОТЛАДКА УЗЛОВ ДЕРЕВА ДАТ ===";

    auto root = dateTree.getRoot();
    if (!root) {
        qDebug().noquote() << "Дерево дат пустое!";
        return;
    }

    std::map<std::string, std::vector<std::size_t>> dateNodes;

    // Собираем все узлы дерева
    dateTree.traverseIndex([&](std::size_t index, const std::string& dateKey) {
        dateNodes[dateKey].push_back(index);
    });

    qDebug().noquote() << QString("Найдено уникальных дат: %1").arg(dateNodes.size());

    for (const auto& [dateKey, indices] : dateNodes) {
        Date date = stringToDate(dateKey);
        QString displayDate = QString("%1.%2.%3")
                                  .arg(date.day, 2, 10, QChar('0'))
                                  .arg(static_cast<int>(date.month), 2, 10, QChar('0'))
                                  .arg(date.year);

        qDebug().noquote() << QString("\n📅 ДАТА: %1 (ключ: %2)")
                                  .arg(displayDate)
                                  .arg(QString::fromStdString(dateKey));
        qDebug().noquote() << QString("   Приёмов: %1").arg(indices.size());

        // Показываем детали каждого приёма
        for (size_t i = 0; i < indices.size() && i < 5; ++i) {
            std::size_t idx = indices[i];
            if (idx < AppointmentArray.Size()) {
                const Appointment& app = AppointmentArray[idx];
                QString policy = (idx < appointmentPolicies.size()) ?
                                     QString::fromStdString(appointmentPolicies[idx]) : "НЕТ_ПОЛИСА";

                qDebug().noquote() << QString("   %1. [%2] %3 → %4 (полис: %5)")
                                          .arg(i + 1)
                                          .arg(idx)
                                          .arg(QString::fromStdString(app.doctorType))
                                          .arg(QString::fromStdString(app.diagnosis))
                                          .arg(policy);
            } else {
                qDebug().noquote() << QString("   %1. [%2] ОШИБКА: индекс вне массива!")
                                          .arg(i + 1).arg(idx);
            }
        }

        if (indices.size() > 5) {
            qDebug().noquote() << QString("   ... и ещё %1 приёмов").arg(indices.size() - 5);
        }
    }

    qDebug().noquote() << "=== КОНЕЦ ОТЛАДКИ УЗЛОВ ===\n";
}


void MainWindow::saveFullReportToFile(const QString& filePath, const std::vector<FullReportRecord>& reportData) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось сохранить файл отчёта.");
        return;
    }

    QTextStream out(&file);

    out << "=== ПОЛНЫЙ ОТЧЁТ О ПРИЁМАХ ПАЦИЕНТОВ ===\n\n";
    out << "Дата формирования: " << QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm") << "\n";
    out << "Система: Медицинские справочники с AVL-деревьями\n\n";

    out << "ПАРАМЕТРЫ ОТЧЕТА:\n";
    out << "ФИО пациента: " << (fioFilterEdit->text().isEmpty() ? "Все" : fioFilterEdit->text()) << "\n";
    out << "Тип врача: " << (doctorFilterEdit->text().isEmpty() ? "Все" : doctorFilterEdit->text()) << "\n";
    out << "Дата приёма: " << dateFilterEdit->date().toString("dd.MM.yyyy") << "\n\n";

    out << QString("РЕЗУЛЬТАТ: %1 записей\n").arg(reportData.size());
    out << QString("=").repeated(120) << "\n\n";

    // Заголовки
    out << QString("%-12s %-15s %-20s %-18s %-15s %-12s %-15s %-12s %-8s %s\n")
               .arg("Дата приёма")
               .arg("Врач")
               .arg("Диагноз")
               .arg("Полис ОМС")
               .arg("Фамилия")
               .arg("Имя")
               .arg("Отчество")
               .arg("Дата рожд.")
               .arg("Индекс")
               .arg("Статус");

    out << QString("-").repeated(120) << "\n";

    // Данные
    for (const auto& record : reportData) {
        out << QString("%-12s %-15s %-20s %-18s %-15s %-12s %-15s %-12s %-8d %s\n")
        .arg(formatDate(record.appointmentDate))
            .arg(QString::fromStdString(record.doctorType))
            .arg(QString::fromStdString(record.diagnosis))
            .arg(QString::fromStdString(record.patientPolicy))
            .arg(QString::fromStdString(record.patientSurname))
            .arg(QString::fromStdString(record.patientName))
            .arg(QString::fromStdString(record.patientMiddlename))
            .arg(formatDate(record.patientBirthDate))
            .arg(record.appointmentIndex)
            .arg(record.patientFound ? "ОК" : "НЕ НАЙДЕН");
    }

    out << "\n" << QString("=").repeated(120) << "\n";
    out << "Конец отчета\n";

    file.close();
    QMessageBox::information(this, "Отчёт сохранён",
                             QString("Полный отчёт сохранён в файл:\n%1\n\nЗаписей: %2")
                                 .arg(filePath)
                                 .arg(reportData.size()));
}

// ===================================================================
// ВИЗУАЛИЗАЦИЯ ВТОРОГО ДЕРЕВА (по датам):

void MainWindow::updateTreeVisualization() {
    qDebug().noquote() << "[updateTreeVisualization] Автоматическое обновление";

    // Просто обновляем текущее дерево без лишних проверок
    updateCurrentTree();
}

void MainWindow::showDateTreeForDebugging() {
    qDebug().noquote() << "[showDateTreeForDebugging] Автоматическое отображение дерева дат";

    currentTreeType = CurrentTreeType::DateTree;

    // Переключаемся на вкладку дерева автоматически
    tabWidget->setCurrentIndex(5);

    // Обновляем дерево
    updateCurrentTree();

    // Обновляем состояние кнопок
    showPolicyTreeAction->setEnabled(true);
    showDateTreeAction->setEnabled(false);
}

void MainWindow::drawTreeByPolicy(AVLNode<std::string, Appointment, Array<Appointment, 1000>>* root) {
    if (!root) return;

    int treeHeight = calculateTreeHeight(root);
    int treeWidth = calculateTreeWidth(root);

    int sceneWidth = std::max(800, treeWidth * 120);
    int sceneHeight = std::max(600, treeHeight * 100);
    treeScene->setSceneRect(-sceneWidth/2, -50, sceneWidth, sceneHeight);

    drawNode(root, 0, 80, sceneWidth * 0.3, 1);
}

Date MainWindow::stringToDate(const std::string& dateStr) {
    if (dateStr.length() != 8) {
        return {1, Month::янв, 2000}; // Значение по умолчанию
    }

    int year = std::stoi(dateStr.substr(0, 4));
    int month = std::stoi(dateStr.substr(4, 2));
    int day = std::stoi(dateStr.substr(6, 2));

    return {day, static_cast<Month>(month), year};
}
void MainWindow::drawTreeByDate(AVLNode<std::string, Appointment, Array<Appointment, 1000>>* root) {
    if (!root) return;

    int treeHeight = calculateTreeHeight(root);
    int treeWidth = calculateTreeWidth(root);

    int sceneWidth = std::max(800, treeWidth * 120);
    int sceneHeight = std::max(600, treeHeight * 100);
    treeScene->setSceneRect(-sceneWidth/2, -50, sceneWidth, sceneHeight);

    drawDateNode(root, 0, 80, sceneWidth * 0.3, 1);
}

void MainWindow::drawDateNode(AVLNode<std::string, Appointment, Array<Appointment, 1000>>* node,
                              qreal x, qreal y, qreal horizontalSpacing, int level) {
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

    // Форматируем дату для отображения
    Date date = stringToDate(node->key);
    QString displayDate = QString("%1.%2.%3")
                              .arg(date.day, 2, 10, QChar('0'))
                              .arg(static_cast<int>(date.month), 2, 10, QChar('0'))
                              .arg(date.year);

    // ИСПРАВЛЕНИЕ: Конвертируем std::string в QString
    QString dateKey = QString::fromStdString(node->key);

    // Создаем узел для дерева дат
    DateTreeNodeItem* nodeItem = new DateTreeNodeItem(displayDate, dateKey, indices, &hashTable, x, y);
    treeScene->addItem(nodeItem);

    // ИСПРАВЛЕНИЕ: Добавляем как QGraphicsItem*, а не в вектор treeNodes
    // treeNodes.push_back(nodeItem);  // ← ЗАКОММЕНТИРУЙТЕ ЭТУ СТРОКУ

    // Рисуем связи с дочерними узлами
    qreal childSpacing = horizontalSpacing / 2;
    qreal childY = y + 100;

    if (node->left) {
        qreal leftX = x - horizontalSpacing;
        QGraphicsLineItem* leftLine = treeScene->addLine(x, y + 20, leftX, childY - 20,
                                                         QPen(QColor(100, 100, 100), 2));
        drawDateNode(node->left, leftX, childY, childSpacing, level + 1);
    }

    if (node->right) {
        qreal rightX = x + horizontalSpacing;
        QGraphicsLineItem* rightLine = treeScene->addLine(x, y + 20, rightX, childY - 20,
                                                          QPen(QColor(100, 100, 100), 2));
        drawDateNode(node->right, rightX, childY, childSpacing, level + 1);
    }
}

void MainWindow::showEmptyTreeMessage(const QString& message) {
    QGraphicsTextItem* emptyText = treeScene->addText(message, QFont("Arial", 16));
    emptyText->setPos(-150, -20);
    emptyText->setDefaultTextColor(QColor(128, 128, 128));
}

void MainWindow::generateReport() {
    qDebug().noquote() << "=== ФОРМИРОВАНИЕ ПОЛНОГО ОТЧЕТА ===";

    reportTable->clearContents();
    reportTable->setRowCount(0);

    // Расширенная структура таблицы отчета - ВСЕ ПОЛЯ из обоих справочников
    reportTable->setColumnCount(10);
    reportTable->setHorizontalHeaderLabels({
        "Дата приёма",           // Справочник 2
        "Врач",                  // Справочник 2
        "Диагноз",              // Справочник 2
        "Полис ОМС",            // Связь
        "Фамилия",              // Справочник 1
        "Имя",                  // Справочник 1
        "Отчество",             // Справочник 1
        "Дата рождения",        // Справочник 1
        "Индекс приёма",        // Техническое поле
        "Статус пациента"       // Для отладки
    });

    // Получаем значения фильтров
    QString fioText = fioFilterEdit->text().trimmed();
    QString doctorText = doctorFilterEdit->text().trimmed();
    QDate qdate = dateFilterEdit->date();

    std::string fioFilter = fioText.toStdString();
    std::string doctorFilter = doctorText.toStdString();
    Date filterDate = {qdate.day(), static_cast<Month>(qdate.month()), qdate.year()};

    qDebug().noquote() << QString("Фильтры: ФИО='%1', Врач='%2', Дата=%3.%4.%5")
                              .arg(fioText)
                              .arg(doctorText)
                              .arg(filterDate.day)
                              .arg(static_cast<int>(filterDate.month))
                              .arg(filterDate.year);

    // Строим дерево отчетов по дате
    buildDateTreeForReport();

    // Получаем полные данные с применением фильтров
    std::vector<FullReportRecord> reportData = generateFullReportData(fioFilter, doctorFilter, filterDate);

    qDebug().noquote() << QString("Получено записей для отчета: %1").arg(reportData.size());

    // Заполняем таблицу
    for (const auto& record : reportData) {
        int row = reportTable->rowCount();
        reportTable->insertRow(row);

        // Заполняем все колонки полными данными
        reportTable->setItem(row, 0, new QTableWidgetItem(formatDate(record.appointmentDate)));
        reportTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(record.doctorType)));
        reportTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(record.diagnosis)));
        reportTable->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(record.patientPolicy)));
        reportTable->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(record.patientSurname)));
        reportTable->setItem(row, 5, new QTableWidgetItem(QString::fromStdString(record.patientName)));
        reportTable->setItem(row, 6, new QTableWidgetItem(QString::fromStdString(record.patientMiddlename)));
        reportTable->setItem(row, 7, new QTableWidgetItem(formatDate(record.patientBirthDate)));
        reportTable->setItem(row, 8, new QTableWidgetItem(QString::number(record.appointmentIndex)));
        reportTable->setItem(row, 9, new QTableWidgetItem(record.patientFound ? "ОК" : "НЕ НАЙДЕН"));

        // Подсветка строк где пациент не найден
        if (!record.patientFound) {
            for (int col = 0; col < reportTable->columnCount(); ++col) {
                reportTable->item(row, col)->setBackground(QColor(255, 200, 200)); // Светло-красный
            }
        }
    }

    // Переключаемся на вкладку отчета
    tabWidget->setCurrentIndex(2);

    // Результат
    QString message;
    if (reportData.empty()) {
        message = "По заданным фильтрам записи не найдены.\n\n"
                  "Проверьте:\n"
                  "- Правильность написания ФИО (точное совпадение)\n"
                  "- Правильность типа врача\n"
                  "- Наличие приёмов на выбранную дату";
        QMessageBox::information(this, "Результат поиска", message);
    } else {
        int validRecords = 0;
        int invalidRecords = 0;
        for (const auto& record : reportData) {
            if (record.patientFound) validRecords++;
            else invalidRecords++;
        }

        message = QString("Сформирован отчет:\n"
                          "Всего записей: %1\n"
                          "Корректных: %2\n"
                          "С проблемами: %3")
                      .arg(reportData.size())
                      .arg(validRecords)
                      .arg(invalidRecords);

        QMessageBox::information(this, "Отчет готов", message);

        // Предлагаем сохранить отчет
        QString savePath = QFileDialog::getSaveFileName(this,
                                                        "Сохранить отчёт в файл", "", "Текстовые файлы (*.txt)");

        if (!savePath.isEmpty()) {
            saveFullReportToFile(savePath, reportData);
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
    if (!isValidStringField(appointment.doctorType) || !isValidStringField(appointment.diagnosis)) {
        QMessageBox::warning(this, "Ошибка валидации",
                             "Тип врача и диагноз должны содержать минимум 2 буквы и не содержать цифр или других символов!");
        return false;
    }


    if (!isValidDate(appointment.appointmentDate)) {
        QMessageBox::warning(this, "Ошибка валидации",
                             "Некорректная дата приёма для выбранного месяца!");
        return false;
    }

    // Проверка: нельзя два приёма к одному врачу в один день
    bool duplicate = false;
    avlTree.traverseByKey(policy, [&](std::size_t index) {
        if (index >= AppointmentArray.Size()) return;

        const Appointment& existing = AppointmentArray[index];

        if (existing.doctorType == appointment.doctorType &&
            existing.appointmentDate == appointment.appointmentDate) {
            duplicate = true;
        }
    });

    if (duplicate) {
        QMessageBox::warning(nullptr, "Ошибка валидации",
                             "Пациент уже записан к этому врачу в указанную дату.");
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

        // ИСПРАВЛЕНИЕ: Перестраиваем дерево дат после удаления
        buildDateTreeForReport();
    } else {
        qDebug().noquote() << "→ Приёмы с данным полисом не найдены";
    }
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

std::string MainWindow::dateToString(const Date& date) {
    return QString("%1%2%3")
    .arg(date.year, 4, 10, QChar('0'))
        .arg(static_cast<int>(date.month), 2, 10, QChar('0'))
        .arg(date.day, 2, 10, QChar('0'))
        .toStdString();
}

void MainWindow::buildDateTreeForReport() {
    qDebug().noquote() << "=== ПОСТРОЕНИЕ ДЕРЕВА ПО ДАТАМ ===";

    // Всегда очищаем дерево перед построением
    dateTree.clear();

    if (AppointmentArray.Size() == 0) {
        qDebug().noquote() << "→ Нет приёмов для построения дерева дат";
        return;
    }

    int addedCount = 0;
    std::set<std::string> uniqueDates;  // Для подсчета уникальных дат

    for (std::size_t i = 0; i < AppointmentArray.Size(); ++i) {
        const Appointment& appointment = AppointmentArray[i];
        std::string dateKey = dateToString(appointment.appointmentDate);

        uniqueDates.insert(dateKey);

        if (dateTree.insertIndex(dateKey, i)) {
            addedCount++;
            qDebug().noquote() << QString("→ [%1] %2: %3 у %4")
                                      .arg(i)
                                      .arg(QString::fromStdString(dateKey))
                                      .arg(QString::fromStdString(appointment.diagnosis))
                                      .arg(QString::fromStdString(appointment.doctorType));
        } else {
            qDebug().noquote() << QString("✗ Ошибка добавления индекса %1 для даты %2")
                                      .arg(i)
                                      .arg(QString::fromStdString(dateKey));
        }
    }

    qDebug().noquote() << QString("Результат: %1 приёмов добавлено, %2 уникальных дат")
                              .arg(addedCount)
                              .arg(uniqueDates.size());

    auto root = dateTree.getRoot();
    if (root) {
        qDebug().noquote() << "✓ Корень дерева дат создан";
    } else {
        qDebug().noquote() << "✗ КРИТИЧЕСКАЯ ОШИБКА: Корень дерева дат НЕ создан!";
    }
}

std::vector<MainWindow::FullReportRecord> MainWindow::generateFullReportData(
    const std::string& fioFilter,
    const std::string& doctorFilter,
    const Date& dateFilter) {

    std::vector<FullReportRecord> results;
    std::string dateKey = dateToString(dateFilter);

    qDebug().noquote() << QString("Поиск в дереве отчетов по дате: %1")
                              .arg(QString::fromStdString(dateKey));

    // Поиск по дереву дат
    dateTree.traverseFiltered(
        [&](const Appointment& appointment) -> bool {
            // Фильтр по дате (должен совпадать с ключом)
            if (dateToString(appointment.appointmentDate) != dateKey) {
                return false;
            }

            // Фильтр по врачу
            if (!doctorFilter.empty() && appointment.doctorType != doctorFilter) {
                return false;
            }

            return true; // Пока что пропускаем, ФИО проверим отдельно
        },
        [&](const Appointment& appointment) {
            // Находим индекс приёма в массиве
            std::size_t appointmentIndex = SIZE_MAX;
            for (std::size_t i = 0; i < AppointmentArray.Size(); ++i) {
                if (AppointmentArray[i] == appointment) {
                    appointmentIndex = i;
                    break;
                }
            }

            if (appointmentIndex == SIZE_MAX || appointmentIndex >= appointmentPolicies.size()) {
                qDebug().noquote() << "Не найден индекс для приёма";
                return;
            }

            // Получаем полис для этого приёма
            std::string policy = appointmentPolicies[appointmentIndex];

            // Получаем данные пациента из справочника 1
            const Patient* patient = hashTable.get(policy);

            FullReportRecord record;
            record.appointmentIndex = appointmentIndex;
            record.doctorType = appointment.doctorType;
            record.diagnosis = appointment.diagnosis;
            record.appointmentDate = appointment.appointmentDate;
            record.patientPolicy = policy;

            if (patient) {
                record.patientSurname = patient->surname;
                record.patientName = patient->name;
                record.patientMiddlename = patient->middlename;
                record.patientBirthDate = patient->birthDate;
                record.patientFound = true;

                // Применяем фильтр по ФИО если указан
                if (!fioFilter.empty()) {
                    std::string fullFIO = patient->surname + " " + patient->name + " " + patient->middlename;
                    if (fullFIO != fioFilter) {
                        return; // Не подходит по ФИО
                    }
                }
            } else {
                record.patientSurname = "НЕ";
                record.patientName = "НАЙДЕН";
                record.patientMiddlename = "";
                record.patientBirthDate = {1, Month::янв, 1900};
                record.patientFound = false;

                // Если фильтр по ФИО задан, а пациент не найден - пропускаем
                if (!fioFilter.empty()) {
                    return;
                }
            }

            results.push_back(record);

            qDebug().noquote() << QString("✓ Добавлен: %1 %2 %3 → %4 у %5")
                                      .arg(QString::fromStdString(record.patientSurname))
                                      .arg(QString::fromStdString(record.patientName))
                                      .arg(QString::fromStdString(record.patientMiddlename))
                                      .arg(QString::fromStdString(record.diagnosis))
                                      .arg(QString::fromStdString(record.doctorType));
        },
        AppointmentArray
        );

    return results;
}

void MainWindow::showIntegrityReport() {
    generateIntegrityReport();
}
