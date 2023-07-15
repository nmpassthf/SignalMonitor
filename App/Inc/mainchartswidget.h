#ifndef MAINCHARTSWIDGET_H
#define MAINCHARTSWIDGET_H
/**
 * @file mainchartswidget.h
 * @author nmpassthf (nmpassthf@gmail.com)
 * @brief 
 * @date 2023-07-15
 * 
 * @copyright Copyright (c) nmpassthf 2023
 * 
 */

#include <QWidget>

namespace Ui {
class MainChartsWidget;
}

class MainChartsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainChartsWidget(QWidget *parent = nullptr);
    ~MainChartsWidget();

private:
    Ui::MainChartsWidget *ui;
};

#endif // MAINCHARTSWIDGET_H
