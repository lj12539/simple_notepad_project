#include "main_window.h"
#include "notepad_exception.h"
#include "spell_checker.h"
#include "spell_checker_highlighter.h"

#include "ui_find_replace_dialog.h"
#include "ui_word_frequency_dialog.h"
#include <QAction>
#include <QApplication>
#include <QDialog>
#include <QFile>
#include <QFileDialog>
#include <QFont>
#include <QKeySequence>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QTextStream>
#include <QToolBar>

main_window::main_window()
{
    setWindowTitle("Notepad");
    resize(800, 600);

    editor = new QTextEdit(this);
    setCentralWidget(editor);

    checker = std::make_unique<spell_checker>("data/words.txt");
    highlighter = new spell_checker_highlighter(editor->document(), *checker);

    transforms.push_back(std::make_unique<uppercase_transform>());
    transforms.push_back(std::make_unique<lowercase_transform>());
    transforms.push_back(std::make_unique<capitalized_transform>());
    transforms.push_back(std::make_unique<sentence_case_transform>());
    transforms.push_back(std::make_unique<swap_case_transform>());

    setup_file_menu();
    setup_edit_menu();
    setup_format_menu();
    setup_format_toolbar();
    setup_search_menu();
    setup_tools_menu();

    connect(editor, &QTextEdit::textChanged, this, [this] {
        QString text = editor->toPlainText();
        QStringList words = text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);

        int word_count = words.size();
        int line_count = text.isEmpty() ? 0 : text.count('\n') + 1;

        statusBar()->showMessage(QString("Words: %1  Lines: %2").arg(word_count).arg(line_count));
    });

    editor->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(editor, &QTextEdit::customContextMenuRequested, this, [this](const QPoint& pos) {
        QMenu* menu = editor->createStandardContextMenu(pos);

        QTextCursor cursor = editor->cursorForPosition(pos);

        cursor.select(QTextCursor::WordUnderCursor);
        QString selected_word = cursor.selectedText();

        if (!selected_word.isEmpty() && !checker->is_correct(selected_word.toStdString())) {
            auto suggestions = checker->get_suggestions(selected_word.toStdString());

            if (!suggestions.empty()) {
                QAction* first_action = menu->actions().isEmpty() ? nullptr : menu->actions().first();

                QMenu* suggest_menu = new QMenu("Spelling Suggestions", menu);

                for (const auto& word : suggestions) {
                    QString q_word = QString::fromStdString(word);
                    QAction* suggest_action = suggest_menu->addAction(q_word);

                    connect(suggest_action, &QAction::triggered, this, [this, cursor, q_word]() mutable {
                        cursor.insertText(q_word);
                    });
                }

                menu->insertMenu(first_action, suggest_menu);
                menu->insertSeparator(first_action);
            }
        }

        menu->exec(editor->mapToGlobal(pos));
        delete menu;
    });
}

main_window::~main_window() = default;

void main_window::setup_file_menu()
{
    auto* file_menu = menuBar()->addMenu("File");

    const auto* action_new = file_menu->addAction("New");
    connect(action_new, &QAction::triggered, this, [this] {
        editor->clear();
        current_file.clear();
        update_title();
    });

    file_menu->addSeparator();

    const auto* action_open = file_menu->addAction("Open...");
    connect(action_open, &QAction::triggered, this, [this] {
        open_file();
    });
    const auto* action_save = file_menu->addAction("Save");
    connect(action_save, &QAction::triggered, this, [this] {
        save_file();
    });

    const auto* action_save_as = file_menu->addAction("Save As...");
    connect(action_save_as, &QAction::triggered, this, [this] {
        save_file_as();
    });

    file_menu->addSeparator();

    const auto* action_exit = file_menu->addAction("Exit");
    connect(action_exit, &QAction::triggered, this, [] {
        QApplication::quit();
    });
}

void main_window::setup_edit_menu()
{
    auto* edit_menu = menuBar()->addMenu("Edit");

    auto* action_undo = edit_menu->addAction("Undo");
    action_undo->setShortcut(QKeySequence::Undo);
    connect(action_undo, &QAction::triggered, editor, &QTextEdit::undo);

    auto* action_redo = edit_menu->addAction("Redo");
    action_redo->setShortcut(QKeySequence::Redo);
    connect(action_redo, &QAction::triggered, editor, &QTextEdit::redo);

    edit_menu->addSeparator();

    auto* action_cut = edit_menu->addAction("Cut");
    action_cut->setShortcut(QKeySequence::Cut);
    connect(action_cut, &QAction::triggered, editor, &QTextEdit::cut);

    auto* action_copy = edit_menu->addAction("Copy");
    action_copy->setShortcut(QKeySequence::Copy);
    connect(action_copy, &QAction::triggered, editor, &QTextEdit::copy);

    auto* action_paste = edit_menu->addAction("Paste");
    action_paste->setShortcut(QKeySequence::Paste);
    connect(action_paste, &QAction::triggered, editor, &QTextEdit::paste);

    edit_menu->addSeparator();

    auto* action_select_all = edit_menu->addAction("Select All");
    action_select_all->setShortcut(QKeySequence::SelectAll);
    connect(action_select_all, &QAction::triggered, editor, &QTextEdit::selectAll);
}

void main_window::setup_format_menu()
{
    auto* format_menu = menuBar()->addMenu("Format");
    auto* text_case_menu = format_menu->addMenu("Text Case");

    for (const auto& transform : transforms) {
        const auto* action = text_case_menu->addAction(QString::fromStdString(transform->name()));
        connect(action, &QAction::triggered, this, [this, &transform] {
            apply_transform(*transform);
        });
    }
}

void main_window::setup_format_toolbar()
{
    auto* toolbar = addToolBar("Format");
    toolbar->setIconSize(QSize(16, 16));

    auto* action_bold = toolbar->addAction(QIcon("data/images/bold.svg"), "Bold");
    action_bold->setCheckable(true);
    action_bold->setShortcut(QKeySequence("Ctrl+B"));
    connect(action_bold, &QAction::triggered, this, [this](const bool checked) {
        QTextCharFormat fmt;
        fmt.setFontWeight(checked ? QFont::Bold : QFont::Normal);
        editor->mergeCurrentCharFormat(fmt);
    });
    auto* action_italic = toolbar->addAction(QIcon("data/images/italic.svg"), "Italic");
    action_italic->setCheckable(true);
    action_italic->setShortcut(QKeySequence("Ctrl+I"));
    connect(action_italic, &QAction::triggered, this, [this](const bool checked) {
        QTextCharFormat fmt;
        fmt.setFontItalic(checked);
        editor->mergeCurrentCharFormat(fmt);
    });

    auto* action_underline = toolbar->addAction(QIcon("data/images/underline.svg"), "Underline");
    action_underline->setCheckable(true);
    action_underline->setShortcut(QKeySequence("Ctrl+U"));
    connect(action_underline, &QAction::triggered, this, [this](const bool checked) {
        QTextCharFormat fmt;
        fmt.setFontUnderline(checked);
        editor->mergeCurrentCharFormat(fmt);
    });

    connect(editor, &QTextEdit::currentCharFormatChanged,
        this, [action_bold, action_italic, action_underline](const QTextCharFormat& fmt) {
            action_bold->setChecked(fmt.fontWeight() == QFont::Bold);
            action_italic->setChecked(fmt.fontItalic());
            action_underline->setChecked(fmt.fontUnderline());
        });
}

void main_window::apply_transform(const text_transform& transform) const
{
    auto cursor = editor->textCursor();
    if (!cursor.hasSelection()) {
        cursor.select(QTextCursor::Document);
    }
    const int start = cursor.selectionStart();
    const QString selected = cursor.selectedText().replace(QChar::ParagraphSeparator, '\n');
    const std::string original = selected.toStdString();
    const auto result = transform.apply(original);

    cursor.beginEditBlock();
    for (std::size_t i = 0; i < result.size(); ++i) {
        if (original[i] != result[i]) {
            cursor.setPosition(start + static_cast<int>(i));
            cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
            cursor.insertText(QString(QChar(result[i])), cursor.charFormat());
        }
    }
    cursor.endEditBlock();
}

void main_window::open_file()
{
    const auto path = QFileDialog::getOpenFileName(this, "Open File");
    if (path.isEmpty()) {
        return;
    }

    try {
        QFile file(path);

        if (!file.exists()) {
            throw file_not_found_exception(path.toStdString());
        }

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            throw file_read_exception(path.toStdString());
        }

        QTextStream in(&file);
        editor->setPlainText(in.readAll());
        file.close();

        current_file = path;
        update_title();

    } catch (const notepad_exception& ex) {
        QMessageBox::critical(this, "Error", QString::fromStdString(ex.what()));
    }
}

void main_window::save_file()
{
    if (current_file.isEmpty()) {
        save_file_as();
        return;
    }

    try {
        QFile file(current_file);

        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            throw file_write_exception(current_file.toStdString());
        }

        QTextStream out(&file);
        out << editor->toPlainText();
        file.close();

    } catch (const notepad_exception& ex) {
        QMessageBox::critical(this, "Error", QString::fromStdString(ex.what()));
    }
}
void main_window::save_file_as()
{
    const auto path = QFileDialog::getSaveFileName(this, "Save File As");
    if (path.isEmpty()) {
        return;
    }
    current_file = path;
    save_file();
    update_title();
}
void main_window::update_title()
{
    if (current_file.isEmpty()) {
        setWindowTitle("Notepad");
    } else {
        setWindowTitle("Notepad: " + current_file);
    }
}

void main_window::find_next(const QString& term, QTextDocument::FindFlags flags) const
{
    if (term.isEmpty()) {
        return;
    }
    auto found = editor->document()->find(term, editor->textCursor(), flags);
    if (found.isNull()) {
        auto from_start = editor->textCursor();
        from_start.movePosition(QTextCursor::Start);
        found = editor->document()->find(term, from_start, flags);
    }
    if (!found.isNull()) {
        editor->setTextCursor(found);
    }
}

void main_window::replace_current(const QString& term, const QString& replacement,
    QTextDocument::FindFlags flags) const
{
    if (auto cursor = editor->textCursor(); cursor.hasSelection()) {
        cursor.insertText(replacement);
        editor->setTextCursor(cursor);
    }
    find_next(term, flags);
}

void main_window::replace_all(const QString& term, const QString& replacement,
    QTextDocument::FindFlags flags) const
{
    if (term.isEmpty()) {
        return;
    }
    auto start_cursor = editor->textCursor();
    start_cursor.movePosition(QTextCursor::Start);
    editor->setTextCursor(start_cursor);

    while (true) {
        const auto found = editor->document()->find(term, editor->textCursor(), flags);
        if (found.isNull()) {
            break;
        }
        editor->setTextCursor(found);
        auto c = editor->textCursor();
        c.insertText(replacement);
        editor->setTextCursor(c);
    }
}

void main_window::setup_search_menu()
{
    auto* search_menu = menuBar()->addMenu("Search");

    auto* action_find_replace = search_menu->addAction("Find / Replace...");
    action_find_replace->setShortcut(QKeySequence::Find);
    connect(action_find_replace, &QAction::triggered, this, [this] {
        show_find_replace_dialog();
    });
}

void main_window::setup_tools_menu()
{
    auto* tool_menu = menuBar()->addMenu("Tools");

    auto* word_frequency = tool_menu->addAction("Word Frequency");
    connect(word_frequency, &QAction::triggered, this, [this] {
        show_word_frequency_dialog();
    });

    auto* check_spelling_action = tool_menu->addAction("Check Spelling...");
    connect(check_spelling_action, &QAction::triggered, this, [this] {
        if (highlighter) {
            highlighter->rehighlight();
        }
    });
}

void main_window::show_find_replace_dialog()
{
    if (!find_replace_dlg) {

        find_replace_dlg = new QDialog(this);

        find_replace_ui = std::make_unique<Ui::find_replace_dialog>();

        find_replace_ui->setupUi(find_replace_dlg);

        connect(find_replace_ui->find_next_button,
            &QPushButton::clicked,
            this,
            [this] {
                QTextDocument::FindFlags flags;

                if (find_replace_ui->case_sensitive_check->isChecked()) {
                    flags |= QTextDocument::FindCaseSensitively;
                }

                find_next(find_replace_ui->find_input->text(), flags);
            });

        connect(find_replace_ui->replace_button,
            &QPushButton::clicked,
            this,
            [this] {
                QTextDocument::FindFlags flags;

                if (find_replace_ui->case_sensitive_check->isChecked()) {
                    flags |= QTextDocument::FindCaseSensitively;
                }

                replace_current(
                    find_replace_ui->find_input->text(),
                    find_replace_ui->replace_input->text(),
                    flags);
            });

        connect(find_replace_ui->replace_all_button,
            &QPushButton::clicked,
            this,
            [this] {
                QTextDocument::FindFlags flags;

                if (find_replace_ui->case_sensitive_check->isChecked()) {
                    flags |= QTextDocument::FindCaseSensitively;
                }

                replace_all(
                    find_replace_ui->find_input->text(),
                    find_replace_ui->replace_input->text(),
                    flags);
            });

        connect(find_replace_ui->close_button,
            &QPushButton::clicked,
            find_replace_dlg,
            &QDialog::close);
    }

    find_replace_dlg->show();
    find_replace_dlg->raise();
    find_replace_dlg->activateWindow();
}

void main_window::show_word_frequency_dialog()
{

    if (!word_frequency_dlg) {
        word_frequency_dlg = new QDialog(this);
        word_frequency_ui = std::make_unique<Ui::word_frequency_dialog>();
        word_frequency_ui->setupUi(word_frequency_dlg);
    }

    std::map<std::string, int> counts;
    QString text = editor->toPlainText().toLower();
    QStringList words = text.split(QRegularExpression("[^a-zA-Z]+"), Qt::SkipEmptyParts);

    for (const QString& word : words) {
        counts[word.toStdString()]++;
    }

    std::vector<std::pair<std::string, int>> frequencies(counts.begin(), counts.end());
    std::sort(frequencies.begin(), frequencies.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });

    auto* table = word_frequency_ui->frequency_table;

    table->setRowCount(
        static_cast<int>(frequencies.size()));

    for (int row = 0; row < frequencies.size(); ++row) {
        auto* word_item = new QTableWidgetItem(QString::fromStdString(frequencies[row].first));
        auto* count_item = new QTableWidgetItem(QString::number(frequencies[row].second));

        count_item->setTextAlignment(
            Qt::AlignRight | Qt::AlignVCenter);

        table->setItem(row, 0, word_item);
        table->setItem(row, 1, count_item);
    }

    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    table->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);
    table->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignRight);

    word_frequency_dlg->exec();
}