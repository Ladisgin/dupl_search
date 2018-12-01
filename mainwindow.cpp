#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCommonStyle>
#include <QDesktopWidget>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QBrush>
#include <QThread>
#include <QMimeDatabase>
#include <QUrl>
#include <QFileSystemModel>
#include <QFileInfo>
#include <QFileIconProvider>
#include <QtDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), thread(nullptr) {
    ui->setupUi(this);
    qRegisterMetaType<duplicates>("duplicates");

    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(1);
    ui->progressBar->setValue(1);
//    setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), qApp->desktop()->availableGeometry()));

    ui->toolBar->setStyleSheet(" QToolBar {background-color: rgb(31, 33, 37); border: 1px solid black}");

    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->treeWidget->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->treeWidget->setStyleSheet("background-color: rgb(31, 33, 37); alternate-background-color: rgb(42, 43, 47);");
    ui->treeWidget->setAlternatingRowColors(true);

    QCommonStyle style;
    ui->actionScan_Directory->setIcon(style.standardIcon(QCommonStyle::SP_DialogOpenButton));
    ui->actionExit->setIcon(style.standardIcon(QCommonStyle::SP_DialogCloseButton));
    ui->actionAbout->setIcon(style.standardIcon(QCommonStyle::SP_DialogHelpButton));
    ui->action_duplicate_find->setIcon(style.standardIcon(QCommonStyle::SP_DialogYesButton));
    ui->action_search_cancel->setIcon(style.standardIcon(QCommonStyle::SP_DialogNoButton));

    connect(ui->actionScan_Directory, &QAction::triggered, this, &MainWindow::select_directory);
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::show_about_dialog);
    connect(ui->action_duplicate_find, &QAction::triggered, this, &MainWindow::duplicate_find);
    connect(ui->action_search_cancel, &QAction::triggered, this, &MainWindow::search_cancel);
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
    setWindowTitle(QString("current directory - %1").arg(dir));
//    auto white = QColor();
//    white.setRgb(255, 255, 255);
//    QDir d(dir);
//    QFileInfoList list = d.entryInfoList();
//    for (QFileInfo file_info : list) {
//        QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);
//        item->setText(0, file_info.fileName());
//        item->setText(2, QString::number(file_info.size()));
//        item->setTextColor(0, white);
//        ui->treeWidget->addTopLevelItem(item);
//    }
}

void MainWindow::duplicate_find(){
    search_cancel();
    ui->treeWidget->clear();
    time.start();
    thread = new QThread();
    duplicate_search* worker = new duplicate_search(cur_dir.toStdString());
    worker->moveToThread(thread);

    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(1);
    ui->progressBar->setValue(0);
    connect(worker, SIGNAL(set_max_progress(int)), ui->progressBar, SLOT(setMaximum(int)));
    connect(worker, SIGNAL(set_progress(int)), ui->progressBar, SLOT(setValue(int)));

    connect(worker, SIGNAL (display_duplicates(duplicates)), this, SLOT(display_table(duplicates)));
    connect(worker, SIGNAL (finished()), this, SLOT (search_end()));

    connect(thread, SIGNAL (started()), worker, SLOT (get_dublicate()));
    connect(worker, SIGNAL (finished()), thread, SLOT (quit()));
    connect(worker, SIGNAL (finished()), worker, SLOT (deleteLater()));
    connect(thread, SIGNAL (finished()), thread, SLOT (deleteLater()));

    thread->start();
}

QString fileSize(uint64_t nSize) {
    static const QString size_names[] = {"B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB", "ZiB", "YiB"};
    size_t i = 0;
    double dsize = nSize;
    for (; dsize > 1023; dsize /= 1024, ++i) { }
    return QString::number(dsize, 'g', 4) + " " + size_names[i];
}

void MainWindow::display_table(duplicates dups) {
    setWindowTitle(QString("Dublicate from - %1").arg(cur_dir));
    auto white = QColor();
    auto grey = QColor();
    white.setRgb(255, 255, 255);
    grey.setRgb(184, 187, 198);
    for (auto v : dups.duplicates) {
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);

        QFileInfo fin = QFileInfo(QString::fromStdString(v.paths.front()));

        item->setText(0, fin.fileName());
        item->setIcon(0, QFileIconProvider().icon(fin));
        item->setText(1, QString::number(v.paths.size()));
        item->setText(2, fileSize(v.size));
        item->setTextColor(0, white);

        int gb_value = static_cast<int>(255 - std::min(255ul, v.paths.size()*25));
        item->setTextColor(1, QColor::fromRgb(255, gb_value, gb_value));
        gb_value = static_cast<int>(255 - std::min(static_cast<uint64_t>(255), v.size/256));
        item->setTextColor(2, QColor::fromRgb(255, gb_value, gb_value));

        for (auto file : v.paths) {
            QTreeWidgetItem* child = new QTreeWidgetItem();
            child->setText(0, QString::fromStdString(file));
            child->setTextColor(0, grey);
            item->addChild(child);
        }
        ui->treeWidget->addTopLevelItem(item);
    }
    if(dups.pd_paths.size()){
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);
        item->setText(0, "permission denied files");
        item->setText(1, QString::number(dups.pd_paths.size()));
        item->setTextColor(0, white);
        for (auto file : dups.pd_paths) {
            QTreeWidgetItem* child = new QTreeWidgetItem();
            child->setText(0, QString::fromStdString(file));
            child->setTextColor(0, grey);
            item->addChild(child);
        }
        ui->treeWidget->addTopLevelItem(item);
    }
}

void MainWindow::search_end() {
    ui->progressBar->setValue(ui->progressBar->maximum());
    thread = nullptr;
    qDebug() << QTime::fromMSecsSinceStartOfDay(time.restart()).toString("HH:mm:ss:zzz");
//    QMessageBox::information(this, QString::fromUtf8("Notice"), "time: " + QTime::fromMSecsSinceStartOfDay(time.restart()).toString("HH:mm:ss:zzz"));
}

void MainWindow::show_about_dialog() {
    QMessageBox::aboutQt(this);
}

void MainWindow::search_cancel() {
    if(thread != nullptr && thread->isRunning()){
//        thread->requestInterruption();
        thread->quit();
        qDebug() << thread->wait();;
        thread = nullptr;
    }
}
