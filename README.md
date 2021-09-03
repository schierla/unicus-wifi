# unicus-wifi

WLAN-Gateway für Unicus-Liedanzeiger

Dieses Projekt dient dazu, die Liednummern, die mit dem Funksender an
UNICUS-LED-Liedanzeiger gesendet werden, zu empfangen und an einen
Computer zu übertragen (um zum Beispiel Liednummer und Liedtitel
automatisch im Livestream einzublenden).

## Hardware

Das Projekt besteht aus einem ESP8266-Modul (hier in Form eines Wemos D1 
Mini) für den WLAN-Teil und einem HOPERF-Funkmodul RFM23B-868D zur 
Kommunikation mit Liedanzeiger bzw. Funksender. 

Diese sind folgendermaßen zu verbinden:

| D1 mini | RFM23B-868D |
| :-----: | :---------: |
|    -    |     NC      |
|    -    |     NC      |
|   GND   |     GND     |
|   3V3   |     VCC     |
|    -    |   GPIO_0    |
|    -    |   GPIO_1    |
|    -    |   GPIO_2    |
|   D6    |     SDO     |
|   D7    |     SDI     |
|   D5    |     SCK     |
|   D1    |    NSEL     |
|   D2    |    NIRQ     |
|   GND   |     SDN     |
|   GND   |     GND     |

Alternativ ist es ausreichend, die nebeneinander liegenden Pins 
SDN und GND direkt auf dem RFM23B zu verbinden, und nur den anderen 
GND-Pin mit dem D1 Mini zu verbinden.

Weiterhin muss am Antennen-Anschluss des RFM23B-D ein Drahtstück mit
ca. 86mm Länge angebracht werden.

Hinweis: Das RFM23B-868D-Modul hat ein Rastermaß von 2 mm, so dass 
zur Montage auf einer 2,54 mm-Lochrasterplatine die Pins verbogen 
werden müssen. Da viele Pins nicht belegt sind, ist das jedoch möglich.

## Software

Als Software muss der Arduino-Sketch aus dem Ordner `unicus-wifi` auf
dem Wemos D1 Mini installiert werden. Dafür ist die Arduino-Software
mit dem passenden Board-Package erforderlich.

Weiterhin muss die Datei `WifiSettings.h.example` nach `WifiSettings.h`
kopiert und mit den gewünschten WLAN-Zugangsdaten ausgefüllt werden.

Nach erfolgreicher Installation verbindet sich das Projekt mit dem
angegebenen WLAN und wartet auf TCP Port 2022 auf Verbindungen.
Jede neue Verbindung wird zuerst mit den Zeilen

```
READY

```

beantwortet. Sobald eine Liednummer empfangen wird, wir diese als eine Zeile
in Textform an alle verbundenen TCP-Gegenstellen (maximal 5 Stück)
gesendet. Diese Zeilen können z.B. so aussehen:

```
123
112.4
40, 1-4
L3, 1-3+5
```

Ebenso wird, sobald die Liednummer gelöscht wird, eine leere
Zeile gesendet.

```

```

## Erweiterung

Als Erweiterung kann das Projekt auch über WLAN empfangenen Liednummern
an den Liedanzeiger senden und anzeigen lassen. Aufgrund des größeren
Missbrauchspotentials ist dieser Code nur auf Anfrage verfügbar.
