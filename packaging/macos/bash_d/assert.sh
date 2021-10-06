# SPDX-FileCopyrightText: 2021 René de Hesselle <dehesselle@web.de>
#
# SPDX-License-Identifier: GPL-2.0-or-later

### description ################################################################

# This file provides a mechanism to restrict sourcing a script to specific
# platforms.

### shellcheck #################################################################

# shellcheck shell=bash # no shebang as this file is intended to be sourced

### dependencies ###############################################################

bash_d_include echo

### variables ##################################################################

ASSERT_RC=0

### functions ##################################################################

# Nothing here.

### aliases ####################################################################

# Bash version
alias assert_bash4_or_above='[ ${BASH_VERSINFO[0]} -lt 4 ] && echo_e "$(basename ${BASH_SOURCE[0]}) will be unavailable (depends on bash >= 4)" && return $LINENO || true'

# Git
alias assert_git='[ ! -x "$(command -v git)" ] && echo_e "$(basename ${BASH_SOURCE[0]}) will be unavailable (depends on git)" && return $LINENO || true'

# Unix platforms
alias assert_darwin='[ "$(uname)" != "Darwin" ] && echo_e "Darwin required" && return $LINENO || true'
alias assert_linux='[ "$(uname)" != "Linux" ] && echo_e "Linux required" && return $LINENO || true'

alias assert_ok='ASSERT_RC=$?; [ $ASSERT_RC -ne 0 ] && echo_e "assert_ok rc=$ASSERT_RC" && return $LINENO'

### main #######################################################################

# Nothing here.
