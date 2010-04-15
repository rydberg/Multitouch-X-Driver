VERSION = 1
PATCHLEVEL = 0
EXTRAVERSION = alpha1

LIBRARY	= multitouch.so
FDIS	= 11-multitouch.fdi
MODULES = match src

o_match	= match

o_src	= capabilities \
	iobuffer \
	hwdata \
	state \
	mtouch \
	gestures \
	multitouch

TARGETS	= $(addsuffix /test,$(MODULES))

OBJECTS	= $(addsuffix .o,\
	$(foreach mod,$(MODULES),\
	$(addprefix $(mod)/,$(o_$(mod)))))

TBIN	= $(addprefix bin/,$(TARGETS))
TLIB	= $(addprefix obj/,$(LIBRARY))
TOBJ	= $(addprefix obj/,$(addsuffix .o,$(TARGETS)))
TFDI	= $(addprefix fdi/,$(FDIS))
OBJS	= $(addprefix obj/,$(OBJECTS))
LIBS	= -lX11 -lpixman-1

DLIB	= usr/lib/xorg/modules/input
DFDI	= usr/share/hal/fdi/policy/20thirdparty

INCLUDE = -I. -I/usr/include/xorg -I/usr/include/pixman-1
OPTS	= -O3 -fPIC

.PHONY: all clean
.PRECIOUS: obj/%.o

all:	$(OBJS) $(TLIB) $(TOBJ) $(TBIN)

bin/%:	obj/%.o
	@mkdir -p $(@D)
	gcc $< -o $@

$(TLIB): $(OBJS)
	@rm -f $(TLIB)
	gcc -shared $(OBJS) -Wl,-soname -Wl,$(LIBRARY) -o $@

obj/%.o: %.c
	@mkdir -p $(@D)
	gcc $(INCLUDE) $(OPTS) -c $< -o $@

obj/%.o: %.cc
	@mkdir -p $(@D)
	gcc $(INCLUDE) $(OPTS) -c $< -o $@

clean:
	rm -rf bin obj

distclean: clean
	rm -rf debian/*.log debian/files

install: $(TLIB) $(TFDI)
	install -d "$(DESTDIR)/$(DLIB)"
	install -d "$(DESTDIR)/$(DFDI)"
	install -m 755 $(TLIB) "$(DESTDIR)/$(DLIB)"
	install -m 644 $(TFDI) "$(DESTDIR)/$(DFDI)"

test:
	gcc $< $(OBJS) -o LINKTEST

obj/match/test.o: match/match.c
