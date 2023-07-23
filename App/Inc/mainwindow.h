/**
 * @file mainwindow.h
 * @author nmpassthf (nmpassthf@gmail.com)
 * @brief
 * @date 2023-07-15
 *
 * @copyright Copyright (c) nmpassthf 2023
 *
 */
#ifndef __M_MAINWINDOW_H__
#define __M_MAINWINDOW_H__

#include <QCloseEvent>
#include <QMap>
#include <QVector>
#include <QWidget>
#include <memory>

#include "chartwidget.h"
#include "datasource.h"
#include "pch.h"
#include "serial.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QWidget {
    Q_OBJECT

    inline DataSource *getDataSourceSender() {
        return qobject_cast<DataSource *>(sender());
    }

   public:
    using NewDataStrategy = enum {
        ReusePlot,
        InsertAtMainWindow,
        PopUpNewWindow,
    };

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

   signals:
    void windowExited();

   public slots:
    void onSourceError(QString);
    void onSourceControlWordReceived(DataSource::DataControlWords controlWord,
                                     QByteArray DCWData);

   protected:
    virtual void closeEvent(QCloseEvent *event) override;

   private:
    void bindPushButtons();

    /**
     * @brief Create a New Plot object
     *
     * @param source
     * @return QCPGraph* Series Object
     */
    QCPGraph *createNewPlot(DataSource *source, QString title, QPen color,
                            NewDataStrategy strategy,
                            ChartWidget::PlotPos_t pos = {-1, -1});
    void bindDataSource(DataSource *source, QCPGraph *series);

    void createSerialDataSource(SerialSettings settings,
                                NewDataStrategy strategy);

   private:
    Ui::MainWindow *ui;

    ChartWidget *currentSelectedPlot = nullptr;

    QMap<DataSource::DSID, QPair<DataSource *, QThread *>> sourceToThreadMap;
    QVector<ChartWidget *> popUpPlots;
};

#endif /* __M_MAINWINDOW_H__ */
