German version [below](#deutsche-version).

# Purpose of the program

With this program it is possible to call up updates to a Telegram chat and to save the updates as individual text files. The text files then reflect the content of the chat and the updates are then deleted in Telegram. Updates can therefore only be called up and saved once.
The program is designed so that it can run as a daemon [systemd service](#systemd-service).

# How does the program work?

The program uses a chat ID and a token from a bot to read updates from Telegram and then save them as a text file. The updates come back from Telegram as JSON results and text files are then built from these JSON results. After querying the updates, Telegram is informed by means of an update counter which update ID was queried last so that Telegram deletes all old updates.
While the program is running, various information is recorded in a log file. This log file can be helpful when looking for errors. The log file has the same name as the program and has the extension ```.log```.

# Parameters
In order to use the program, these parameters must be given with the call:

```Chat ID + token of the bot```: The chat ID is given together with the token of the bot. Both values are separated with a colon ':'.
 
```Number of repetitions of the call```: If the value = 0, the call is repeated until the program is stopped. The program can be stopped as follows:
a) The program regularly searches for a so-called stop file. If the program finds this stop file, it will terminate. The stop file has the same name as the executable program plus a ```.stop``` suffix. This file can easily be created with a touch command: ```touch /home/pi/telegram_check_updates/telegram_check_updates.stop```
b) The simple but not so nice variant is to press Ctrl-C.

```Waiting time between 2 calls in 0.1 seconds```: This is an integer value from 1. Please note that the program does not make a new call exactly after the specified waiting time, but only "sleeps" this waiting time after processing a call. This was easier to implement than having to wait an exact wait regardless of the processing.

Directory for saving the text files: By default, the text files are saved in the current directory. Here you can specify the directory in which the files are to be stored. The user under whom the program is started must have write access to this directory.

Example # 1: An update should only be fetched and stored once. The update should be stored in the directory ```/home/pi/```. The call looks like this:
```
./telegram_check_updates 123456:ABCDEFGE_gjklpoiuztr 1 1 /home/pi/
```

Example # 2: You should try 10 times to get updates and wait 5 seconds between calls. The same directory where the program is located should be used as the directory for storing the updates:
```
./telegram_check_updates 123456:ABCDEFGE_gjklpoiuztr 10 50
```
# Updates as text files

The text files created by the program have these conventions for the name:
```
<year><month><day>_<time>_<random number with 4 digits>.txt
```
The content of the file has 3 lines and looks like this:
```
Name: messages_from_chat
 
Chat ID: <the ID of the chat>
 
Message: <text that was entered in the chat>
```

# How is the program created?

The program is written in C++. The make file called ```makefile``` can be used to create an executable program from the source code.
Note that this program uses C++ classes I created that I built for general purposes. These classes are also used in other programs. I have these C++ classes in the ```/home/pi/cpp_sources``` directory. The makefile also looks for these classes in it.
The current version of the files is stored in the repository.
The program uses libraries from ```curl``` and ```JSON```. In order to install these libraries, enter this commands:\
```sudo apt-get install libcurl4-openssl-dev```\
```sudo apt-get install libjsoncpp-dev```

The program has only been tested on the Raspberry Pi. But it should also work on another Linux.

# Sample files

The directory ```samples``` contains examples of what the files with the updates look like.

# systemd-service

In order to let the program run as a systemd-service you can use the file ```telegram_check_updates.service```.

# Outlook

The program will be integrated into my program [message2action](https://github.com/Sunblogger/message2action). With this integration, there is no need to store and read in text files. It is then possible to process the data in the memory. 
# Deutsche Version

# Zweck des Programms

Mit diesem Programm ist es möglich, Updates zu einem Telegram-Chat abzurufen und die Updates als einzelne Text-Dateien zu speichern. Die Textdateien geben dann den Inhalt des Chats wieder und in Telegram werden die Updates dann gelöscht. Updates können also nur einmal abgerufen und abgespeichert werden.
Das Programm ist so ausgelegt, dass es als Daemon [systemd service](#systemd-service-1) laufen kann.

# Wie funktioniert das Programm?

Das Programm nutzt eine Chat-ID und einen Token eines Bots, um Updates aus Telegram auszulesen und dann als Text-Datei abzuspeichern. Die Updates kommen als JSON-Ergebnisse von Telegram zurück und aus diesen JSON-Ergebnissen werden dann Textdateien aufgebaut. Nach dem Abfragen der Updates wird Telegram mittels eines Update-Counters mitgeteilt, welche Update-ID als letztes abgefragt wurde so dass Telegram alle alten Updates löscht.
Während das Programm läuft, werden verschiedene Informationen in einer Log-Datei mitgeschrieben. Diese Logdatei kann hilfreich sein, wenn man Fehler sucht. Die Log-Datei heißt genauso wie das Programm und hat die Endung ```.log```.

# Parameter 
Um das Program zu nutzen, sind diese Parameter mit dem Aufruf mitzugeben:

```Chat-ID + Token des Bot```: Die Chat-ID wird zusammen mit dem Token des Bot angegeben. Beide Werte werden mit einem Doppelpunkt ':' getrennt.
 
```Anzahl an Wiederholungen des Abrufes```: Bei einem Wert=0 wird so lange der Abruf wiederholt, bis das Programm gestoppt wird. Das Programm kann wie folgt gestoppt werden:
a) Das Programm sucht regelmässig nache einer sogenannten Stop-Datei. Findet das Programm diese Stop-Datei, so beendet es sich. Die Stop-Datei heißt genauso wie das ausführbare Programm plus ein Suffix '.stop'. Diese Datei kann einfach mit einem touch-Befehl erzeugt werden: touch /home/pi/telegram_check_updates/telegram_check_updates.stop
b) Die einfache aber nicht so schöne Variante ist, Strg-C zu drücken. 

```Wartezeit zwischen 2 Abrufen in 0.1 Sekunden```: Das ist ein ganzzahliger Wert ab 1. Es ist zu beachten, dass das Programm nicht exakt nach der angegebenen Wartezeit einen neuen Aufruf durchführt sondern erst nach Verarbeitung eines Abrufs diese Wartezeit "schläft". Dies war einfacher zu implementieren als eine exakte Wartezeit unabhängig von der Verarbeitung zu warten.

Verzeichnis zum Abspeichern der Textdateien: Per Default werden die Textdateien im aktuellen Verzeichnis abgespeichert. Man kann hier angeben, in welchem Verzeichnis die Dateien abgelegt werden sollen. Für dieses Verzeichnis muss der User, unter dem das Programm gestartet wird, Schreibrechte haben.

Beispiel #1: Es soll nur ein einziges Mal ein Updates geholt und abgelegt werden. Das Update soll im Verzeichnis /home/pi/ abgelegt werden. Der Aufruf sieht so aus:
```
./telegram_check_updates 123456:ABCDEFGE_gjklpoiuztr 1 1 /home/pi/
```
Beispiel #2: Es soll 10 mal versucht werden, Updates zu holen und zwischen den Aufrufen soll 5 Sekunden gewartet werden. Als Verzeichnis zum Ablegen der Updates soll das gleiche Vereichnis genutzt werden, wo das Programm selbst liegt:
```
./telegram_check_updates 123456:ABCDEFGE_gjklpoiuztr 10 50
```
# Updates als Textdateien

Die Textdateien, die das Programm anlegt, haben diese Konventionen für den Namen:
```
<Jahr><Monat><Tag>_<Uhrzeit>_<Zufallszahl mit 4 Stellen>.txt
```
Der Inhalt der Datei hat 3 Zeilen sieht wie folgt aus:
```
Name: messages_from_chat
Chat-ID: <die ID des Chats>
Message: <Text, der im Chat eingegeben wurde>
```

# Wie wird das Programm erstellt?

Das Programm ist in C++ geschrieben. Um aus dem Quelltext ein ausführbares Programm zu erzeugen, kann die Make-Datei namens ```makefile``` genutzt werden.
Beachte, dass dieses Programm von mir erstellte C++-Klassen nutzt, die ich für allgemeine Zwecke aufgebaut habe. Diese Klassen werden auch in anderen Programmen benutzt. Diese C++-Klassen liegen bei mir im Verzeichnis ```/home/pi/cpp_sources```. Das makefile sucht darin auch diese Klassen.
Die aktuelle Fassung der Dateien ist im Repository abgelegt.
Das Programm nutzt die Bibliotheken von ```curl``` und ```JSON```. Um diese beiden Bibliotheken zu installieren, ist dies auszuführen:\
```sudo apt-get install libcurl4-openssl-dev```\
```sudo apt-get install libjsoncpp-dev```

Das Programm ist nur auf dem Raspberry Pi getestet worden. Es sollte aber auch auf einem anderem Linux funktionieren.

# Beispieldateien

Im Verzeichnis ```samples``` sind Beispiele abgelegt, wie die Dateien mit den Updates aussehen.

# systemd-service

Um das Program als systemd-Dienst zu nutzen, kann man die Datei ```telegram_check_updates.service``` nutzen.

# Ausblick

Das Programm wird in mein Programm [message2action](https://github.com/Sunblogger/message2action) integriert werden. Mit dieser Integration wird auf das Ablegen und Einlesen von Textdateien verzichtet. Es ist dann eine Verarbeitung der Daten im Speicher möglich.
