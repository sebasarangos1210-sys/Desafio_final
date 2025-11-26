#ifndef NIVELISO_H
#define NIVELISO_H

#include <QWidget>
#include <QVector>
#include <QPointF>
#include <QSoundEffect>
#include "barco.h"
#include "obstaculon2.h"
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
    void updateObstaculos();
    void generarNuevosObstaculos();
    void dibujarFondoScrolling(QPainter &painter);
    void dibujarVidas(QPainter &painter);
    void dibujarTiempo(QPainter &painter);
    void dibujarMunicion(QPainter &painter);
    void reiniciarNivel();
    void mostrarVictoria();
    void mostrarGameOver();

    // Funciones para torpedos
    void dispararTorpedo();
    void updateTorpedos();
    void verificarColisionesTorpedos();
    void updateDificultad();

    Barco m_barco;
    QVector<Obstaculon2> m_obstaculos;
    QVector<Torpedo> m_torpedos;
    QTimer *m_timer;

    // Input
    bool m_moveLeft;
    bool m_moveRight;
    bool m_sprint;

    QRectF m_playArea;

    // Variables para scrolling
    qreal m_scrollOffset;
    qreal m_scrollSpeed;
    qreal m_scrollSpeedBase;
    qreal m_scrollSpeedSprint;
    qreal m_limiteEliminacion;
    qreal m_limiteGeneracion;
    int m_contadorFrames;

    // Sistema de vidas
    int m_vidas;
    int m_vidasMaximas;
    bool m_invulnerable;
    int m_contadorInvulnerabilidad;

    // Sistema de tiempo y victoria
    int m_tiempoTranscurrido;
    int m_tiempoParaGanar;
    bool m_nivelCompletado;

    // Sistema de disparo
    int m_cooldownDisparo;
    int m_cooldownActual;

    // Sistema de munición
    int m_municionActual;
    int m_municionMaxima;
    int m_contadorRecarga;
    int m_tiempoRecarga;

    // Dificultad progresiva
    int m_frecuenciaGeneracion;
    int m_cantidadObstaculos;

    // Sistema de sonido
    QSoundEffect *m_sonidoDisparo;
    QSoundEffect *m_sonidoExplosion;  // Cuando torpedo destruye obstáculo

    void cargarSonidosDesdeArchivos();
    void cargarSonidos();
};

#endif // NIVELISO_H
