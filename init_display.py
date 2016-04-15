import urllib
import json
import serial
import getpass
 
# Port Serie utilisez par le gyropode
SERIALPORT = "COM4"
# Vitesse du port serie
SERIALSPEED = 9600
print " ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ "
print " ~~~~ Desktop Bus Station - Init Tool ~~~~ "
print " ~~~~ Par Mathieu Garivet             ~~~~ "
print " ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ "

# Recuperation des informations de connexion WiFi
print " ##### INFOS WIFI #####"
SSID = raw_input("Entrez le nom de votre reseau WiFi : ")
password = raw_input("Entrez le mot de passe de votre reseau WiFi : ")

# Recuperation des informations de l'arret de bus
print " "
print " ##### INFO LIGNE #####"
myligne = input("Entrez votre ligne de bus : ")
url = "http://api-ratp.pierre-grimaud.fr/v2/bus/" + str(myligne) + "/stations"
reponse = urllib.urlopen(url)
data = json.loads(reponse.read())

print " "
print " ##### INFO STATION #####"
print 'Liste des stations :'
for station in data['response']['stations'] :
	print station['id_station'] + ' : ' + station['name']
	
mystation = input("Entrez votre numero de station : ")

url = "http://api-ratp.pierre-grimaud.fr/v2/bus/"
reponse = urllib.urlopen(url)
data = json.loads(reponse.read())

print " "
print " ##### INFO DESTINATION #####"
print 'Liste des destinations possibles :'
for ligne in data['response']['bus'] :
	if ligne['line'] == str(myligne) :
		for destination in ligne['destinations'] :
			print destination['id_destination'] + ' : ' + destination['name']

mydestination = input("Entrez votre numero de destination : ")

# Envoi des informations a la carte Arduino
print " "
print " ##### INITIALISATION DE LA DESKTOP STATION #####" 
print "Connexion a l'arduino ... ",
try:
    arduino = serial.Serial(SERIALPORT, SERIALSPEED, timeout=1)
except:
    print "ECHEC : Veuillez brancher la DBS a l'ordinateur !"
    exit(-1)
print "OK !"
 
arduino.setDTR(level = True)
time.sleep(0.5)
arduino.setDTR(level = False)
ligne = arduino.readline()
while not "READY" in ligne: 
    ligne = arduino.readline()
arduino.write(str(myligne)) # Envoi de l'id ligne
ligne = arduino.readline()
while not "ACK" in ligne: 
    ligne = arduino.readline()
arduino.write(str(mystation)) # Envoi de l'id station
ligne = arduino.readline()
while not "ACK" in ligne: 
    ligne = arduino.readline()
arduino.write(str(mydestination)) # Envoi de l'id destination
ligne = arduino.readline()
while not "ACK" in ligne: 
    ligne = arduino.readline()
print "Donnees envoyees !"
print "Fermeture de la connexion avec l'arduino ... ",
arduino.close()
print "Connexion fermee !"


"""url = "http://api-ratp.pierre-grimaud.fr/v2/bus/"+str(myligne)+"/stations/"+str(mystation)+"?destination="+str(mydestination)
reponse = urllib.urlopen(url)
data = json.loads(reponse.read())

for horaire in data['response']['schedules'] :
	print 'Prochain bus dans : ' + horaire['message']

date, heure = data['_meta']['date'].split("T", 1)
heure, offset = heure.split("+", 1)
heure = heure.split(":")
offset = offset.split(":")
print 'Il est : ' + heure[0]+':' + str(int(heure[1]) + int(offset[0])) + ':' + str(int(heure[2]) + int(offset[1]))"""