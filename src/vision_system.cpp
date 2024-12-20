#include "vision_system.h"
#include <QDebug>

VisionSystem::VisionSystem(QObject *parent)
    : QObject(parent)
    , isInitialized(false)
{
}

bool VisionSystem::initialize() {
    // Kamera initialisieren (ID 0 für die erste verfügbare Kamera)
    if (!camera.open(0)) {
        emit errorOccurred("Kamera konnte nicht initialisiert werden");
        return false;
    }

    // Standardeinstellungen laden
    camera.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    camera.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
    camera.set(cv::CAP_PROP_FPS, 30);

    isInitialized = true;
    return true;
}

bool VisionSystem::startCamera() {
    if (!isInitialized) {
        return false;
    }
    return camera.isOpened();
}

bool VisionSystem::stopCamera() {
    if (camera.isOpened()) {
        camera.release();
        return true;
    }
    return false;
}

void VisionSystem::setExposure(double value) {
    if (camera.isOpened()) {
        camera.set(cv::CAP_PROP_EXPOSURE, value);
    }
}

void VisionSystem::setGain(double value) {
    if (camera.isOpened()) {
        camera.set(cv::CAP_PROP_GAIN, value);
    }
}

QImage VisionSystem::getCurrentFrame() {
    if (!camera.isOpened()) {
        return QImage();
    }

    cv::Mat frame;
    camera.read(frame);
    
    if (frame.empty()) {
        return QImage();
    }

    // OpenCV Mat zu QImage konvertieren
    cv::Mat rgb;
    cv::cvtColor(frame, rgb, cv::COLOR_BGR2RGB);
    return QImage(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888).copy();
}

cv::Mat VisionSystem::getProcessedFrame() {
    cv::Mat frame;
    camera.read(frame);
    return preprocessImage(frame);
}

SolderJointAnalysis VisionSystem::analyzeSolderJoint(const cv::Mat &image) {
    SolderJointAnalysis analysis;
    
    // Vorverarbeitung
    cv::Mat processed = preprocessImage(image);
    
    // Lötstelle segmentieren
    cv::Mat joint = segmentSolderJoint(processed);
    
    // Eigenschaften analysieren
    cv::Moments moments = cv::moments(joint);
    analysis.diameter = std::sqrt(4 * moments.m00 / M_PI); // Approximierte Durchmesser
    
    // Oberflächenqualität berechnen
    analysis.surfaceQuality = calculateSurfaceQuality(joint);
    
    // Defekte klassifizieren
    analysis.defectType = classifyDefect(joint);
    
    // Akzeptanzkriterien prüfen
    analysis.isAcceptable = analysis.surfaceQuality > 0.8 && 
                           analysis.defectType == "none" &&
                           analysis.diameter > 0.8 && 
                           analysis.diameter < 1.2;
    
    analysis.image = joint;
    return analysis;
}

QVector<cv::Point2f> VisionSystem::detectSolderPoints(const cv::Mat &image) {
    cv::Mat processed = preprocessImage(image);
    return findFeaturePoints(processed);
}

bool VisionSystem::calibrateCamera() {
    std::vector<std::vector<cv::Point3f>> objectPoints;
    std::vector<std::vector<cv::Point2f>> imagePoints;
    cv::Size patternSize(9, 6);
    float squareSize = 20.0f;

    // Schachbrettmuster in mehreren Ansichten aufnehmen
    for (int i = 0; i < 10; ++i) {
        cv::Mat frame;
        camera.read(frame);
        
        std::vector<cv::Point2f> corners;
        bool found = cv::findChessboardCorners(frame, patternSize, corners);
        
        if (found) {
            std::vector<cv::Point3f> obj;
            for (int y = 0; y < patternSize.height; ++y) {
                for (int x = 0; x < patternSize.width; ++x) {
                    obj.push_back(cv::Point3f(x * squareSize, y * squareSize, 0));
                }
            }
            
            objectPoints.push_back(obj);
            imagePoints.push_back(corners);
            
            emit calibrationProgress((i + 1) * 10);
        }
    }

    // Kalibrierung durchführen
    cv::Mat distCoeffs;
    std::vector<cv::Mat> rvecs, tvecs;
    double rms = cv::calibrateCamera(objectPoints, imagePoints, 
                                   cv::Size(1280, 720), 
                                   cameraMatrix, distCoeffs, 
                                   rvecs, tvecs);

    return rms < 1.0; // RMS-Fehler sollte klein sein
}

bool VisionSystem::loadCalibration(const QString &filename) {
    cv::FileStorage fs(filename.toStdString(), cv::FileStorage::READ);
    if (!fs.isOpened()) {
        return false;
    }
    
    fs["camera_matrix"] >> cameraMatrix;
    fs["distortion_coefficients"] >> distCoeffs;
    
    return true;
}

bool VisionSystem::saveCalibration(const QString &filename) {
    cv::FileStorage fs(filename.toStdString(), cv::FileStorage::WRITE);
    if (!fs.isOpened()) {
        return false;
    }
    
    fs << "camera_matrix" << cameraMatrix;
    fs << "distortion_coefficients" << distCoeffs;
    
    return true;
}

cv::Mat VisionSystem::preprocessImage(const cv::Mat &input) {
    cv::Mat result;
    
    // Rauschreduzierung
    cv::GaussianBlur(input, result, cv::Size(5, 5), 0);
    
    // Kontrastverstärkung
    cv::convertScaleAbs(result, result, 1.2, 0);
    
    // Graustufen
    cv::cvtColor(result, result, cv::COLOR_BGR2GRAY);
    
    return result;
}

cv::Mat VisionSystem::segmentSolderJoint(const cv::Mat &input) {
    cv::Mat result;
    
    // Schwellenwertbildung
    cv::threshold(input, result, 0, 255, cv::THRESH_BINARY + cv::THRESH_OTSU);
    
    // Morphologische Operationen
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
    cv::morphologyEx(result, result, cv::MORPH_CLOSE, kernel);
    
    return result;
}

QVector<cv::Point2f> VisionSystem::findFeaturePoints(const cv::Mat &input) {
    QVector<cv::Point2f> points;
    std::vector<cv::KeyPoint> keypoints;
    
    // FAST Feature Detector
    cv::Ptr<cv::FastFeatureDetector> detector = cv::FastFeatureDetector::create();
    detector->detect(input, keypoints);
    
    // Keypoints in Points konvertieren
    for (const auto &kp : keypoints) {
        points.push_back(kp.pt);
    }
    
    return points;
}

double VisionSystem::calculateSurfaceQuality(const cv::Mat &joint) {
    // Gradientenanalyse für Oberflächenqualität
    cv::Mat gradX, gradY;
    cv::Sobel(joint, gradX, CV_32F, 1, 0);
    cv::Sobel(joint, gradY, CV_32F, 0, 1);
    
    cv::Mat magnitude;
    cv::magnitude(gradX, gradY, magnitude);
    
    // Durchschnittliche Gradientenstärke berechnen
    cv::Scalar mean = cv::mean(magnitude);
    
    // Qualität zwischen 0 und 1 normalisieren
    return 1.0 - std::min(1.0, mean[0] / 100.0);
}

QString VisionSystem::classifyDefect(const cv::Mat &joint) {
    // Konturanalyse
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(joint, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
    if (contours.empty()) {
        return "insufficient";
    }
    
    // Größte Kontur analysieren
    double area = cv::contourArea(contours[0]);
    double perimeter = cv::arcLength(contours[0], true);
    double circularity = 4 * M_PI * area / (perimeter * perimeter);
    
    if (circularity < 0.8) {
        return "irregular";
    }
    
    if (area < 100) {
        return "insufficient";
    }
    
    if (area > 500) {
        return "excessive";
    }
    
    return "none";
}
