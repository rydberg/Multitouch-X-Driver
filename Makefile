LIBRARY	= multitouch.so
FDIS	= 11-multitouch.fdi
MODULES = src

o_src	= multitouch

TARGETS	= $(addsuffix /test,$(MODULES))

OBJECTS	= $(addsuffix .o,\
	$(foreach mod,$(MODULES),\
	$(addprefix $(mod)/,$(o_$(mod)))))

TBIN	= $(addprefix bin/,$(TARGETS))
TLIB	= $(addprefix obj/,$(LIBRARY))
TOBJ	= $(addprefix obj/,$(addsuffix .o,$(TARGETS)))
TFDI	= $(addprefix fdi/,$(FDIS))
OBJS	= $(addprefix obj/,$(OBJECTS))
LIBS	= #-lhal -ldbus-glib-1

DLIB	= usr/lib/xorg/modules/input
DFDI	= usr/share/hal/fdi/policy/20thirdparty

INCLUDE = -I.
OPTS	= -O3

.PHONY: all clean
.PRECIOUS: obj/%.o

all:	$(OBJS) $(TLIB) $(TOBJ) $(TBIN)

bin/%:	obj/%.o
	@mkdir -p $(@D)
	gcc $(INCLUDE) $(OPTS) $< $(TLIB) $(LIBS) -o $@

$(TLIB): $(OBJS)
	@rm -f $(TLIB)
	gcc -shared $(OBJS) -Wl,-soname -Wl,$(LIBRARY) -o $@

obj/%.o: %.c
	@mkdir -p $(@D)
	gcc -c $< -o $@

obj/%.o: %.cc
	@mkdir -p $(@D)
	gcc -c $< -o $@

clean:
	rm -rf bin obj

distclean: clean
	rm -rf debian/*.log debian/files

install: $(TLIB) $(TFDI)
	install -d "$(DESTDIR)/$(DLIB)"
	install -d "$(DESTDIR)/$(DFDI)"
	install -m 755 $(TBIN) "$(DESTDIR)/$(DLIB)"
	install -m 644 $(TFDI) "$(DESTDIR)/$(DFDI)"
