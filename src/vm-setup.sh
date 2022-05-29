#! /bin/bash

cp /usr/data/sopena/pnl/pnl-tp-2021.img /tmp/
tar -xvJf /usr/data/sopena/pnl/linux-5.10.17.tar.xz -C /tmp/
cd /tmp/linux-5.10.17
make nconfig
\cp /users/Etu1/21113691/pnl/config-TP-05 /tmp/linux-5.10.17/.config
make -j 12
ln -s ${HOME}/pnl/pnl-config-linux-5.10.17 /tmp/linux-5.10.17/.config
