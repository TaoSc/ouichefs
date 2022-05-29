
SCHREINER Tao, Falguerolle Louis, Prostakov Andrey

README
======

Pour monter une partition ouichefs :

- Créer une image vide

    dd if=/dev/zero of=test.img bs=1M count=50

- La formatter à l'aide de l'utilitaire mkfs.ouichefs

    ./mkfs.ouichefs ./test.img

- La rajouter dans les paramètres de QEMU :

    HDC="-drive file=%dir%/test.img,format=raw"

- Le reste dans QEMU :
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

Pour chainer les blocs d'index de façon à créer un historique, nous avons 
utilisé les cases du tableau de blocs "blocks" de la structure 
"ouichefs_file_index_block" (-> l'attribut b_data de la structure 
"buffer_head"). A chaque écriture, nous créons un nouveau bloc, nous stockons dans l'ancien bloc, la valeur 
du nouveau bloc à la dernière case; et dans le nouveau bloc, la valeur de 
l'ancien bloc à l'avant dernière case tout en mettant la case réservée au
nouveau bloc à -1 (on l'utilise pour s'arrêter lorsque l'on veut itérer sur 
l'historique). Cela va nous permettre de constituer une liste doublement 
chainée. Nous modifions donc l'attribut "index_block" contenu dans la structure 
"ouichefs_inode" pour qu'elle pointe toujours vers la dernière version.

* Modification des fonctions d'écriture

Nous avons surtout modifié la fonction "ouichefs_write_begin" qui s'occupe 
d'allouer les blocs nécessaires à l'écriture. Au début de la fonction, nous 
récupérons l'inode associé au fichier cible et le superblock associé à l'inode.
Ils vont nous permettre de déduire le numéro de bloc qui contient l'inode et
ainsi récupérer le tableau de blocs de l'ancien bloc (son index est contenu 
dans la structure "ouichefs_inode"). Ensuite, nous récupérons un numéro de bloc
non utilisé grâce à la fonction "get_free_block", on pourra ainsi récupérer le
tableau de blocs associés au nouveau bloc. On va donc mettre à jour ces
tableaux comme énoncé lors du chainage des blocs d'index. A la fin, nous
déclarons "dirty" les structures "buffer_head" et "inode", pour répercuter les
changements effectués sur le disque et nous utilisons la fonction "brelse"
pour relâcher les pointeurs sur les structures "buffer_head".

* État de la fonctionnalité

La fonctionnalité a été implementée et est fonctionnelle, seulement, 
nous avons eu quelques difficultés sur l'enregistrement des informations dans les blocs.
En effet, lorsque l'on modifie un fichier plusieurs fois à la suite, 
on perd, de façon aleatoire et non fréquente, notre historique. Cela signifie 
que la variable "index_block" de la structure ouichefs_inode pointe vers une ancienne version.

Cette fonction est testée dans les fichiers test/exo1.sh et test/exo2.sh.

Étape 2 : utilitaire de déboguage
---------------------------------

* Fichier du debugfs fonctionnel

Dans le fichier "fs.c", nous avons mis à jour la fonction d'initialisation du module pour qu'à l'insertion,
un fichier de debug "ouichefs" soit créé dans le dossier "/sys/kernel/debug". 
Nous avons aussi modifié la fonction de sortie pour qu'elle supprime le fichier. 
C'est ce fichier-là qui contient toutes les informations sur les modifications des fichiers de la partition /wish. 
Le tableau est composé des colonnes "inode", "version" et "block history".
La première correspond aux numéros d'inode attribués aux fichier.
La seconde correspond aux numéros de version des fichiers (nombre de fois que l'on a modifié ces fichiers).
La derniere correspond à une liste des blocks contenant les differentes
versions d'un fichier (on rappelle qu'un nouveau bloc
est créé à chaque nouvelle écriture dans un fichier).

Ici, on parcourt la liste d'inode "i_sb_list" contenue dans la "struct dentry mount_point".
Nous avons donc du modifié la fonction "ouchefs_mount" pour recuperer la "struct dentry" associée
au dossier sur lequel on a monté le systeme de fichiers. 
Ensuite, on recupere les informations necessaires provenant des "struct inode"
et on parcourt leur liste de blocs pour trouver les blocs associés à l'historique.

* État de la fonctionnalité

La fonctionnalité est implementée, on verifie avec "cat /sys/kernel/debug/ouichefs".
On peut observer que lorsqu'on retire le module, que le fichier est supprimé, et que l'on insere le module,
les inodes des fichiers toujours existants n'est plus présent dans la table. La ligne de chaque fichier réapparait lorsqu'on écrit à l'interieur.
Le probleme cité à l'etape précedente est bien retranscrit dans le fichier.

Le fichier de test test/exo2.sh permet de tester toutes les fonctionnalités
implémentées à cette étape.


Étape 3 : vue courante
----------------------

* Modification de la structure ouichefs_inode

Nous avons ajouté un parametre "uint32_t last_index_block" dans la structure "ouichefs_inode" qui va nous permettre de toujours pointer vers la derniere version du bloc. Ce changement a aussi été repercuté dans la structure similaire contenue dans le fichier "mkfs-ouichefs.c" pour pouvoir réinitialiser l'image disque.

* Requête ioctl changement vue courante

---RÉPONSES---

* Modification des fonctions d'écriture

Nous avons modifié la fonction d'ecriture "ouichefs_write_begin" pour qu'elle verifie si l'on essaie bien de modifier la derniere version du fichier.
On modifie la valeur de "last_index_block avec la derniere version du fichier.

* État de la fonctionnalité

La fonctionnalité est implementée, on a bien le numero de bloc du plus recent bloc ("ouichefs_inode->index_block") remplacé par la version courante en fonction du paramètre "nb_version".

Étape 4 : restauration
----------------------

* Requête ioctl restauration

---RÉPONSES---

* Blocs libérés utilisables

---RÉPONSES---

* État de la fonctionnalité


Étape 5 : déduplication
-----------------------

* Modification des fonctions d'écriture

---RÉPONSES---

* Libération synchrone

---RÉPONSES---

* Libération asynchrone

---RÉPONSES---

