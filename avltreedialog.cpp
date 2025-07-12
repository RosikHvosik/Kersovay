#include "avltreedialog.h"
#include "ui_avltreedialog.h"

AVLtreeDialog::AVLtreeDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AVLtreeDialog)
{
    ui->setupUi(this);
}

AVLtreeDialog::~AVLtreeDialog()
{
    delete ui;
}
