#ifndef __M_DATASTREAMPARSER_H__
#define __M_DATASTREAMPARSER_H__

#include <QByteArray>
#include <QObject>
#include <QPointF>
#include <QQueue>

class DataStreamParser : public QObject {
    Q_OBJECT;

   public:
    DataStreamParser(QObject* parent = nullptr);
    ~DataStreamParser();

    enum class SourceType { Serial, CSV_File };

   public:
    /**
     * @brief
     *
     * @param data, type
     * @return {data PointsX, data PointsY, controlWord, error}
     */
    std::tuple<QVector<double>, QVector<double>, QQueue<QByteArray>,
               QQueue<QByteArray>>
    parse(const QByteArray& data, SourceType type);

   private:
    QByteArray buffer;

    qreal x = 0;
    qreal step = 0;
};

#endif /* __M_DATASTREAMPARSER_H__ */
