import urllib
import json

myligne = input("Entrez votre ligne de bus : ")
url = "http://api-ratp.pierre-grimaud.fr/v2/bus/" + str(myligne) + "/stations"
reponse = urllib.urlopen(url)
data = json.loads(reponse.read())

print 'Liste des stations :'
for station in data['response']['stations'] :
	print station['id_station']+' : '+station['name']
	
mystation = input("Entrez votre numero de station : ")

url = "http://api-ratp.pierre-grimaud.fr/v2/bus/"
reponse = urllib.urlopen(url)
data = json.loads(reponse.read())

print 'Liste des destinations possibles :'
for ligne in data['response']['bus'] :
	if ligne['line'] == str(myligne) :
		for destination in ligne['destinations'] :
			print destination['id_destination']+' : '+destination['name']

mydestination = input("Entrez votre numero de destination : ")

url = "http://api-ratp.pierre-grimaud.fr/v2/bus/"+str(myligne)+"/stations/"+str(mystation)+"?destination="+str(mydestination)
reponse = urllib.urlopen(url)
data = json.loads(reponse.read())

for horaire in data['response']['schedules'] :
	print 'Prochain bus dans : ' + horaire['message']

date, heure = data['_meta']['date'].split("T", 1)
heure, offset = heure.split("+", 1)
heure = heure.split(":")
offset = offset.split(":")
print 'Il est : ' + heure[0]+':' + str(int(heure[1]) + int(offset[0])) + ':' + str(int(heure[2]) + int(offset[1]))