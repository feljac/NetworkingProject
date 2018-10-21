#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
PURPLE='\033[1;35m'
NC='\033[0m' # No Color
size=(0 512 513 131072 133584 262144 265656)

for j in 0 5 15 30
do
echo -e "${PURPLE}Série de tests, taux de perte : $j%!${NC}"
for i in "${size[@]}"
do
# cleanup d'un test précédent
rm -f received_file input_file

# Fichier au contenu aléatoire de i octets
dd if=/dev/urandom of=input_file bs=1 count=$i &> /dev/null

# On lance le simulateur de lien avec $j% de pertes et un délais de 50ms et un taux de troncation de 5%
./LINGI1341-linksim-master/link_sim -p 1341 -P 2456 -l $j -d 50 -c 5 -R  &> link.log &
link_pid=$!

# On lance le receiver et capture sa sortie standard
./receiver -f received_file :: 2456  2> receiver.log &
receiver_pid=$!

cleanup()
{
    kill -9 $receiver_pid
    kill -9 $link_pid
    exit 0
}
trap cleanup SIGINT  # Kill les process en arrière plan en cas de ^-C

# On démarre le transfert
if ! ./sender ::1 1341 < input_file 2> sender.log ; then
  echo -e "${RED}Crash du sender!${NC}"
  cat sender.log
  err=1  # On enregistre l'erreur
fi

sleep 1 # On attend 1 seconde que le receiver finisse

if kill -0 $receiver_pid &> /dev/null ; then
  echo -e "${RED}Le receiver ne s'est pas arreté à la fin du transfert!${NC}"
  kill -9 $receiver_pid
  err=1
else  # On teste la valeur de retour du receiver
  if ! wait $receiver_pid ; then
    echo -e "${RED}Crash du receiver!${NC}"
    cat receiver.log
    err=1
  fi
fi

# On arrête le simulateur de lien
kill -9 $link_pid &> /dev/null

# On vérifie que le transfert s'est bien déroulé
if [[ "$(md5sum input_file | awk '{print $1}')" != "$(md5sum received_file | awk '{print $1}')" ]]; then
  echo -e "${RED}Le transfert a corrompu le fichier!${NC}"
  echo -e "${RED}Diff binaire des deux fichiers: (attendu vs produit)${NC}"
  diff -C 9 <(od -Ax -t x1z input_file) <(od -Ax -t x1z received_file)
  exit 1
else
  echo -e "${GREEN}Le transfert est réussi!${NC} : $i octets"
fi
done
done
exit ${err:-0}  # En cas d'erreurs avant, on renvoie le code d'erreur
