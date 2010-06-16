VERSION = 1
PATCHLEVEL = 0
EXTRAVERSION = alpha3

LIBRARY	= multitouch.so
FDIS	= 11-multitouch.fdi
MODULES = match mtdev src
XMODULES = driver

o_match	= match

o_mtdev	= iobuf caps hwdata

o_src	= hwstate mtstate memory mtouch gestures

o_driver= multitouch

TARGETS	+= match/test
TARGETS	+= src/test

OBJECTS	= $(addsuffix .o,\
	$(foreach mod,$(MODULES),\
	$(addprefix $(mod)/,$(o_$(mod)))))
XOBJECTS= $(addsuffix .o,\
	$(foreach mod,$(XMODULES),\
	$(addprefix $(mod)/,$(o_$(mod)))))

TBIN	= $(addprefix bin/,$(TARGETS))
TLIB	= $(addprefix obj/,$(LIBRARY))
TOBJ	= $(addprefix obj/,$(addsuffix .o,$(TARGETS)))
TFDI	= $(addprefix fdi/,$(FDIS))
OBJS	= $(addprefix obj/,$(OBJECTS))
XOBJS	= $(addprefix obj/,$(XOBJECTS))
LIBS	= -lm

DLIB	= usr/lib/xorg/modules/input
DFDI	= usr/share/hal/fdi/policy/20thirdparty

INCLUDE = -Iinclude -I/usr/include/xorg -I/usr/include/pixman-1
OPTS	= -O3 -fPIC

.PHONY: all clean
.PRECIOUS: obj/%.o

all:	$(OBJS) $(TLIB) $(TOBJ) $(TBIN)

bin/%:	obj/%.o $(OBJS)
	@mkdir -p $(@D)
	gcc $< -o $@ $(OBJS) $(LIBS)

$(TLIB): $(OBJS) $(XOBJS)
	@rm -f $(TLIB)
	gcc -shared $(OBJS) $(XOBJS) -Wl,-soname -Wl,$(LIBRARY) -o $@

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
