#include "nivel.h"
#include "cadete.h"
#include "fuerzaarmada.h"

#include <QVBoxLayout>
#include <QGraphicsRectItem>
#include <QKeyEvent>
#include <QBrush>
#include <QPixmap>
#include <QDebug>
#include <QMouseEvent>

#include "bala.h"


Nivel::Nivel(int numeroNivel, QWidget *parent, qreal _v_alto, qreal _v_ancho)
    : QWidget(parent),
    numNivel(numeroNivel),
    viewportSize(_v_ancho, _v_alto),
    fondoSize(0.0, 0.0),
    limiteMapa(fondoSize - viewportSize),
    camara(0.0, 0.0),
    vista(nullptr),
    escena(nullptr),
    fondoScroll(nullptr),
    timer(nullptr),
    jugador(nullptr),
    ia(nullptr),
    mouseDir(0.0, 0.0),
    m_moveLeft(false),
    m_moveRight(false),
    m_moveUp(false),
    m_moveDown(false)
{
    inicializarUI();
    inicializarEscena();
    cargarElementosNivel();

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Nivel::actualizarJuego);
    timer->start(16);
}

Nivel::~Nivel()
{
    if (timer) timer->stop();

    for (auto e : enemigos) delete e;
    // Obstáculos son hijos de fondoScroll → los borra la escena
    obstaculos.clear();

    delete jugador;
    delete ia;
}

void Nivel::inicializarUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);

    vista  = new QGraphicsView(this);
    escena = new QGraphicsScene(this);

    escena->setSceneRect(-viewportSize.x()/2.0,
                         -viewportSize.y()/2.0,
                         viewportSize.x(),
                         viewportSize.y());

    QPixmap imagenFondo(":/ui/ui/Fondo_Nivel_3.png");

    fondoSize.set(imagenFondo.width(), imagenFondo.height());
    limiteMapa = fondoSize - viewportSize;

    if (limiteMapa.x() < 0) limiteMapa.setX(0);
    if (limiteMapa.y() < 0) limiteMapa.setY(0);

    camara = limiteMapa / 2.0;

    fondoScroll = new QGraphicsPixmapItem(imagenFondo);
    fondoScroll->setZValue(-1000);

    actualizarPosicionFondo();
    escena->addItem(fondoScroll);

    vista->setScene(escena);
    vista->setFocusPolicy(Qt::NoFocus);
    vista->setMouseTracking(true);
    vista->viewport()->setMouseTracking(true);
    vista->viewport()->installEventFilter(this);
    vista->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    vista->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    vista->viewport()->setMouseTracking(true);
    this->setMouseTracking(true);

    btnVolver = new QPushButton("Volver al Menú", this);
    btnVolver->setGeometry(10,10,120,30);
    connect(btnVolver, &QPushButton::clicked, this, &Nivel::onVolverClicked);

    vista->centerOn(0,0);
    layout->addWidget(vista);
    setLayout(layout);
    setFocusPolicy(Qt::StrongFocus);
}

void Nivel::inicializarEscena()
{
    switch (numNivel) {
    case 1: escena->setBackgroundBrush(QColor(135,206,235)); break;
    case 2: escena->setBackgroundBrush(QColor(25,25,112)); break;
    case 3: break;
    }
}

void Nivel::cargarElementosNivel()
{
    // --- JUGADOR ---
    jugador = new Cadete(10);
    jugador->setPos(0,0);
    escena->addItem(jugador);

    auto fondoCentro = Vector2D(fondoSize.x() / 2.0, fondoSize.y() / 2.0);

    // --- enemigos de prueba ---
    // Los colocamos en el mapa, así que deben ser hijos del fondo
    Cadete *e1 = new Cadete(10);
    e1->setParentItem(fondoScroll);
    e1->setPos(fondoSize.x()/2.0 + 200, fondoSize.y()/2.0);
    enemigos.push_back(e1);

    Cadete *e2 = new Cadete(10);
    e2->setParentItem(fondoScroll);
    e2->setPos(fondoSize.x()/2.0 - 300, fondoSize.y()/2.0 + 150);
    enemigos.push_back(e2);

    Cadete *e3 = new Cadete(10);
    e3->setParentItem(fondoScroll);
    e3->setPos(fondoSize.x()/2.0 + 100, fondoSize.y()/2.0 - 200);
    enemigos.push_back(e3);

    for (auto e : enemigos)
        escena->addItem(e);

    for (auto *e : enemigos){
        QPointF pj = jugador->scenePos();
        QPointF pe = e->scenePos();

        Vector2D dir(pj.x() - pe.x(),
                     pj.y() - pe.y());

        if (dir.magnitud2() > 0)
            dir = dir.normalizado();

        e->setDireccion(dir);
    }

    // --- Timer para disparos enemigos ---
    timerDisparoEnemigos = new QTimer(this);
    connect(timerDisparoEnemigos, &QTimer::timeout, this, &Nivel::disparosEnemigos);
    timerDisparoEnemigos->start(1000); // cada 1 segundo

    // ===========================
    //   CREAR 4 OBSTÁCULOS
    // ===========================

    // 1) Piedra circular (radio 30)
    Obstaculo *piedra = new Obstaculo(30);
    piedra->setParentItem(fondoScroll);
    piedra->setPos(fondoCentro.x() - 200, fondoCentro.y() - 50);
    obstaculos.push_back(piedra);

    // 2) Caja rectangular (60x40)
    Obstaculo *caja = new Obstaculo(60, 40);
    caja->setParentItem(fondoScroll);
    caja->setPos(fondoCentro.x()+150, fondoCentro.y()-80);
    obstaculos.push_back(caja);

    // 3) Bulto pequeño circular (radio 20)
    Obstaculo *bulto = new Obstaculo(20);
    bulto->setParentItem(fondoScroll);
    bulto->setPos(fondoCentro.x()-120, fondoCentro.y()+120);
    obstaculos.push_back(bulto);

    // 4) Pared larga (120x20)
    Obstaculo *pared = new Obstaculo(120, 20);
    pared->setParentItem(fondoScroll);
    pared->setPos(fondoCentro.x()/2.0+250, fondoCentro.y()+150);
    obstaculos.push_back(pared);

}

void Nivel::disparosEnemigos()
{
    // Cada enemigo dispara en su dirección actual
    for (auto *e : enemigos) disparar(e);
}

void Nivel::actualizarJuego()
{
    if (m_moveLeft || m_moveRight || m_moveUp || m_moveDown){
        actualizarFondo();

        for (auto *e : enemigos){
            QPointF pj = jugador->scenePos();
            QPointF pe = e->scenePos();

            Vector2D dir(pj.x() - pe.x(),
                         pj.y() - pe.y());

            if (dir.magnitud2() > 0)
                dir = dir.normalizado();

            e->setDireccion(dir);
        }
    }

    for (auto *p : proyectiles)
        if (p)
            p->avanzar();

    // Limpiar los que ya murieron
    proyectiles.erase(
        std::remove_if(
            proyectiles.begin(),
            proyectiles.end(),
            [&](Proyectil* p){
                if (p->muerto) {
                    escena->removeItem(p);
                    delete p;
                    return true;
                }
                return false;
            }
            ),
        proyectiles.end()
        );

    actualizarIA();
    manejarColisiones();
}

void Nivel::mouseMoveEvent(QMouseEvent *event)
{
    if (!jugador) return;

    QPointF posEscena = vista->mapToScene(event->pos());
    QPointF posJugador = jugador->scenePos();

    Vector2D dirMouse(posEscena.x() - posJugador.x(),
                      posEscena.y() - posJugador.y());

    if (dirMouse.magnitud2() > 0) {
        mouseDir = dirMouse.normalizado();
        jugador->setDireccion(mouseDir);
        jugador->update();
    }
}

void Nivel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        disparar(jugador);
}


bool Nivel::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == vista->viewport()) {

        // Bloquear scroll con la rueda
        if (event->type() == QEvent::Wheel) {
            return true; // consumimos el evento, la vista no lo procesa
        }

        // Movimiento del mouse (lo que ya tenías)
        if (event->type() == QEvent::MouseMove) {
            auto *me = static_cast<QMouseEvent*>(event);
            mouseMoveEvent(me);
            return false;
        }
    }

    return QWidget::eventFilter(obj, event);
}


void Nivel::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_A:
    case Qt::Key_Left:  m_moveLeft = true; break;
    case Qt::Key_D:
    case Qt::Key_Right: m_moveRight = true; break;
    case Qt::Key_W:
    case Qt::Key_Up:    m_moveUp = true; break;
    case Qt::Key_S:
    case Qt::Key_Down:  m_moveDown = true; break;
    }
}

void Nivel::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_A:
    case Qt::Key_Left:  m_moveLeft = false; break;
    case Qt::Key_D:
    case Qt::Key_Right: m_moveRight = false; break;
    case Qt::Key_W:
    case Qt::Key_Up:    m_moveUp = false; break;
    case Qt::Key_S:
    case Qt::Key_Down:  m_moveDown = false; break;
    }
}

void Nivel::actualizarFondo()
{
    const qreal speed = 3.0;
    Vector2D dir(0,0);

    if (m_moveUp)    dir.setY(dir.y() + 1);
    if (m_moveDown)  dir.setY(dir.y() - 1);
    if (m_moveLeft)  dir.setX(dir.x() + 1);
    if (m_moveRight) dir.setX(dir.x() - 1);

    if (dir.magnitud2() == 0)
        return;

    dir = dir.normalizado();
    Vector2D nuevaCamara = camara - (dir * speed);

    if (nuevaCamara.x() < 0) nuevaCamara.setX(0);
    if (nuevaCamara.x() > limiteMapa.x()) nuevaCamara.setX(limiteMapa.x());
    if (nuevaCamara.y() < 0) nuevaCamara.setY(0);
    if (nuevaCamara.y() > limiteMapa.y()) nuevaCamara.setY(limiteMapa.y());

    camara = nuevaCamara;
    actualizarPosicionFondo();
}

void Nivel::actualizarPosicionFondo()
{
    Vector2D origenFondo = (viewportSize / -2.0) - camara;
    fondoScroll->setPos(origenFondo.x(), origenFondo.y());
}

void Nivel::disparar(FuerzaArmada *emisor)
{
    if (!emisor) return;

    // Convertir posición del jugador (escena) → coordenadas del fondo
    QPointF posJugadorEnFondo = fondoScroll->mapFromScene(emisor->scenePos());

    // Crear la bala con dirección del emisor
    Bala *b = new Bala(emisor, emisor->getDireccion());

    // Hacer que la bala sea HIJA del fondo
    b->setParentItem(fondoScroll);

    // Posición en el mapa
    b->setPos(posJugadorEnFondo);

    // Guardar en lista
    proyectiles.push_back(b);
}


void Nivel::manejarColisiones() {}
void Nivel::actualizarIA() {}
void Nivel::onVolverClicked()
{
    if (timer) timer->stop();
    emit volverAlMenu();
}

