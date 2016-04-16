import urllib
import json
import serial
import getpass, time

def reglageWIFI() :
	# Recuperation des informations de connexion WiFi
	print " "
	print " ##### INFOS WIFI #####"
	SSID = raw_input("Entrez le nom de votre reseau WiFi : ")
	password = raw_input("Entrez le mot de passe de votre reseau WiFi : ")
	return[SSID, password]

def reglageBUS() :
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
	return [myligne, mystation, mydestination]


def envoiReglages(choix, reglages) :
	# Envoi des informations a la carte Arduino
	print " "
	print " ##### INITIALISATION DE LA DESKTOP STATION #####" 
	print "Connexion a l'arduino ... ",
	try:
		arduino = serial.Serial('COM4', 9600, timeout=1)
	except:
		print "ECHEC : Veuillez brancher la DBS a l'ordinateur !"
		exit(-1)
	print "OK"

	arduino.setDTR(level = True)
	time.sleep(0.5)
	arduino.setDTR(level = False)

	ligne = arduino.readline()
	if choix == 1 :
		while not "INIT ?" in ligne: 
			ligne = arduino.readline()
		arduino.write("INIT WIFI")

		print "Envoi de la valeur SSID ...",
		ligne = arduino.readline()
		while not "READY" in ligne: 
			ligne = arduino.readline()
		arduino.write(reglages[0]) # Envoi du SSID
		print "OK"
		print "Envoi de la valeur password ...",
		ligne = arduino.readline()
		while not "ACK" in ligne: 
			ligne = arduino.readline()
		arduino.write(reglages[1]) # Envoi du password
		print "OK"
		ligne = arduino.readline()
		while not "ACK" in ligne: 
			ligne = arduino.readline()
	
	elif choix == 2 :
		while not "INIT ?" in ligne: 
			ligne = arduino.readline()
		arduino.write("INIT BUS")

		print "Envoi de la valeur ligne ...",
		ligne = arduino.readline()
		while not "READY" in ligne: 
			ligne = arduino.readline()
		arduino.write(str(reglages[0])) # Envoi de l'id ligne
		print "OK"
		print "Envoi de la valeur station ...",
		ligne = arduino.readline()
		while not "ACK" in ligne: 
			ligne = arduino.readline()
		arduino.write(str(reglages[1])) # Envoi de l'id station
		print "OK"
		print "Envoi de la valeur destination ...",
		ligne = arduino.readline()
		while not "ACK" in ligne: 
			ligne = arduino.readline()
		arduino.write(str(reglages[2])) # Envoi de l'id destination
		print "OK"
		ligne = arduino.readline()
		while not "ACK" in ligne: 
			ligne = arduino.readline()
		
	print "Donnees envoyees !"
	print "Fermeture de la connexion avec l'arduino ... ",
	arduino.close()
	print "Connexion fermee !"