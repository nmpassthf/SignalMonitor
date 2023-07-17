#ifndef __M_DATASTREAMPARSER_H__
#define __M_DATASTREAMPARSER_H__

#include <QByteArray>
#include <QObject>
#include <QQueue>

class DataStreamParser : public QObject {
    Q_OBJECT;

   public:
    DataStreamParser(QObject* parent = nullptr);
    ~DataStreamParser();

   public:
    /**
     * @brief
     *
     * @param data
     * @return {data, controlWord, error}
     */
    std::tuple<QVector<qreal>, QQueue<QByteArray>, QQueue<QByteArray>> parse(
        const QByteArray& data);

   private:
    QByteArray buffer;
};

#endif /* __M_DATASTREAMPARSER_H__ */
