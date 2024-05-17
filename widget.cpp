#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);//默认显示welcome页
    connect(ui->welcomeButton,&QPushButton::clicked,this,[=](){
        ui->stackedWidget->setCurrentIndex(0);
    });
    connect(ui->rulesButton,&QPushButton::clicked,this,[=](){
        ui->stackedWidget->setCurrentIndex(1);
    });
    connect(ui->twoplayerButton,&QPushButton::clicked,this,[=](){
        ui->stackedWidget->setCurrentIndex(2);
    });
    connect(ui->tournamentButton,&QPushButton::clicked,this,[=](){
        ui->stackedWidget->setCurrentIndex(3);
    });
    connect(ui->allplayerButton,&QPushButton::clicked,this,[=](){
        ui->stackedWidget->setCurrentIndex(4);
    });
}


Widget::~Widget()
{
    delete ui;
}
