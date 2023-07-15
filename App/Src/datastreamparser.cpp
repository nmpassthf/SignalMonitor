#include "dataStreamParser.h"

DataStreamParser::DataStreamParser(QObject *parent) : QObject(parent) {
    buffer = {};
}
DataStreamParser::~DataStreamParser() {}

void DataStreamParser::parse(const QByteArray& data) {
    buffer.append(data);

    // IFA split by ' '
    enum class State { Num, Cmd, Gap, Begin } state = State::Begin;
    auto isGapChar = [](auto c) { return c == ' ' || c == '\n' || c == '\t'; };
    auto isNumberChar = [](auto c) {
        return (c >= '0' && c <= '9') || c == '.';
    };
    QByteArray numOrCmdWord{};
    for (auto c : buffer) {
        switch (state) {
            case State::Num:
                if (isGapChar(c)) {
                    emit dataReceived(numOrCmdWord);
                    numOrCmdWord.clear();
                    state = State::Gap;
                } else if (isNumberChar(c)) {
                    numOrCmdWord.append(c);
                } else {
                    emit error("Invalid char in number. \nraw:" + buffer);
                }
                break;
            case State::Cmd:
                if (isGapChar(c)) {
                    emit controlWordReceived(numOrCmdWord);
                    numOrCmdWord.clear();
                    state = State::Gap;
                } else if (isNumberChar(c)) {
                    emit error("Invalid char in command word. Terminated\nraw:" + buffer);
					buffer.clear();
					return;
                } else {
                    numOrCmdWord.append(c);
                }
                break;
            case State::Gap:
                if (isGapChar(c)) {
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
}
