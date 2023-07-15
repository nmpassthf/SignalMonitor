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
#include <QThread>
#include <QThreadPool>
#include <QWidget>

#include "mainchartswidget.h"
#include "pch.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QWidget {
    Q_OBJECT

   public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

   protected:
    virtual void closeEvent(QCloseEvent *event) override;

   private:
    Ui::MainWindow *ui;
    QThreadPool *threadPool;

   signals:
    void serialCloseRequest();

   public slots:
    void onSerialError(QString);
    void onSerialDataReceived(QByteArray);
    void onSerialControlWordReceived(QByteArray);

   private:
    void bindPushButtons();

    template <typename T>
    inline T *getSignalSender() {
        return qobject_cast<T *>(sender());
    }
};

#endif /* __M_MAINWINDOW_H__ */
