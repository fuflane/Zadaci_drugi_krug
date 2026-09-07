# 1. Prije nego što išta radite s uređajem, koje 3–5 pitanja postavljate kupcu? Zašto baš ta pitanja?

- Jeste li spojili uređaj prema uputama?
- Jeste li sigurni da uređaj nije oštećen vanjskim utjecajem? Fizička oštećenja kabela ili npr. prolijevanje tekućine.
- Postoji li mogući izvor elektromagnetskih smetnji u blizini uređaja? Npr. transformator ili kakav drugi uređaj koji može zahtjevati mnogo snage.
- Jeste li provjerili ispravnost vašeg računala? oštećenja USB porta ili sistemske greške uzrokovane kvarovima ili mmalwareom mogu lako utjecati na komunikaciju s uređajem.
- Jesu li u logu vidljive linije besmislenog teksta i ispisuju li se tijekom prekida prazne linije? Indicira problem na senzorskoj strani
  
# 2. Nabrojite hipoteze koje treba eliminirati, poredane od najjeftinije za testiranje prema najskupljoj

## 1. Provjera ispravnosti napajanja
 - ako zbog neispravnog napajanja uređaj dolazi u reset, 200ms je smisleno vrijeme za boot
 - zammoliti da pokušaju sa zamijenjenim napajanjem pa pogledati log 
  
## 2. Timer overflow
   - Provjeriti događa li se zastoj u smislenim vremennskim razmacima koji bi odgovarali maksimalnoj veličini neke varijable
   - provjeriti kod za moguću grešku
## 3. Hang-up u threadu ili interruptu
 - provjeriti kod na ključnnim mjestima u potrazi za bugom zbog kojeg kod stoji ili se može vrtiti u petlji
 - vidljivo iz rupe u log-u i slijeda određenih radnji

## 4. Testiranje UART pretvornika
 - Poslati drugi UART pretvornik i pričekati log
 - U slučaju ovog problema, log bi trebao biti ispravan

# 3. Što biste promijenili u sljedećoj verziji firmware-a da se ovakav problem u budućnosti dijagnosticira brže?
- Dodao bih debug log dostupan korisniku: omogućio bih brojanje ms sa strane sustava i broj ciklusa sa strane senzorskog čvora; uvesti debug poruku s trenutnim stanjem sustava koja se javlja u pravilnim intervalima (npr. heap/stack counter, broj propuštenih interruptova); ispis startup poruke;
- Ako je moguće potražio bih način za lakši daljinski pristup uređaju