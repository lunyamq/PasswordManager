#include "mainwindow.h"
#include "../core/utils.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QStatusBar>
#include <QHBoxLayout>
#include <QComboBox>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QHeaderView>
#include <QSplitter>
#include <QStyle>
#include <QIcon>
#include <QFormLayout>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    createUI();
    showStatusMessage("Ready. Open or create a vault.");
}

MainWindow::~MainWindow() = default;

void MainWindow::createUI()
{
    setWindowTitle("Password Manager");
    resize(1000, 650);

    // Глобальный стиль (тёмная тема)
    this->setStyleSheet(R"(
        QMainWindow { background-color: #1e1e2f; }
        QWidget { background-color: #1e1e2f; color: #cdd6f4; font-family: 'Segoe UI', 'Roboto'; font-size: 13px; }
        QTableWidget {
            background-color: #181825;
            alternate-background-color: #1e1e2f;
            gridline-color: #313244;
            selection-background-color: #cba6f7;
            selection-color: #11111b;
            border: none;
            outline: none;
        }
        QTableWidget::item { padding: 8px; border-bottom: 1px solid #313244; }
        QHeaderView::section {
            background-color: #181825; color: #89b4fa; padding: 8px;
            border: none; border-bottom: 2px solid #cba6f7; font-weight: bold;
        }
        QTableWidget::item:selected {
            background-color: #cba6f7;
            color: #11111b;
        }
        QTableWidget::item:selected:focus {
            background-color: #cba6f7;
            color: #11111b;
        }
        QTableWidget::item:selected QHeaderView::section {
            background-color: #cba6f7;
            color: #11111b;
        }
        QPushButton {
            background-color: #313244; color: #cdd6f4; border: none;
            border-radius: 8px; padding: 8px 16px; font-weight: bold;
            transition: background-color 0.2s ease;
        }
        QPushButton:hover { background-color: #45475a; }
        QPushButton:pressed { background-color: #585b70; }
        QLineEdit {
            background-color: #181825; border: 1px solid #45475a;
            border-radius: 8px; padding: 6px 10px;
        }
        QLineEdit:focus { border: 1px solid #cba6f7; }
        QMenuBar { background-color: #181825; color: #cdd6f4; border-bottom: 1px solid #313244; }
        QMenuBar::item:selected { background-color: #45475a; }
        QMenu { background-color: #1e1e2f; border: 1px solid #313244; }
        QMenu::item:selected { background-color: #cba6f7; color: #11111b; }
        QStatusBar { color: #6c7086; border-top: 1px solid #313244; }
        QSplitter::handle { background-color: #313244; }
        QLabel { color: #cdd6f4; }
        QLabel[heading="true"] { font-size: 14px; font-weight: bold; margin-bottom: 8px; color: #cba6f7; }
    )");

    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    QHBoxLayout* mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(8, 8, 8, 8);

    // ----- Левая панель -----
    QWidget* leftPanel = new QWidget();
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setSpacing(12);

    // Поиск
    searchLineEdit = new QLineEdit();
    searchLineEdit->setPlaceholderText("Search by title...");
    searchLineEdit->addAction(style()->standardIcon(QStyle::SP_FileDialogContentsView), QLineEdit::LeadingPosition);
    QAction* clearAction = new QAction(searchLineEdit);
    clearAction->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
    connect(clearAction, &QAction::triggered, searchLineEdit, &QLineEdit::clear);
    searchLineEdit->addAction(clearAction, QLineEdit::TrailingPosition);
    leftLayout->addWidget(searchLineEdit);

    // Таблица записей
    entryTable = new QTableWidget();
    entryTable->setColumnCount(3);
    entryTable->setHorizontalHeaderLabels({ "Title", "Username", "ID" });
    entryTable->setColumnHidden(2, true); // скрываем ID
    entryTable->horizontalHeader()->setStretchLastSection(true);
    entryTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    entryTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    entryTable->setAlternatingRowColors(true);
    leftLayout->addWidget(entryTable);

    // Кнопки действий
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    addButton = new QPushButton("Add");
    addButton->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
    editButton = new QPushButton("Edit");
    editButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    deleteButton = new QPushButton("Delete");
    deleteButton->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
    saveButton = new QPushButton("Save");
    saveButton->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(editButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(saveButton);
    leftLayout->addLayout(buttonLayout);

    // ----- Правая панель (детали) -----
    QWidget* rightPanel = new QWidget();
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
    QLabel* detailsHeading = new QLabel("Entry Details");
    detailsHeading->setProperty("heading", true);
    rightLayout->addWidget(detailsHeading);

    QFormLayout* formLayout = new QFormLayout();
    titleValue = new QLabel("-");
    titleValue->setWordWrap(true);
    titleValue->setTextInteractionFlags(Qt::TextSelectableByMouse);
    usernameValue = new QLabel("-");
    usernameValue->setTextInteractionFlags(Qt::TextSelectableByMouse);
    passwordValue = new QLabel("-");
    passwordValue->setTextInteractionFlags(Qt::TextSelectableByMouse);
    urlValue = new QLabel("-");
    urlValue->setOpenExternalLinks(true);
    urlValue->setTextInteractionFlags(Qt::TextBrowserInteraction);
    notesValue = new QLabel("-");
    notesValue->setWordWrap(true);
    notesValue->setTextInteractionFlags(Qt::TextSelectableByMouse);

    formLayout->addRow("Title:", titleValue);
    formLayout->addRow("Username:", usernameValue);
    formLayout->addRow("Password:", passwordValue);
    formLayout->addRow("URL:", urlValue);
    formLayout->addRow("Notes:", notesValue);
    rightLayout->addLayout(formLayout);
    rightLayout->addStretch();

    // Сплиттер
    QSplitter* splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(leftPanel);
    splitter->addWidget(rightPanel);
    splitter->setSizes({ 550, 350 });
    mainLayout->addWidget(splitter);

    // Меню
    QMenu* fileMenu = menuBar()->addMenu("&File");
    QAction* createAction = fileMenu->addAction("Create Vault");
    QAction* openAction = fileMenu->addAction("Open Vault");
    QAction* saveAction = fileMenu->addAction("Save");
    QAction* closeAction = fileMenu->addAction("Close Vault");
    fileMenu->addSeparator();
    QAction* exitAction = fileMenu->addAction("Exit");
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);

    QAction* createActionMenu = menuBar()->addAction("&Create");
    QAction* openActionMenu = menuBar()->addAction("&Open");
    QAction* saveActionMenu = menuBar()->addAction("&Save");
    QAction* closeActionMenu = menuBar()->addAction("&Close");

    // Сигналы
    connect(createAction, &QAction::triggered, this, &MainWindow::onCreateVault);
    connect(openAction, &QAction::triggered, this, &MainWindow::onOpenVault);
    connect(saveAction, &QAction::triggered, this, &MainWindow::onSaveVault);
    connect(closeAction, &QAction::triggered, this, &MainWindow::onCloseVault);
    connect(addButton, &QPushButton::clicked, this, &MainWindow::onAddEntry);
    connect(editButton, &QPushButton::clicked, this, &MainWindow::onEditEntry);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::onDeleteEntry);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::onSaveVault);
    connect(searchLineEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    connect(entryTable, &QTableWidget::itemSelectionChanged, this, &MainWindow::onEntrySelectionChanged);
    connect(entryTable, &QTableWidget::doubleClicked, this, &MainWindow::onEntryDoubleClicked);
    connect(createActionMenu, &QAction::triggered, this, &MainWindow::onCreateVault);
    connect(openActionMenu, &QAction::triggered, this, &MainWindow::onOpenVault);
    connect(saveActionMenu, &QAction::triggered, this, &MainWindow::onSaveVault);
    connect(closeActionMenu, &QAction::triggered, this, &MainWindow::onCloseVault);

    clearDetailsPanel();
}

void MainWindow::updateEntryList(const std::string& filter)
{
    entryTable->setRowCount(0);
    if (!isOpen) return;

    auto entries = vault.get_all_entries();
    int row = 0;
    for (const auto& e : entries) {
        // Фильтрация
        if (!filter.empty()) {
            std::string lowFilter = filter;
            std::transform(lowFilter.begin(), lowFilter.end(), lowFilter.begin(), ::tolower);
            std::string titleLow = e.title;
            std::transform(titleLow.begin(), titleLow.end(), titleLow.begin(), ::tolower);
            if (titleLow.find(lowFilter) == std::string::npos) {
                continue;
            }
        }
        entryTable->insertRow(row);
        entryTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(e.title)));
        entryTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(e.username)));
        entryTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(e.id)));
        row++;
    }
    // Если после фильтрации selection изменился, обновим детали
    if (entryTable->currentRow() != -1)
        onEntrySelectionChanged();
    else
        clearDetailsPanel();
}

void MainWindow::clearDetailsPanel()
{
    titleValue->setText("-");
    usernameValue->setText("-");
    passwordValue->setText("-");
    urlValue->setText("-");
    notesValue->setText("-");
}

void MainWindow::showStatusMessage(const QString& msg)
{
    statusBar()->showMessage(msg, 3000);
}

void MainWindow::onCreateVault()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Create Vault", "", "Vault Files (*.pwm)");
    if (fileName.isEmpty()) return;
    if (!fileName.endsWith(".pwm", Qt::CaseInsensitive))
        fileName += ".pwm";
    QString password = QInputDialog::getText(this, "Master Password", "Enter master password:", QLineEdit::Password);
    if (password.isEmpty()) return;
    QString confirm = QInputDialog::getText(this, "Confirm Password", "Confirm master password:", QLineEdit::Password);
    if (password != confirm) {
        QMessageBox::critical(this, "Error", "Passwords do not match.");
        return;
    }
    CipherType type = showCipherDialog();

    if (vault.create(fileName.toStdString(), password.toStdString(), type)) {
        isOpen = true;
        currentFilename = fileName;
        updateEntryList();
        showStatusMessage("Vault created and opened.");
    }
    else {
        QMessageBox::critical(this, "Error", "Failed to create vault.");
    }
}

void MainWindow::onOpenVault()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open Vault", "", "Vault Files (*.pwm);;All Files (*)");
    if (fileName.isEmpty()) return;
    if (!fileName.endsWith(".pwm", Qt::CaseInsensitive))
        fileName += ".pwm";
    QString password = QInputDialog::getText(this, "Master Password", "Enter master password:", QLineEdit::Password);
    if (vault.open(fileName.toStdString(), password.toStdString())) {
        isOpen = true;
        currentFilename = fileName;
        updateEntryList();
        showStatusMessage("Vault opened. " + QString::number(vault.get_all_entries().size()) + " entries.");
    }
    else {
        QMessageBox::critical(this, "Error", "Wrong password or corrupt file.");
    }
}

void MainWindow::onSaveVault()
{
    if (!isOpen) {
        QMessageBox::warning(this, "Warning", "No vault open.");
        return;
    }
    if (vault.save()) {
        showStatusMessage("Vault saved.");
    }
    else {
        QMessageBox::critical(this, "Error", "Save failed.");
    }
}

void MainWindow::onCloseVault()
{
    vault.close();
    isOpen = false;
    currentFilename.clear();
    updateEntryList();
    clearDetailsPanel();
    showStatusMessage("Vault closed.");
}

void MainWindow::onAddEntry()
{
    if (!isOpen) {
        QMessageBox::warning(this, "Warning", "No vault open.");
        return;
    }
    QString title = QInputDialog::getText(this, "Add Entry", "Title:");
    if (title.isEmpty()) return;
    QString username = QInputDialog::getText(this, "Add Entry", "Username:");
    QString password = QInputDialog::getText(this, "Add Entry", "Password:");
    QString url = QInputDialog::getText(this, "Add Entry", "URL:");
    QString notes = QInputDialog::getText(this, "Add Entry", "Notes:");

    Entry e;
    e.id = generate_uuid();
    e.title = title.toStdString();
    e.username = username.toStdString();
    e.password = password.toStdString();
    e.url = url.toStdString();
    e.notes = notes.toStdString();
    e.created = get_current_time();
    e.updated = e.created;
    vault.add_entry(e);
    updateEntryList();
    showStatusMessage("Entry added.");
}

void MainWindow::onEditEntry()
{
    if (!isOpen) {
        QMessageBox::warning(this, "Warning", "No vault open.");
        return;
    }
    int row = entryTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Edit", "Please select an entry first.");
        return;
    }
    QString id = entryTable->item(row, 2)->text();
    Entry* e = vault.find_entry(id.toStdString());
    if (!e) return;

    QString title = QInputDialog::getText(this, "Edit Entry", "Title:", QLineEdit::Normal, QString::fromStdString(e->title));
    if (!title.isEmpty()) e->title = title.toStdString();
    QString username = QInputDialog::getText(this, "Edit Entry", "Username:", QLineEdit::Normal, QString::fromStdString(e->username));
    if (!username.isEmpty()) e->username = username.toStdString();
    QString password = QInputDialog::getText(this, "Edit Entry", "Password:", QLineEdit::Normal, QString::fromStdString(e->password));
    if (!password.isEmpty()) e->password = password.toStdString();
    QString url = QInputDialog::getText(this, "Edit Entry", "URL:", QLineEdit::Normal, QString::fromStdString(e->url));
    if (!url.isEmpty()) e->url = url.toStdString();
    QString notes = QInputDialog::getText(this, "Edit Entry", "Notes:", QLineEdit::Normal, QString::fromStdString(e->notes));
    if (!notes.isEmpty()) e->notes = notes.toStdString();
    e->updated = get_current_time();
    updateEntryList();
    showStatusMessage("Entry updated.");
}

void MainWindow::onDeleteEntry()
{
    if (!isOpen) {
        QMessageBox::warning(this, "Warning", "No vault open.");
        return;
    }
    int row = entryTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Delete", "Please select an entry first.");
        return;
    }
    QString id = entryTable->item(row, 2)->text();
    if (vault.delete_entry(id.toStdString())) {
        updateEntryList();
        showStatusMessage("Entry deleted.");
    }
    else {
        QMessageBox::critical(this, "Error", "Deletion failed.");
    }
}

void MainWindow::onSearchTextChanged(const QString& text)
{
    updateEntryList(text.toStdString());
}

void MainWindow::onEntrySelectionChanged()
{
    int row = entryTable->currentRow();
    if (row < 0 || !isOpen) {
        clearDetailsPanel();
        return;
    }
    QString id = entryTable->item(row, 2)->text();
    Entry* e = vault.find_entry(id.toStdString());
    if (e) {
        titleValue->setText(QString::fromStdString(e->title));
        usernameValue->setText(QString::fromStdString(e->username));
        passwordValue->setText(QString::fromStdString(e->password));
        QString urlStr = QString::fromStdString(e->url);
        if (urlStr.isEmpty()) {
            urlValue->setText("-");
        }
        else {
            urlValue->setText(QString("<a href=\"%1\">%1</a>").arg(urlStr));
        }
        notesValue->setText(QString::fromStdString(e->notes).isEmpty() ? "-" : QString::fromStdString(e->notes));
    }
    else {
        clearDetailsPanel();
    }
}

void MainWindow::onEntryDoubleClicked(const QModelIndex& index)
{
    Q_UNUSED(index);
    onEntrySelectionChanged(); // уже показывает детали, но можно сделать отдельное окно.
    // Для удобства просто покажем детали в правой панели – они уже там.
}

CipherType MainWindow::showCipherDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Select Cipher");
    QVBoxLayout layout(&dialog);
    QLabel label("Choose encryption algorithm:");
    QComboBox combo;
    combo.addItems({ "AES-256", "ChaCha20", "Salsa20" });
    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout.addWidget(&label);
    layout.addWidget(&combo);
    layout.addWidget(&buttons);

    connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        QString selected = combo.currentText();
        if (selected == "AES-256") return CipherType::AES_256;
        if (selected == "ChaCha20") return CipherType::CHACHA20;
        return CipherType::SALSA20;
    }
    
    return CipherType::AES_256;
}