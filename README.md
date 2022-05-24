
SCHREINER Tao, Falguerolle Louis, Prostakov Andrey

README
======

Pour monter une partition ouichefs :

- Créer une image vide

    dd if=/dev/zero of=test.img bs=1M count=50

- La formatter à l'aide de l'utilitaire mkfs.ouichefs

    mkfs.ouichefs ./test.img

- La rajouter dedans les paramètres de QEMU :

    HDC="-drive file=%dir%/test.img,format=raw"

- Le reste dedans QEMU :
-- Charger le module ouichefs :
    
    insmod /share/ouichefs.ko

-- Créer un dossier dans lequel sera montée l'image :

    mkdir /disk

-- Monter la partition :

    mount -t ouichefs /dev/sdc disk


Étape 1 : écriture
------------------

* Chaînage des blocs d'index

---RÉPONSES---

* Modification des fonctions d'écriture

---RÉPONSES---


Étape 2 : utilitaire de déboguage
---------------------------------

* Fichier du debugfs fonctionnel

---RÉPONSES---


Étape 3 : vue courante
----------------------

* Modification de la structure ouichefs_inode

---RÉPONSES---

* Requête ioctl changement vue courante

---RÉPONSES---

* Modification des fonctions d'écriture

---RÉPONSES---


Étape 4 : restauration
----------------------

* Requête ioctl restauration

---RÉPONSES---

* Blocs libérés utilisables

---RÉPONSES---


Étape 5 : déduplication
-----------------------

* Modification des fonctions d'écriture

---RÉPONSES---

* Libération synchrone

---RÉPONSES---

* Libération asynchrone

---RÉPONSES---

