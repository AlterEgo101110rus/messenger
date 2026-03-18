#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    chat = new ChatWidget(this);

    setCentralWidget(chat);
}

MainWindow::~MainWindow()
{
    delete ui;
}
