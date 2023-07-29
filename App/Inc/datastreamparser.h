/**
 * @file datastreamparser.h
 * @author nmpassthf (nmpassthf@gmail.com)
 * @brief
 * @date 2023-07-20
 *
 * @copyright Copyright (c) nmpassthf 2023
 *
 */
#ifndef __M_DATASTREAMPARSER_H__
#define __M_DATASTREAMPARSER_H__

#include <QByteArray>
#include <QPointF>
#include <QQueue>
#include <QVariant>

class DataStreamParser {
   public:
    using SourceType = enum class SourceType { StringStream, CSV_File };
    using RDataType = enum class RDataType {
        RDataPointF,
        RDataControlWord,
        RDataErrorString
    };

    constexpr static auto maxWordSize = 128;

    DataStreamParser(SourceType type);
    ~DataStreamParser() = default;

   public:
    /**
     * @brief append new data to buffer to ready for parse
     *
     * @param data
     */
    inline void appendData(const QByteArray& data) { buffer.append(data); }

    /**
     * @brief parse data from buffer
     *
     * @param type
     * @return std::optional<QPair<RDataType, QVariant>>
     * return std::nullopt if buffer is not enough to parse
     */
    std::optional<QPair<RDataType, QVariant>> parseData();

   protected:
    QVector<qreal> x;
    QVector<qreal> step;
    qsizetype currentSelectIndex = 0;

   private:
    std::optional<QPair<RDataType, QVariant>> parseAsStringStream();
    std::optional<QPair<RDataType, QVariant>> parseAsCSVFile();

    QByteArray buffer = {};
    SourceType type;
    bool isStarted = false;

   private:
    static inline bool isGapChar(char c) {
        return c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\0';
    };
    static inline bool isNumberChar(char c) { return (c >= '0' && c <= '9'); };
    static inline bool isNumberPrefixChar(char c) {
        return c == '-' || c == '+';
    };
    static inline bool isNumberInterfixChar(char c) {
        return c == '.' || c == 'e' || c == 'E';
    };
};

#endif /* __M_DATASTREAMPARSER_H__ */
