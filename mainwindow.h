#ifndef GTKMM_MAINWINDOW_H
#define GTKMM_MAINWINDOW_H

#include "Server.h"
#include <gtkmm.h>
#include <gtkmm/button.h>
#include <gtkmm/window.h>

class MainWindow : public Gtk::Window
{

public:
    MainWindow();
    virtual ~MainWindow();

protected:
    game::Server* m_server;
    // Signal handlers:
    void on_button_clicked();

    // Member widgets:
    Gtk::Button m_button;
    Gtk::Entry m_entry;
    Gtk::Box m_vbox;
};

#endif // GTKMM_MAINWINDOW_H