#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QGraphicsLineItem>
#include <QGraphicsSceneMouseEvent>
#include <QToolTip>
#include <QApplication>
#include <QDialog>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <vector>
#include <map>
#include <set>
#include <QDateEdit>
#include <QLineEdit>
#include "hashtable.hpp"
#include "array.h"
#include "avltree3.hpp"
#include <QMainWindow>
#include <QToolBar>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QAction>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include "types.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

// Объявляем глобальные массивы как extern
extern Array<Patient, 1000> PatientArray;
extern Array<Appointment, 1000> AppointmentArray;

Month monthFromShortString(const QString& shortMonth);

// Предварительные объявления
class TreeNodeItem;
template<typename KeyType, typename T, typename ArrayType>
struct AVLNode;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // ИСПРАВЛЕНИЕ: Делаем appointmentPolicies публичным для доступа из DateTreeNodeItem
    std::vector<std::string> appointmentPolicies;

private slots:
    void showSplitSearchDialog();
    void showIntegrityReport();
    void loadPatientsFromFile();
    void loadAppointmentsFromFile();
    void addPatient();
    void addAppointment();
    void deletePatient();
    void deleteAppointment();
    void generateReport();
    void showDebugWindow();
    void updateAllTables();
    void showPolicyTree();          // Показать дерево по ОМС
    void showDateTree();            // Показать дерево по датам
    void updateCurrentTree();
    void showDateTreeForDebugging();
    void debugDateTreeNodes();      // Отладка узлов дерева дат

private:
    enum class CurrentTreeType {
        PolicyTree,    // Дерево по ОМС
        DateTree       // Дерево по датам
    };
    CurrentTreeType currentTreeType = CurrentTreeType::DateTree; // По умолчанию дерево дат

    Ui::MainWindow *ui;

    // Структура для объединения данных из обоих справочников
    struct FullReportRecord {
        // Справочник 1 - Пациент
        std::string patientSurname;
        std::string patientName;
        std::string patientMiddlename;
        std::string patientPolicy;
        Date patientBirthDate;

        // Справочник 2 - Приём
        std::string doctorType;
        std::string diagnosis;
        Date appointmentDate;
        std::size_t appointmentIndex;

        // Для отладки
        bool patientFound;
    };

    // UI компоненты
    QLineEdit* fioFilterEdit;
    QLineEdit* doctorFilterEdit;
    QDateEdit* dateFilterEdit;
    QTableWidget *avlTreeTableView;
    QToolBar *toolBar;
    QTabWidget *tabWidget;
    QTableWidget *patientTable;
    QTableWidget *appointmentTable;
    QTableWidget *reportTable;
    QTableWidget *hashTableView;
    QGraphicsView *treeGraphicsView;
    QGraphicsScene *treeScene;

    // Actions
    QAction *loadPatientsAction;
    QAction *loadAppointmentsAction;
    QAction *addPatientAction;
    QAction *addAppointmentAction;
    QAction *deletePatientAction;
    QAction *deleteAppointmentAction;
    QAction *debugAction;
    QAction* searchSplitAction;
    QAction *reportAction;
    QAction *showPolicyTreeAction;     // Дерево по ОМС
    QAction *showDateTreeAction;       // Дерево по датам
    QAction *updateTreeAction;         // Обновить текущее дерево

    // Структуры данных
    HashTable hashTable;                                                        // Пациенты
    AVLTree<std::string, Appointment, Array<Appointment, 1000>> avlTree;        // ОМС → приёмы (основное)
    AVLTree<std::string, Appointment, Array<Appointment, 1000>> dateTree;       // Дата → приёмы (для отчетов)
    std::vector<TreeNodeItem*> treeNodes;

    // Методы для работы с датами и отчетами
    std::string dateToString(const Date& date);
    Date stringToDate(const std::string& dateStr);
    void buildDateTreeForReport();
    std::vector<FullReportRecord> generateFullReportData(
        const std::string& fioFilter,
        const std::string& doctorFilter,
        const Date& dateFilter
        );
    void saveFullReportToFile(const QString& filePath, const std::vector<FullReportRecord>& reportData);

    // Методы визуализации двух деревьев
    void drawTreeByPolicy(AVLNode<std::string, Appointment, Array<Appointment, 1000>>* root);
    void drawTreeByDate(AVLNode<std::string, Appointment, Array<Appointment, 1000>>* root);
    void drawDateNode(AVLNode<std::string, Appointment, Array<Appointment, 1000>>* node,
                      qreal x, qreal y, qreal horizontalSpacing, int level);
    void showEmptyTreeMessage(const QString& message);

    // Методы инициализации и настройки UI
    void setupUI();
    void setupTreeVisualization();
    void createToolBar();
    void createTabs();

    // Методы обновления таблиц
    void updatePatientTable();
    void updateAppointmentTable();
    void updateHashTableView();
    void updateTreeView();
    void updateAVLTreeTableView();

    // Методы визуализации дерева
    void updateTreeVisualization();
    void drawTree();
    void clearTreeVisualization();
    int calculateTreeHeight(AVLNode<std::string, Appointment, Array<Appointment, 1000>>* node);
    int calculateTreeWidth(AVLNode<std::string, Appointment, Array<Appointment, 1000>>* node);
    void drawNode(AVLNode<std::string, Appointment, Array<Appointment, 1000>>* node,
                  qreal x, qreal y, qreal horizontalSpacing, int level);

    // Вспомогательные методы
    QString formatDate(const Date& date);
    void checkReferentialIntegrity();
    void generateIntegrityReport();

    // Методы валидации и проверки
    bool patientExists(const std::string& policy) const;
    void deleteAllAppointmentsForPatient(const std::string& policy);
    bool validateAppointmentData(const std::string& policy, const Appointment& appointment);
    bool parsePatientLine(const QString& line, std::string& policy, Patient& patient);
    bool parseAppointmentLine(const QString& line, std::string& policy, Appointment& appointment);
    bool isValidPolicy(const std::string& policy) const;
};

#endif // MAINWINDOW_H
