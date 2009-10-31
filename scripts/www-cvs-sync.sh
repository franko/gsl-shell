#!/bin/bash

BASE_DIR="$HOME/sviluppo/gsl-shell"
#BASE_DIR="."
WWW_DIR="$BASE_DIR/www/gsl-shell"
SRC_DIR="$BASE_DIR/doc/html"

SRCLST=`find $SRC_DIR -type f -a -not -path '*/.doctree*' -a -not -iname '.buildinfo'`

ADDLST=()
FULLLST=()
for NM in $SRCLST; do
    DNM=`echo $NM | awk '{gsub(/doc\/html/, "www/gsl-shell"); print $0 }'`
    echo "Treating: $NM"
    RDNM=`echo $DNM | awk '{gsub(/^.*www\/gsl-shell\//, ""); print $0 }'`
    if test ! -f $DNM; then
        ADDLST[${#ADDLST[@]}]=$RDNM
    fi
    FULLLST[${#FULLLST[@]}]=$RDNM
done

WWWLST=`find $WWW_DIR -type f -a -not -path '*CVS*'`

RMLST=()
for NM in $WWWLST; do
    SNM=`echo $NM | awk '{gsub(/www\/gsl-shell/, "doc/html"); print $0 }'`
    echo "Treating: $NM"
    if test ! -f $SNM; then
        RMLST[${#RMLST[@]}]=`echo $NM | awk '{gsub(/^.*www\/gsl-shell\//, ""); print $0 }'`
    fi;
done

cd www/gsl-shell
for NM in ${FULLLST[@]}; do
    cp "$SRC_DIR/$NM" "$NM"
done
if test ${#ADDLST[@]} -gt 0; then
    cvs add "${ADDLST[@]}"
fi
if test ${#RMLST[@]} -gt 0; then
    rm "${RMLST[@]}"
    cvs remove "${RMLST[@]}"
fi
cd -
