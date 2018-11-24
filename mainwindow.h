#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <vector>
#include <string>
#include "duplicate_search.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void select_directory();
    void scan_directory(QString const& dir);
    void duplicate_find();
    void show_about_dialog();
    void display_table(duplicates const & dups);

private:
    std::unique_ptr<Ui::MainWindow> ui;
    QString cur_dir;
};

#endif // MAINWINDOW_H
