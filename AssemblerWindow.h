#ifndef ASSEMBLERWINDOW_H
#define ASSEMBLERWINDOW_H

#include "Assembler.h"
#include <QMainWindow>
#include <QString>

class QPlainTextEdit;
class QAction;

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
    void onSourceModified(bool modified);

private:
    bool maybeSave();
    bool writeToFile(const QString &path);
    bool readFromFile(const QString &path);
    void setCurrentFile(const QString &path);
    void appendOutput(const QString &text, bool isError);
    void clearOutput();
    void updateTitle();

    QPlainTextEdit *source;
    QPlainTextEdit *output;
    QAction *actNew;
    QAction *actOpen;
    QAction *actSave;
    QAction *actSaveAs;
    QAction *actUndo;
    QAction *actRedo;
    QAction *actCut;
    QAction *actCopy;
    QAction *actPaste;
    QAction *actSelectAll;
    QAction *actAssemble;
    QString currentFile;
    bool dirty;
    Assembler assembler;
};

#endif // ASSEMBLERWINDOW_H
