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

#include <ranges>

DataStreamParser::DataStreamParser(SourceType type) : type{type} {
    x.append(0);
    step.append(0);
}

std::optional<QPair<DataStreamParser::RDataType, QVariant>>
DataStreamParser::parseData() {
    switch (type) {
        case SourceType::StringStream:
            return parseAsStringStream();
            break;
        case SourceType::CSV_File:
            return parseAsCSVFile();
            break;

        default:
            break;
    }

    return std::nullopt;
}

std::optional<QPair<DataStreamParser::RDataType, QVariant>>
DataStreamParser::parseAsStringStream() {
    using namespace std::ranges;
    using namespace std;

    enum State {
        NumPrefix,
        Num,
        NumInterfix,
        Cmd,
        Gap,
        Begin
    } state = State::Begin;

    QByteArray wordBuffer{};
    wordBuffer.reserve(maxWordSize);

    for (const auto& [c, i] : zip_view(buffer, views::iota(0, buffer.size()))) {
        auto makeErrorString = [&](auto errLocateStr) {
            return QString(
                       "Error: Invalid char %1(0x%2) in %3. \nraw "
                       "data:\n%4\n\tat ->%5\n")
                .arg(c)
                .arg((uint8_t)c, 0, 16)
                .arg(errLocateStr)
                .arg(buffer)
                .arg(i);
        };
        auto removeBufferFront = [&]() { buffer.remove(0, i + 1); };

        switch (state) {
            case State::NumPrefix:
                if (isNumberChar(c)) {
                    state = State::Num;
                    wordBuffer.append(c);
                } else {
                    buffer.clear();
                    return qMakePair(
                        RDataType::RDataErrorString,
                        makeErrorString("number prefix is end with non-number "
                                        "char or number interfix char"));
                }
                break;
            case State::Num:
                if (isGapChar(c)) {
                    bool ok = false;
                    qreal rYVal = wordBuffer.toDouble(&ok);
                    if (!ok) {
                        buffer.clear();
                        return qMakePair(
                            RDataType::RDataErrorString,
                            makeErrorString("number is not valid"));
                    }

                    auto rVal = QVariant::fromValue(QPointF{x[currentSelectIndex], rYVal});
                    x[currentSelectIndex] += step[currentSelectIndex];
                    removeBufferFront();
                    return qMakePair(RDataType::RDataPointF, rVal);
                } else if (isNumberChar(c)) {
                    wordBuffer.append(c);
                } else if (isNumberInterfixChar(c)) {
                    state = State::NumInterfix;
                    wordBuffer.append(c);
                } else {
                    buffer.clear();
                    return qMakePair(
                        RDataType::RDataErrorString,
                        makeErrorString("number is end with non-number char"));
                }
                break;
            case State::NumInterfix:
                if (isNumberChar(c)) {
                    state = State::Num;
                    wordBuffer.append(c);
                } else if (isNumberPrefixChar(c)) {
                    wordBuffer.append(c);
                    state = State::NumPrefix;
                } else {
                    buffer.clear();
                    return qMakePair(
                        RDataType::RDataErrorString,
                        makeErrorString("number interfix is end with "
                                        "non-number char"));
                };
            case State::Cmd:
                if (c != '%') {
                    wordBuffer.append(c);
                } else {
                    wordBuffer.append(c);
                    removeBufferFront();
                    return qMakePair(RDataType::RDataControlWord, wordBuffer);
                }
                break;
            case State::Gap:
                if (isGapChar(c)) {
                    // do nothing
                } else if (c == '%') {
                    state = State::Cmd;
                    wordBuffer.append(c);
                } else if (isNumberPrefixChar(c)) {
                    state = State::NumPrefix;
                    wordBuffer.append(c);
                } else if (isNumberChar(c)) {
                    state = State::Num;
                    wordBuffer.append(c);
                } else {
                    buffer.clear();
                    return qMakePair(RDataType::RDataErrorString,
                                     makeErrorString("gap"));
                }
                break;
            case State::Begin:
                if (isGapChar(c)) {
                    state = State::Gap;
                } else if (c == '%') {
                    state = State::Cmd;
                    wordBuffer.append(c);
                } else if (isNumberPrefixChar(c)) {
                    state = State::NumPrefix;
                    wordBuffer.append(c);
                } else if (isNumberChar(c)) {
                    state = State::Num;
                    wordBuffer.append(c);
                } else {
                    buffer.clear();
                    return qMakePair(RDataType::RDataErrorString,
                                     makeErrorString("begin"));
                }
                break;
        }
    }

    return std::nullopt;
}

std::optional<QPair<DataStreamParser::RDataType, QVariant>>
DataStreamParser::parseAsCSVFile() {
    // TODO Support CSV file
    return std::nullopt;
}
