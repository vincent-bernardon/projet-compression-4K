#include "customgraphicsview.h"

CustomGraphicsView::CustomGraphicsView(QWidget *parent)
    : QGraphicsView(parent)
{
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    setDragMode(QGraphicsView::ScrollHandDrag);
}

void CustomGraphicsView::wheelEvent(QWheelEvent *event)
{
    // Zoom avant ou arrière avec la molette
    if (event->angleDelta().y() > 0) {
        scale(1.2, 1.2);  // Zoom avant
    } else {
        scale(0.8, 0.8);  // Zoom arrière
    }
    event->accept();
}

void CustomGraphicsView::mousePressEvent(QMouseEvent *event)
{
    // Gestion standard des événements souris
    QGraphicsView::mousePressEvent(event);
}

void CustomGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    // Gestion standard des événements souris
    QGraphicsView::mouseReleaseEvent(event);
}
