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
  