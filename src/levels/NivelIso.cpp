#include "NivelIso.h"

#include <QtMath>
#include <QTimer>
#include <QPainter>
#include <QPaintEvent>
#include <QKeyEvent>
#include <QRectF>
#include <QPushButton>
#include <QRandomGenerator>
#include <QMessageBox>

#include "proyeccioniso.h"

NivelIso::NivelIso(QWidget *parent)
    : QWidget(parent),
    m_timer(new QTimer(this)),
    m_moveLeft(false),
    m_moveRight(false),
    m_sprint(false),             // Sprint desactivado al inicio
    m_scrollOffset(0.0),
    m_scrollSpeed(2.0),          // Velocidad inicial
    m_scrollSpeedBase(2.0),      // Velocidad normal
    m_scrollSpeedSprint(4.5),    // Velocidad con sprint (más rápido)
    m_limiteEliminacion(-200.0),
    m_limiteGeneracion(300.0),
    m_contadorFrames(0),
    m_vidas(4),
    m_vidasMaximas(4),
    m_invulnerable(false),
    m_contadorInvulnerabilidad(0),
    m_tiempoTranscurrido(0),     // Iniciar en 0 frames
    m_tiempoParaGanar(1200),     // 1200 frames = 20 segundos (60 FPS)
    m_nivelCompletado(false)
{
    // Este nivel necesita recibir eventos de teclado directamente
    setFocusPolicy(Qt::StrongFocus);

    // Configura el estado inicial del mundo lógico (barco, obstáculos, área jugable)
    initScene();

    // Bucle del juego: cada timeout dispara updateGame()
    connect(m_timer, &QTimer::timeout,
            this, &NivelIso::updateGame);

    // Aprox. 60 FPS (1000 ms / 16 ≈ 62.5)
    m_timer->start(16);

    // Botón para volver al menú principal (señal personalizada volverAlMenu)
    QPushButton *btnVolver = new QPushButton("Volver", this);
    btnVolver->setGeometry(10, 10, 100, 30);
    connect(btnVolver, &QPushButton::clicked, this, [this]() {
        emit volverAlMenu();
    });
}

NivelIso::~NivelIso()
{
}

// Qt usará este tamaño como mínimo aceptable
QSize NivelIso::minimumSizeHint() const
{ return QSize(640, 480); }

// Qt usará este tamaño como "ideal" al crear o ajustar la ventana
QSize NivelIso::sizeHint() const
{ return QSize(800, 600); }

/*
 * initScene configura el estado inicial del nivel:
 * - Define el tamaño del área jugable (m_playArea) en coordenadas de mundo.
 * - Posiciona el barco FIJO en la parte inferior.
 * - Crea obstáculos iniciales.
 */
void NivelIso::initScene()
{
    // Dimensiones del campo de juego en el plano lógico (mundo 2D):
    // x = "profundidad" (fondo → frente)
    // y = "ancho"       (izquierda ↔ derecha)
    qreal ancho = 300.0;

    // Barco FIJO en la parte inferior (posición Y negativa = cerca del jugador)
    m_barco.setPosition(QPointF(-150.0, 0.0));  // Posición fija en X (cerca), Y centrado

    // Limpiamos obstáculos
    m_obstaculos.clear();

    // Generar algunos obstáculos iniciales
    for (int i = 0; i < 3; i++) {
        Obstaculon2 o;
        qreal x = 100.0 + i * 80.0;  // Lejos del barco
        // Usar bounded con enteros y convertir a qreal
        qreal y = static_cast<qreal>(QRandomGenerator::global()->bounded(-100, 100));
        o.setPosition(QPointF(x, y));
        m_obstaculos.append(o);
    }

    /*
     * m_playArea es el rectángulo que define el espacio jugable en el mundo 2D.
     * - El barco solo puede moverse en el eje Y (izquierda/derecha)
     * - El eje X está reservado para el scrolling automático
     *
     * Convención de ejes:
     *     x (profundidad)
     *     ^
     *    /
     *   /
     *  /
     *  -------> y (ancho)
     */
    m_playArea = QRectF(-200.0, -ancho/2.0, 400.0, ancho);
}

/*
 * paintEvent construye cada fotograma del nivel:
 * - pinta fondo con efecto de scrolling
 * - proyecta y dibuja el área jugable
 * - proyecta y dibuja barco y obstáculos
 * - dibuja las hitboxes de depuración
 */
void NivelIso::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // NUEVO: Dibujar fondo con scrolling
    dibujarFondoScrolling(painter);

    /*
     * Transformación base:
     * - Colocamos el origen del sistema de coordenadas en el centro de la ventana.
     * - A partir de aquí, el mundo 2D se dibuja "alrededor" del centro.
     */
    painter.translate(width() / 2.0, height() / 2.0);

    // Dibujar indicador de vidas (antes de resetear transformación)
    painter.save();
    painter.resetTransform();
    dibujarVidas(painter);
    dibujarTiempo(painter);  // Dibujar tiempo restante
    painter.restore();

    // Volver a aplicar la transformación para el resto
    painter.resetTransform();
    painter.translate(width() / 2.0, height() / 2.0);

    // Dibujar marco de la zona jugable (mundo 2D → pantalla isométrica)
    if (!m_playArea.isNull()) {
        // Esquinas del rectángulo en coordenadas de mundo
        QPointF tl = m_playArea.topLeft();
        QPointF tr = m_playArea.topRight();
        QPointF br = m_playArea.bottomRight();
        QPointF bl = m_playArea.bottomLeft();

        // Proyección isométrica de las 4 esquinas
        QVector<QPointF> pts;
        pts << ProyeccionIso::toScreen(tl)
            << ProyeccionIso::toScreen(tr)
            << ProyeccionIso::toScreen(br)
            << ProyeccionIso::toScreen(bl);

        painter.save();
        painter.setPen(QPen(Qt::white, 2));
        painter.setBrush(Qt::NoBrush);
        painter.drawPolygon(QPolygonF(pts)); // marco blanco del área jugable
        painter.restore();
    }

    // Dibujar barco (rectángulo centrado en la posición proyectada del barco)
    // Barco FIJO en posición de mundo, no se mueve en X
    // Parpadeo cuando está invulnerable
    int bar_profundidad = 40;
    int bar_ancho = 20;
    int bar_posicionx = -bar_profundidad/2;
    int bar_posiciony = -bar_ancho/2;

    QPointF barcoWorld = m_barco.position();
    QPointF barcoScreen = ProyeccionIso::toScreen(barcoWorld);

    // Solo dibujar barco si no está en frame de parpadeo
    bool dibujarBarco = true;
    if (m_invulnerable) {
        // Parpadear cada 5 frames
        dibujarBarco = (m_contadorInvulnerabilidad % 10 < 5);
    }

    if (dibujarBarco) {
        painter.save();
        painter.translate(barcoScreen);
        painter.setBrush(Qt::yellow);
        painter.setPen(Qt::black);
        painter.drawRect(bar_posicionx, bar_posiciony, bar_profundidad, bar_ancho);
        painter.restore();
    }

    // Dibujar obstáculos (mismo patrón que el barco)
    for (const Obstaculon2 &o : m_obstaculos) {
        QPointF oWorld = o.position();
        QPointF oScreen = ProyeccionIso::toScreen(oWorld);

        int obs_profundidad = 40;
        int obs_ancho = 20;
        int obs_posicionx = -obs_profundidad/2;
        int obs_posiciony = -obs_ancho/2;

        painter.save();
        painter.translate(oScreen);
        painter.setBrush(Qt::gray);
        painter.setPen(Qt::black);
        painter.drawRect(obs_posicionx, obs_posiciony, obs_profundidad, obs_ancho);
        painter.restore();
    }

    // Dibujar hitboxes (verde si no colisiona, rojo si está en colisión)
    drawHitbox(painter, m_barco.hitbox(), m_barco.position());

    for (const Obstaculon2 &o : m_obstaculos) {
        drawHitbox(painter, o.hitbox(), o.position());
    }
}

// Dibujar indicador de vidas
void NivelIso::dibujarVidas(QPainter &painter)
{
    // Dibujar corazones en la esquina superior izquierda
    int x = 120;  // Posición X inicial (debajo del botón Volver)
    int y = 15;   // Posición Y
    int size = 20; // Tamaño de cada corazón
    int spacing = 25; // Espacio entre corazones

    painter.save();

    for (int i = 0; i < m_vidasMaximas; i++) {
        QRect corazonRect(x + i * spacing, y, size, size);

        if (i < m_vidas) {
            // Corazón lleno (vida disponible)
            painter.setBrush(Qt::red);
            painter.setPen(Qt::darkRed);
        } else {
            // Corazón vacío (vida perdida)
            painter.setBrush(Qt::darkGray);
            painter.setPen(Qt::gray);
        }

        // Dibujar corazón simple (rombo/rectángulo)
        painter.drawEllipse(corazonRect);
    }

    // Dibujar texto de vidas
    painter.setPen(Qt::white);
    QFont font = painter.font();
    font.setPointSize(12);
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(x + m_vidasMaximas * spacing + 10, y + 15,
                     QString("x %1").arg(m_vidas));

    painter.restore();
}

// Dibujar tiempo restante
void NivelIso::dibujarTiempo(QPainter &painter)
{
    painter.save();

    // Posición en la esquina superior derecha
    int x = width() - 220;
    int y = 20;

    // Calcular tiempo restante en segundos
    int tiempoRestanteFrames = m_tiempoParaGanar - m_tiempoTranscurrido;
    int segundosRestantes = tiempoRestanteFrames / 60;
    int decimas = (tiempoRestanteFrames % 60) * 10 / 60;

    // Configurar fuente
    QFont font = painter.font();
    font.setPointSize(16);
    font.setBold(true);
    painter.setFont(font);

    // Texto del tiempo
    QString texto = QString("Tiempo: %1.%2s")
                        .arg(segundosRestantes, 2, 10, QChar('0'))
                        .arg(decimas);

    QRect textRect = painter.fontMetrics().boundingRect(texto);
    textRect.adjust(-15, -8, 15, 8);
    textRect.moveTo(x - 15, y - 8);

    // Fondo semi-transparente
    painter.setBrush(QColor(0, 0, 0, 180));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(textRect, 5, 5);

    // Color del texto según tiempo restante
    QColor colorTexto;
    if (segundosRestantes <= 5) {
        colorTexto = Qt::red;  // Últimos 5 segundos en rojo
    } else if (segundosRestantes <= 10) {
        colorTexto = Qt::yellow;  // 5-10 segundos en amarillo
    } else {
        colorTexto = Qt::green;  // Más de 10 segundos en verde
    }

    painter.setPen(colorTexto);
    painter.drawText(x, y + 12, texto);

    // Barra de progreso del tiempo
    int barWidth = 180;
    int barHeight = 8;
    int barX = x;
    int barY = y + 25;

    // Fondo de la barra
    painter.setBrush(Qt::darkGray);
    painter.setPen(QPen(Qt::white, 1));
    painter.drawRect(barX, barY, barWidth, barHeight);

    // Progreso de la barra (se llena con el tiempo)
    qreal progreso = qMin(1.0, static_cast<qreal>(m_tiempoTranscurrido) / m_tiempoParaGanar);
    int progressWidth = static_cast<int>(barWidth * progreso);

    // Color de la barra según progreso
    QColor colorBarra;
    if (progreso < 0.5) {
        colorBarra = Qt::red;
    } else if (progreso < 0.75) {
        colorBarra = Qt::yellow;
    } else {
        colorBarra = Qt::green;
    }

    painter.setBrush(colorBarra);
    painter.setPen(Qt::NoPen);
    painter.drawRect(barX, barY, progressWidth, barHeight);

    // Indicador de sprint
    if (m_sprint) {
        painter.setPen(Qt::cyan);
        QFont sprintFont = painter.font();
        sprintFont.setPointSize(12);
        sprintFont.setBold(true);
        painter.setFont(sprintFont);
        painter.drawText(x + barWidth + 10, y + 15, "⚡SPRINT");
    }

    painter.restore();
}

// Reiniciar el nivel
void NivelIso::reiniciarNivel()
{
    // Detener el timer temporalmente
    m_timer->stop();

    // Reiniciar todas las variables
    m_vidas = m_vidasMaximas;
    m_invulnerable = false;
    m_contadorInvulnerabilidad = 0;
    m_scrollOffset = 0.0;
    m_scrollSpeed = m_scrollSpeedBase;  // Volver a velocidad base
    m_sprint = false;                   // Desactivar sprint
    m_contadorFrames = 0;
    m_tiempoTranscurrido = 0;           // Reiniciar tiempo
    m_nivelCompletado = false;

    // Reconfigurar escena (barco, obstáculos, etc.)
    initScene();

    // Reiniciar timer
    m_timer->start(16);
}

// Mostrar mensaje de victoria
void NivelIso::mostrarVictoria()
{
    // Detener el timer
    m_timer->stop();

    // Calcular tiempo final en segundos
    qreal segundosTotales = m_tiempoTranscurrido / 60.0;

    // Crear mensaje de victoria
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("¡Victoria!");
    msgBox.setText("¡Ganaste!");
    msgBox.setInformativeText(QString("¡Felicitaciones! Has sobrevivido 20 segundos.\n\n"
                                      "Tiempo final: 20. segundos\n"
                                      "Vidas restantes: %1\n\n"
                                      "¿Qué deseas hacer?")
                                  .arg(segundosTotales, 0, 'f', 1)
                                  .arg(m_vidas)
                                  .arg(m_vidasMaximas));

    QPushButton *btnReiniciar = msgBox.addButton("Reiniciar Nivel", QMessageBox::ActionRole);
    QPushButton *btnMenu = msgBox.addButton("Volver al Menú", QMessageBox::ActionRole);

    msgBox.setDefaultButton(btnReiniciar);
    msgBox.exec();

    if (msgBox.clickedButton() == btnReiniciar) {
        // Reiniciar el nivel
        reiniciarNivel();
    } else if (msgBox.clickedButton() == btnMenu) {
        // Volver al menú principal
        emit volverAlMenu();
    }
}

// Dibujar fondo con efecto de scrolling
void NivelIso::dibujarFondoScrolling(QPainter &painter)
{
    // Fondo base (mar nocturno)
    painter.fillRect(rect(), Qt::darkBlue);

    // Dibujar líneas de grid que se mueven
    painter.setPen(QColor(40, 60, 120, 100));  // Azul semitransparente

    int gridSize = 50;
    int offsetY = static_cast<int>(m_scrollOffset) % gridSize;

    // Líneas horizontales que se mueven hacia abajo
    for (int y = -gridSize + offsetY; y < height() + gridSize; y += gridSize) {
        painter.drawLine(0, y, width(), y);
    }

    // Líneas verticales (fijas)
    for (int x = 0; x < width(); x += gridSize) {
        painter.drawLine(x, 0, x, height());
    }
}

void NivelIso::keyPressEvent(QKeyEvent *event)
{
    // Movimiento lateral + sprint
    switch (event->key()) {
    case Qt::Key_A:
    case Qt::Key_Left:
        m_moveLeft = true;
        break;
    case Qt::Key_D:
    case Qt::Key_Right:
        m_moveRight = true;
        break;
    case Qt::Key_Shift:  // Tecla Shift para sprint
    case Qt::Key_W:      // También W para sprint
    case Qt::Key_Up:     // También flecha arriba
        m_sprint = true;
        m_scrollSpeed = m_scrollSpeedSprint;  // Aumentar velocidad
        break;
    default:
        QWidget::keyPressEvent(event);
        break;
    }
}

void NivelIso::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_A:
    case Qt::Key_Left:
        m_moveLeft = false;
        break;
    case Qt::Key_D:
    case Qt::Key_Right:
        m_moveRight = false;
        break;
    case Qt::Key_Shift:  // Soltar sprint
    case Qt::Key_W:
    case Qt::Key_Up:
        m_sprint = false;
        m_scrollSpeed = m_scrollSpeedBase;  // Volver a velocidad normal
        break;
    default:
        QWidget::keyReleaseEvent(event);
        break;
    }
}

void NivelIso::updateGame()
{
    //  No actualizar si el nivel está completado
    if (m_nivelCompletado) {
        return;
    }

    // Incrementar tiempo transcurrido
    m_tiempoTranscurrido++;

    // Verificar victoria por tiempo
    if (m_tiempoTranscurrido >= m_tiempoParaGanar) {
        m_nivelCompletado = true;
        mostrarVictoria();
        return;
    }

    // 1) Actualizar posición del barco (solo movimiento lateral)
    updateBarcoFromInput();

    // 2) Actualizar invulnerabilidad
    if (m_invulnerable) {
        m_contadorInvulnerabilidad--;
        if (m_contadorInvulnerabilidad <= 0) {
            m_invulnerable = false;
        }
    }

    // 3) Actualizar posición de obstáculos (scrolling automático)
    updateObstaculos();

    // 4) Generar nuevos obstáculos periódicamente
    generarNuevosObstaculos();

    // Limite de stamina
    if (m_sprint) {
        m_stamina--;
        if (m_stamina <= 0) {
            m_sprint = false;
            m_scrollSpeed = m_scrollSpeedBase;
        }
    } else {
        m_stamina = qMin(m_stamina + 2, m_staminaMax);  // Recuperar
    }

    // 5) Detectar colisiones
    updateCollisions();

    // 6) Actualizar offset del fondo
    m_scrollOffset += m_scrollSpeed;

    // 7) Pedir repintado → desencadena una nueva llamada a paintEvent()
    update();
}

void NivelIso::updateBarcoFromInput()
{
    // Solo movimiento lateral (en el eje Y)
    const qreal speed = 3.0;

    qreal dirY = 0.0;

    // Solo movimiento lateral
    if (m_moveLeft)
        dirY -= 1.0;  // izquierda

    if (m_moveRight)
        dirY += 1.0;  // derecha

    // Guardar posición actual
    QPointF posicionActual = m_barco.position();

    // Calcular nueva posición (solo cambia Y)
    QPointF posicionNueva = posicionActual;
    posicionNueva.setY(posicionNueva.y() + dirY * speed);

    // Limitar al área jugable (solo en Y)
    if (!m_playArea.isNull()) {
        if (posicionNueva.y() < m_playArea.top())
            posicionNueva.setY(m_playArea.top());
        if (posicionNueva.y() > m_playArea.bottom())
            posicionNueva.setY(m_playArea.bottom());
    }

    // Aplicar posición temporalmente para verificar colisiones
    m_barco.setPosition(posicionNueva);

    // Verificar si hay colisión con algún obstáculo
    bool hayColision = false;
    for (const Obstaculon2 &o : m_obstaculos) {
        if (m_barco.hitbox().intersects(o.hitbox(),
                                        m_barco.position(),
                                        o.position())) {
            hayColision = true;
            break;
        }
    }

    // Si hay colisión, revertir el movimiento
    if (hayColision) {
        m_barco.setPosition(posicionActual);
    }
}

// Actualizar obstáculos (moverlos hacia el barco)
void NivelIso::updateObstaculos()
{
    // Mover todos los obstáculos hacia el barco (disminuir X)
    for (int i = m_obstaculos.size() - 1; i >= 0; --i) {
        QPointF pos = m_obstaculos[i].position();
        pos.setX(pos.x() - m_scrollSpeed);  // Mover hacia el jugador
        m_obstaculos[i].setPosition(pos);

        // Eliminar obstáculos que ya pasaron al barco
        if (pos.x() < m_limiteEliminacion) {
            m_obstaculos.removeAt(i);
        }
    }
}

// Genera nuevos obstáculos
void NivelIso::generarNuevosObstaculos()
{
    m_contadorFrames++;

    // Generar un nuevo obstáculo cada 60 frames (1 segundo a 60 FPS)
    if (m_contadorFrames >= 60) {
        m_contadorFrames = 0;

        // Generar 1-2 obstáculos aleatorios
        int cantidad = QRandomGenerator::global()->bounded(1, 3);

        for (int i = 0; i < cantidad; i++) {
            Obstaculon2 o;

            // Posición X: lejos (donde aparecen)
            qreal x = m_limiteGeneracion;

            // Posición Y: aleatoria dentro del área jugable
            // Usar bounded con enteros y convertir a qreal
            int minY = static_cast<int>(m_playArea.top() + 20.0);
            int maxY = static_cast<int>(m_playArea.bottom() - 20.0);
            qreal y = static_cast<qreal>(QRandomGenerator::global()->bounded(minY, maxY));

            o.setPosition(QPointF(x, y));
            m_obstaculos.append(o);
        }
    }
}

void NivelIso::updateCollisions()
{
    // Resetear estado de colisión en todas las hitboxes
    m_barco.hitbox().setColliding(false);
    for (Obstaculon2 &o : m_obstaculos) {
        o.hitbox().setColliding(false);
    }

    // Solo detectar colisiones si no está invulnerable
    if (!m_invulnerable) {
        // Revisar barco vs cada obstáculo (pensado en 2D)
        for (Obstaculon2 &o : m_obstaculos) {
            bool col = m_barco.hitbox().intersects(
                o.hitbox(),
                m_barco.position(),
                o.position());

            if (col) {
                m_barco.hitbox().setColliding(true);
                o.hitbox().setColliding(true);

                // Perder una vida
                m_vidas--;

                // sebas: Activar invulnerabilidad temporal (2 segundos = 120 frames a 60 FPS)
                m_invulnerable = true;
                m_contadorInvulnerabilidad = 120;

                // sebas: Verificar Game Over
                if (m_vidas <= 0) {
                    reiniciarNivel();
                }

                // Solo procesar una colisión por frame
                break;
            }
        }
    } else {
        // Aunque esté invulnerable, marcar colisiones para debug visual
        for (Obstaculon2 &o : m_obstaculos) {
            bool col = m_barco.hitbox().intersects(
                o.hitbox(),
                m_barco.position(),
                o.position());

            if (col) {
                m_barco.hitbox().setColliding(true);
                o.hitbox().setColliding(true);
            }
        }
    }
}

void NivelIso::drawHitbox(QPainter &painter,
                          const Hitbox &hitbox,
                          const QPointF &worldPos)
{
    // Coordenadas de la hitbox en el mundo 2D (antes de proyectar)
    QVector<QPointF> worldPoints = hitbox.worldPoints(worldPos);

    if (worldPoints.isEmpty())
        return;

    // Convertir cada punto de la hitbox a coordenadas de pantalla isométrica
    QVector<QPointF> screenPoints;
    screenPoints.reserve(worldPoints.size());

    for (const QPointF &p : worldPoints) {
        screenPoints.append(ProyeccionIso::toScreen(p));
    }

    painter.save();
    // Verde si no colisiona, rojo si la hitbox está marcada en colisión
    painter.setPen(hitbox.isColliding() ? Qt::red : Qt::green);
    painter.setBrush(Qt::NoBrush);

    // Dibujar el polígono de la hitbox proyectada
    QPolygonF poly(screenPoints);
    painter.drawPolygon(poly);

    painter.restore();
}
