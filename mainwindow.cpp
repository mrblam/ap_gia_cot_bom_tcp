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
    qry_cmd.append("SELECT * FROM product");
    query.prepare(qry_cmd);
    if (!query.exec()) {
        qDebug() << "Select product failed:" << query.lastError();
    }
    while(query.next()){
        if(fuel_id == query.value("Mã nhiên liệu").toString()){
            unit_price = query.value("Đơn giá").toUInt();
                      break;
        }
    }
    return unit_price;
}

static void update_product(QString price ,QString fuel_id){
    QSqlQuery query;
    QDateTime currentTime = QDateTime::currentDateTime();
    QString qry_cmd = "UPDATE product SET [Đơn giá] = '";
    qry_cmd.append(price);
    qry_cmd.append("' ,[Ngày cập nhật] = '");
    qry_cmd.append(currentTime.toString("dd/MM/yyyy hh:mm:ss"));
    qry_cmd.append("' WHERE [Mã nhiên liệu] = ");
    qry_cmd.append(fuel_id);
    query.prepare(qry_cmd);
    qDebug()<< qry_cmd;
    if (!query.exec()) {
        qDebug() << "Insert product failed:" << query.lastError();
    }
}
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    QSqlQuery query;
    ui->setupUi(this);
    this->setWindowTitle("Phần mềm áp giá vòi bơm");
    db.setDatabaseName("mydatabase.db");
    bool dbExists = QFile::exists("mydatabase.db");
    if(!dbExists){
        if (!db.open()) {
            qDebug() << "Cannot create new database. Error:" << db.lastError().text();
        } else {
            qDebug() << QObject::tr("Database is open!");
            query.exec("create table mapping ([Mã vòi] TEXT,[Mã nhiên liệu] TEXT)");
            query.exec("create table product ([Mã nhiên liệu] TEXT,[Tên nhiên liệu] TEXT,[Đơn giá] INT,[Ngày cập nhật] DATETIME)");
            query.exec("create table log_state (Time DATETIME,State TEXT)");
            query.exec("create table tcp ([Host address] TEXT,Port TEXT)");
            query.exec("insert into tcp values('192.168.3.206','65000')");
            query.exec("insert into product values(1,'E5',20000,0)");
            query.exec("insert into product values(2,'A95',20000,0)");
            query.exec("insert into product values(3,'DS',20000,0)");
            query.exec("insert into product values(4,'D001',20000,0)");
        }
    }else{
        if (!db.open()) {
            qDebug() << "Cannot open exist database. Error:" << db.lastError().text();
        }else{
            qDebug() << QObject::tr("Database is open!");
        }
    }
    /**/
    query.prepare("SELECT * FROM tcp");
    if (!query.exec()) {
        qDebug() << "Select tcp failed:" << query.lastError();
    }else{
        if(query.next()){
            ui->host_address_input->setText(query.value("Host address").toString());
            ui->host_address_input->setDisabled(true);
            ui->port_input->setText(query.value("Port").toString());
            ui->port_input->setEnabled(false);
        }
    }
    /**/
    ui->nameFuel_input->setEnabled(false);
    ui->fuelID_input->setEnabled(false);
    ui->price_input->setEnabled(false);
    ui->updateProductDone_btn->setEnabled(false);
    ui->updateMappingDone_btn->setEnabled(false);
    ui->updateIPDone_btn->setEnabled(false);
    ui->addFuel_btn->setEnabled(false);
    MainWindow::showCurrentPrice();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showCurrentPrice()
{
    static QSqlTableModel *assignNozzleModel = new QSqlTableModel;
    assignNozzleModel->setTable("product");
    assignNozzleModel->setEditStrategy(QSqlTableModel::OnFieldChange);
    assignNozzleModel->select();
    ui->product_table->setModel(assignNozzleModel);
    ui->product_table->resizeColumnsToContents();
    ui->product_table->show();
}

void MainWindow::showMapping()
{
    static QSqlTableModel *mappingTable = new QSqlTableModel;
    mappingTable->setTable("mapping");
    mappingTable->setEditStrategy(QSqlTableModel::OnFieldChange);
    mappingTable->select();
    ui->product_table->setModel(mappingTable);
    ui->product_table->resizeColumnsToContents();
    ui->product_table->show();
}

bool MainWindow::on_release_btn_clicked()
{
    uint32_t unit_price = 0;
    QString nozzle_id;
    QString request_cmd;
    QSqlQuery query ;
    QString qry_cmd = "SELECT * FROM mapping";
    /**/
    if(!MainWindow::on_set_btn_clicked()){
        QMessageBox::critical(this,"Error","Đơn giá không hợp lệ!");
        return false ;
    }
    /**/
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
    return true;
}

bool MainWindow::on_set_btn_clicked()
{
    QSqlQuery query;
    QDateTime currentTime;
    QString fuel_id;
    QString unit_price;
    bool isValid = true;
    currentTime = QDateTime::currentDateTime();
    /***********************************************************/
    query.prepare("SELECT * FROM product");
    if (!query.exec()) {
        qDebug() << "Select product failed:" << query.lastError();
    }else{
        while(query.next()){
            fuel_id = query.value("Mã nhiên liệu").toString();
            unit_price = query.value("Đơn giá").toString();
            unit_price.toInt(&isValid, 10);
            if(unit_price == '0' || isValid == false){
                return false;
            }
            update_product(unit_price,fuel_id);
        }
        query.prepare("insert into log_State values (:time,'Set giá')");//
            query.bindValue(":time", currentTime.toString("dd/MM/yyyy hh:mm:ss"));
        if (!query.exec()) {
            qDebug() << "Insert log_State failed:" << query.lastError();
        }
    }
    MainWindow::showCurrentPrice();
    return true;
}

void MainWindow::on_updateIP_btn_clicked()
{
    ui->host_address_input->setEnabled(true);
    ui->port_input->setEnabled(true);
    ui->updateIPDone_btn->setEnabled(true);
}

void MainWindow::on_updateIPDone_btn_clicked()
{ //UPDATE tcp SET [Host address] = '192.168.1.1' ,[Port] = '24384'
    QSqlQuery query;
    QString cmd = "UPDATE tcp SET [Host address] = '";
    cmd.append(ui->host_address_input->text());
    cmd.append("',[Port] = '");
    cmd.append(ui->port_input->text());
    cmd.append("'");
    query.prepare(cmd);
    qDebug()<< cmd;
    if (!query.exec()) {
        qDebug() << "UPDATE tcp failed:" << query.lastError();
    }
    ui->host_address_input->setEnabled(false);
    ui->port_input->setEnabled(false);
    ui->updateIPDone_btn->setEnabled(false);
}

void MainWindow::on_updateProduct_btn_clicked()
{
    ui->fuelID_input->setEnabled(true);
    ui->nameFuel_input->setEnabled(true);
    ui->price_input->setEnabled(true);
    ui->addFuel_btn->setEnabled(true);
    ui->updateProductDone_btn->setEnabled(true);
}

void MainWindow::on_updateProductDone_btn_clicked()
{
    MainWindow::showCurrentPrice();
    ui->updateProductDone_btn->setEnabled(false);
    ui->addFuel_btn->setEnabled(false);
    ui->nameFuel_input->setEnabled(false);
    ui->fuelID_input->setEnabled(false);
    ui->price_input->setEnabled(false);
}

void MainWindow::on_updateMapping_btn_clicked()
{
    MainWindow::showMapping();
    ui->updateMappingDone_btn->setEnabled(true);
}

void MainWindow::on_updateMappingDone_btn_clicked()
{
    MainWindow::showCurrentPrice();
    ui->updateMappingDone_btn->setEnabled(false);
}

void MainWindow::on_addFuel_btn_clicked()
{//insert into product values(1,'E5',20000,0)
    QSqlQuery query;
    QDateTime currentTime = QDateTime::currentDateTime();
    if(ui->fuelID_input->text() != nullptr && ui->nameFuel_input->text() != nullptr && ui->price_input->text() != nullptr){
        query.prepare("insert into product values (:fuelID,:name,:price,:time)");//
        query.bindValue(":fuelID", ui->fuelID_input->text() );
        query.bindValue(":name", ui->nameFuel_input->text());
        query.bindValue(":price", ui->price_input->text());
        query.bindValue(":time", currentTime.toString("dd/MM/yyyy hh:mm:ss"));
        if (!query.exec()) {
            qDebug() << "Insert product failed:" << query.lastError();
        }
        MainWindow::showCurrentPrice();
    }
}

