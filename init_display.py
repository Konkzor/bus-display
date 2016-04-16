from myfonctions import reglageWIFI, reglageBUS, envoiReglages

print " ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ "
print " ~~~~ Desktop Bus Station - Init Tool ~~~~ "
print " ~~~~ Par Mathieu Garivet             ~~~~ "
print " ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ "

while 1 :
	print " MENU :"
	print " 1 - Reglages WIFI"
	print " 2 - Reglages station de bus"
	choix = input(" Choix ? ")

	if choix == 1 :
		reglages = reglageWIFI()
		break
	elif choix == 2 :
		reglages = reglageBUS()
		break
		
envoiReglages(choix, reglages)


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