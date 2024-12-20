# Solder-Robot

Ein automatisierter Lötroboter mit fortschrittlicher Steuerung und Bildverarbeitung.

![Solder-Robot Logo](docs/images/logo.png)

## 🌟 Features

### Kernfunktionen
- Automatisiertes Löten von THT- und SMD-Komponenten
- Präzise Temperatur- und Bewegungssteuerung
- Intelligente Bildverarbeitung zur Qualitätskontrolle
- Benutzerfreundliche GUI mit PCB-Editor

### Erweiterte Funktionen
- Programm-Management für Lötsequenzen
- Predictive Maintenance System
- Energiemanagement und Optimierung
- Umfangreiches Sicherheitssystem
- Detaillierte Datenprotokollierung

## 🛠 Technische Details

### Hardware-Anforderungen
- CNC-Controller (Duet 2/Smoothieboard) oder STM32/ESP32
- Schrittmotoren für X-, Y-, Z-Achse (NEMA 17/23)
- Professionelles Lötsystem (z.B. JBC/Weller)
- Kamerasystem für Qualitätskontrolle
- Optional: Google Coral für KI-Beschleunigung

### Software-Stack
- C++ mit Qt Framework
- OpenCV für Bildverarbeitung
- SQLite für Datenspeicherung
- Boost für Systemfunktionen
- WebSocket-Server für Remote-Zugriff

## 📋 Installation

1. **Abhängigkeiten installieren**
   ```bash
   # Qt Framework
   sudo apt-get install qt6-base-dev

   # OpenCV
   sudo apt-get install libopencv-dev

   # Boost
   sudo apt-get install libboost-all-dev
   ```

2. **Projekt kompilieren**
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

3. **Programm starten**
   ```bash
   ./solder_robot
   ```

## 💻 Verwendung

### PCB-Editor
1. Platinenbild laden
2. Lötpunkte markieren (manuell oder automatisch)
3. Parameter anpassen (Temperatur, Verweilzeit)
4. Job speichern und ausführen

### Lötprozess
1. Platine einlegen und ausrichten
2. Lötprogramm auswählen
3. Prozess starten und überwachen
4. Qualitätskontrolle durchführen

## 🔧 Konfiguration

Die Konfiguration erfolgt über mehrere JSON-Dateien:

- `config/system.json`: Systemeinstellungen
- `config/motion.json`: Bewegungsparameter
- `config/temperature.json`: Temperaturprofile
- `config/security.json`: Sicherheitseinstellungen

## 📊 Datenprotokollierung

Der Roboter protokolliert:
- Lötprozesse und Parameter
- Qualitätsdaten
- Wartungsinformationen
- Energieverbrauch
- Sicherheitsereignisse

## 🛡 Sicherheitsfunktionen

- Kollisionserkennung
- Temperaturüberwachung
- Rauchmelder-Integration
- Not-Aus-System
- Zugriffskontrolle

## 🔄 Wartung

### Regelmäßige Wartung
- Lötspitzen reinigen und prüfen
- Linearführungen schmieren
- Sensoren kalibrieren
- Systemtests durchführen

### Predictive Maintenance
- Verschleißüberwachung
- Wartungsvorhersage
- Automatische Benachrichtigungen

## 📝 Dokumentation

Ausführliche Dokumentation finden Sie in:
- [Benutzerhandbuch](docs/user_manual.md)
- [Technische Dokumentation](docs/technical.md)
- [API-Referenz](docs/api.md)
- [Wartungsanleitung](docs/maintenance.md)

## 🤝 Beitragen

1. Fork erstellen
2. Feature Branch erstellen (`git checkout -b feature/AmazingFeature`)
3. Änderungen committen (`git commit -m 'Add some AmazingFeature'`)
4. Branch pushen (`git push origin feature/AmazingFeature`)
5. Pull Request erstellen

## 📄 Lizenz

Dieses Projekt ist unter der MIT-Lizenz lizenziert - siehe [LICENSE](LICENSE) für Details.

## ✨ Danksagung

- Qt Framework Team
- OpenCV Community
- Alle Mitwirkenden und Tester

## 📧 Kontakt

Projektlink: [https://github.com/EinsPommes/Solder-Robot](https://github.com/EinsPommes/Solder-Robot)

---
⌨️ mit ❤️ erstellt von [Mika]
