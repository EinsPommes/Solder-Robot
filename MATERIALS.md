# Materialien und Komponenten für den Solder-Robot

## Mechanische Komponenten

### Rahmen und Grundstruktur
- Aluminiumprofile 2020/4040
- Verbindungswinkel
- Schrauben und Muttern M4/M5
- Montagezubehör

### Bewegungssystem
- Linearführungen für X-, Y-, Z-Achse
- NEMA 17/23 Schrittmotoren
- GT2 Zahnriemen und Zahnscheiben
- Kugellager
- Motorhalterungen

### Lötkopf-System
- JBC/Weller Lötkolbenmodul
- Heizpatrone
- Thermoelement Typ K
- Lötspitzenhalterung
- Hitzebeständige Isolierung

### Fördersystem
- PU-Gummi Förderband
- DC- oder Schrittmotor für Bandantrieb
- Umlenkrollen
- Bandspanner

## Elektronische Komponenten

### Steuerung
- CNC-Controller (Duet 2 oder Smoothieboard)
- Alternative: STM32/ESP32 Mikrocontroller
- TMC2209/TMC5160 Motortreiber
- 24V 10A Netzteil

### Sensoren
- Endschalter (mechanisch/induktiv)
- Lichtschranken
- Farbsensoren
- Magnetsensoren
- Temperatursensoren

### Regelung und Kommunikation
- PID-Temperaturregler
- Leistungs-MOSFETs/Relais
- USB-Interface
- RS485-Modul
- Kabel und Steckverbinder (JST, Molex)

## Optionale Erweiterungen

### Vision System
- USB-Kamera für Qualitätskontrolle
- Google Coral USB Accelerator für KI
- LED-Beleuchtung

### Sicherheit und Überwachung
- YDLIDAR SDM18 Single-point LiDAR
- Not-Aus-Schalter
- Rauchmelder
- Temperatursensoren

### Computing
- Raspberry Pi 5
- SD-Karte (min. 32GB)
- Kühlkörper
- Netzteil

## Verbrauchsmaterialien

### Lötmaterial
- Bleihaltiges/bleifreies Lötzinn
- Flussmittel
- Reinigungsmittel

### Wartung
- Schmiermittel für Linearführungen
- Ersatz-Lötspitzen
- Reinigungswerkzeug

## Werkzeuge für Montage und Wartung

### Handwerkzeug
- Innensechskantschlüssel
- Schraubendreher-Set
- Zangen-Set
- Multimeter
- Lötstation

### Prototyping
- 3D-Drucker
- CNC-Fräse
- Messschieber
- Wasserwaage

### Verbrauchsmaterial
- Kabelbinder
- Schrumpfschläuche
- Klebeband
- Reinigungstücher

## Empfohlene Ersatzteile

### Mechanik
- Zahnriemen
- Kugellager
- Schrauben und Muttern
- Verbindungselemente

### Elektronik
- Endschalter
- Sicherungen
- Kabel
- Steckverbinder

### Lötsystem
- Lötspitzen
- Thermoelement
- Heizpatrone
- Flussmittel

## Hinweise zur Beschaffung

1. **Qualität**: Bei sicherheitsrelevanten Komponenten auf Zertifizierung achten
2. **Kompatibilität**: Spannungen und Schnittstellen prüfen
3. **Verfügbarkeit**: Ersatzteile sollten gut verfügbar sein
4. **Support**: Auf Herstellersupport und Dokumentation achten

## Sicherheitshinweise

1. **Temperatur**: Hitzebeständige Materialien für den Lötkopfbereich verwenden
2. **Elektrik**: Auf korrekte Isolation und Erdung achten
3. **Mechanik**: Bewegliche Teile entsprechend sichern
4. **Wartung**: Regelmäßige Kontrolle aller Komponenten
