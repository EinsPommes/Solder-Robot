#ifndef SOLDERROBOT_VISION_SYSTEM_H
#define SOLDERROBOT_VISION_SYSTEM_H

#include <QObject>
#include <opencv2/opencv.hpp>
#include <QImage>

struct SolderJointAnalysis {
    bool isAcceptable;
    double diameter;
    double height;
    double surfaceQuality;  // 0.0 - 1.0
    QString defectType;     // "none", "insufficient", "excessive", "void", etc.
    cv::Mat image;
};

class VisionSystem : public QObject {
    Q_OBJECT

public:
    explicit VisionSystem(QObject *parent = nullptr);
    
    // Kamera-Steuerung
    bool initialize();
    bool startCamera();
    bool stopCamera();
    void setExposure(double value);
    void setGain(double value);
    
    // Bildverarbeitung
    QImage getCurrentFrame();
    cv::Mat getProcessedFrame();
    SolderJointAnalysis analyzeSolderJoint(const cv::Mat &image);
    QVector<cv::Point2f> detectSolderPoints(const cv::Mat &image);
    
    // Kalibrierung
    bool calibrateCamera();
    bool loadCalibration(const QString &filename);
    bool saveCalibration(const QString &filename);
    
signals:
    void frameReady(const QImage &frame);
    void solderJointAnalyzed(const SolderJointAnalysis &analysis);
    void calibrationProgress(int progress);
    void errorOccurred(const QString &error);

private:
    cv::VideoCapture camera;
    cv::Mat cameraMatrix;
    cv::Mat distCoeffs;
    bool isInitialized;
    
    // Bildverarbeitungsfunktionen
    cv::Mat preprocessImage(const cv::Mat &input);
    cv::Mat segmentSolderJoint(const cv::Mat &input);
    QVector<cv::Point2f> findFeaturePoints(const cv::Mat &input);
    double calculateSurfaceQuality(const cv::Mat &joint);
    QString classifyDefect(const cv::Mat &joint);
};

#endif // SOLDERROBOT_VISION_SYSTEM_H
