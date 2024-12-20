#ifndef SOLDERROBOT_PCB_EDITOR_H
#define SOLDERROBOT_PCB_EDITOR_H

#include <QWidget>
#include <QImage>
#include <QPainter>
#include <QMouseEvent>
#include <QVector>
#include <QPoint>
#include "job_manager.h"

class PCBEditor : public QWidget {
    Q_OBJECT

public:
    explicit PCBEditor(QWidget *parent = nullptr);

    // Bild laden und anzeigen
    void loadPCBImage(const QString &imagePath);
    void loadPCBImage(const QImage &image);
    
    // Lötpunkte setzen und bearbeiten
    void setPoints(const QVector<SolderPoint> &points);
    QVector<SolderPoint> getPoints() const;
    void clearPoints();
    
    // Editier-Modi
    void setEditMode(bool enabled);
    void setEraseMode(bool enabled);
    void setPointType(const QString &type);
    
    // Zoom und Pan
    void zoomIn();
    void zoomOut();
    void resetView();

signals:
    void pointAdded(const SolderPoint &point);
    void pointRemoved(const SolderPoint &point);
    void pointSelected(const SolderPoint &point);
    void pointsChanged();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    QImage pcbImage;              // Original PCB-Bild
    QVector<SolderPoint> points;  // Liste der Lötpunkte
    bool editMode;                // Editiermodus aktiv?
    bool eraseMode;               // Löschmodus aktiv?
    QString currentPointType;     // Aktueller Punkttyp
    double zoomFactor;            // Aktueller Zoom-Faktor
    QPoint panOffset;             // Verschiebung des Bildes
    QPoint lastMousePos;          // Letzte Mausposition
    int selectedPoint;            // Index des ausgewählten Punkts

    // Hilfsfunktionen
    QPoint imageToWidget(const QPoint &imagePos) const;
    QPoint widgetToImage(const QPoint &widgetPos) const;
    void drawPoint(QPainter &painter, const SolderPoint &point);
    int findPointAt(const QPoint &pos) const;
    void updatePointCoordinates();
};

#endif // SOLDERROBOT_PCB_EDITOR_H
