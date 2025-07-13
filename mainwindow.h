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
#include <QDateEdit>
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

private:
    Ui::MainWindow *ui;

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

    // Структуры данных
    HashTable hashTable;
    AVLTree<std::string, Appointment, Array<Appointment, 1000>> avlTree;
    std::vector<std::string> appointmentPolicies; // Для связи приёмов с полисами
    std::vector<TreeNodeItem*> treeNodes; // Убрана дублированная объявление

    // Вспомогательные методы
    void checkReferentialIntegrity();
    void generateIntegrityReport();
    void setupUI();
    void setupTreeVisualization(); // Добавлен метод
    void createToolBar();
    void createTabs();
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

    QString formatDate(const Date& date);

    // Методы валидации и проверки
    bool patientExists(const std::string& policy) const;
    void deleteAllAppointmentsForPatient(const std::string& policy);
    bool validateAppointmentData(const std::string& policy, const Appointment& appointment);
    bool parsePatientLine(const QString& line, std::string& policy, Patient& patient);
    bool parseAppointmentLine(const QString& line, std::string& policy, Appointment& appointment);
    bool isValidPolicy(const std::string& policy) const;
};

#endif // MAINWINDOW_H
