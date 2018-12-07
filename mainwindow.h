#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTime>
#include <QMetaType>
#include <QTreeWidgetItem>

#include <vector>
#include <string>
#include <memory>
#include "duplicate_search.h"

Q_DECLARE_METATYPE(duplicates)

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void display_bad_files(std::vector<std::string> const &bads, QString error_info);
    void select_directory();
    void scan_directory(QString const& dir);
    void duplicate_find();
    void show_about_dialog();
    void search_end();
    void display_table(duplicates dups);
    void search_cancel();
    void open_file(QTreeWidgetItem *item, int column);
    void delete_duplicate();

private:
    bool delete_file(QTreeWidgetItem *item);

    std::unique_ptr<Ui::MainWindow> ui;
    QString cur_dir;
    QTime time;
    std::unique_ptr<QThread> thread;
};

#endif // MAINWINDOW_H
