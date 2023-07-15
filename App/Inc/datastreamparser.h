#ifndef __M_DATASTREAMPARSER_H__
#define __M_DATASTREAMPARSER_H__

#include <QByteArray>
#include <QObject>

class DataStreamParser : public QObject {
    Q_OBJECT;

   public:
    DataStreamParser(QObject *parent = nullptr);
    ~DataStreamParser();

   signals:
    void error(QString);
    void dataReceived(QByteArray);
    void controlWordReceived(QByteArray);

   public:
    void parse(const QByteArray& data);

   private:
    QByteArray buffer;
};

#endif /* __M_DATASTREAMPARSER_H__ */
