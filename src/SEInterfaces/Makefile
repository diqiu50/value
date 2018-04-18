LIBNAME = SEInterfaces

BASEDIR = ../..
LIBDIR = $(BASEDIR)/obj
OBJDIR = $(LIBDIR)/$(LIBNAME)
TARGETLIB = $(LIBDIR)/lib$(LIBNAME).a

include $(BASEDIR)/Makerules.dep

CFLAGS += -I$(BASEDIR)/src -I$(BASEDIR)/Include

all: $(OBJECTS)
	ar crs $(TARGETLIB) $^

$(OBJECTS): $(OBJDIR)/%.o: %.cpp
	$(CREATEDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJECTS) $(TARGETLIB)
