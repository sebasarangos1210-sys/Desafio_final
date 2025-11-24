#ifndef NIVELISO_H
#define NIVELISO_H

#include <QWidget>
#include <QVector>
#include <QPointF>
#include "barco.h"
#include "obstaculon2.h"
#include "obstaculo.h"
#include "torpedo.h"

class QTimer;

class NivelIso : public QWidget
{
    Q_OBJECT

public:
    explicit NivelIso(QWidget *parent = nullptr);
    ~NivelIso() override;

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private slots:
    void updateGame();

signals:
    void volverAlMenu();

private:
    void initScene();
    void updateBarcoFromInput();
    void updateCollisions();
    void drawHitbox(QPainter &painter,
                    const Hitbox &hitbox,
                    const QPointF &worldPos);

    // Funciones para el sistema de scrolling
    void updateObstaculos();      // Mueve los obstáculos hacia el barco
    void generarNuevosObstaculos(); // Genera obstáculos nuevos
    void dibujarFondoScrolling(QPainter &painter); // Fondo en movimiento
    void dibujarVidas(QPainter &painter); // Dibujar indicador de vidas
    void dibujarTiempo(QPainter &painter); // Dibujar tiempo restante
    void reiniciarNivel();        // Reiniciar el nivel
    void mostrarVictoria();       // Mostrar mensaje de victoria

    // NUEVO: Funciones para torpedos
    void dispararTorpedo();       // Crear y disparar un torpedo
    void updateTorpedos();        // Actualizar posición de torpedos
    void verificarColisionesTorpedos(); // Verificar si torpedos golpean obstáculos
    void updateDificultad();      // Ajustar dificultad según tiempo

    Barco m_barco;
    QVector<Obstaculon2> m_obstaculos;
    QVector<Torpedo> m_torpedos;  // Lista de torpedos activos
    QTimer *m_timer;

    // Input simple (solo movimiento lateral ahora)
    bool m_moveLeft;
    bool m_moveRight;
    bool m_sprint;  //Estado del sprint

    QRectF m_playArea;

    // Variables para el scrolling automático
    qreal m_scrollOffset;         // Desplazamiento acumulado del fondo
    qreal m_scrollSpeed;          // Velocidad del scrolling
    qreal m_scrollSpeedBase;      // Velocidad base (sin sprint)
    qreal m_scrollSpeedSprint;    // Velocidad con sprint
    qreal m_limiteEliminacion;    // Posición Y donde se eliminan obstáculos
    qreal m_limiteGeneracion;     // Posición Y donde aparecen obstáculos
    int m_contadorFrames;         // Contador para generar obstáculos periódicamente

    // Sistema de vidas
    int m_vidas;                  // Vidas actuales del jugador
    int m_vidasMaximas;           // Vidas máximas
    bool m_invulnerable;          // Estado de invulnerabilidad temporal
    int m_contadorInvulnerabilidad; // Frames restantes de invulnerabilidad

    // Sistema de tiempo y victoria
    int m_tiempoTranscurrido;     // Tiempo en frames (60 frames = 1 segundo)
    int m_tiempoParaGanar;        // Meta en frames (1200 frames = 20 segundos)
    bool m_nivelCompletado;       // Flag de victoria

    // Sistema de disparo
    int m_cooldownDisparo;        // Frames de espera entre disparos
    int m_cooldownActual;         // Contador actual de cooldown

    // Dificultad progresiva
    int m_frecuenciaGeneracion;   // Frames entre generación de obstáculos
    int m_cantidadObstaculos;     // Cantidad de obstáculos a generar
};

#endif // NIVELISO_H
