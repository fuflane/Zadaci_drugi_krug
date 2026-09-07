# 1. Topologije

# Topologija A: Sve na jednu I²C sabirnicu - ne preporučam
 ## prednosti
 - mala cijena zbog jednostavne izvedbe i jeftinih sennzora
 - izvedivo izravno s ESP32 I2C kontrolerom
 - dovoljno brz odaziv
## mane
 - niska otpornnost na elektromagnetske smetnje zbog open-drain konfiguracije
 - vrlo male duljine sabirnice moguće
 - u slučaju oklopljavanja povećava se cijena izvedbe
-  
# Topologija B: 1-Wire s DS18B20 senzorima - ne preporučam
 ## prednosti
 - jednostavnije adresiranje u odnosu na I2C
 - jednostavno povezivanje na MCU s mogućnošću povezivanja sa samo 1 žicom
 - mogućnost većih udaljenosti sennzora od MCU-a
 ## mane
 - vrlo niske brzine prijenosa  onemogućuju zahtjev za 8 paralelno spojenih senzora pri adekvatnim temperaturnim rezolucijama ()
 - neotporan na naponske spikeove zbog pull-upova

# Topologija C: RS-485 half-duplex s „pametnim" senzorskim čvorovima - preporučam
## prednosti
 - diferencijalni prijenos signala otporan na smetnje
 - velike duljine sigurnog prijenosa
 - dovoljna brzina prijenosa za zadatak
## mane 
 -  skuplja izvedba - pojedinačni MCU-ovi, pojedinačni transcieveri + senzor
 -  Potreban firmware i za pojedine senzore
 -  kompliciranije održavanje

# 2. Preporuka

Zbog industrijskog okruženja od navedenih je jedino RS-485 povoljan i namijenjen za ovu izvedbu radi diferencijalnog signala i dostupnosti specijalizirane opreme. Zadovoljava udaljennost i brzinu, a također čini implementaciju znatno fleksibilnijom radi zasebnih MCU-ova na svakom senzorskom čvoru. Smatram da u industrijskoj primjeni razlika u cijeni nije toliko značajna.

# Skeleton driver-a u pseudokodu

## Taskovi:
- 1. Uzorkovanje - 1. prioritet, 10Hz
- 2. Obrada podataka - 2. prioritet, ovisan o brzini MCU-a, ali svakako dosta brži od 10Hz
- 3. odašiljanje na centralni MCU - 3. prioritet

## State Machine


## Error Handling

- Odvojeni threadovi za pollanje.
- Timeout za svaki senzor u vrijednosti maksimalno dvostruke očekivane periode uzorkovanja po senzoru.
- U slučaju CRC errora, odrediti razuman maksimalan broj ponovnih pokušaja zahtjeva za izmjerom. U slučaju učestalosti grešaka, obaveznno javiti grešku sustava i savjetovati servis.

## Plan bring-upa i verifikacije komunikacije

Nemam iskustva s RS-485 sabirnicom pa bih morao puno detaljnije istražiti.
Logičan slijed koraka bio bi:
- ### 1. provjera napajanja i spojnih kablova
- provjera kontakta i ako je potrebno impedancije
- zadovoljeno ako je uspostavljena veza između ćvora i cenntralne jedinice
- ### 2. Provjera integriteta diferencijalnog signala
- provjera otpornnosti na šum
- ### 3. Validacija veze unutar softvera za pojedinačne senzore
- prolazak CRC testova na probnim mjerenjima 
- ### 4. Dugotrajni test svih senzora odjednom
- pollanje svih 8 čvorova pri 10 Hz
- prolazak u slučaju vrlo malih primiječenih grešaka (npr. <2%)
- ### 5. Paralelno testiranje Wi-Fi veze centralnog MCU-a
- očekuje se stalna veza bez padova
 