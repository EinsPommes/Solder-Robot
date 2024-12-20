#include "pcb_editor.h"
#include <QFileDialog>
#include <QMessageBox>
#include <cmath>

PCBEditor::PCBEditor(QWidget *parent)
    : QWidget(parent)
    , editMode(false)
    , eraseMode(false)
    , currentPointType("PTH")
    , zoomFactor(1.0)
    , selectedPoint(-1)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

void PCBEditor::loadPCBImage(const QString &imagePath) {
    QImage newImage(imagePath);
    if (newImage.isNull()) {
        QMessageBox::warning(this, tr("Fehler"),
                           tr("Bild konnte nicht geladen werden: %1").arg(imagePath));
        return;
    }
    
    pcbImage = newImage;
    resetView();
    update();
}

void PCBEditor::loadPCBImage(const QImage &image) {
    if (image.isNull()) {
        QMessageBox::warning(this, tr("Fehler"),
                           tr("Ungültiges Bild"));
        return;
    }
    
    pcbImage = image;
    resetView();
    update();
}

void PCBEditor::setPoints(const QVector<SolderPoint> &newPoints) {
    points = newPoints;
    update();
    emit pointsChanged();
}

QVector<SolderPoint> PCBEditor::getPoints() const {
    return points;
}

void PCBEditor::clearPoints() {
    points.clear();
    update();
    emit pointsChanged();
}

void PCBEditor::setEditMode(bool enabled) {
    editMode = enabled;
    if (enabled) {
        eraseMode = false;
        setCursor(Qt::CrossCursor);
    } else {
        setCursor(Qt::ArrowCursor);
    }
}

void PCBEditor::setEraseMode(bool enabled) {
    eraseMode = enabled;
    if (enabled) {
        editMode = false;
        setCursor(Qt::ForbiddenCursor);
    } else {
        setCursor(Qt::ArrowCursor);
    }
}

void PCBEditor::setPointType(const QString &type) {
    currentPointType = type;
}

void PCBEditor::zoomIn() {
    zoomFactor *= 1.2;
    update();
}

void PCBEditor::zoomOut() {
    zoomFactor /= 1.2;
    if (zoomFactor < 0.1) zoomFactor = 0.1;
    update();
}

void PCBEditor::resetView() {
    zoomFactor = 1.0;
    panOffset = QPoint(0, 0);
    update();
}

void PCBEditor::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Hintergrund zeichnen
    painter.fillRect(rect(), Qt::black);
    
    // Bild zeichnen
    if (!pcbImage.isNull()) {
        QSize scaledSize = pcbImage.size() * zoomFactor;
        QPoint center = rect().center() - scaledSize.toPoint() / 2;
        QPoint pos = center + panOffset;
        
        painter.drawImage(QRect(pos, scaledSize), pcbImage);
        
        // Lötpunkte zeichnen
        for (const auto &point : points) {
            drawPoint(painter, point);
        }
        
        // Ausgewählten Punkt hervorheben
        if (selectedPoint >= 0 && selectedPoint < points.size()) {
            const SolderPoint &point = points[selectedPoint];
            QPen pen(Qt::yellow, 2);
            painter.setPen(pen);
            QPoint pos = imageToWidget(QPoint(point.position.x(), point.position.y()));
            painter.drawEllipse(pos, 12, 12);
        }
    }
}

void PCBEditor::mousePressEvent(QMouseEvent *event) {
    lastMousePos = event->pos();
    
    if (event->button() == Qt::LeftButton) {
        if (editMode) {
            // Neuen Punkt hinzufügen
            QPoint imagePos = widgetToImage(event->pos());
            SolderPoint newPoint;
            newPoint.position = QVector3D(imagePos.x(), imagePos.y(), 0);
            newPoint.type = currentPointType;
            newPoint.temperature = 350.0; // Standard-Temperatur
            newPoint.dwellTime = 1000;    // Standard-Verweilzeit
            
            points.append(newPoint);
            selectedPoint = points.size() - 1;
            
            update();
            emit pointAdded(newPoint);
            emit pointsChanged();
        }
        else if (eraseMode) {
            // Punkt löschen
            int index = findPointAt(event->pos());
            if (index >= 0) {
                SolderPoint removedPoint = points[index];
                points.remove(index);
                selectedPoint = -1;
                
                update();
                emit pointRemoved(removedPoint);
                emit pointsChanged();
            }
        }
        else {
            // Punkt auswählen
            int index = findPointAt(event->pos());
            if (index >= 0) {
                selectedPoint = index;
                emit pointSelected(points[index]);
                update();
            }
        }
    }
}

void PCBEditor::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::RightButton) {
        // Bild verschieben
        panOffset += event->pos() - lastMousePos;
        lastMousePos = event->pos();
        update();
    }
}

void PCBEditor::mouseReleaseEvent(QMouseEvent *event) {
    Q_UNUSED(event);
}

void PCBEditor::wheelEvent(QWheelEvent *event) {
    // Zoom mit Mausrad
    QPoint numDegrees = event->angleDelta() / 8;
    
    if (!numDegrees.isNull()) {
        QPoint numSteps = numDegrees / 15;
        
        if (numSteps.y() > 0) {
            zoomFactor *= 1.2;
        } else {
            zoomFactor /= 1.2;
            if (zoomFactor < 0.1) zoomFactor = 0.1;
        }
        
        update();
    }
    
    event->accept();
}

QPoint PCBEditor::imageToWidget(const QPoint &imagePos) const {
    QSize scaledSize = pcbImage.size() * zoomFactor;
    QPoint center = rect().center() - scaledSize.toPoint() / 2;
    QPoint pos = center + panOffset;
    
    return pos + QPoint(imagePos.x() * zoomFactor, imagePos.y() * zoomFactor);
}

QPoint PCBEditor::widgetToImage(const QPoint &widgetPos) const {
    QSize scaledSize = pcbImage.size() * zoomFactor;
    QPoint center = rect().center() - scaledSize.toPoint() / 2;
    QPoint pos = center + panOffset;
    
    QPoint relativePos = widgetPos - pos;
    return QPoint(relativePos.x() / zoomFactor, relativePos.y() / zoomFactor);
}

void PCBEditor::drawPoint(QPainter &painter, const SolderPoint &point) {
    QPoint pos = imageToWidget(QPoint(point.position.x(), point.position.y()));
    
    // Punkt-Typ bestimmt das Aussehen
    if (point.type == "PTH") {
        // Through-hole Punkt
        painter.setPen(QPen(Qt::green, 2));
        painter.setBrush(Qt::transparent);
        painter.drawEllipse(pos, 10, 10);
        painter.drawLine(pos + QPoint(-7, -7), pos + QPoint(7, 7));
        painter.drawLine(pos + QPoint(-7, 7), pos + QPoint(7, -7));
    }
    else if (point.type == "SMD") {
        // SMD Punkt
        painter.setPen(QPen(Qt::blue, 2));
        painter.setBrush(Qt::transparent);
        painter.drawRect(QRect(pos.x() - 8, pos.y() - 8, 16, 16));
    }
    else {
        // Standard Punkt
        painter.setPen(QPen(Qt::red, 2));
        painter.setBrush(Qt::transparent);
        painter.drawEllipse(pos, 10, 10);
    }
}

int PCBEditor::findPointAt(const QPoint &pos) const {
    // Suche nach Punkt in der Nähe der Mausposition
    for (int i = points.size() - 1; i >= 0; --i) {
        QPoint pointPos = imageToWidget(QPoint(points[i].position.x(), 
                                             points[i].position.y()));
        if ((pos - pointPos).manhattanLength() < 15) {
            return i;
        }
    }
    return -1;
}
