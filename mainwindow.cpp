#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "duplicate_search.h"

#include <QCommonStyle>
#include <QDesktopWidget>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QBrush>

#include <vector>
#include <string>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);
    setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), qApp->desktop()->availableGeometry()));

    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->treeWidget->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->treeWidget->setStyleSheet("background-color: rgb(31, 33, 37); alternate-background-color: rgb(42, 43, 47); branch-background-color: rgb(255, 0, 0);");
    ui->treeWidget->setAlternatingRowColors(true);

    QCommonStyle style;
    ui->actionScan_Directory->setIcon(style.standardIcon(QCommonStyle::SP_DialogOpenButton));
    ui->actionExit->setIcon(style.standardIcon(QCommonStyle::SP_DialogCloseButton));
    ui->actionAbout->setIcon(style.standardIcon(QCommonStyle::SP_DialogHelpButton));
    ui->action_duplicate_find->setIcon(style.standardIcon(QCommonStyle::SP_DialogYesButton));

    connect(ui->actionScan_Directory, &QAction::triggered, this, &MainWindow::select_directory);
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::show_about_dialog);
    connect(ui->action_duplicate_find, &QAction::triggered, this, &MainWindow::duplicate_find);
}

MainWindow::~MainWindow(){}

void MainWindow::select_directory() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Directory for Scanning",
                                                    QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    scan_directory(dir);
}

void MainWindow::scan_directory(QString const& dir) {
    ui->treeWidget->clear();
    cur_dir = dir;
    auto white = QColor();
    white.setRgb(255, 255, 255);
    setWindowTitle(QString("Directory Content - %1").arg(dir));
    QDir d(dir);
    QFileInfoList list = d.entryInfoList();
    for (QFileInfo file_info : list) {
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);
        item->setText(0, file_info.fileName());
        item->setText(2, QString::number(file_info.size()));
        item->setTextColor(0, white);
        ui->treeWidget->addTopLevelItem(item);
    }
}

void MainWindow::duplicate_find(){
    ui->treeWidget->clear();
    auto ds = duplicate_search(cur_dir.toStdString());
    auto pd = std::vector<std::string>();
    auto v = ds.get_dublicate(pd);
    display_table(v, pd);
}

void MainWindow::display_table(const std::vector<std::pair<std::vector<std::string>, uint64_t>> &duplicate_list, const std::vector<std::string> &p_denied) {
    setWindowTitle(QString("Dublicate from - %1").arg(cur_dir));
    auto white = QColor();
    auto grey = QColor();
    auto red = QColor();
    white.setRgb(255, 255, 255);
    red.setRgb(255, 0, 0);
    grey.setRgb(184, 187, 198);
    for (auto v : duplicate_list) {
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);
        item->setText(0, QString::fromStdString(v.first.front()));
        item->setText(1, QString::number(v.first.size()));
        item->setText(2, QString::number(v.second));
        item->setTextColor(0, white);
        item->setTextColor(1, red);
        item->setTextColor(2, red);
        for (auto file : v.first) {
            QTreeWidgetItem* child = new QTreeWidgetItem();
            child->setText(0, QString::fromStdString(file));
            child->setTextColor(0, grey);
            item->addChild(child);
        }
        ui->treeWidget->addTopLevelItem(item);
    }
    QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);
    item->setText(0, "permission denied files");
    item->setText(1, QString::number(p_denied.size()));
    item->setTextColor(0, white);
    for (auto file : p_denied) {
        QTreeWidgetItem* child = new QTreeWidgetItem();
        child->setText(0, QString::fromStdString(file));
        child->setTextColor(0, grey);
        item->addChild(child);
    }
    ui->treeWidget->addTopLevelItem(item);
    QMessageBox::information(this, QString::fromUtf8("Notice"), "Find duplicate end");
}

void MainWindow::show_about_dialog() {
    QMessageBox::aboutQt(this);
}
