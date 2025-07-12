#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "hashtable.hpp"
#include"array.h"
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
#include "types.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:

    void loadPatientsFromFile(const QString& filename);
    void loadAppointmentsFromFile(const QString& filename);
    Month monthFromShortString(const QString& shortMonth);

    HashTable hashTable;
    AVLTree<std::string, Appointment, Array<Appointment, 1000>> avlTree;
    QTableWidget* hashTableView;
    QGraphicsView* treeGraphicsView;
    QGraphicsScene* treeScene;
    Ui::MainWindow *ui;
    QTextEdit *treeVisualization;
    QTextEdit *hashTableVisualization;
    QTableWidget *appointmentTable;
    QToolBar *toolBar;
    QTabWidget *tabWidget;
    QTableWidget *patientTable;
    QTextEdit *treeView;
    QTableWidget *reportTable;

    // Actions
    QAction *loadPatientsAction;
    QAction *loadAppointmentsAction;
    QAction *addPatientAction;
    QAction *addAppointmentAction;
    QAction *deletePatientAction;
    QAction *deleteAppointmentAction;
    QAction *debugAction;
    QAction *reportAction;
    //мб обратно их в cpp файл
    Array<Patient, 1000> patientArray;
    Array<Appointment, 1000> appointmentArray;

    void setupUI();
    void createToolBar();
    void createTabs();
    void updateAppointmentTable(const std::vector<std::string>& policies);
    void updatePatientTable();  // если ещё не было

};

#endif // MAINWINDOW_H
