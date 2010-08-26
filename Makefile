#
# Top level Makefile
#
# vim: noet sts=8 sw=8

BASE ?= /usr/local

# Installed Buddy paths:
BDD_BASE    ?= $(BASE)
BDD_INCLUDE ?= $(BDD_BASE)/include
BDD_LIBDIR  ?= $(BDD_BASE)/lib

# STS library install paths:
STSLIB_BASE    ?= $(BASE)
STSLIB_INCLUDE ?= $(STSLIB_BASE)/include
STSLIB_LIBDIR  ?= $(STSLIB_BASE)/lib

STSLIB_CFLAGS = -Wall -I$(BDD_INCLUDE) -L$(BDD_LIBDIR)

# Applications paths:
STSAPP_BASE ?= $(BASE)
STSAPP_BIN  ?= $(STSAPP_BASE)/bin

STSAPP_CFLAGS = -Wall -I$(STSLIB_INCLUDE) -I$(BDD_INCLUDE) \
	-L$(STSLIB_LIBDIR) -L$(BDD_LIBDIR)

STSAPP_APPLICATIONS = nbc project simsup sync

##############

.PHONY: all stslib applications clean

all: applications

stslib:
	$(MAKE) -C STSLib install \
		CFLAGS="$(STSLIB_CFLAGS)" \
		INCLUDE="$(STSLIB_INCLUDE)" \
		LIB="$(STSLIB_LIBDIR)"

applications: stslib
	test -d $(STSAPP_BIN) || mkdir $(STSAPP_BIN)
	for p in $(STSAPP_APPLICATIONS);\
	do $(MAKE) -C applications/$$p install \
			CFLAGS="$(STSAPP_CFLAGS)" \
			STSAPP_BIN="$(STSAPP_BIN)";\
	done

clean:
	$(MAKE) -C STSLib clean
	for p in $(STSAPP_APPLICATIONS);\
	do $(MAKE) -C applications/$$p clean;\
	done
