#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTimer>
#include <QDateTime>
#include <QDebug>
#include <QString>
#include <QFile>
#include <QSqlTableModel>
static uint32_t get_unit_price(QString nozzle_id)
{
    uint32_t unit_price = 0;
    QSqlQuery query ;
    QString fuel_id;
    QString qry_cmd = "SELECT * FROM mapping";
    query.prepare(qry_cmd);
    if (!query.exec()) {
        qDebug() << "Select mapping failed:" << query.lastError();
    }
    while(query.next()){
        if(nozzle_id == query.value("Mã vòi").toString()){
            fuel_id = query.value("Mã nhiên liệu").toString();
                      break;
        }
    }
    qry_cmd.clear();
    qry_cmd.append("SELECT * FROM current_price");
    query.prepare(qry_cmd);
    if (!query.exec()) {
        qDebug() << "Select current_price failed:" << query.lastError();
    }
    while(query.next()){
        if(fuel_id == query.value("Mã nhiên liệu").toString()){
            unit_price = query.value("Đơn giá").toUInt();
                      break;
        }
    }
    return unit_price;
}

static void update_current_price(QString price ,QString fuel_id){
    QSqlQuery query;
    QDateTime currentTime = QDateTime::currentDateTime();
    QString qry_cmd = "UPDATE current_price SET [Đơn giá] = '";
    qry_cmd.append(price);
    qry_cmd.append("' ,[Ngày cập nhật] = '");
    qry_cmd.append(currentTime.toString("dd/MM/yyyy hh:mm:ss"));
    qry_cmd.append("' WHERE [Mã nhiên liệu] = ");
    qry_cmd.append(fuel_id);
    query.prepare(qry_cmd);
    qDebug()<< qry_cmd;
    if (!query.exec()) {
        qDebug() << "Insert current_price failed:" << query.lastError();
    }
}
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    QSqlQuery query;
    ui->setupUi(this);
    ui->release_btn->setEnabled(false);
    db.setDatabaseName("mydatabase.db");
    bool dbExists = QFile::exists("mydatabase.db");
    if(!dbExists){
        if (!db.open()) {
            qDebug() << "Cannot create new database. Error:" << db.lastError().text();
        } else {
            qDebug() << QObject::tr("Database is open!");
            query.exec("create table mapping ([Mã vòi] TEXT,[Mã nhiên liệu] TEXT)");
            query.exec("create table current_price ([Mã nhiên liệu] TEXT,[Tên nhiên liệu] TEXT,[Đơn giá] INT,[Ngày cập nhật] DATETIME)");
            query.exec("create table log_state (Time DATETIME,State TEXT)");
            query.exec("insert into current_price values(1,'E5',20000,0)");
            query.exec("insert into current_price values(2,92,20000,0)");
            query.exec("insert into current_price values(3,'DS',20000,0)");
            query.exec("insert into current_price values(4,'D001',20000,0)");
        }
    }else{
        if (!db.open()) {
            qDebug() << "Cannot open exist database. Error:" << db.lastError().text();
        }else{
            qDebug() << QObject::tr("Database is open!");
        }
    }
    MainWindow::showCurrentPrice();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showCurrentPrice()
{
    static QSqlTableModel *assignNozzleModel = new QSqlTableModel;
    assignNozzleModel->setTable("current_price");
    assignNozzleModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    assignNozzleModel->select();
    ui->current_price_table->setModel(assignNozzleModel);
    ui->current_price_table->resizeColumnsToContents();
    ui->current_price_table->show();
}

void MainWindow::on_release_btn_clicked()
{
    uint32_t unit_price = 0;
    QString nozzle_id;
    QString request_cmd;
    QSqlQuery query ;
    QString qry_cmd = "SELECT * FROM mapping";
    query.prepare(qry_cmd);
    if (!query.exec()) {
        qDebug() << "Select mapping failed:" << query.lastError();
    }
    request_cmd.clear();
    request_cmd.append("APGIA : {");
    while(query.next()){
        nozzle_id = query.value("Mã vòi").toString();
        unit_price = get_unit_price(nozzle_id);
        request_cmd.append(nozzle_id);
        request_cmd.append("=");
        request_cmd.append(QString::number(unit_price));
        request_cmd.append(",");
    }
    request_cmd.removeLast();
    request_cmd.append("}");
    qDebug()<< request_cmd;
    if(myClient.requestToServer(request_cmd,ui->host_address_input->text(),ui->port_input->text().toInt())){
        QMessageBox::information(this,"Result","Áp giá thành công");
    }else{
        QMessageBox::critical(this,"False","Áp giá thất bại!");
    }
}

void MainWindow::on_set_btn_clicked()
{
    QSqlQuery query;
    QDateTime currentTime;
    ui->release_btn->setEnabled(true);
    currentTime = QDateTime::currentDateTime();
    /***********************************************************/
    if(ui->E5_Price->text() != nullptr){
        update_current_price(ui->E5_Price->text(),QString::number(1));
        query.prepare("insert into log_State values (:time,'Set giá E5')");//
            query.bindValue(":time", currentTime.toString("dd/MM/yyyy hh:mm:ss"));
        if (!query.exec()) {
            qDebug() << "Insert log_State failed:" << query.lastError();
        }
    }
    /***********************************************************/
    if(ui->D001->text() != nullptr){
        update_current_price(ui->D001->text(),QString::number(4));
        query.prepare("insert into log_State values (:time,'Set giá D001')");//
            query.bindValue(":time", currentTime.toString("dd/MM/yyyy hh:mm:ss"));
        if (!query.exec()) {
            qDebug() << "Insert log_State failed:" << query.lastError();
        }
    }
    /***********************************************************/
    if(ui->DS->text() != nullptr){
        update_current_price(ui->DS->text(),QString::number(3));
        query.prepare("insert into log_State values (:time,'Set giá DS')");//
            query.bindValue(":time", currentTime.toString("dd/MM/yyyy hh:mm:ss"));
        if (!query.exec()) {
            qDebug() << "Insert log_State failed:" << query.lastError();
        }
    }
    /***********************************************************/
    if(ui->x92_price->text() != nullptr){
        update_current_price(ui->x92_price->text(),QString::number(2));
        query.prepare("insert into log_State values (:time,'Set giá 92')");//
            query.bindValue(":time", currentTime.toString("dd/MM/yyyy hh:mm:ss"));
        if (!query.exec()) {
            qDebug() << "Insert log_State failed:" << query.lastError();
        }
    }
    /***********************************************************/
    MainWindow::showCurrentPrice();
}

