#include "editorview.h"

#include "console.h"

#include <cstdio>

#include <QDebug>
#include <QTimer>
#include <QMenuBar>
#include <QStatusBar>
#include <QSplitter>
#include <QListWidget>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QTabWidget>

QTimer *console_updater;

QMenuBar *menu_bar;
QStatusBar *status_bar;

QSplitter *sidebar_splitter;
QSplitter *src_con_splitter;
QSplitter *con_splitter;

QListWidget  *sidebar;

Console *console;
QLineEdit *console_command;
QAbstractButton *console_command_send;
QTabWidget *editors;
QList<QPlainTextEdit*> *editor_list;
QList<QString> *filenames;
QPlainTextEdit *editor;

QLineEdit *serial_device_path;

QComboBox *baud_combobox;

//TODO
// connect buttons or any other widget
// tab and texedits for each tab


// some settings with their default values
int sidebar_size = 120;
int console_height = 100;
int console_width = 400;

EditorView::EditorView(QWidget *parent) :
    QMainWindow(parent)
{
    console_text_changed = false;
    console_updater = new QTimer(this);
    connect(console_updater,SIGNAL(timeout()),SLOT(update_console()));
    //console_updater->start(10);
    serial_device = nullptr;
    editor_list = new QList<QPlainTextEdit*>();
    filenames = new QList<QString>();
    setup_gui();
}

EditorView::~EditorView()
{
    if(serial_device)
    {
        if(serial_device->isOpen())serial_device->close();
        delete serial_device;
        serial_device = nullptr;
    }
    delete editor_list;
    delete filenames;
}

void EditorView::setup_gui()
{
    this->setGeometry(0,0,1000,800);
    add_widgets();
    add_menu();
    add_status();
    connect_more_signals();
    setup_baud_combo();
    load_gui_settings();
    status_bar->showMessage(tr("There is neither an open file nor an active connection."));
}

void EditorView::load_gui_settings()
{

}

void EditorView::add_widgets()
{
    sidebar_splitter = new QSplitter(this);
    this->setCentralWidget(sidebar_splitter);

    // Sidebar
    sidebar = new QListWidget(sidebar_splitter);
    sidebar_splitter->addWidget(sidebar);

    // set size
    QList<int> sizes;
    sizes.append(sidebar_size);
    sizes.append(sidebar_splitter->sizeHint().width() - sidebar_size);
    sidebar_splitter->setSizes(sizes);

    //
    src_con_splitter = new QSplitter(sidebar_splitter);
    src_con_splitter->setOrientation(Qt::Vertical);
    sidebar_splitter->addWidget(src_con_splitter);

    // Editor
    editor = new QPlainTextEdit(src_con_splitter);
    src_con_splitter->addWidget(editor);

    // Console and Connection splitter
    con_splitter = new QSplitter(src_con_splitter);
    src_con_splitter->addWidget(con_splitter);

    con_splitter->addWidget(create_console_widget(con_splitter));
    con_splitter->addWidget(create_connection_widget(con_splitter));

    sizes.clear();
    sizes.append(src_con_splitter->sizeHint().height() - console_height);
    sizes.append(console_height);
    src_con_splitter->setSizes(sizes);

    sizes.clear();
    sizes.append(console_width);
    sizes.append(con_splitter->sizeHint().width() - console_width);
    con_splitter->setSizes(sizes);
}

void EditorView::add_menu()
{
    menu_bar = new QMenuBar(this);
    this->setMenuBar(menu_bar);
    menu_bar->addMenu(create_file_menu(menu_bar));
    menu_bar->addMenu(create_edit_menu(menu_bar));
}

void EditorView::add_status()
{
    status_bar = new QStatusBar(this);
    this->setStatusBar(status_bar);
}

void EditorView::setup_baud_combo()
{
    baud_combobox->addItem("1200",1200);
    baud_combobox->addItem("2400",2400);
    baud_combobox->addItem("4800",4800);
    baud_combobox->addItem("9600",9600);
    baud_combobox->addItem("19200",19200);
    baud_combobox->addItem("38400",38400);
    baud_combobox->addItem("57600",57600);
    baud_combobox->addItem("115200",115200);
    baud_combobox->setCurrentIndex(7);
}

void EditorView::connect_more_signals()
{

}

QWidget* EditorView::create_connection_widget(QWidget *root)
{
    // Connection Gridlayout
    QWidget *connection_root_widget = new QWidget(root);
    QGridLayout *connection_layout = new QGridLayout(connection_root_widget);
    connection_root_widget->setLayout(connection_layout);

    // Add widgets to connection Layout
    QPushButton *con = new QPushButton("Connect",connection_root_widget);
    connect(con,SIGNAL(pressed()),SLOT(serial_connect_clicked()));

    serial_device_path = new QLineEdit(connection_root_widget);
    serial_device_path->setText("/dev/ttyUSB");
    QLabel *baud_label = new QLabel("Baudrate",connection_root_widget);
    baud_combobox = new QComboBox(connection_root_widget);
    connection_layout->addWidget(con,0,0);
    connection_layout->addWidget(serial_device_path,0,1);
    connection_layout->addWidget(baud_label,1,0);
    connection_layout->addWidget(baud_combobox,1,1);
    return connection_root_widget;
}

QWidget* EditorView::create_console_widget(QWidget *root)
{
    // Console V_Layout
    QWidget *console_root_widget = new QWidget(root);
    QVBoxLayout *con_v_layout = new QVBoxLayout(console_root_widget);
    console_root_widget->setLayout(con_v_layout);

    QLabel *console_label = new QLabel(tr("Console"),console_root_widget);
    con_v_layout->addWidget(console_label);

    console = new Console(console_root_widget);
    con_v_layout->addWidget(console);
    connect(console,SIGNAL(external_text_change(QString)),SLOT(console_text_edited(QString)));

    // extra lineedit for commands
//    QWidget *console_send_root_widget = new QWidget(console_root_widget);
//    QHBoxLayout *con_h_layout = new QHBoxLayout(console_send_root_widget);
//    console_command = new QLineEdit(console_root_widget);
//    con_h_layout->addWidget(console_command);

//    console_command_send = new QPushButton(console_root_widget);
//    console_command_send->setText(tr("Send"));
//    con_h_layout->addWidget(console_command_send);


//    console_send_root_widget->setLayout(con_h_layout);
//    con_v_layout->addWidget(console_send_root_widget);

    return console_root_widget;
}

QMenu* EditorView::create_file_menu(QWidget *parent)
{
    QMenu *file = new QMenu(tr("File"),parent);
    menu_bar->addMenu(file);

    QAction *file_new = new QAction(tr("New..."),file);
    file_new->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_N));
    connect(file_new,SIGNAL(triggered(bool)),SLOT(menu_file_new_clicked()));
    file->addAction(file_new);

    QAction *file_open = new QAction(tr("Open file or project..."),file);
    file_open->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_O));
    connect(file_open,SIGNAL(triggered(bool)),SLOT(menu_file_open_clicked()));
    file->addAction(file_open);

    file->addSeparator();

    QAction *file_save_cur = new QAction(tr("Save current file"),file);
    file_save_cur->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
    connect(file_save_cur,SIGNAL(triggered(bool)),SLOT(menu_file_save_current_clicked()));
    file->addAction(file_save_cur);

    QAction *file_save_as = new QAction(tr("Save current file as..."),file);
    connect(file_save_as,SIGNAL(triggered(bool)),SLOT(menu_file_save_current_as_clicked()));
    file->addAction(file_save_as);

    QAction *file_save_all = new QAction(tr("Save all files"),file);
    file_save_all->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_S));
    connect(file_save_all,SIGNAL(triggered(bool)),SLOT(menu_file_save_all_clicked()));
    file->addAction(file_save_all);

    file->addSeparator();

    QAction *file_quit = new QAction(tr("Quit"),file);
    file_quit->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));
    connect(file_quit,SIGNAL(triggered(bool)),SLOT(menu_file_quit_clicked()));
    file->addAction(file_quit);

    return file;
}

QMenu* EditorView::create_edit_menu(QWidget *parent)
{
    QMenu *edit = new QMenu(tr("Edit"),parent);

    return edit;
}

void EditorView::update_tabs()
{
    if(editor_list->size() == editors->count())
        return;
    for(int i = 0; i < editor_list->size(); ++i)
    {
        if(i >= editors->count() - 1)
        {
            editors->addTab(editor_list->at(i),filenames->at(i));
        }
        if(editors->widget(i) != editor_list->at(i))
        {
            // replace widget and name
        }

    }
}

void EditorView::update_console()
{
    if(console_text_changed)
    {
        /*console->setText(console_text);
        QTextCursor cursor = console->textCursor();
        cursor.movePosition(QTextCursor::End,QTextCursor::MoveAnchor,0);
        console->setTextCursor(cursor);
        console_text_changed = false;*/
    }
}

void EditorView::menu_file_new_clicked()
{
    qDebug("Test1234");
}

void EditorView::menu_file_open_clicked()
{

}

void EditorView::menu_file_save_current_clicked()
{

}

void EditorView::menu_file_save_current_as_clicked()
{

}

void EditorView::menu_file_save_all_clicked()
{

}

void EditorView::menu_file_close_current_clicked()
{

}

void EditorView::menu_file_close_all_clicked()
{

}

void EditorView::menu_file_quit_clicked()
{

}

void EditorView::serial_connect_clicked()
{
    serial_device = new Serial(serial_device_path->text());
    serial_device->setBaudrate(baud_combobox->currentData().toInt());
    if(serial_device->open())
    {
        status_bar->showMessage(tr("Successfully opened ") + serial_device_path->text() + "!");
        //console->setText(QString());
        console->clear();
        connect(serial_device,SIGNAL(signal_data_available(int)),SLOT(serial_recieve_bytes(int)));
    }
    else
    {
        status_bar->showMessage(tr("Could not open serial device: ") + serial_device_path->text(),2000);
    }
}

void EditorView::serial_recieve_bytes(int num_bytes)
{
    char *buffer = new char[num_bytes];
    serial_device->read(buffer,num_bytes);
    QByteArray data;
    for(int i = 0; i < num_bytes; ++i)
    {
        if(buffer[i] == 0x0d)
        {
            //console->putData("\n");
            data.append(QChar('\n'));
            ++i;
        }
        else if(buffer[i] == 0x08)
        {
            console->delete_last_char();
            if(i + 3 <= num_bytes)
            {
                i += 3;
            }
            else
            {
                // this has to be validated
                i = num_bytes;
            }
        }
        else
        {
            //console->putData(QChar(buffer[i]));
            data.append(QChar(buffer[i]));
        }
    }
    console->putData(data);
    data.clear();
    delete buffer;
}

void EditorView::console_text_edited(QString text)
{
    if(serial_device)
        serial_device->write(text.toLatin1().data(),text.length());
    qDebug() << text;
}
