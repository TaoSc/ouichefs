
Schreiner Tao, Falguerolle Louis, Prostakov Andrey

README
======

Étape 1 : écriture
------------------

* Chaînage des blocs d'index

Pour chaîner les blocs d'index de façon à créer un historique, nous avons 
utilisé les deux dernières cases du tableau de blocs "blocks" de la structure 
"`ouichefs_file_index_block`" de façon à créer une liste doublement chaînée.
À chaque écriture, nous créons un nouveau `index_block` et allouons un nombre de blocs
suffisant pour accueillir le contenu du fichier nouvellement modifié.
Nous stockons dans la dernière case de l'ancien `index_block` le numéro de bloc dans
lequel sera rangé le nouvel `index_block` ; et réciproquement dans l'avant-dernière
case de ce dernier. Pour s'assurer que la liste s'arrête bien à un moment, la valeur
-1 est utilisée pour signifier qu'on a atteint son dernier élement.
Nous modifions finalement l'attribut "index_block" contenu dans la structure 
"`ouichefs_inode`" pour qu'elle pointe toujours vers la dernière version.

* Modification des fonctions d'écriture

Nous avons surtout modifié la fonction "ouichefs_write_begin" qui s'occupe 
d'allouer les blocs nécessaires à l'écriture. Au début de la fonction, nous 
récupérons l'inode associé au fichier cible et le superblock associé à l'inode.
Ils vont nous permettre de déduire le numéro de bloc qui contient l'inode et
ainsi récupérer le tableau de blocs de l'ancien `index_block` (valeur contenue 
dans la structure "`ouichefs_inode`"). Ensuite, nous récupérons un numéro de bloc
non utilisé grâce à la fonction "get_free_block", on pourra ainsi commencer à
écrire dans le tableau de blocs associé au nouvel `index_block`.
On va donc mettre à jour ces tableaux comme énoncé lors du chainage des blocs d'index.
À la fin, nous déclarons "dirty" les "buffer_head" et "inode" que nous avons modifié,
pour répercuter les changements effectués sur le disque et nous utilisons la fonction
"brelse" pour relâcher les pointeurs sur les structures "buffer_head".

* État de la fonctionnalité

La fonctionnalité a été implementée et est fonctionnelle, seulement, 
nous avons eu quelques difficultés sur l'enregistrement des informations dans les blocs.
En effet, lorsque l'on modifie un fichier plusieurs fois à la suite, 
on perd, de façon aléatoire et non fréquente, notre historique. Cela signifie 
que la variable "index_block" de la structure ouichefs_inode pointe vers une ancienne version.
Un problème similaire apparaît lorsque l'on unmount et re-mount une partition, cela
indique qu'un des blocs n'est pas correctement réécrit sur disque.

Cette fonction est testée dans les fichiers exo1.sh et exo2.sh.

Étape 2 : utilitaire de déboguage
---------------------------------

* Accès à l'état du système de fichier grâce au debugfs

Dans le fichier "fs.c", nous avons mis à jour la fonction d'initialisation du module pour
qu'à l'insertion, un fichier de debug "ouichefs" soit créé dans le dossier "/sys/kernel/debug". 
Nous avons aussi modifié la fonction de sortie pour qu'elle supprime le fichier. 
C'est ce fichier qui contient toutes les informations sur les modifications des fichiers de la partition /wish. 
Le tableau est composé des colonnes "inode", "version" et "block history".
La première correspond aux numéros d'inode attribués aux fichier.
La seconde correspond aux numéros de version des fichiers (nombre de fois que l'on a modifié ces fichiers).
La dernière correspond à une liste des blocs contenant les index_blocks des différentes
versions d'un fichier (un nouveau bloc étant alloué à chaque nouvelle écriture dans un fichier).

Ici, on parcourt la liste d'inodes "i_sb_list" contenue dans la `struct dentry` associée au mount point
de la partition. Pour la récupérer nous avons modifié la fonction "ouichefs_mount" afin de rendre cette valeur globale. 
Ensuite, on récupère les informations nécessaires provenant des "struct inode"
et on parcourt leur liste doublement chaînée d'`index_blocks` pour trouver les blocs associés à l'historique.

* État de la fonctionnalité

La fonctionnalité est implementée, on le vérifie avec "cat /sys/kernel/debug/ouichefs".
On peut observer que lorsqu'un fichier est supprimé, que l'on retire le module, et que l'on ré-insére celui-ci,
l'affichage peut parfois être incohérent.
Ceci est une conséquence du problème que nous avons mentionné à l'étape précedente et nouus n'avons pas
constaté de problèmes spécifiques à l'étape 2.

Le fichier de test exo2.sh permet de tester toutes les fonctionnalités
implémentées à cette étape.

Étape 3 : vue courante
----------------------

* Modification de la structure ouichefs_inode

Nous avons ajouté une entrée "`uint32_t last_index_block`" dans la structure "ouichefs_inode" qui va nous permettre de
toujours pointer vers la dernière version du bloc. Ce changement a aussi été répercuté dans le fichier "mkfs-ouichefs.c"
pour pouvoir réinitialiser l'image disque.
Nous ne donnons pas de valeur par défaut à cette variable, ce qui fait que par défaut, lorsqu'aucune écriture n'a eu
lieu, elle possède la valeur 0.

* Requête ioctl changement vue courante

Pour enregistrer le nouvel ioctl nous avons modifié la fonction d'initialisation du module pour créer un nouveau device et, réciproquement, le 

* Modification des fonctions d'écriture

Nous avons modifié la fonction d'écriture "ouichefs_write_begin" pour qu'elle vérifie si l'on essaie bien de modifier la dernière version du fichier.
On modifie la valeur de "last_index_block avec l'adresse du bloc nouvellement alloué si l'on est autorisé à modifier.

* Etat de la fonctionnalité

La fonctionnalité est implementée, on a bien le numero de bloc du plus récent bloc ("ouichefs_inode->index_block") remplacé par la version courante en fonction du paramètre "nb_version".

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

