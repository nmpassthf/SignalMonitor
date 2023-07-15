#include "mainchartswidget.h"
#include "ui_mainchartswidget.h"

MainChartsWidget::MainChartsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainChartsWidget)
{
    ui->setupUi(this);
}

MainChartsWidget::~MainChartsWidget()
{
    delete ui;
}
