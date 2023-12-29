#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "tcpsocket.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void showCurrentPrice();
private slots:
    void on_release_btn_clicked();

    void on_set_btn_clicked();

private:
    Ui::MainWindow *ui;
    Client myClient;
};
#endif // MAINWINDOW_H
