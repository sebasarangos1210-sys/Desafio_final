#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , nivelActual(nullptr)
{
    ui->setupUi(this);

    // Configurar ventana
    setWindowTitle("Dia D");
    setMinimumSize(800, 600);

    // Genera un contenedor estatico
    contenedor = new QStackedWidget(this);

    // Agregar el menú como primera página
    contenedor->addWidget(ui->centralwidget);

    // Establecer el conteneder estatico como principal
    setCentralWidget(contenedor);

    // Mostrar el menú al inicio
    contenedor->setCurrentIndex(0);

    // Conectar botones del menú
    connect(ui->btnNivel1, &QPushButton::clicked, this, &MainWindow::cargarNivel1);
    connect(ui->btnNivel2, &QPushButton::clicked, this, &MainWindow::cargarNivel2);
    connect(ui->btnNivel3, &QPushButton::clicked, this, &MainWindow::cargarNivel3);
}

MainWindow::~MainWindow()
{
    if (nivelActual != nullptr) {
        delete nivelActual;
    }
    delete ui;
}

void MainWindow::cargarNivel1()
{
    // Eliminar nivel anterior
    if (nivelActual != nullptr) {
        contenedor->removeWidget(nivelActual);
        delete nivelActual;
        nivelActual = nullptr;
    }

    // Crear nuevo nivel
    nivelActual = new Nivel(1, this);
    connect(nivelActual, &Nivel::volverAlMenu, this, &MainWindow::mostrarMenuPrincipal);

    // Agregar nivel al contenedor
    contenedor->addWidget(nivelActual);

    // Cambiar a la página del nivel
    contenedor->setCurrentWidget(nivelActual);
}

void MainWindow::cargarNivel2()
{
    // Eliminar nivel anterior
    if (nivelActual != nullptr) {
        contenedor->removeWidget(nivelActual);
        delete nivelActual;
        nivelActual = nullptr;
    }

    // Crear nuevo nivel
    nivelActual = new Nivel(2, this);
    connect(nivelActual, &Nivel::volverAlMenu, this, &MainWindow::mostrarMenuPrincipal);

    // Agregar nivel al contenedor
    contenedor->addWidget(nivelActual);

    // Cambiar a la página del nivel
    contenedor->setCurrentWidget(nivelActual);
}

void MainWindow::cargarNivel3()
{
    // Eliminar nivel anterior
    if (nivelActual != nullptr) {
        contenedor->removeWidget(nivelActual);
        delete nivelActual;
        nivelActual = nullptr;
    }

    // Crear nuevo nivel
    nivelActual = new Nivel(3, this);
    connect(nivelActual, &Nivel::volverAlMenu, this, &MainWindow::mostrarMenuPrincipal);

    // Agregar nivel al contenedor
    contenedor->addWidget(nivelActual);

    // Cambiar a la página del nivel
    contenedor->setCurrentWidget(nivelActual);
}

void MainWindow::mostrarMenuPrincipal()
{
    // Cambiar a la página del menú
    contenedor->setCurrentIndex(0);

    // Eliminar el nivel después de cambiar de página
    if (nivelActual != nullptr) {
        contenedor->removeWidget(nivelActual);
        nivelActual->deleteLater();
        nivelActual = nullptr;
    }
}
