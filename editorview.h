#ifndef EDITORVIEW_H
#define EDITORVIEW_H

#include <QMainWindow>

#include "serial.h"

class EditorView : public QMainWindow
{
    Q_OBJECT

public:
    explicit EditorView(QWidget *parent = 0);
    ~EditorView();

private:
    Serial *serial_device;
    QString console_text;
    bool console_text_changed;
    void setup_gui();
    void load_gui_settings();
    void add_widgets();
    void add_menu();
    void add_status();
    void setup_baud_combo();
    void connect_more_signals();
    QWidget* create_connection_widget(QWidget *root);
    QWidget* create_console_widget(QWidget *root);

    QMenu* create_file_menu(QWidget *parent);
    QMenu* create_edit_menu(QWidget *parent);

    void update_tabs();

private slots:
    void update_console();

    // ==== menu slots ====
    // == file slots ==
    void menu_file_new_clicked();
    void menu_file_open_clicked();
    void menu_file_save_current_clicked();
    void menu_file_save_current_as_clicked();
    void menu_file_save_all_clicked();
    void menu_file_close_current_clicked();
    void menu_file_close_all_clicked();
    void menu_file_quit_clicked();

    // == serial connection slots ==
    void serial_connect_clicked();
    void serial_recieve_bytes(int num_bytes);
    void console_text_edited(QString text);
};

#endif // EDITORVIEW_H
