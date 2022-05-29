
SCHREINER Tao, Falguerolle Louis, Prostakov Andrey

README
======

Pour monter une partition ouichefs :

- Créer une image vide

    dd if=/dev/zero of=test.img bs=1M count=50

- La formatter à l'aide de l'utilitaire mkfs.ouichefs

    ./mkfs.ouichefs ./test.img

- La rajouter dedans les paramètres de QEMU :

    HDC="-drive file=%dir%/test.img,format=raw"

- Le reste dedans QEMU :
    - Charger le module ouichefs :
    
        insmod /share/ouichefs.ko

    - Créer un dossier dans lequel sera montée l'image :

        mkdir /wish

    - Monter la partition :

        mount -t ouichefs /dev/sdc /wish

    - Lier le péripherique avec le driver :
    
        mknod /dev/ouichefs_ioctl c 248 0



Étape 1 : écriture
------------------

* Chaînage des blocs d'index

---RÉPONSES---

* Modification des fonctions d'écriture

---RÉPONSES---


Étape 2 : utilitaire de déboguage
---------------------------------

* Fichier du debugfs fonctionnel

On a mis à jour le init du module ouichefs pour qu'à l'insértion du module un
fichier debugfs "ouichefs" soit créé dans /sys/kernel/debug. C'est ce fichier-
là qui contient le tableau avec toutes les informations sur les modifications
des ficheirs de la partition /wish dont les colonnes sont "inodes - versions - 
block hist". 
Le nombre d'inodes correspond au nobre de fichiers dans la 
partition +1 qui est l'inode du fichier "ouichefs".
la colonne "block hist" contient la liste des blocks qui contiennent une
version d'un fichier pour un inode donné (on rappelle qu'un nouveau block_index
est créé à chaque nouvelle écriture dans un fichier).

Le fichier de test exo2.sh permet de tester toutes les fonctionnalités
implémentées à cette étape.


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

