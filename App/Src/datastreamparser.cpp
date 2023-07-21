/**
 * @file datastreamparser.cpp
 * @author nmpassthf (nmpassthf@gmail.com)
 * @brief
 * @date 2023-07-20
 *
 * @copyright Copyright (c) nmpassthf 2023
 *
 */
#include "dataStreamParser.h"

#include <optional>

DataStreamParser::DataStreamParser(QObject* parent) : QObject(parent) {
    buffer = {};
}
DataStreamParser::~DataStreamParser() {}

auto DataStreamParser::parse(const QByteArray& data, SourceType type)
    -> std::tuple<QVector<double>, QVector<double>, QQueue<QByteArray>,
                  QQueue<QByteArray>> {
    QVector<double> dataX{}, dataY{};
    QQueue<QByteArray> controlWordQueue{};
    QQueue<QByteArray> errorQueue{};

    buffer.append(data);

    if (type == SourceType::Serial) {
        // IFA split by ' '
        enum class State { Num, Cmd, Gap, Begin } state = State::Begin;

        auto isGapChar = [](auto c) {
            return c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\0';
        };
        auto isNumberChar = [](auto c) {
            return (c >= '0' && c <= '9') || c == '.' || c == '-' || c == '+' ||
                   c == 'e' || c == 'E';
        };
        auto isSetStepCmd = [](auto cmd) { return cmd.startsWith("%t"); };

        QByteArray numOrCmdWord{};
        for (auto& c : buffer) {
            switch (state) {
                case State::Num:
                    if (isGapChar(c)) {
                        dataX.append(x);
                        dataY.append(numOrCmdWord.toDouble());
                        x += step;
                        numOrCmdWord.clear();
                        state = State::Gap;
                    } else if (isNumberChar(c)) {
                        numOrCmdWord.append(c);
                    } else {
                        errorQueue.enqueue("Invalid char in number. \nraw:" +
                                           buffer);
                    }
                    break;
                case State::Cmd:
                    if (isGapChar(c)) {
                        if (isSetStepCmd(numOrCmdWord)) {
                            step = numOrCmdWord.mid(2).toDouble();
                        }

                        controlWordQueue.enqueue(numOrCmdWord);
                        numOrCmdWord.clear();

                        state = State::Gap;
                    } else {
                        numOrCmdWord.append(c);
                    }
                    break;
                case State::Gap:
                    if (isGapChar(c)) {
                        // do nothing
                    } else if (c == '%') {
                        state = State::Cmd;
                        numOrCmdWord.append(c);
                    } else {
                        state = State::Num;
                        numOrCmdWord.append(c);
                    }
                    break;
                case State::Begin:
                    if (isGapChar(c)) {
                        state = State::Gap;
                    } else if (c == '%') {
                        state = State::Cmd;
                        numOrCmdWord.append(c);
                    } else {
                        state = State::Num;
                        numOrCmdWord.append(c);
                    }
                    break;
            }
        }

        buffer.clear();
        buffer = numOrCmdWord;

        return {dataX, dataY, controlWordQueue, errorQueue};
    }

    if (type == SourceType::CSV_File) {
        // TODO Support CSV file
        return {};
    }

    return {};
}
