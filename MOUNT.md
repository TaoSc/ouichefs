Monter une partition OUICHEFS
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
