#include "mainwindow.h"
#include "proto/message.pb.h"
#include <iostream>

MainWindow::MainWindow()
    : m_button("Hello World")
    , m_entry() // creates a new button with label "Hello World".
{
    set_title("Server");
    set_border_width(10);
    set_size_request(200, 100);

    m_button.signal_clicked().connect(
        sigc::mem_fun(*this, &MainWindow::on_button_clicked));

    add(m_vbox);
    m_vbox.set_spacing(10);
    m_vbox.pack_start(m_button);
    m_vbox.pack_start(m_entry);

    show_all_children();

    m_server = new game::Server();
    m_server->Run();
}

MainWindow::~MainWindow()
{
    delete m_server;
}

void MainWindow::on_button_clicked()
{
    game::GreetMessage greetMessage;
    greetMessage.set_text(m_entry.get_text().c_str());
    m_server->SendMessage(greetMessage);
}