#ifndef ASSEMBLERWINDOW_H
#define ASSEMBLERWINDOW_H

#include "Assembler.h"
#include <QFont>
#include <QList>
#include <QMainWindow>
#include <QMetaObject>
#include <QString>

class QPlainTextEdit;
class QAction;
class QTabWidget;

class AssemblerWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit AssemblerWindow(QWidget *parent = nullptr);
    ~AssemblerWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onNew();
    void onOpen();
    void onSave();
    void onSaveAs();
    void onAssemble();
    void onCloseTab();
    void onTabCloseRequested(int index);
    void onTabChanged(int index);

private:
    bool maybeSaveEditor(QPlainTextEdit *ed);
    bool writeEditorToFile(QPlainTextEdit *ed, const QString &path);
    bool readEditorFromFile(QPlainTextEdit *ed, const QString &path);
    void setEditorFile(QPlainTextEdit *ed, const QString &path);
    QString editorFile(QPlainTextEdit *ed) const;
    QPlainTextEdit *currentEditor() const;
    QPlainTextEdit *newEditorTab(const QString &path = QString());
    void updateTabLabel(QPlainTextEdit *ed);
    void appendOutput(const QString &text, bool isError);
    void clearOutput();
    void updateTitle();
    void wireEditorSignals(QPlainTextEdit *ed);

    QTabWidget *tabs;
    QPlainTextEdit *output;
    QAction *actNew;
    QAction *actOpen;
    QAction *actSave;
    QAction *actSaveAs;
    QAction *actCloseTab;
    QAction *actUndo;
    QAction *actRedo;
    QAction *actCut;
    QAction *actCopy;
    QAction *actPaste;
    QAction *actSelectAll;
    QAction *actAssemble;
    Assembler assembler;
    QList<QMetaObject::Connection> editorConnections;
    QFont monoFont;
};

#endif // ASSEMBLERWINDOW_H
