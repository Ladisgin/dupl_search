#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <vector>
#include <string>

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
    void display_table(const std::vector<std::pair<std::vector<std::string>, uint64_t>> &duplicate_list, const std::vector<std::string> &p_denied);

private:
    std::unique_ptr<Ui::MainWindow> ui;
    QString cur_dir;
};

#endif // MAINWINDOW_H
