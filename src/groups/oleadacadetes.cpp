#include "oleadacadetes.h"
#include "nivel.h"
#include "cadete.h"
#include <algorithm>
#include <QRandomGenerator>
#include "vector2d.h"
#include <QtMath>

OleadaCadetes::OleadaCadetes(Nivel *nivelPtr)
    : Agente(nivelPtr)
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

    enemigosRestantes = cantidad;
    enemigosTotales = cantidad;

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

    enemigosRestantes = enemigosTotales;
    for(auto *a : grupo){
        if(!a) continue;

        if(a->muerto){
            enemigosRestantes--;
        }
    }

    if (enemigosRestantes == 0) {
        activo = false;
        estado = EstadoGrupo::Muerto;
        return;
    }

    // Si el grupo ya está marcado como muerto, no hacemos nada
    if (estado == EstadoGrupo::Muerto)
        return;

    // Más adelante, si quieres, puedes también comprobar aquí
    // si todos los cadetes de `grupo` están muertos y poner:
    // estado = EstadoGrupo::Muerto;  (pero eso lo dejamos para luego)

    switch (modo)
    {
    case ModoGrupo::AtaqueDirecto:

        actualizarAtaqueDirecto();
        break;

    case ModoGrupo::Flanqueo:
        actualizarFlanqueo();
        break;

    case ModoGrupo::Campamento:
        // MODO: CAMPAMENTO
        //
        //   - if (estado == EstadoGrupo::EnPosicion):
        //         * el grupo se queda quieto o patrulla un poquito
        //         * si el jugador entra en radioActivacion -> estado = Atacando
        //
        //   - if (estado == EstadoGrupo::Atacando):
        //         * usar la lógica de persecución estándar hacia el jugador
        break;

    case ModoGrupo::Emboscada:
        actualizarEmboscada();
        break;

    default:
        // Por si en el futuro aparece algún modo no manejado,
        // de momento no hacemos nada.
        break;
    }

    // Más adelante, aquí podría ir algún chequeo común:
    //   - contar cuántos cadetes siguen vivos en `grupo`
    //   - si llega a 0 -> estado = EstadoGrupo::Muerto;

}

void OleadaCadetes::actualizarAtaqueDirecto()
{
    // Por ahora, solo queremos movernos cuando estamos atacando.
    // Si más tarde quieres usar "Preparando" como ataque directo también,
    // puedes permitir ambos estados.
    if (estado != EstadoGrupo::Atacando &&
        estado != EstadoGrupo::Preparando) {
        return;
    }

    // paso base
    const qreal step = 2.0;

    // radio mínimo de cercanía al jugador (en escena)
    const qreal minDistJugador  = 25.0;
    const qreal minDistJugador2 = minDistJugador * minDistJugador;

    // radio de "espacio personal" entre cadetes (en escena)
    const qreal radioSep  = 40.0;
    const qreal radioSep2 = radioSep * radioSep;

    for (auto *e : grupo) {
        if (!e || e->muerto) continue;

        // ----- POSICIONES -----
        // local al fondo (para setPos)
        QPointF  pLocalQ  = e->pos();
        Vector2D posLocal(pLocalQ.x(), pLocalQ.y());

        // en escena (jugador en 0,0)
        Vector2D posScene(e->scenePos());

        // ----- DIRECCIÓN BASE HACIA EL JUGADOR -----
        Vector2D dirSeek = e->getDireccion();      // ya la calculas hacia el jugador en Nivel
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

        // Si no hubo bloqueos, e ya está en newLocal y no hay nada más que hacer
    }

    // Más adelante: aquí puedes revisar si todos los miembros del grupo
    // están muertos y poner estado = EstadoGrupo::Muerto;
}

void OleadaCadetes::actualizarFlanqueo()
{
    // Si no tengo slots de flanqueo definidos, caigo al comportamiento estándar
    if (puntosObjetivo.empty()) {
        actualizarAtaqueDirecto();
        return;
    }

    // El jugador está en (0,0) en coords de escena (según tu diseño)
    const Vector2D posJugadorScene(0.0, 0.0);

    // Paso de movimiento cuando están preparando
    const qreal stepPrep = 3.0;

    // Radio para considerar que un cadete "llegó" a su punto de flanqueo
    const qreal radioLlegada        = 20.0;
    const qreal radioLlegada2       = radioLlegada * radioLlegada;

    // Radio de activación del ataque (distancia jugador–centro del grupo)
    const qreal radioAct = (radioActivacion > 0.0 ? radioActivacion : 220.0);
    const qreal radioAct2 = radioAct * radioAct;

    switch (estado)
    {
    case EstadoGrupo::Preparando:
    {
        int enPosicion = 0;
        int totalVivos = 0;

        // Para cada cadete, lo mandamos a su slot de flanqueo
        for (std::size_t i = 0; i < grupo.size(); ++i) {
            FuerzaArmada *e = grupo[i];
            if (!e || e->muerto) continue;
            ++totalVivos;

            // Posiciones
            QPointF  pLocalQ  = e->pos();
            Vector2D posLocal(pLocalQ.x(), pLocalQ.y());
            Vector2D posScene(e->scenePos());

            // Slot de flanqueo en coordenadas de escena
            // (si hay menos slots que soldados, usamos módulo)
            const Vector2D &slotScene = puntosObjetivo[i % puntosObjetivo.size()];

            // Vector hacia el slot
            Vector2D toSlot = slotScene - posScene;
            qreal dist2 = toSlot.magnitud2();

            // ¿Ya está lo suficientemente cerca de su slot?
            if (dist2 <= radioLlegada2) {
                ++enPosicion;

                // Solo orientamos hacia el jugador
                Vector2D dirToPlayer = posJugadorScene - posScene;
                if (dirToPlayer.magnitud2() > 0.0)
                    e->setDireccion(dirToPlayer.normalizado());

                continue;
            }

            // Dirección principal hacia el slot
            Vector2D dir = toSlot.normalizado();

            // --- Separación entre cadetes, igual que en AtaqueDirecto ---
            const qreal radioSep  = 40.0;
            const qreal radioSep2 = radioSep * radioSep;

            Vector2D dirSeparation = Vector2D::nulo();

            for (auto *otro : grupo) {
                if (!otro || otro == e || otro->muerto) continue;

                Vector2D posOtherScene(otro->scenePos());
                Vector2D diff = posScene - posOtherScene;
                qreal d2 = diff.magnitud2();

                if (d2 < 1e-3) {
                    // Muy pegados: empuje perpendicular a la dirección hacia el slot
                    Vector2D perp(-dir.y(), dir.x());
                    if (perp.magnitud2() > 0.0) {
                        perp = perp.normalizado();
                        dirSeparation += perp * 5.0;
                    }
                }
                else if (d2 < radioSep2) {
                    dirSeparation += diff / d2;
                }
            }

            // Combinar ir al slot + separación
            Vector2D dirFinal = dir + dirSeparation;
            if (dirFinal.magnitud2() == 0.0)
                dirFinal = dir;

            dirFinal = dirFinal.normalizado();

            // Propuesta nueva posición en coords locales (fondo)
            Vector2D newLocal = posLocal + dirFinal * stepPrep;
            e->setPos(newLocal.x(), newLocal.y());

            // Colisiones con obstáculos (igual que en AtaqueDirecto)
            QList<QGraphicsItem*> cols = e->collidingItems();
            bool bloqueado = false;

            for (QGraphicsItem *item : cols) {
                if (auto *obs = dynamic_cast<Obstaculo*>(item)) {
                    bloqueado = true;

                    // Volver a la posición anterior
                    e->setPos(pLocalQ);

                    // Reacción de “deslizamiento”
                    Vector2D react = react_colision(obs, e, stepPrep);
                    e->setPos(react.x(), react.y());
                    break;
                }
            }

            // Actualizar dirección visual mirando al jugador
            Vector2D dirToPlayer = posJugadorScene - Vector2D(e->scenePos());
            if (dirToPlayer.magnitud2() > 0.0)
                e->setDireccion(dirToPlayer.normalizado());
        }

        // ¿Suficientes cadetes ya en posición? -> cambiar a EnPosicion
        if (totalVivos > 0) {
            qreal ratio = static_cast<qreal>(enPosicion) /
                          static_cast<qreal>(totalVivos);
            if (ratio >= 0.7) {    // 70% ya llegó a su slot
                estado = EstadoGrupo::EnPosicion;
            }
        }
        break;
    }

    case EstadoGrupo::EnPosicion:
    {
        // Grupo quieto, solo mirando al jugador.
        // Cuando el jugador se acerca al centro del grupo -> empiezan a atacar.

        Vector2D centro = calcularCentroGrupoScene();
        Vector2D diff   = posJugadorScene - centro;

        if (diff.magnitud2() <= radioAct2) {
            estado = EstadoGrupo::Atacando;
        }

        // Orientar a todos hacia el jugador
        for (auto *e : grupo) {
            if (!e || e->muerto) continue;

            Vector2D posScene(e->scenePos());
            Vector2D dirToPlayer = posJugadorScene - posScene;
            if (dirToPlayer.magnitud2() > 0.0)
                e->setDireccion(dirToPlayer.normalizado());
        }

        break;
    }

    case EstadoGrupo::Atacando:
        // Una vez activado el ataque, reutilizamos la lógica estándar
        // de persecución (seek + separación + obstáculos)
        actualizarAtaqueDirecto();
        break;

    default:
        // Otros estados no hacen nada de momento
        break;
    }
}

void OleadaCadetes::actualizarCampamento()
{
    // Si el grupo ya está muerto, nada que hacer
    if (estado == EstadoGrupo::Muerto)
        return;

    // 1) Si estamos en EN_POSICION:
    //    - el grupo se queda quieto
    //    - solo vigilamos si el jugador entra en radioActivacion
    if (estado == EstadoGrupo::EnPosicion) {

        // Jugador en (0,0) en coordenadas de escena (como venimos asumiendo)
        Vector2D posJugador(0.0, 0.0);

        // Centro del grupo (promedio de los cadetes vivos)
        Vector2D centro(0.0, 0.0);
        int vivos = 0;

        for (auto *e : grupo) {
            if (!e || e->muerto) continue;

            Vector2D ps(e->scenePos());
            centro += ps;
            ++vivos;
        }

        if (vivos == 0) {
            // No queda nadie en el grupo → se marca como muerto e inactivo
            estado = EstadoGrupo::Muerto;
            activo = false;
            return;
        }

        centro /= static_cast<qreal>(vivos);

        Vector2D diff = centro - posJugador;
        qreal dist2   = diff.magnitud2();
        qreal radio2  = radioActivacion * radioActivacion;

        // ¿El jugador ha entrado en el radio de activación?
        if (dist2 <= radio2) {
            // El grupo reacciona y pasa a Atacando
            estado = EstadoGrupo::Atacando;
        } else {
            // Todavía no ha entrado → se quedan camperos ahí quietos
            // (Aquí podrías meter una patrulla suave si quieres)
            return;
        }
    }

    // 2) Si estamos en ATACANDO:
    //    - reusamos la lógica estándar de persecución (igual que AtaqueDirecto)
    if (estado == EstadoGrupo::Atacando) {
        actualizarAtaqueDirecto();
    }

    // 3) Si el estado fuese Preparando podríamos, en el futuro,
    //    hacer que se muevan a puntosObjetivo y luego pasar a EnPosicion.
    //    De momento asumimos que los grupos de Campamento se crean directamente
    //    en estado EnPosicion.
}

void OleadaCadetes::actualizarEmboscada()
{
    // Si ya está muerto el grupo, nada que hacer
    if (estado == EstadoGrupo::Muerto)
        return;

    // Filosofía:
    //  - El NIVEL solo hace: setModo(ModoGrupo::Emboscada) y, si acaso, setActivo(true)
    //  - El GRUPO decide internamente qué hacer con su estado.

    // En este diseño simple:
    //  - Si nos acaban de poner en Emboscada y estábamos "quietos"
    //    (Preparando o EnPosicion), nosotros mismos decidimos empezar a atacar.
    if (estado == EstadoGrupo::Preparando ||
        estado == EstadoGrupo::EnPosicion)
    {
        estado = EstadoGrupo::Atacando;
    }

    // Una vez estamos en Atacando, reutilizamos la lógica estándar
    if (estado == EstadoGrupo::Atacando) {
        actualizarAtaqueDirecto();
    }

    // (Más adelante, si quieres emboscadas más finas, aquí puedes:
    //  - hacer que primero se reorganicen a una posición
    //  - usar otra velocidad
    //  - etc.)
}

Vector2D OleadaCadetes::calcularCentroGrupoScene() const
{
    Vector2D acumulado = Vector2D::nulo();
    int conteo = 0;

    for (auto *e : grupo) {
        if (!e || e->muerto) continue;

        Vector2D p(e->scenePos());
        acumulado += p;
        ++conteo;
    }

    if (conteo == 0) return Vector2D::nulo();
    return acumulado / static_cast<qreal>(conteo);
}

Vector2D OleadaCadetes::react_colision(Obstaculo* obs, FuerzaArmada* cadet, qreal step){


    // pos local (respecto al fondo)
    Vector2D pos_act(cadet->pos().x(), cadet->pos().y());

    Vector2D dir_act = cadet->getDireccion();
    if (dir_act.magnitud2() == 0.0) return pos_act;
    Vector2D dir_normal_obs(obs->scenePos());
    dir_normal_obs = dir_normal_obs.normalizado().getNormal();

    qreal dot = dir_act.dot(dir_normal_obs);
    //qDebug() << "dir cadete : x " << dir_act.x() << " y " << dir_act.y();
    //qDebug() << "dir normal obs : x " << dir_normal_obs.x() << " y " << dir_normal_obs.y();
    //qDebug() << "dot : " << dot;

    Vector2D new_pos = pos_act;

    dir_act = dir_act.normalizado();
    if(dot >= 0)
        new_pos = pos_act + (dir_act - dir_normal_obs).normalizado()*step;
    else
        new_pos = pos_act + (dir_act + dir_normal_obs).normalizado()*step;

    return new_pos;

}

bool OleadaCadetes::rondaCompletada() const {
    return enemigosRestantes == 0;
}
