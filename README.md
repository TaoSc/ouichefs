
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

Pour chainer les blocs d'index de façon à créer un historique, nous avons utilisé les cases du tableau de blocs "blocks" de la structure "ouichefs_file_index_block" (-> l'attribut b_data de la structure "buffer_head"). A chaque écriture, nous stockons dans l'ancien bloc, la valeur du nouveau bloc à la dernière case; et dans le nouveau bloc, la valeur de l'ancien bloc à l'avant derniere case tout en mettant la case reservée pour le nouveau bloc à -1 (on l'utilise pour s'arreter lorsque l'on veut itérer sur l'historique). Cela va nous permettre de constituer une liste doublement chainée. On modifie donc l'attribut "index_block" contenu dans la structure "ouichefs_inode" pour qu'elle pointe toujours vers la derniere version.

* Modification des fonctions d'écriture

Nous avons surtout modifié la fonction "ouichefs_write_begin" qui s'occupe d'allouer les blocs nécessaires à l'écriture. Au debut de la fonction, nous recuperons l'inode associé au fichier cible et le superblock associé à l'inode. Ils vont nous permettre de deduire le numero de bloc qui contient l'inode et ainsi recuperer le tableau de blocs de l'ancien bloc (son index est contenu dans la structure "ouichefs_inode"). Ensuite, nous recuperons un numero de bloc non utilisé graçe à la fonction "get_free_block", on pourra ainsi recuperer le tableau de blocs associé au nouveau bloc. On va donc mettre à jour ces tableaux comme énoncé lors du chainage des blocs d'index. A la fin, nous déclarons "dirty" les structures "buffer_head" et "inode", pour repercuter les changements effectués sur le disque.


Étape 2 : utilitaire de déboguage
---------------------------------

* Fichier du debugfs fonctionnel

On a mis à jour le init du module ouichefs pour qu'à l'insértion du module un fichier debugfs "ouichefs" soit créé dans /sys/kernel/debug. C'est ce fichier-
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

Nous avons ajouté un parametre uint32_t last_index_block dans la structure ouichefs_inode qui va nous permettre de toujours pointer vers la derniere version du block. Ce changement a aussi été repercuté dans la structure similaire contenue dans le fichier mkfs-ouichefs.c.

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

