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
    void showMapping();
private slots:
    void on_release_btn_clicked();

    void on_set_btn_clicked();

    void on_updateIP_btn_clicked();

    void on_updateIPDone_btn_clicked();

    void on_updateProduct_btn_clicked();

    void on_updateProductDone_btn_clicked();

    void on_updateMapping_btn_clicked();

    void on_updateMappingDone_btn_clicked();

    void on_addFuel_btn_clicked();

private:
    Ui::MainWindow *ui;
    Client myClient;
};
#endif // MAINWINDOW_H
