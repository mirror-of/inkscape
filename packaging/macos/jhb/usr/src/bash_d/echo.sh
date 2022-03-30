# SPDX-FileCopyrightText: 2021 René de Hesselle <dehesselle@web.de>
#
# SPDX-License-Identifier: GPL-2.0-or-later

### description ################################################################

# Provide colorful convenience functions for echo.

### shellcheck #################################################################

# shellcheck shell=bash # no shebang as this file is intended to be sourced

### includes ###################################################################

bash_d_include ansi

### variables ##################################################################

# Nothing here.

### functions ##################################################################

function echo_message__
{
  local funcname=$1   # empty if outside function
  local filename=$2
  local type=$3
  local color=$4
  local args=${*:5}

  if [ -z "$funcname" ] || [ "$funcname" = "source" ]; then
    funcname=$(basename "$filename")
  fi

  if ansi_is_usable; then
    echo -e "${color}$type:$ANSI_FG_RESET $args ${ANSI_FG_BLACK_BRIGHT}[$funcname]$ANSI_FG_RESET"
  else
    echo "$type: $args [$funcname]"
  fi
}

### aliases ####################################################################

alias echo_d='>&2 echo_message__ "$FUNCNAME" "${BASH_SOURCE[0]}" "debug" "$ANSI_FG_BLUE_BOLD"'
alias echo_e='>&2 echo_message__ "$FUNCNAME" "${BASH_SOURCE[0]}" "error" "$ANSI_FG_RED_BRIGHT"'
alias echo_i='>&2 echo_message__ "$FUNCNAME" "${BASH_SOURCE[0]}" "info" "$ANSI_FG_BLUE_BRIGHT"'
alias echo_o='>&2 echo_message__ "$FUNCNAME" "${BASH_SOURCE[0]}" "ok" "$ANSI_FG_GREEN_BRIGHT"'
alias echo_w='>&2 echo_message__ "$FUNCNAME" "${BASH_SOURCE[0]}" "warning" "$ANSI_FG_YELLOW_BRIGHT"'

### main #######################################################################

# Nothing here.
