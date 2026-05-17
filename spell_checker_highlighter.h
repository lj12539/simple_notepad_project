#ifndef SPELL_CHECKER_HIGHLIGHTER_H
#define SPELL_CHECKER_HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include "spell_checker.h"

class spell_checker_highlighter : public QSyntaxHighlighter {
    Q_OBJECT
public:
    spell_checker_highlighter(QTextDocument* parent, const spell_checker& checker)
        : QSyntaxHighlighter(parent), m_checker(checker)
    {
        m_error_format.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
        m_error_format.setUnderlineColor(Qt::red);
    }

protected:
    void highlightBlock(const QString &text) override {
        QRegularExpression word_regex("[a-zA-Z]+");
        QRegularExpressionMatchIterator i = word_regex.globalMatch(text);

        while (i.hasNext()) {
            QRegularExpressionMatch match = i.next();
            QString word = match.captured(0);
            int index = match.capturedStart();
            int length = match.capturedLength();

            if (!m_checker.is_correct(word.toStdString())) {
                setFormat(index, length, m_error_format);
            }
        }
    }

private:
    const spell_checker& m_checker;
    QTextCharFormat m_error_format;
};

#endif