# Makefile -- makefile for calendar.
# Author: Luis Colorado <luis.colorado@ericsson.com>
# Date: Tue Apr  7 14:46:26 EEST 2015

targets=calendar

all: $(targets)
clean:
	$(RM) $(targets) $(foreach i, $(targets), $($(i)_objs))

.PHONY: all clean

calendar_objs=calendar.o easter/easter.o
calendar_libs=
calendar: $(calendar_objs)
	$(CC) $(LDFLAGS) -o $@ $(calendar_objs) $(calendar_libs)

easter/easter.o:
	$(MAKE) -C easter easter.o
