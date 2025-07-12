#ifndef HASHTABLEDIALOG_H
#define HASHTABLEDIALOG_H

#include <QDialog>

namespace Ui {
class hashtableDialog;
}

class hashtableDialog : public QDialog
{
    Q_OBJECT

public:
    explicit hashtableDialog(QWidget *parent = nullptr);
    ~hashtableDialog();

private:
    Ui::hashtableDialog *ui;
};

#endif // HASHTABLEDIALOG_H
