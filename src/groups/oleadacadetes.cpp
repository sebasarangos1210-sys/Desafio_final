#include "oleadacadetes.h"
#include "nivel.h"
#include "cadete.h"
#include <algorithm>
#include <QRandomGenerator>
#include "vector2d.h"
#include <QtMath>

OleadaCadetes::OleadaCadetes(Nivel *nivelPtr)
    : Agente(nivelPtr),
    enemigosRestantes(0)
{
}

OleadaCadetes::~OleadaCadetes()
{
}

void OleadaCadetes::inicializarRondas(int total)
{
    totalRondas = total;
    rondaActual = 1;
}

void OleadaCadetes::spawnRonda(int cantidad,
                               qreal radioMin, qreal radioMax,
                               qreal angMinRad, qreal angMaxRad)
{
    if (cantidad <= 0) return;
    if (radioMax <= radioMin) return;
    if (angMaxRad <= angMinRad) return;

    // El jugador está en (0,0) en coords de escena
    Vector2D centro(0.0, 0.0);

    auto *fondo  = nivel->getFondoScroll();
    auto *escena = nivel->getEscena();

    const qreal deltaAng = (angMaxRad - angMinRad) / cantidad;

    for (int i = 0; i < cantidad; ++i) {

        // 1) Ángulo estratificado
        qreal angBase = angMinRad + i * deltaAng;
        qreal uAng    = QRandomGenerator::global()->generateDouble(); // [0,1)
        qreal angulo  = angBase + uAng * deltaAng;

        // 2) Radio sesgado hacia radioMax
        qreal uRad = QRandomGenerator::global()->generateDouble();    // [0,1)
        qreal r    = radioMin + (radioMax - radioMin) * (qPow(uRad, 1.5));

        // 3) Posición en coordenadas de escena
        Vector2D offset   = Vector2D::desdePolar(r, angulo);
        Vector2D posScene = centro + offset;

        // Convertir escena → sistema local del fondo
        QPointF posEnFondo = fondo->mapFromScene(posScene.toPointF());

        // 4) Crear cadete
        Cadete *e = new Cadete(10.0, 0.0, 0.0, /*esjug=*/false);
        e->setParentItem(fondo);
        e->setPos(posEnFondo);

        escena->addItem(e);

        grupo.push_back(e);
        nivel->registrarEnemigo(e);  // <- importante: ahora también dispara y lo limpia el Nivel
    }
}

void OleadaCadetes::actualizar()
{
    // paso base
    const qreal step = 2.0;

    // radio mínimo de cercanía al jugador (en escena)
    const qreal minDistJugador = 25.0;
    const qreal minDistJugador2 = minDistJugador * minDistJugador;

    // radio de "espacio personal" entre cadetes (en escena)
    const qreal radioSep = 40.0;
    const qreal radioSep2 = radioSep * radioSep;

    for (auto *e : grupo) {
        if (!e || e->muerto) continue;

        // ----- POSICIONES -----
        // local al fondo (para setPos)
        QPointF pLocalQ = e->pos();
        Vector2D posLocal(pLocalQ.x(), pLocalQ.y());

        // en escena (jugador en 0,0)
        Vector2D posScene(e->scenePos());

        // ----- DIRECCIÓN BASE HACIA EL JUGADOR -----
        Vector2D dirSeek = e->getDireccion();      // tú ya la calculas hacia el jugador
        if (dirSeek.magnitud2() == 0.0) continue;
        dirSeek = dirSeek.normalizado();

        // Si estamos demasiado cerca del jugador, no avanzamos más
        if (posScene.magnitud2() <= minDistJugador2)
            continue;

        // ----- SEPARACIÓN ENTRE CADETES (espacio personal) -----
        Vector2D dirSeparation = Vector2D::nulo();

        for (auto *otro : grupo) {
            if (!otro || otro == e || otro->muerto) continue;

            Vector2D posOtherScene(otro->scenePos());
            Vector2D diff = posScene - posOtherScene;
            qreal dist2 = diff.magnitud2();

            if (dist2 < 1e-3) {
                // Están prácticamente en el mismo punto.
                // Empujamos al cadete lateralmente (perpendicular a la dirección hacia el jugador)
                Vector2D perp(-dirSeek.y(), dirSeek.x()); // perpendicular a dirSeek
                if (perp.magnitud2() > 0.0) {
                    perp = perp.normalizado();
                    dirSeparation += perp * 5.0;  // empuje fuerte para separarlos
                }
            }
            else if (dist2 < radioSep2) {
                // Caso normal: separación suave inversa a la distancia
                Vector2D push = diff / dist2;
                dirSeparation += push;
            }

        }

        // ----- COMBINAR: ir al jugador + separar amigos -----
        // Peso 0.5 para separación (ajustable)
        Vector2D dirFinal = dirSeek + dirSeparation * 1.0;
        if (dirFinal.magnitud2() == 0.0)
            dirFinal = dirSeek;

        dirFinal = dirFinal.normalizado();

        // Propuesta de nueva posición local
        Vector2D newLocal = posLocal + dirFinal * step;

        // Nos movemos provisionalmente para chequear colisiones
        e->setPos(newLocal.x(), newLocal.y());

        QList<QGraphicsItem*> cols = e->collidingItems();
        bool bloqueado = false;

        for (QGraphicsItem *item : cols) {

            // --- Obstáculo físico: usamos tu react_colision ---
            if (auto *obs = dynamic_cast<Obstaculo*>(item)) {
                bloqueado = true;

                // Volver a la posición anterior
                e->setPos(pLocalQ);

                // Calcular nueva posición "deslizándose" alrededor
                Vector2D react = react_colision(obs, e, step);
                e->setPos(react.x(), react.y());
                break; // con un obstáculo fuerte es suficiente para este frame
            }

            // --- Otro FuerzaArmada (jugador u otro cadete) ---
            if (auto *fa = dynamic_cast<FuerzaArmada*>(item)) {
                if (fa == e) continue; // ignorar self

                // Por ahora: simplemente no empujamos, dejamos
                // que la fuerza de separación haga el trabajo.
                bloqueado = true;
                e->setPos(pLocalQ);
                break;
            }
        }


        // Si no hubo bloqueos, nos quedamos con newLocal (ya está puesto con setPos)
        if (!bloqueado) {
            // nada que hacer: e ya está en newLocal
        }
    }
}

Vector2D OleadaCadetes::react_colision(Obstaculo* obs, FuerzaArmada* cadet, qreal step){


    // pos local (respecto al fondo)
    Vector2D pos_act(cadet->pos().x(), cadet->pos().y());

    Vector2D dir_act = cadet->getDireccion();
    if (dir_act.magnitud2() == 0.0) return pos_act;
    Vector2D dir_normal_obs(obs->scenePos());
    dir_normal_obs = dir_normal_obs.normalizado().getNormal();

    qreal dot = dir_act.dot(dir_normal_obs);
    qDebug() << "dir cadete : x " << dir_act.x() << " y " << dir_act.y();
    qDebug() << "dir normal obs : x " << dir_normal_obs.x() << " y " << dir_normal_obs.y();
    qDebug() << "dot : " << dot;

    Vector2D new_pos = pos_act;

    dir_act = dir_act.normalizado();
    if(dot >= 0)
        new_pos = pos_act + (dir_act - dir_normal_obs).normalizado()*step;
    else
        new_pos = pos_act + (dir_act + dir_normal_obs).normalizado()*step;

    return new_pos;

}

bool OleadaCadetes::rondaCompletada() const
{
    return false; // placeholder
}

bool OleadaCadetes::juegoCompletado() const
{
    return false; // placeholder
}
