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
#include <QDesktopServices>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), thread(new QThread()) {
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
//    ui->actionScan_Directory->setIcon(QIcon("icons/icons8-folder-512.png"));
    ui->actionScan_Directory->setIcon(style.standardIcon(QCommonStyle::SP_DialogOpenButton));
    ui->actionExit->setIcon(style.standardIcon(QCommonStyle::SP_DialogCloseButton));
    ui->actionExit->setIcon(style.standardIcon(QCommonStyle::SP_DialogCloseButton));
    ui->actionAbout->setIcon(style.standardIcon(QCommonStyle::SP_DialogHelpButton));
//    ui->action_duplicate_find->setIcon(QIcon("icons/icons8-round-500"));
    ui->action_duplicate_find->setIcon(style.standardIcon(QCommonStyle::SP_DialogYesButton));
//    ui->action_search_cancel->setIcon(QIcon("icons/icons8-cancel-500"));
    ui->action_search_cancel->setIcon(style.standardIcon(QCommonStyle::SP_DialogNoButton));
    connect(ui->treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(open_file(QTreeWidgetItem*, int)));

    connect(ui->actionScan_Directory, &QAction::triggered, this, &MainWindow::select_directory);
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::show_about_dialog);
    connect(ui->action_duplicate_find, &QAction::triggered, this, &MainWindow::duplicate_find);
    connect(ui->action_search_cancel, &QAction::triggered, this, &MainWindow::search_cancel);
}

MainWindow::~MainWindow(){
    search_cancel();
}

void MainWindow::select_directory() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Directory for Scanning",
                                                    QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    scan_directory(dir);
}

void MainWindow::scan_directory(QString const& dir) {
    ui->treeWidget->clear();
    cur_dir = dir;
    setWindowTitle(QString("current directory - %1").arg(dir));
//    auto QColor::fromRgb(255, 255, 255) = QColor();
//    QColor::fromRgb(255, 255, 255).setRgb(255, 255, 255);
//    QDir d(dir);
//    QFileInfoList list = d.entryInfoList();
//    for (QFileInfo file_info : list) {
//        QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);
//        item->setText(0, file_info.fileName());
//        item->setText(2, QString::number(file_info.size()));
//        item->setTextColor(0, QColor::fromRgb(255, 255, 255));
//        ui->treeWidget->addTopLevelItem(item);
//    }
}

void MainWindow::duplicate_find() {
    search_cancel();

    ui->treeWidget->clear();
    time.start();
    duplicate_search* worker = new duplicate_search(cur_dir.toStdString());
    worker->moveToThread(thread.get());

    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(1);
    ui->progressBar->setValue(0);
    connect(worker, SIGNAL(set_max_progress(int)), ui->progressBar, SLOT(setMaximum(int)));
    connect(worker, SIGNAL(set_progress(int)), ui->progressBar, SLOT(setValue(int)));

    connect(worker, SIGNAL (display_duplicates(duplicates)), this, SLOT(display_table(duplicates)));
    connect(worker, SIGNAL (finished()), this, SLOT (search_end()));

    connect(thread.get(), SIGNAL (started()), worker, SLOT (get_dublicate()));
    connect(worker, SIGNAL (finished()), thread.get(), SLOT (quit()));
    connect(worker, SIGNAL (finished()), worker, SLOT (deleteLater()));
//    connect(thread, SIGNAL (finished()), thread, SLOT (deleteLater()));

    thread->start();
    qDebug() << "thread start";
}

QString fileSize(uint64_t nSize) {
    static const QString size_names[] = {"B", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB", "ZiB", "YiB"};
    size_t i = 0;
    double dsize = nSize;
    for (; dsize > 1023; dsize /= 1024, ++i) { }
    return QString::number(dsize, 'g', 4) + " " + size_names[i];
}

void MainWindow::display_bad_files(std::vector<std::string> const &bads, QString error_info) {
    if(bads.size()){
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);
        item->setText(0, error_info);
        item->setText(1, QString::number(bads.size()));
        item->setTextColor(0, QColor::fromRgb(255, 255, 255));
        for (auto file : bads) {
            QTreeWidgetItem* child = new QTreeWidgetItem();
            child->setText(0, QString::fromStdString(file));
            child->setTextColor(0, QColor::fromRgb(184, 187, 198));
            item->addChild(child);
        }
        ui->treeWidget->addTopLevelItem(item);
    }
}

void MainWindow::display_table(duplicates dups) {
    setWindowTitle(QString("Dublicate from - %1").arg(cur_dir));
    for (auto v : dups.duplicates) {
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);

        QFileInfo fin = QFileInfo(QString::fromStdString(v.paths.front()));

        item->setText(0, fin.fileName());
        item->setIcon(0, QFileIconProvider().icon(fin));
        item->setText(1, QString::number(v.paths.size()));
        item->setText(2, fileSize(v.size));
        item->setTextColor(0, QColor::fromRgb(255, 255, 255));

        int gb_value = static_cast<int>(255 - std::min(255ul, v.paths.size()*25));
        item->setTextColor(1, QColor::fromRgb(255, gb_value, gb_value));
        gb_value = static_cast<int>(255 - std::min(static_cast<uint64_t>(255), v.size/256));
        item->setTextColor(2, QColor::fromRgb(255, gb_value, gb_value));

        for (auto file : v.paths) {
            QTreeWidgetItem* child = new QTreeWidgetItem();
            child->setText(0, QString::fromStdString(file));
            child->setTextColor(0, QColor::fromRgb(184, 187, 198));
            item->addChild(child);
        }
        ui->treeWidget->addTopLevelItem(item);
    }
    display_bad_files(dups.pd_paths, "permisson denied");
    display_bad_files(dups.read_error, "open error");
}

void MainWindow::search_end() {
    ui->progressBar->setValue(ui->progressBar->maximum());
//    thread = nullptr;
    qDebug() << "thread finished time: " << QTime::fromMSecsSinceStartOfDay(time.restart()).toString("HH:mm:ss:zzz");
//    QMessageBox::information(this, QString::fromUtf8("Notice"), "time: " + QTime::fromMSecsSinceStartOfDay(time.restart()).toString("HH:mm:ss:zzz"));
}

void MainWindow::show_about_dialog() {
    QMessageBox::aboutQt(this);
}

void MainWindow::search_cancel() {
    if(thread != nullptr && thread->isRunning()){
        thread->requestInterruption();
        thread->quit();
        thread->wait();
        qDebug() << "process canceled";
    }
}

void MainWindow::open_file(QTreeWidgetItem *item, int){
    if(item->childCount() == 0) {
        QDesktopServices::openUrl(QUrl("file:" + item->text(0)));
    }
}
