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

#include <QCloseEvent>
#include <QMap>
#include <QThread>
#include <QThreadPool>
#include <QVector>
#include <QWidget>

#include "chartwidget.h"
#include "datasource.h"
#include "pch.h"

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
    QThreadPool *threadPool;
    QThread* serialThread;

    enum class NewDataStrategy {
        ReusePlot,
        InsertAtMainWindow,
        PopUpNewWindow,
    } newDataStrategy = NewDataStrategy::ReusePlot;
    QVector<ChartWidget *> plots;
    ChartWidget *currentSelectedPlot = nullptr;
    QMap<DataSource *,MySeries* > dataSources;

   signals:
    void serialCloseRequest();

   public slots:
    void onSourceError(QString);
    void onSourceControlWordReceived(QByteArray);

   private:
    void bindPushButtons();

    void bindDataSourceToPlot(DataSource *source, ChartWidget *plot);

    ChartWidget* createNewPlot(QString title);
};

#endif /* __M_MAINWINDOW_H__ */
