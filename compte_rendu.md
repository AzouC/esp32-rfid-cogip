# Mission « Pointeuse 2.0 » — COGIP
**Projet :** Lecture de badges RFID depuis un microcontrôleur  
**Filière :** BTS CIEL (Option IR)

---

### Question 1 : Inconvénients d'une pointeuse électromécanique ancienne

* **Maintenance (faible MTBF et absence de télémétrie) :**  
  Forte sensibilité à l'usure mécanique (engrenages, têtes d'impression), pénurie de pièces de rechange et impossibilité d'effectuer un autodiagnostic ou un dépannage à distance.
* **Exploitation des données (rupture de la chaîne numérique) :**  
  Stockage analogique (fiches carton/papier) sans synchronisation horaire par protocole NTP (dérive temporelle) et absence d'interfaces réseau (Ethernet/Wi-Fi, API REST) pour l'intégration automatique et en temps réel dans le SI de l'entreprise.
* **Évolutivité (architecture matérielle figée) :**  
  Système électromécanique non reprogrammable rendant impossibles les mises à jour logicielles à distance (OTA), la gestion dynamique des droits d'accès ou l'ajout d'authentifications modernes (RFID, NFC).

---

### Question 2 : Diagramme de cas d'utilisation (Use Case)

* **Acteur « Salarié de la COGIP » :**  
  * Relié au cas d'utilisation principal : *Badger pour obtenir l'autorisation d'accès*.
  * Relations d'inclusion (`<<include>>`) :
    * *Badger pour obtenir l'autorisation d'accès* `<<include>>` *Identifier le salarié*
    * *Badger pour obtenir l'autorisation d'accès* `<<include>>` *Vérifier les droits d'accès*
    * *Badger pour obtenir l'autorisation d'accès* `<<include>>` *Enregistrer le pointage*
  * Relation d'extension (`<<extend>>`) :
    * *Afficher le résultat sur une page web* `<<extend>>` *Badger pour obtenir l'autorisation d'accès*
* **Acteur « Responsable RH » :**  
  * Relié aux cas d'utilisation d'administration : *Consulter les droits d'accès* et *Modifier les droits d'accès*.

*(Insérer le schéma graphique complété dans le rapport Word).*

---

### Question 3 : Programme Blink et moniteur série

* **Fonctionnement :** Le programme téléversé fait clignoter la LED bleue (GPIO 2) toutes les secondes et transmet l'adresse MAC (Chip ID) sur l'UART.
* **Impact de la vitesse en bauds :** Si une vitesse différente de 115 200 bauds est sélectionnée dans le moniteur série, l'affichage devient illisible (caractères erronés). L'ESP32 et le terminal PC doivent être calés sur la même fréquence d'échantillonnage pour décoder les trames UART asynchrones.

---

### Question 4 : Analyse des instructions du programme de test

* `uint64_t chipid;` : déclaration d'un entier non signé sur 64 bits (8 octets), format adapté pour accueillir l'adresse MAC matérielle codée sur 48 bits (6 octets) renvoyée par `ESP.getEfuseMac()`.
* `pinMode(2, OUTPUT);` : configuration de la broche GPIO 2 (reliée à la LED bleue intégrée) en mode sortie logique.
* `digitalWrite(2, HIGH);` : impose une tension de 3,3 V sur le GPIO 2, ce qui polarise la LED et l'allume.
* `digitalWrite(2, LOW);` : impose un potentiel nul (0 V / GND) sur le GPIO 2, ce qui éteint la LED.

---

### Question 5 : Signaux du bus SPI

* **SCK (*Serial Clock*) :** horloge générée par le maître (ESP32) pour cadencer le transfert synchrone des bits.
* **MOSI (*Master Out, Slave In*) :** ligne de données de l'ESP32 vers le module RC522.
* **MISO (*Master In, Slave Out*) :** ligne de données du module RC522 vers l'ESP32.
* **$\overline{\text{SS}}$ (*Slave Select* / NSS) :** sélection du composant esclave par le maître. Signal actif au niveau logique bas (0 V).

---

### Question 6 : Mode duplex et raccordement multi-esclaves

* **Mode de transmission :** Le bus SPI fonctionne en **full-duplex** grâce aux deux lignes distinctes MOSI et MISO qui autorisent l'émission et la réception simultanées sur chaque front d'horloge.
* **Connexion de deux esclaves :** Le maître doit mobiliser **5 broches** : 3 broches communes pour le bus (SCK, MOSI, MISO) et 2 broches de sélection indépendantes ($\overline{\text{SS}}_1$ et $\overline{\text{SS}}_2$).

---

### Question 7 : Description des broches du module RFID-RC522

| Broche | Direction (RC522) | Rôle |
| :--- | :--- | :--- |
| **NSS** (SDA) | Entrée | Sélection esclave (*Slave Select*), active à l'état bas |
| **SCK** | Entrée | Horloge de synchronisation série |
| **MOSI** | Entrée | Réception des commandes envoyées par l'ESP32 |
| **MISO** | Sortie | Émission des données (NUID) vers l'ESP32 |
| **IRQ** | Sortie | Ligne d'interruption matérielle vers l'ESP32 |
| **GND** | Alimentation | Masse de référence (0 V) |
| **RST** | Entrée | Réinitialisation matérielle et mise en veille |
| **3.3V** | Alimentation | Entrée d'alimentation régulée 3,3 V |

---

### Question 8 : Antenne et caractéristiques RF du module RC522

* **Antenne :** matérialisée par les pistes en cuivre concentriques gravées sur la partie gauche du circuit imprimé.
* **Fonctions de l'antenne :** 
  1. Émet un champ électromagnétique à 13,56 MHz pour télé-alimenter la puce passive du badge par couplage inductif.
  2. Assure la transmission des données avec le transpondeur par modulation et rétromodulation de charge.
* **Portée typique :** entre 2 et 5 cm.

---

### Question 9 : Câblage ESP32 — RC522 (Bus VSPI)

| Broche RC522 | Broche ESP32 | Signal ESP32 |
| :--- | :--- | :--- |
| **NSS** | GPIO 5 | VSPI-SS |
| **SCK** | GPIO 18 | VSPI-SCK |
| **MOSI** | GPIO 23 | VSPI-MOSI |
| **MISO** | GPIO 19 | VSPI-MISO |
| **IRQ** | *Non connecté* | Scrutation logicielle par défaut (*polling*) |
| **GND** | GND | Masse commune |
| **RST** | GPIO 4 | Commande Reset |
| **3.3V** | 3V3 | Alimentation 3,3 V |

---

### Question 10 : Validation du montage électrique

Le câblage a été réalisé hors tension, puis validé avant le raccordement du câble USB.

---

### Question 11 : Test de lecture avec le programme d'exemple

Le téléversement du croquis d'exemple `read_nuid` permet de détecter les badges MIFARE Classic et de valider la communication sur le bus SPI.

Sortie console du moniteur série :
```text
Scan du NUID MIFARE Classic prêt.
PICC type: MIFARE 1KB
Nouveau badge détecté !
NUID (Hex) : F7 CD 8E 62
NUID (Dec) : 247 205 142 98
PICC type: MIFARE 1KB
Nouveau badge détecté !
NUID (Hex) : 29 69 06 B3
NUID (Dec) : 41 105 06 179
```

---


### Question 12 : Relevé et analyse des NUID

* **Identifiants relevés :**
  * Badge 1 : `F7 CD 8E 62`
  * Badge 2 : `29 69 06 B3`
* **Nombre d'octets :** 4 octets (32 bits).
* **Signification du « N » :** *Non-Unique IDentifier*. Face au volume industriel de badges MIFARE produits mondialement, l'espace d'adressage de 32 bits a été saturé : les constructeurs ne garantissent plus l'unicité globale de cet identifiant en sortie d'usine.

---

### Question 13 : Signalisation visuelle par LED

La LED intégrée (GPIO 2) est configurée en sortie dans `setup()`. Dès qu'un badge valide est décodé par `rfid.PICC_ReadCardSerial()`, la broche 2 passe au niveau haut (`HIGH`) pendant 500 ms via `delay(500)`, puis repasse au repos au niveau bas (`LOW`).

---

### Question 14 : IRQ, polling et interruption

* **Signification d'IRQ :** *Interrupt Request* (demande d'interruption).
* **Polling (scrutation) :** le processeur interroge le composant en continu dans la boucle `loop()` pour savoir si une donnée est disponible. Ce mécanisme consomme des cycles CPU et de l'énergie inutilement.
* **Interruption :** le microcontrôleur n'interroge pas le composant. Dès qu'un événement survient (badge détecté), le RC522 envoie un signal matériel sur la broche IRQ, forçant l'ESP32 à exécuter immédiatement une fonction dédiée (ISR).

---

### Question 15 : Avantages de l'IRQ pour le projet COGIP

* **Économie d'énergie :** permet de placer l'ESP32 en mode veille (*light sleep* ou *deep sleep*) et de le réveiller uniquement lors de la présentation d'un badge.
* **Disponibilité CPU et réactivité :** libère les ressources de calcul pour les traitements réseau (Wi-Fi, HTTP) et garantit un temps de réponse immédiat (< 300 ms) sans blocage.

---

### Question 16 : Mise en œuvre d'une interruption sur l'ESP32

* **Résistance de pull-up (`INPUT_PULLUP`) :** polarise l'entrée à 3,3 V au repos pour neutraliser les signaux parasites.
* **Front descendant (`FALLING`) :** déclenche l'interruption dès que la sortie IRQ commute vers 0 V.
* **Fonction `attachInterrupt()` :** associe la broche à une routine de service d'interruption (ISR) marquée `IRAM_ATTR`. L'ISR modifie un drapeau booléen (`volatile bool badgePresent = true`) qui est ensuite traité par la boucle principale.

---

### Question 17 : Rôle du volume Docker et persistance

* **Rôle de `cogip-data` :** ce volume associe le dossier interne du conteneur (`/var/lib/postgresql/data`) à un espace de stockage persistant géré par Docker sur la machine hôte.
* **Conséquence sans volume :** un conteneur étant éphémère par conception, toute suppression du conteneur (`docker rm` ou `docker compose down`) supprimerait définitivement sa couche d'écriture locale. La base PostgreSQL, les salariés déclarés et l'historique des pointages seraient perdus.

---


### Question 18 : Sécurité des mots de passe dans Docker

* **La mauvaise pratique (à éviter) :**  
  Écrire un mot de passe en clair (en dur) directement dans le fichier `docker-compose.yml` (par exemple : `POSTGRES_PASSWORD=mon_mot_de_passe`). Comme ce fichier est destiné à être versionné sur un dépôt Git (comme GitLab), le mot de passe serait exposé à toutes les personnes ayant accès au code source.
* **La bonne pratique (recommandée) :**  
  Utiliser des variables d'environnement stockées dans un fichier externe (généralement nommé `.env`). Le fichier `docker-compose.yml` fait alors appel à une variable (ex: `POSTGRES_PASSWORD=${DB_PASSWORD}`). Il est crucial d'ajouter ensuite ce fichier `.env` dans le fichier `.gitignore` afin qu'il ne soit jamais publié sur le dépôt Git. En production avancée, on utilise des gestionnaires dédiés comme *Docker Secrets*.

---
---

### Question 19 : Modèle de données (Diagramme de classes) et clé primaire

#### 1. Entités et cardinalités
* **Users** (`id` [PK], `nom`, `prenom`, `email`, `actif`)
* **Badges** (`id` [PK], `nuid` [Unique, Index], `user_id` [FK], `actif`)
* **Zones** (`id` [PK], `nom` [Unique], `description`)
* **AccessRight** (`id` [PK], `user_id` [FK], `zone_id` [FK], `Unique(user_id, zone_id)`)
* **AccessLog** (`id` [PK], `nuid`, `user_id` [FK, Nullable], `zone_id` [FK], `timestamp`, `granted`)

*(Insérer le diagramme de classes validé dans le rapport Word).*

#### 2. Justification de la clé primaire du badge
Le NUID n'est pas un bon candidat pour faire office de clé primaire (PK) :
* **Non-unicité absolue :** L'espace d'adressage sur 4 octets ($2^{32}$) a été saturé industriellement par les constructeurs, ne garantissant plus l'unicité mondiale (d'où le terme *Non-Unique IDentifier*). De plus, l'existence de badges à UID modifiable (*magic cards*) permet d'usurper n'importe quelle valeur.
* **Évolutivité de l'architecture :** Utiliser une clé primaire artificielle technique (`id: int`) évite de répercuter d'éventuels changements de technologie de badges (ex. passage à MIFARE DESFire 7 octets) sur toutes les clés étrangères des tables liées (`access_logs`). Le NUID doit donc rester une clé métier candidate indexée (`index=True`) et unique (`unique=True`).

---
### Question 20 : Rôle du fichier requirements.txt

* **Rôle général :**  
  Le fichier `requirements.txt` permet de figer et de centraliser la liste de toutes les dépendances logicielles (bibliothèques tierces) requises pour exécuter l'application Python. Il assure la portabilité et la reproductibilité de l'environnement de développement et de déploiement en permettant une installation automatisée via la commande `pip install -r requirements.txt`.

* **Détail des bibliothèques utilisées dans le projet :**
  * `fastapi` : Framework web asynchrone pour l'exposition de l'API REST et la documentation OpenAPI/Swagger.
  * `uvicorn[standard]` : Serveur web ASGI exécutant l'application FastAPI.
  * `sqlalchemy >= 2.0` : ORM assurant la correspondance entre les objets Python et les tables de la base de données.
  * `psycopg2-binary` : Connecteur (driver) PostgreSQL permettant les échanges réseau avec le conteneur de base de données.
  * `pydantic >= 2` : Bibliothèque de validation, de contrôle de type et de sérialisation des structures de données JSON.

---
### Question 21 : Analyse des contraintes SQLAlchemy dans `models.py`

* **`UniqueConstraint("user_id", "zone_id")` :**  
  Contrainte d'intégrité composite au niveau de la table `access_rights`. Elle garantit l'absence de doublons dans la gestion des droits : un utilisateur ne peut pas posséder deux fois l'autorisation d'accès pour une même zone.

* **`unique=True` sur le champ `nuid` :**  
  Empêche d'insérer deux enregistrements portant le même identifiant matériel de badge dans la table `badges`. Un badge physique donné ne peut ainsi être rattaché qu'à un seul utilisateur à la fois.

* **`index=True` sur le champ `nuid` :**  
  Crée un index (B-Tree) dans la base de données sur la colonne `nuid`. Sachant que chaque pointage interroge la base via cet identifiant pour identifier le salarié, l'index remplace un parcours séquentiel complet (*table scan*) par une recherche logarithmique $O(\log N)$. Cela accélère considérablement la requête SQL et permet de garantir un temps de validation sous les 300 ms.

  ---
### Question 22 : Configuration réseau d'Uvicorn (`0.0.0.0` vs `127.0.0.1`)

* **Raison du choix `0.0.0.0` :**  
  L'adresse `127.0.0.1` restreint les connexions à la boucle locale de l'ordinateur. Le paramètre `--host 0.0.0.0` lie le serveur Uvicorn à l'ensemble des interfaces réseau de la machine hôte. Cela permet à des clients distants sur le réseau local, en particulier l'ESP32 connecté en Wi-Fi, d'atteindre l'API via son adresse IP locale.

* **Risque de sécurité :**  
  L'application et son interface d'administration interactive (`/docs`) sont exposées à l'ensemble des postes présents sur le réseau local sans restriction d'origine, ce qui ouvre la porte aux accès non autorisés et aux attaques par déni de service.

* **Moyens d'atténuation :**  
  1. Configurer une règle de pare-feu pour filtrer les flux entrants sur le port 8000 et restreindre l'accès à l'adresse IP de l'ESP32.
  2. Isoler les flux dans un VLAN dédié aux objets connectés (IoT).
  3. Mettre en place une authentification par clé d'API (ou jeton) dans les en-têtes HTTP et déployer un reverse proxy chiffré en HTTPS.

  ---
### Question 23 : Configuration des entités et des droits d'accès via Swagger

L'initialisation des enregistrements et des autorisations a été effectuée depuis l'interface interactive Swagger UI accessible à l'adresse `http://localhost:8000/docs`.

#### 1. Création des utilisateurs (`POST /api/users`)
* **Utilisateur 1 (salarié autorisé) :**
  * **Corps de la requête :**
    ```json
    {
      "nom": "Dupont",
      "prenom": "Alice",
      "email": "alice.dupont@cogip.fr",
      "actif": true
    }
    ```
  * **Réponse :** Code HTTP `200` — `{"id": 1}`.
* **Utilisateur 2 (salarié non autorisé) :**
  * **Corps de la requête :**
    ```json
    {
      "nom": "Martin",
      "prenom": "Bob",
      "email": "bob.martin@cogip.fr",
      "actif": true
    }
    ```
  * **Réponse :** Code HTTP `200` — `{"id": 2}`.

#### 2. Création de la zone (« Salle serveur ») (`POST /api/zones`)
* **Corps de la requête :**
  ```json
  {
    "nom": "Salle serveur",
    "description": "Local technique et serveurs informatiques"
  }
  ```
* **Réponse :** Code HTTP `200` — `{"id": 1}`.

#### 3. Attribution des badges RFID (`POST /api/badges`)
* **Badge d'Alice Dupont (Badge 1) :**
  * **Corps de la requête :** `{"nuid": "F7CD8E62", "user_id": 1, "actif": true}`
  * **Réponse :** Code HTTP `200` — Badge associé à l'utilisateur `id: 1`.
* **Badge de Bob Martin (Badge 2) :**
  * **Corps de la requête :** `{"nuid": "296906B3", "user_id": 2, "actif": true}`
  * **Réponse :** Code HTTP `200` — Badge associé à l'utilisateur `id: 2`.

#### 4. Attribution des droits d'accès (`POST /api/access-rights`)
* **Droit d'accès pour Alice Dupont :**
  * **Corps de la requête :** `{"user_id": 1, "zone_id": 1}`
  * **Réponse :** Code HTTP `200` — Droit d'accès accordé pour la zone 1.
* **Gestion de l'accès pour Bob Martin :**
  * Conformément au principe du **refus par défaut (*default deny*)**, aucun enregistrement n'a été créé pour l'utilisateur `id: 2`. En l'absence de correspondance dans la table `access_rights`, le système refusera systématiquement l'accès à la « Salle serveur » pour son badge.

  ---
### Question 24 : Vérification du contenu de la base de données PostgreSQL

La persistance des données a été vérifiée directement dans le conteneur Docker à l'aide de l'outil `psql`.

#### 1. Connexion au conteneur
```bash
docker exec -it cogip-db psql -U cogip -d pointeuse
```
---
### Question 25 : Intégration Wi-Fi et transmission HTTP sur l'ESP32

Le programme de l'ESP32 a été enrichi pour intégrer les bibliothèques réseau standard (`WiFi.h` et `HTTPClient.h`) afin de faire communiquer le lecteur RFID avec l'API FastAPI.

#### 1. Organisation du code
* **Bibliothèques ajoutées :** inclusion de `WiFi.h` pour la pile réseau 802.11 et de `HTTPClient.h` pour la gestion du protocole HTTP client.
* **Constantes déclarées :** identification du SSID, de la clé WPA/WPA2, de la ressource ciblée (`Salle serveur`) et de l'URI d'API (`/api/scan`).
* **Initialisation (`setup`) :** établissement de la liaison sans fil via `WiFi.begin()` et attente bloquante jusqu'à l'obtention d'un bail (`WL_CONNECTED`), puis affichage de l'adresse IP locale attribuée.
* **Formatage du NUID (`nuidToString`) :** conversion de chaque octet hexadécimal lu dans `rfid.uid.uidByte` en chaîne de caractères majuscule, avec ajout d'un zéro initial si l'octet est inférieur à `0x10`.
* **Émission HTTP (`loop`) :** à chaque détection réussie, une trame HTTP `POST` est générée avec l'en-tête `Content-Type: application/json` et un corps formaté (`{"nuid":"...", "zone":"Salle serveur"}`). Le code retour serveur (200, 403, 404) et le JSON de réponse sont journalisés sur le port série.

---
### Question 26 : Adresse IP du serveur et politique d'adressage

#### 1. Commande utilisée
* **Sous Linux (VM) :** `hostname -I` (ou `ip a`)
* **Sous Windows (machine hôte en mode NAT avec redirection de port) :** `ipconfig`

#### 2. Nécessité d'une IP fixe ou d'une réservation DHCP
L'URI cible de l'API FastAPI est inscrite en dur dans le microprogramme de l'ESP32 (`API_URL`). Dans un réseau configuré en DHCP dynamique sans assignation pérenne, le serveur d'API peut changer d'adresse IP à l'expiration du bail ou après un redémarrage. Cela entraîne la rupture immédiate des communications HTTP de la pointeuse. 

Une adresse IP fixe (ou une réservation statique dans la table du serveur DHCP liée à l'adresse MAC de l'hôte) est indispensable pour garantir la haute disponibilité du système de contrôle d'accès sans exiger une recompilation du firmware embarqué.

---
### Question 27 : Scénario de recette du système de pointage

Le tableau de recette ci-dessous présente la validation fonctionnelle de l'ensemble de la chaîne de transmission (Lecteur RC522 -> ESP32 -> API FastAPI -> Base PostgreSQL -> Interface Web temps réel) selon les 5 cas de test définis :

| N° | Badge présenté | Zone demandée | Résultat attendu | Résultat obtenu | Validation |
|:--:|:---|:---|:---|:---|:--:|
| **1** | Badge A (`F7CD8E62` - droit accordé) | Salle serveur | Autorisé (Alice Dupont) | Accès accordé (Alice Dupont) | **OK** |
| **2** | Badge inconnu (`A1B2C3D4`) | Salle serveur | Refusé, utilisateur « Inconnu » | Refusé (Badge non déclaré) | **OK** |
| **3** | Badge B (`296906B3` - droit accordé) | Salle serveur | Autorisé (Bob Martin) | Accès accordé (Bob Martin) | **OK** |
| **4** | Badge B désactivé (`actif = false`) | Salle serveur | Refusé | Refusé (Badge inactif / révoqué) | **OK** |
| **5** | Badge A (`F7CD8E62` - droit accordé) | Salle serveur | Autorisé (Alice Dupont) | Accès accordé (Alice Dupont) | **OK** |

#### Observations et validation technique :
* **Détection RFID & Transmission HTTP :** L'identifiant UID/NUID lu par le capteur MFRC522 est correctement formaté en JSON et envoyé via une requête `POST /api/scan` par l'ESP32.
* **Traitement API & Base de données :** FastAPI interroge la table PostgreSQL pour vérifier la validité du badge et les autorisations sur la zone cible avant de renvoyer le code `200 OK`.
* **Supervision temps réel :** La page web (`index.html`) actualise instantanément le nom de l'utilisateur, l'horodatage et le statut d'accès (vert pour accordé, rouge pour refusé).

---
### Question 28 : Capture et analyse protocolaire Wireshark

Une capture réseau a été réalisée sur l'interface Ethernet de la machine hôte lors du passage d'un badge valide afin d'analyser la structure des échanges HTTP.

#### 1. Paramètres de capture
* **Interface écoutée :** Ethernet (liaison filaire RJ45 reliée au routeur)
* **Filtre appliqué :** `http && ip.addr == 192.168.1.36`
* **Hôtes :** ESP32 (`192.168.1.36`) et Serveur API (`192.168.1.5:8000`)

#### 2. Détail des paquets analysés

| Paramètre | Requête (Pointeuse -> API) | Réponse (API -> Pointeuse) |
|:---|:---|:---|
| **Trame** | `POST /api/scan HTTP/1.1` | `HTTP/1.1 200 OK` |
| **Source** | `192.168.1.36` | `192.168.1.5` |
| **Destination** | `192.168.1.5:8000` | `192.168.1.36` |
| **Type MIME** | `application/json` | `application/json` |
| **Payload JSON** | `{"nuid":"F7CD8E62","zone":"Salle serveur"}` | `{"nuid":"F7CD8E62","utilisateur":"Alice Dupont","autorise":true}` |

#### 3. Analyse de sécurité
Les échanges transitent par le protocole HTTP en clair sans chiffrement TLS. Les identifiants matériels des badges (NUID) et les informations nominatives des salariés circulent en texte brut sur le réseau local, ce qui expose l'infrastructure aux risques d'écoute passive (*sniffing*), d'usurpation d'identité et d'attaques par rejeu.

---
---

### Question 29 : Analyse du mécanisme de supervision temps réel (Polling vs WebSocket)

#### 1. Méthode actuellement employée
La supervision repose sur une technique de **scrutation périodique (*Short Polling*)** via l'API standard JavaScript `fetch()` cadencée par un `setInterval()` toutes les secondes (1 000 ms). Le navigateur interroge en boucle le point de terminaison `GET /api/last` pour vérifier la présence d'un nouvel événement.

#### 2. Inconvénients majeurs de cette approche
* **Consommation de bande passante et surcharge serveur :** Requêtes répétitives et redondantes générant un surcoût (*overhead*) d'en-têtes HTTP/TCP même en l'absence de pointage, sollicitant inutilement le serveur ASGI et la base PostgreSQL.
* **Latence intrinsèque :** Le temps de réaction dépend de l'intervalle d'échantillonnage (délai d'affichage pouvant atteindre 1 seconde après le scan physique).
* **Faible passage à l'échelle (*Scalability*) :** La charge serveur croît de manière linéaire avec le nombre de clients connectés ($N$ postes de supervision = $N$ requêtes/seconde).

#### 3. Solution alternative recommandée : WebSocket
Pour optimiser la réactivité et les ressources, la mise en œuvre de **WebSockets (`ws://`)** est préconisée :
* Établissement d'une connexion TCP persistante et bidirectionnelle après négociation HTTP.
* Architecture en mode **Push (événementiel)** : le serveur FastAPI émet la trame JSON vers le navigateur uniquement lorsqu'un badge est scanné sur l'ESP32.
* Suppression totale du trafic réseau au repos et latence de notification quasi instantanée (< 10 ms).

---
### Question 30 : Indisponibilité du serveur et continuité de service

#### 1. Conséquences actuelles en cas de panne serveur
Dans l'architecture actuelle, le microcontrôleur ESP32 agit comme un client totalement dépendant du serveur central :
* **Blocage physique de l'accès :** La décision d'ouverture dépendant entièrement du code retour de l'API, l'échec de la requête HTTP (`code <= 0`) maintient la porte fermée, bloquant le passage des salariés légitimes.
* **Perte définitive des pointages :** L'ESP32 ne disposant d'aucune mémoire tampon (*buffer*), l'événement de badgeage n'est pas sauvegardé et n'apparaîtra jamais dans l'historique `access_logs`.
* **Gel du système (*Timeout*) :** La boucle principale reste bloquée pendant l'attente du délai d'expiration TCP (timeout de la socket HTTP), rendant le lecteur RFID inopérant pour les personnes suivantes pendant plusieurs secondes.
* **Point de défaillance unique (SPOF) :** La disponibilité du contrôle d'accès est tributaire à 100 % du serveur Uvicorn, du conteneur PostgreSQL et du commutateur réseau.

#### 2. Évolutions architecturales proposées

Pour assurer la continuité de service et la résilience du système, une architecture en **mode dégradé autonome (*Store & Forward*)** est préconisée :

1. **Mémoire cache locale des droits (Whitelist embarquée) :**  
   Enregistrer la liste des badges actifs et de leurs droits d'accès directement dans la mémoire Flash de l'ESP32 (système de fichiers **LittleFS** ou partition **NVS**). Cette base locale est mise à jour périodiquement par le serveur. En cas de rupture réseau, l'ESP32 valide l'accès localement en interrogeant son cache.
2. **Journalisation locale et synchronisation différée (*Store & Forward*) :**  
   Si l'API est injoignable, les pointages sont enregistrés dans une file d'attente circulaire sur la mémoire Flash. Dès le rétablissement de la connectivité réseau, l'ESP32 transmet les pointages différés au serveur par paquets (*batch*).
3. **Module d'horodatage RTC autonome (ex. DS3231) :**  
   Intégration d'un module d'horloge temps réel avec batterie de secours sur le bus I²C de l'ESP32 pour garantir un horodatage précis et conforme des pointages même hors connexion.
4. **Redondance de l'infrastructure serveur :**  
   Mise en place d'un cluster d'API redondé derrière un répartiteur de charge (*Load Balancer Nginx*) et réplication de la base PostgreSQL pour éliminer tout point unique de défaillance.

   ---
### Question 31 : Échanges en clair ISO/IEC 14443-A et limites de sécurité du NUID

#### 1. Informations transmises sans protection lors de l'anticollision
Durant la phase d'initialisation et d'anticollision de la norme ISO/IEC 14443-A, l'ensemble des trames circule par voie hertzienne en clair (sans chiffrement ni signature cryptographique) :
* **L'ATQA (*Answer To Request Type A*) :** vecteur de 2 octets indiquant le format de trame et le type de transpondeur.
* **Le NUID / UID (*Card Identifier*) :** identifiant matériel sur 4 octets (`F7CD8E62`) transmis par le badge pour permettre au lecteur de l'isoler parmi d'autres badges présents dans le champ électromagnétique.
* **L'octet BCC (*Bit Collision Check*) :** octet de contrôle de parité (XOR des octets de l'UID).
* **Le SAK (*Select Acknowledge*) :** octet d'état confirmant la sélection du badge et spécifiant le protocole de niveau supérieur pris en charge.

#### 2. Conclusion sur l'usage du NUID pour l'authentification
* **Assimilation à un identifiant public :** Le NUID agit comme un simple numéro de série ou une adresse MAC réseau. C'est une donnée d'identification publique et non un secret d'authentification (mot de passe ou clé privée).
* **Absence de preuve de possession légitime :** Le NUID pouvant être intercepté à distance à l'insu du salarié (via un smartphone NFC ou une antenne d'écoute), son utilisation exclusive n'offre aucune garantie d'authenticité.
* **Inadéquation pour les zones sensibles :** Fonder le contrôle d'accès d'un local sécurisé (salle serveur, coffres) sur la simple lecture du NUID expose le système à une usurpation d'identité immédiate. Une authentification robuste requiert un défi-réponse cryptographique (*challenge-response*) exploitant des secteurs chiffrés (normes MIFARE DESFire EV2/EV3 ou AES-128).