#include "mainwindow.h"
#include "ui_mainwindow.h"

void MainWindow::fun1(bool check){
    qDebug() << "来是come\n";
}
void MainWindow::fun2(bool check) {
    qDebug() << "去是go\n";
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->come, &QPushButton::clicked, this, &MainWindow::fun1);
    connect(ui->go, &QPushButton::clicked, this, &MainWindow::fun2);
}

MainWindow::~MainWindow()
{
    delete ui;
}
