# SPDX-FileCopyrightText: 2021 René de Hesselle <dehesselle@web.de>
#
# SPDX-License-Identifier: GPL-2.0-or-later

### description ################################################################

# Provide an include guard to be used in every script to protect them from
# being sourced multiple times.

### shellcheck #################################################################

# shellcheck shell=bash # no shebang as this file is intended to be sourced

### dependencies ###############################################################

# All of these need to be "or true" because they fail on the first run.

bash_d_include echo   2>/dev/null || true
bash_d_include assert 2>/dev/null || true

### variables ##################################################################

if [ -z "$BASH_D_DIR" ]; then
  BASH_D_DIR=$(dirname "${BASH_SOURCE[0]}")
fi

if [ -z "$BASH_D_FAIL_ON_INCLUDE_ERROR" ]; then
  BASH_D_FAIL_ON_INCLUDE_ERROR=true
fi

### functions ##################################################################

function bash_d_include_all
{
  local includes
  includes=$(mktemp "$TMP/${FUNCNAME[0]}".XXXXXX)

  echo "BASH_D_FAIL_ON_INCLUDE_ERROR=false" >> "$includes"
  for file in "$BASH_D_DIR"/*.sh; do
    echo "bash_d_include $(basename "$file")" >> "$includes"
  done
  echo "BASH_D_FAIL_ON_INCLUDE_ERROR=true" >> "$includes"

  # shellcheck disable=SC1090 # dynamic sourcing on purpose
  source "$includes"
  rm "$includes"
}

function bash_d_is_included
{
  local file=$1

  #shellcheck disable=SC2076 # we want literal match, not regex
  if [[ " ${BASH_D_INCLUDE_FILES[*]} " =~ \
        " $BASH_D_DIR/$(basename -s .sh "$file").sh " ]]; then
    return 0
  else
    return 1
  fi
}

### aliases ####################################################################

# shellcheck disable=SC2142 # too much trickery for shellcheck to digest
alias bash_d_include=\
'declare -a BASH_D_INCLUDE_FILES; '\
'BASH_D_INCLUDE_FILE=$(sed -n ${LINENO}p ${BASH_SOURCE[0]} | awk '"'"'{ print $2 }'"'"'); '\
'BASH_D_INCLUDE_FILE=$BASH_D_DIR/$(basename -s .sh $BASH_D_INCLUDE_FILE).sh; '\
'if [[ " ${BASH_D_INCLUDE_FILES[@]} " =~ " ${BASH_D_INCLUDE_FILE} " ]]; then '\
'  : ; '\
'else '\
'  if  [ "$BASH_D_INCLUDE_FILE" = "$BASH_D_DIR/bash_d.sh" ] && '\
'        [ ${#BASH_D_INCLUDE_FILES[@]} -gt 0 ] || '\
'      source $BASH_D_INCLUDE_FILE; then '\
'    BASH_D_INCLUDE_FILE=$(sed -n ${LINENO}p ${BASH_SOURCE[0]} | awk '"'"'{ print $2 }'"'"'); '\
'    BASH_D_INCLUDE_FILE=$BASH_D_DIR/$(basename -s .sh $BASH_D_INCLUDE_FILE).sh; '\
'    BASH_D_INCLUDE_FILES+=("$BASH_D_INCLUDE_FILE"); '\
'  else '\
'    if alias | grep "echo_e" >/dev/null; then '\
'      echo_e "$BASH_D_INCLUDE_FILE will be unavailable"; '\
'    else '\
'      >&2 echo "error: $BASH_D_INCLUDE_FILE will be unavailable"; '\
'    fi; '\
'    if [ "$BASH_D_INCLUDE_FILE" != "$BASH_D_DIR/bash_d.sh" ] && $BASH_D_FAIL_ON_INCLUDE_ERROR; then '\
'      exit 1; '\
'    fi '\
'  fi '\
'fi '\
'# '

### main #######################################################################

if [ ! -d "$BASH_D_DIR" ]; then
  echo "error: BASH_D_DIR=$BASH_D_DIR is invalid [${BASH_SOURCE[0]}]"
  exit 1
fi

shopt -s expand_aliases

bash_d_include bash_d   # Need to rerun so complete setting us up.
