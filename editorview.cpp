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

QMenuBar *menu_bar;
QStatusBar *status_bar;

QSplitter *sidebar_splitter;
QSplitter *src_con_splitter;
QSplitter *con_splitter;

QListWidget  *sidebar;

Console *console;
QLineEdit *console_command;
QAbstractButton *con;
QAbstractButton *console_command_send;
QTabWidget *tabs;
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
    tabs = new QTabWidget(src_con_splitter);
    tabs->setTabsClosable(true);
    connect(tabs,SIGNAL(tabCloseRequested(int)),SLOT(tab_close_requested(int)));

    src_con_splitter->addWidget(tabs);
    editor = new QPlainTextEdit(tabs);
    tabs->addTab(editor,tr("Untitled"));

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
    con = new QPushButton("Connect",connection_root_widget);
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

void EditorView::menu_file_new_clicked()
{
    QPlainTextEdit *new_edit = new QPlainTextEdit(tabs);
    tabs->addTab(new_edit,tr("Untitled"));
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
    if(!serial_device)
    {
        serial_device = new Serial(serial_device_path->text());
        connect(serial_device,SIGNAL(signal_device_error(QSerialPort::SerialPortError)),SLOT(serial_error_handler(QSerialPort::SerialPortError)));
        serial_device->setBaudrate(baud_combobox->currentData().toInt());
        if(serial_device->open())
        {
            status_bar->showMessage(tr("Successfully opened ") + serial_device_path->text() + "!",2500);
            //console->setText(QString());
            console->clear();
            connect(serial_device,SIGNAL(signal_data_available(int)),SLOT(serial_recieve_bytes(int)));
            con->setText(tr("Disconnect"));
        }
        else
        {
            delete serial_device;
            serial_device = nullptr;
        }
    }
    else
    {
        disconnect(serial_device,SIGNAL(signal_device_error(QSerialPort::SerialPortError)),this,SLOT(serial_error_handler(QSerialPort::SerialPortError)));
        //disconnect(this,SLOT(serial_error_handler(QSerialPort::SerialPortError)));
        serial_device->close();
        delete serial_device;
        serial_device = nullptr;
        con->setText(tr("Connect"));
        status_bar->showMessage(tr("Disconnected"),2500);
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

void EditorView::serial_error_handler(QSerialPort::SerialPortError error)
{
    // needs to be improved
    if(error & QSerialPort::DeviceNotFoundError)
    {
        status_bar->showMessage(tr("Device \"") + serial_device_path->text() + tr("\" could not be opened! (Device not found error)"));
        return;
    }
    if(error & QSerialPort::OpenError)
    {
        status_bar->showMessage(tr("Device \"") + serial_device_path->text() + tr("\" could not be opened! (Device busy)"));
        return;
    }
    if(error & QSerialPort::WriteError)
    {
        status_bar->showMessage(tr("Device \"") + serial_device_path->text() + tr("\" was closed! (Write error)"));
        return;
    }
    if(error & QSerialPort::ReadError)
    {
        status_bar->showMessage(tr("Device \"") + serial_device_path->text() + tr("\" was closed! (Read error)"));
        return;
    }
    if(error & QSerialPort::ResourceError)
    {
        status_bar->showMessage(tr("Device \"") + serial_device_path->text() + tr("\" was closed! An I/O error occurred (Device was probably unplugged from system)"));
        return;
    }
    if(error & QSerialPort::UnsupportedOperationError)
    {
        status_bar->showMessage(tr("Device \"") + serial_device_path->text() + tr("\" was closed! An operation was not supported by the running OS."));
        return;
    }
    if(error & QSerialPort::TimeoutError)
    {
        status_bar->showMessage(tr("Device \"") + serial_device_path->text() + tr("\" had a timeout! Disconnecting."));
        return;
    }
    if(error & QSerialPort::UnknownError)
    {
        status_bar->showMessage(tr("An unknown error occurred with the device \"") + serial_device_path->text() + tr("\". Disconnecting."));
        return;
    }

}

void EditorView::tab_close_requested(int index)
{
    // maybe avoid dynamic_cast and use static_cast
    if(QPlainTextEdit *edit = dynamic_cast<QPlainTextEdit*>(tabs->widget(index)))
    {
        // save doc
        qDebug() << edit->toPlainText();
    }
}
