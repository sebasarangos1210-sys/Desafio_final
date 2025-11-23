#include "nivel.h"
#include "cadete.h"
#include "fuerzaarmada.h"
#include <QVBoxLayout>
#include <QGraphicsRectItem>
#include <QKeyEvent>
#include <QBrush>
#include <iostream>

Nivel::Nivel(int numeroNivel, QWidget *parent, qreal _v_alto, qreal _v_ancho)
    : QWidget(parent),
    numNivel(numeroNivel),
    jugador(nullptr),
    ia(nullptr),
    v_alto(_v_alto),
    v_ancho(_v_ancho),
    m_moveLeft(false),
    m_moveRight(false),
    m_moveUp(false),
    m_moveDown(false)
{
    inicializarUI();
    inicializarEscena();
    cargarElementosNivel();

    // Iniciar bucle juego
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Nivel::actualizarJuego);
    timer->start(16);
}

Nivel::~Nivel()
{
    timer->stop();

    // Limpiar
    for (auto p : enemigos) {
        delete p;
    }
    for (auto o : obstaculos) {
        delete o;
    }

    if (jugador) delete jugador;
    if (ia) delete ia;
}

void Nivel::inicializarUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Crear vista y escena
    vista = new QGraphicsView(this);
    escena = new QGraphicsScene(this);

    escena->setSceneRect(-v_ancho/2, -v_alto/2, v_ancho, v_alto);
    QPixmap imagenFondo(":/ui/ui/Fondo_Nivel_3.png");
    if (imagenFondo.isNull()) {
        qDebug() << "ERROR: no se pudo cargar el fondo";
    } else {
        qDebug() << "OK: fondo cargado";
    }


    limx =imagenFondo.width()-v_ancho;
    limy =imagenFondo.height()-v_alto;

    qDebug() << "im W |" << imagenFondo.width() << "|" << "im W |" << imagenFondo.height() << "|";

    fondoOffsetX = -imagenFondo.width()/2;
    fondoOffsetY = -imagenFondo.height()/2;
    fondoScroll = new QGraphicsPixmapItem(imagenFondo);
    fondoScroll->setZValue(-1000);
    fondoScroll->setPos(fondoOffsetX,fondoOffsetY);
    escena->addItem(fondoScroll);

    // Configurar vista
    vista->setScene(escena);
    vista->setRenderHint(QPainter::Antialiasing);
    vista->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    vista->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    vista->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Boton volver
    btnVolver = new QPushButton("Volver al Menú", this);
    btnVolver->setGeometry(10, 10, 120, 30);
    connect(btnVolver, &QPushButton::clicked, this, &Nivel::onVolverClicked);

    vista->centerOn(0, 0);

    layout->addWidget(vista);
    setLayout(layout);

    setFocusPolicy(Qt::StrongFocus);
}

void Nivel::inicializarEscena()
{

    // Fondo según el nivel
    switch(numNivel) {
    case 1:
        escena->setBackgroundBrush(QBrush(QColor(135, 206, 235))); // Cielo azul
        break;
    case 2:
        escena->setBackgroundBrush(QBrush(QColor(25, 25, 112))); // Azul oscuro (noche)
        break;
    case 3:
        //escena->setBackgroundBrush(QBrush(QColor(34, 139, 34)));
        break;
    }
}

void Nivel::cargarElementosNivel()
{
    jugador = new cadete(10,0,0) ;
    escena->addItem(jugador);
}
void Nivel::actualizarJuego()
{
    // Bucle principal

    if (m_moveLeft || m_moveRight || m_moveUp || m_moveDown){
        //updateCollisions();
        qDebug() << "x : |" << fondoOffsetX << "| y : |" << fondoOffsetY << "|";
        qDebug() << "vx: |" << limx << "vy: |" << limy << "|";
        actualizarFondo();
    }

    // 1. Actualizar posiciones de participantes
    for (auto p : enemigos) {
        // p->actualizar();
    }

    // 2. Actualizar IA
    actualizarIA();

    // 3. Detectar colisiones
    manejarColisiones();

    // 4. Actualizar proyectiles
    // ...

    // 5. Verificar condiciones de victoria/derrota
    // ...
}

void Nivel::actualizarIA()
{
    if (ia != nullptr) {
        // ia->Comportamiento();
    }
}

void Nivel::manejarColisiones()
{
    // Usar QGraphicsScene::collidingItems()
    // O implementar tu propia detección según el diagrama
}
void Nivel::keyPressEvent(QKeyEvent *event)
{
    // El teclado solo modifica flags de intención de movimiento;
    // el desplazamiento real se hace en updateBarcoFromInput().
    switch (event->key()) {
        case Qt::Key_A:
            m_moveLeft = true;
            break;
        case Qt::Key_Left:
            m_moveLeft = true;
            break;
        case Qt::Key_D:
            m_moveRight = true;
            break;
        case Qt::Key_Right:
            m_moveRight = true;
            break;
        case Qt::Key_W:
            m_moveUp = true;
            break;
        case Qt::Key_Up:
            m_moveUp = true;
            break;
        case Qt::Key_S:
            m_moveDown = true;
            break;
        case Qt::Key_Down:
            m_moveDown = true;
            break;
        default:
            QWidget::keyPressEvent(event);
            break;
    }
}

void Nivel::keyReleaseEvent(QKeyEvent *event)
{
    // Al soltar la tecla, desactivamos la dirección correspondiente
    switch (event->key()) {
        case Qt::Key_A:
            m_moveLeft = false;
            break;
        case Qt::Key_Left:
            m_moveLeft = false;
            break;
        case Qt::Key_D:
            m_moveRight = false;
            break;
        case Qt::Key_Right:
            m_moveRight = false;
            break;
        case Qt::Key_W:
            m_moveUp = false;
            break;
        case Qt::Key_Up:
            m_moveUp = false;
            break;
        case Qt::Key_S:
            m_moveDown = false;
            break;
        case Qt::Key_Down:
            m_moveDown = false;
            break;
        default:
            QWidget::keyReleaseEvent(event);
            break;
    }
}

void Nivel::actualizarFondo(){

    const qreal speed = 3.0; // velocidad base del jugador

    qreal dx = 0.0;
    qreal dy = 0.0;

    if (m_moveUp)    dy += speed;
    if (m_moveDown)  dy -= speed;
    if (m_moveLeft)  dx += speed;
    if (m_moveRight) dx -= speed;

    // nada presionado → nada se mueve
    if (dx == 0.0 && dy == 0.0) return;

    // Normalizar dirección
    qreal len = qSqrt(dx*dx + dy*dy);
    dx = (dx / len) * speed;
    dy = (dy / len) * speed;

    qreal pos_new_x = fondoOffsetX + dx;
    qreal pos_new_y = fondoOffsetY + dy;

    // Limitar el movimiento al area jugable
    if (pos_new_x >= -limx+(v_ancho/2)
        && pos_new_x <= -v_ancho/2)
        fondoOffsetX = pos_new_x;
    if ( pos_new_y >= -limy+(v_alto/2)
        && pos_new_y <= -v_alto/2)
        fondoOffsetY = pos_new_y;

    fondoScroll->setPos(fondoOffsetX, fondoOffsetY);

}

void Nivel::onVolverClicked()
{
    timer->stop(); // Detener el juego
    emit volverAlMenu();
}
