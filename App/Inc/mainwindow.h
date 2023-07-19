#ifndef __M_MAINWINDOW_H__
#define __M_MAINWINDOW_H__
/**
 * @file mainwindow.h
 * @author nmpassthf (nmpassthf@gmail.com)
 * @brief
 * @date 2023-07-15
 *
 * @copyright Copyright (c) nmpassthf 2023
 *
 */

#include "pch.h"

// 

#include <QCloseEvent>
#include <QMap>
#include <QVector>
#include <QWidget>
#include <memory>

#include "datasource.h"
#include "mycustomplot.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QWidget {
    Q_OBJECT

    inline DataSource *getDataSourceSender() {
        return qobject_cast<DataSource *>(sender());
    }

   public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

   protected:
    virtual void closeEvent(QCloseEvent *event) override;

   private:
    Ui::MainWindow *ui;
    QThread *serialThread = nullptr;

    enum class NewDataStrategy {
        ReusePlot,
        InsertAtMainWindow,
        PopUpNewWindow,
    } newDataStrategy = NewDataStrategy::InsertAtMainWindow;

    QVector<QCustomPlot *> popUpPlots;
    QCustomPlot *currentSelectedPlot = nullptr;
    QMap<DataSource*, QCPGraph *> dataSources;
    QMap<DataSource*, bool> isDataSourceRunning;

   signals:
    void serialCloseRequest();

   public slots:
    void onSourceError(QString);
    void onSourceControlWordReceived(QByteArray);

   private:
    void bindPushButtons();

    QCPGraph *createNewSeries(QString title,QPen color);
    void bindDataSource(DataSource* source, QCPGraph *plot);
};

#endif /* __M_MAINWINDOW_H__ */
