#include "editorview.h"

#include "console.h"
#include "pythonsourceedit.h"

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
#include <QMessageBox>
#include <QFileDialog>

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
    PythonSourceEdit *editor = new PythonSourceEdit(tabs);
    highlighter = new PythonHighlighter(editor->document());
    tabs->addTab(editor,tr("Untitled"));
    editor->document()->setModified(false);
    connect(editor->document(),SIGNAL(contentsChanged()),SLOT(document_changed()));

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
    menu_bar->addMenu(create_view_menu(menu_bar));
    menu_bar->addMenu(create_deploy_menu(menu_bar));
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
    console->setEnabled(false);
    con_v_layout->addWidget(console);
    connect(console,SIGNAL(external_text_change(QString)),SLOT(console_text_edited(QString)));

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

    QAction *edit_undo = new QAction(tr("Undo"),edit);
    edit_undo->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Z));
    connect(edit_undo,SIGNAL(triggered(bool)),SLOT(menu_edit_undo_clicked()));
    edit->addAction(edit_undo);

    QAction *edit_redo = new QAction(tr("Redo"),edit);
    edit_redo->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Z));
    connect(edit_redo,SIGNAL(triggered(bool)),SLOT(menu_edit_redo_clicked()));
    edit->addAction(edit_redo);

    edit->addSeparator();

    QAction *edit_cut = new QAction(tr("Undo"),edit);
    edit_cut->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_X));
    connect(edit_cut,SIGNAL(triggered(bool)),SLOT(menu_edit_cut_clicked()));
    edit->addAction(edit_cut);

    QAction *edit_copy = new QAction(tr("Undo"),edit);
    edit_copy->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_C));
    connect(edit_copy,SIGNAL(triggered(bool)),SLOT(menu_edit_copy_clicked()));
    edit->addAction(edit_copy);

    QAction *edit_paste = new QAction(tr("Undo"),edit);
    edit_paste->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_V));
    connect(edit_paste,SIGNAL(triggered(bool)),SLOT(menu_edit_paste_clicked()));
    edit->addAction(edit_paste);

    return edit;
}

QMenu* EditorView::create_view_menu(QWidget *parent)
{
    QMenu *view = new QMenu(tr("View"),parent);

    QAction *view_mark_whitespaces = new QAction(tr("Mark whitespaces"),view);
    view_mark_whitespaces->setCheckable(true);
    view_mark_whitespaces->setChecked(false);
    view_mark_whitespaces->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Space));
    connect(view_mark_whitespaces,SIGNAL(toggled(bool)),SLOT(menu_view_mark_whitespaces(bool)));
    view->addAction(view_mark_whitespaces);

    return view;
}

QMenu* EditorView::create_deploy_menu(QWidget *parent)
{
    QMenu *deploy = new QMenu(tr("Deploy"),parent);

    QAction *deploy_selected = new QAction(tr("Upload selected"),deploy);
    deploy_selected->setShortcut(QKeySequence(Qt::Key_F5));
    connect(deploy_selected,SIGNAL(triggered(bool)),SLOT(menu_deploy_upload_selected()));
    deploy->addAction(deploy_selected);

    QAction *deploy_file = new QAction(tr("Upload file"),deploy);
    deploy_file->setShortcut(QKeySequence(Qt::Key_F6));
    connect(deploy_file,SIGNAL(triggered(bool)),SLOT(menu_deploy_upload_file()));
    deploy->addAction(deploy_file);

    QAction *deploy_file_to_file = new QAction(tr("Upload file into file"),deploy);
    deploy_file_to_file->setShortcut(QKeySequence(Qt::Key_F7));
    connect(deploy_file_to_file,SIGNAL(triggered(bool)),SLOT(menu_deploy_upload_file_to_file()));
    deploy->addAction(deploy_file_to_file);

    QAction *deploy_all_open = new QAction(tr("Upload all open into file"),deploy);
    deploy_all_open->setShortcut(QKeySequence(Qt::Key_F8));
    connect(deploy_all_open,SIGNAL(triggered(bool)),SLOT(menu_deploy_upload_all_open()));
    deploy->addAction(deploy_all_open);

    return deploy;
}

bool EditorView::save_document(QTextDocument *doc)
{
    QFile *save_file;
    if(doc->metaInformation(QTextDocument::DocumentUrl).isEmpty()    ||
       doc->metaInformation(QTextDocument::DocumentTitle).isEmpty())
    {
        QFileDialog dia;
        dia.setAcceptMode(QFileDialog::AcceptSave);
        if(dia.exec())
        {
            QStringList fl = dia.selectedFiles();
            save_file = new QFile(fl.at(0));
            doc->setMetaInformation(QTextDocument::DocumentUrl,fl.at(0));
            doc->setMetaInformation(QTextDocument::DocumentTitle,QFileInfo(*save_file).baseName());
        }
        else
        {
            return false;
        }
    }
    else
    {
        save_file = new QFile(doc->metaInformation(QTextDocument::DocumentUrl));
    }
    if(save_file->open(QIODevice::ReadWrite | QIODevice::Text))
    {
        status_bar->showMessage(tr("Saving \"") + save_file->fileName() + "\"...");
        QTextStream stream(save_file);
        stream << doc->toPlainText() << endl;
        stream.flush();

        status_bar->showMessage(tr("Successfully saved \"") + save_file->fileName() + "\"!",3000);
        tabs->setTabText(tabs->currentIndex(),doc->metaInformation(QTextDocument::DocumentTitle));
        doc->setModified(false);
        save_file->deleteLater();

        return true;
    }
    else
    {
        // here another message box
        status_bar->showMessage(tr("Could not open \"") + save_file->fileName() + "\"! (" + save_file->errorString() + ")");
        save_file->deleteLater();
        return false;
    }
}

void EditorView::document_changed()
{
    int current_index = tabs->currentIndex();
    if(static_cast<PythonSourceEdit*>(tabs->currentWidget())->document()->isModified() &&
            !tabs->tabText(current_index).endsWith('*'))
    {
        tabs->setTabText(current_index,tabs->tabText(current_index) + "*");
    }


}

void EditorView::menu_file_new_clicked()
{
    PythonSourceEdit *new_edit = new PythonSourceEdit(tabs);
    // should be deleted when textedit is deleted
    new PythonHighlighter(new_edit->document());
    tabs->addTab(new_edit,tr("Untitled"));
    connect(new_edit->document(),SIGNAL(contentsChanged()),SLOT(document_changed()));
}

void EditorView::menu_file_open_clicked()
{

}

void EditorView::menu_file_save_current_clicked()
{
    save_document(static_cast<PythonSourceEdit*>(tabs->currentWidget())->document());
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

void EditorView::menu_edit_undo_clicked()
{
    PythonSourceEdit *edit = static_cast<PythonSourceEdit*>(tabs->currentWidget());
    edit->document()->undo();
}

void EditorView::menu_edit_redo_clicked()
{
    PythonSourceEdit *edit = static_cast<PythonSourceEdit*>(tabs->currentWidget());
    edit->document()->redo();
}

void EditorView::menu_edit_cut_clicked()
{

}

void EditorView::menu_edit_copy_clicked()
{

}

void EditorView::menu_edit_paste_clicked()
{

}

void EditorView::menu_view_mark_whitespaces(bool marked)
{
    highlighter->markWhitespaces(marked);
}

void EditorView::menu_deploy_upload_selected()
{
    if(serial_device)
    {
        //serial_device->write(QChar(0x04),1);
        QByteArray data;
        data.append(QChar(0x05)); // enter past mode
        QString text = static_cast<QTextEdit*>(tabs->currentWidget())->textCursor().selectedText();
        foreach(QString subst,text.split("\n"))
        {
            //TODO
            data.append(subst);
            data.append(QChar('\r'));
            data.append(QChar('\n'));
        }
        data.append(QChar(0x04)); // leave paste mode
        serial_device->write(data.data(),data.length());
        data.clear();
    }
    else
    {
        status_bar->showMessage("Could not deploy. No serial device connected!",2500);
    }
}

void EditorView::menu_deploy_upload_file()
{
    if(serial_device)
    {
        //serial_device->write(QChar(0x04),1);
        QByteArray data;
        data.append(QChar(0x05)); // enter past mode
        QString text = static_cast<QTextEdit*>(tabs->currentWidget())->document()->toPlainText();
        data.append(text);
        data.append(QChar(0x04)); // leave paste mode
        serial_device->write(data.data(),data.length());
        data.clear();
    }
    else
    {
        status_bar->showMessage("Could not deploy. No serial device connected!",2500);
    }
}

void EditorView::menu_deploy_upload_file_to_file()
{
    if(serial_device)
    {

    }
    else
    {
        status_bar->showMessage("Could not deploy. No serial device connected!",2500);
    }
}

void EditorView::menu_deploy_upload_all_open()
{
    if(serial_device)
    {

    }
    else
    {
        status_bar->showMessage("Could not deploy. No serial device connected!",2500);
    }
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
            console->setEnabled(true);
            connect(serial_device,SIGNAL(signal_data_available(int)),SLOT(serial_recieve_bytes(int)));
            con->setText(tr("Disconnect"));
        }
        else
        {
            delete serial_device;
            serial_device = nullptr;
            console->setEnabled(false);
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
    if(PythonSourceEdit *edit = dynamic_cast<PythonSourceEdit*>(tabs->widget(index)))
    {
        QTextDocument *doc = edit->document();
        if(doc->isModified())
        {
            QMessageBox box;
            box.setText(tr("The document has been modified."));
            box.setInformativeText(tr("Do you want to save your changes?"));
            box.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
            box.setDefaultButton(QMessageBox::Save);
            switch (box.exec()) {
            case QMessageBox::Save:
                if(save_document(doc))
                {
                    // save complete, close doc
                }
                else
                {
                    // save incomplete, dont close doc
                }
                break;
            case QMessageBox::Discard:
                // TODO
                break;
            case QMessageBox::Cancel:
                // Cancel was clicked
                break;
            default:
                // should never be reached
                break;
            }
        }
    }
}
