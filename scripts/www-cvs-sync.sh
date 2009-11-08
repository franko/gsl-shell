#!/bin/bash

BASE_DIR="$HOME/sviluppo/gsl-shell"
#BASE_DIR="."
WWW_DIR="$BASE_DIR/www/gsl-shell"
SRC_DIR="$BASE_DIR/doc/html"

SRCLST=`find $SRC_DIR -type f -a -not -path '*/.doctree*' -a -not -iname '.buildinfo'`

ADDLST=()
FULLLST=()
for NM in $SRCLST; do
    DNM=${NM/doc\/html/www\/gsl-shell}
    RDNM=`echo $DNM | awk '{gsub(/^.*www\/gsl-shell\//, ""); print $0 }'`
    if test ! -f $DNM; then
        ADDLST[${#ADDLST[@]}]=$RDNM
	FULLLST[${#FULLLST[@]}]=$RDNM
    else
        cmp -s "$NM" "$DNM"
        if test $? -eq 1; then
           FULLLST[${#FULLLST[@]}]=$RDNM
        fi
    fi
done

WWWLST=`find $WWW_DIR -type f -a -not -path '*CVS*'`

RMLST=()
for NM in $WWWLST; do
    SNM=${NM/www\/gsl-shell/doc\/html}
    if test ! -f $SNM; then
        RMLST[${#RMLST[@]}]=`echo $NM | awk '{gsub(/^.*www\/gsl-shell\//, ""); print $0 }'`
    fi;
done

echo "Files to copy: "
for NM in ${FULLLST[@]}; do
  echo $NM  
done

echo "Files to add:"
for NM in ${ADDLST[@]}; do
  echo $NM  
done

echo "Files to remove:"
for NM in ${RMLST[@]}; do
  echo $NM  
done

echo -n "Commit the changes [Yes/No] ? "
read ANSWER

if test "${ANSWER:-no}" = "Yes" -o "${ANSWER:-no}" = "yes"; then
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
else
  echo "Changes not done"
fi
