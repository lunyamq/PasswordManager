#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QFormLayout>
#include "../core/vault_storage.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onCreateVault();
    void onOpenVault();
    void onSaveVault();
    void onCloseVault();
    void onAddEntry();
    void onEditEntry();
    void onDeleteEntry();
    void onSearchTextChanged(const QString& text);
    void onEntrySelectionChanged();
    void onEntryDoubleClicked(const QModelIndex& index);

private:
    VaultStorage vault;
    bool isOpen = false;
    QString currentFilename;

    // Виджеты
    QTableWidget* entryTable;
    QLineEdit* searchLineEdit;
    QPushButton* addButton;
    QPushButton* editButton;
    QPushButton* deleteButton;
    QPushButton* saveButton;
    QLabel* titleValue, * usernameValue, * passwordValue, * urlValue, * notesValue;

    void createUI();
    void updateEntryList(const std::string& filter = "");
    void showStatusMessage(const QString& msg);
    void clearDetailsPanel();
};

#endif // MAINWINDOW_H