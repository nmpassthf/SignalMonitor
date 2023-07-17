#include "dataStreamParser.h"

DataStreamParser::DataStreamParser(QObject* parent) : QObject(parent) {
    buffer = {};
}
DataStreamParser::~DataStreamParser() {}

std::tuple<QVector<qreal>, QQueue<QByteArray>, QQueue<QByteArray>>
DataStreamParser::parse(const QByteArray& data) {
    QVector<qreal> dataQueue{};
    QQueue<QByteArray> controlWordQueue{};
    QQueue<QByteArray> errorQueue{};

    buffer.append(data);

    // IFA split by ' '
    enum class State { Num, Cmd, Gap, Begin } state = State::Begin;
    auto isGapChar = [](auto c) {
        return c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\0';
    };
    auto isNumberChar = [](auto c) {
        return (c >= '0' && c <= '9') || c == '.' || c == '-' || c == '+' ||
               c == 'e' || c == 'E';
    };

    QByteArray numOrCmdWord{};
    for ( auto& c : buffer) {
        switch (state) {
            case State::Num:
                if (isGapChar(c)) {
                    dataQueue.append(numOrCmdWord.toDouble());
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

    return {dataQueue, controlWordQueue, errorQueue};
}
