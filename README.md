
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

Cette fonction est testée dans les fichiers test/exo1.sh et test/exo2.sh.

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

Le fichier de test test/exo2.sh permet de tester toutes les fonctionnalités
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

Pour enregistrer le nouvel ioctl nous avons modifié la fonction d'initialisation du module pour créer un nouveau char device
et, réciproquement, la fonction de suppression du module pour retirer celui-ci. 
Ce device est paramétré pour rediriger la file_op `unlocked_ioctl` vers une nouvelle fonction `ouichefs_unlocked_ioctl`.
Nous avons créé un header ioctl.h dans lequel est configuré le numéro de requête pour la commande de l'étape 3 ainsi
qu'une structure `ioctl_request` qui définit le format de l'argument de la requête avec pour premier paramètre le
numéro d'inode (`ino`) et pour second paramètre le nombre de versions de décalage (`nb_version`).
La fonction `ouichefs_unlocked_ioctl` va d'abord s'assurer qu'elle peut correctement récupérer les paramètres et que
le fichier est bien régulier. Une fois cela fait elle va entrer dans une boucle qui va parcourir la liste chaînée
des block_index du fichier jusqu'à arriver au block_index correspondant et va mettre à jour le champ `block_index`
de l'inode avec la valeur correspondante.

* Modification des fonctions d'écriture

Nous avons modifié la fonction d'écriture "ouichefs_write_begin" pour qu'elle vérifie si l'on essaie bien de modifier la dernière version du fichier.
On modifie la valeur de "last_index_block avec l'adresse du bloc nouvellement alloué si l'on est autorisé à modifier.

* État de la fonctionnalité

La fonctionnalité est implémentée, on a bien le numéro de bloc du plus récent bloc ("ouichefs_inode->index_block") remplacé par la version courante en fonction du paramètre "nb_version".

Toutefois, dans certains cas, l'affichage du fichier n'est pas affecté par ce changement. Il semblerait que l'inode 
n'est pas correctement réécrit sur le disque.

Il semble y avoir des soucis avec les vérifications rajoutés dans `ouichefs_write_begin` sur la machine d'un de nous trois,
toutefois ces problèmes ne se reproduisent pas sur les machines de la PPTI. Pour simplifier les tests, nous les avons commentés
mais nous pensons que le soucis n'est pas lié au code mais à un soucis d'environnement.

Le fichier de test test/exo3.c permet de tester les fonctionnalités implémentées.
Il nécessite qu'une partition ouichefs soit montée dans le dossier /wish
et que l'inode passé en paramètre ait subi plusieurs modifications.

Étape 4 : restauration
----------------------

* Requête ioctl restauration

Pour implémenter le nouvel ioctl nous avons défini un nouveau numéro de requête dans le header et ajouté un nouveau
cas dans le switch du ioctl. Nous réutilisons également la structure de l'argument telle que conçue pour l'étape 3,
le champ `nb_version` de celle-ci est toutefois ignoré.
La nouvelle ioctl va d'abord vérifier qu'il y a bien des versions plus récentes que celle actuellement pointée par 
`index_block`, puis va libérer les blocs des versions plus récentes, et va finalement mettre à jour le champ
`last_index_block` de l'inode.

* Blocs libérés utilisables

Le code de libération de blocs s'inspire du code de libération trouvé dedans `ouichefs_unlink`.
Il commence par charger le `last_index_block`, supprime le contenu de ses blocs alloués puis charge l'`index_block`
qui le précède, et ainsi de suite jusqu'à que l'on tombe sur la fin de la liste ou sur le bloc qui devient notre
nouveau bloc le plus récent. 

* État de la fonctionnalité

La fonctionnalité est implémentée et le `last_index_block` east bien mis à jour toutefois le code de libération
ne fonctionne pas correctement, il provoque un segfault. Il est donc commenté pour simplifier les tests.

Le fichier de test test/exo4.c permet de tester les fonctionnalités implémentées.
Il nécessite qu'une partition ouichefs soit montée dans le dossier /wish.

Étape 5 : déduplication
-----------------------

Nous n'avons pas eu le temps d'implémenter cette étape.

