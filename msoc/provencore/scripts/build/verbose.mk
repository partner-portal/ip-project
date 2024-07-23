#
# Copyright (c) 2017-2023, ProvenRun and/or its affiliates. All rights reserved.
#

# Verbosity levels
# 0: only warnings and errors are displayed
# 1: command summary only (default)
# 2: executed commands are shown

ifdef V
	VERBOSE = $(V)
else
	VERBOSE = 1
endif

ifeq "$(VERBOSE)" "0"
	Q=@
	ECHO_OUTPUT=>/dev/null
	LATEX_OUTPUT=>/dev/null
	LATEX_QUIET=-q
	CARGO_QUIET=--quiet
else ifeq "$(VERBOSE)" "1"
	Q=@
	ECHO_OUTPUT=
	LATEX_OUTPUT=>/dev/null
	LATEX_QUIET=-q
	CARGO_QUIET=--quiet
else
	Q=
	ECHO_OUTPUT=
	LATEX_OUTPUT=
	LATEX_QUIET=
	CARGO_QUIET=--verbose
endif
