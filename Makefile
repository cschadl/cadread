CXX=g++
MATHSTUFF=../mathstuff
STLIMPORT=../stl-import
STLUTIL=../stlutil
OCCT=/usr/include/oce
OCCT7=/usr/local/include/opencascade
OCCT7_LIBDIR=-L /usr/local/lib
OCCT_LIBDIR=
INCLUDE=-I $(MATHSTUFF) -I $(STLIMPORT) -I $(STLUTIL) -I $(OCCT)
CFLAGS=-Wall -O3 -funroll-loops -std=c++17
PKGFLAGS=-pthread
PKGLIBS=-lTKernel -lTKMath -lTKG2d -lTKBRep -lTKSTEPBase -lTKSTEP -lTKIGES -lTKMesh -lTKTopAlgo -lTKXSBase -lTKG3d -lTKShHealing -lboost_program_options -lboost_system -lboost_filesystem
CFLAGS_DEP=-std=c++17
OUTDIR=Release
EXECUTABLE=cadread

CADREADOBJS=cadread.o Importing.o cadread_ConsoleProgressIndicator.o
STLIMPORTOBJS=triangle_mesh.o

# Release configuration
OBJS=$(CADREADOBJS) $(STLIMPORTOBJS)
OUTOBJS=$(addprefix $(OUTDIR)/, $(OBJS))
OUTEXE=$(OUTDIR)/$(EXECUTABLE)

# Debug configuration
OUTDIR_DEBUG=Debug
OUTOBJS_DEBUG=$(addprefix $(OUTDIR_DEBUG)/, $(OBJS))
OUTEXE_DEBUG=$(OUTDIR_DEBUG)/$(EXECUTABLE)
CFLAGS_DEBUG=-Wall -ggdb3 -std=c++17
CPPFLAGS_DEBUG=-DDEBUG

################################
## Release config

all: $(OUTEXE)

$(OUTEXE): $(OUTOBJS)
	$(CXX) $(OUTOBJS) $(PKGLIBS) $(OCCT_LIBDIR) -o $(OUTEXE)

# This generates dependency rules
-include $(OUTOBJS:.o=.d)

# Rule for STLIMPORT stuff
$(addprefix $(OUTDIR)/, $(STLIMPORTOBJS)): $(OUTDIR)/%.o : $(STLIMPORT)/%.cpp
	$(CXX) -c $(CFLAGS) $(INCLUDE) -o $@ $<	
	$(CXX) -MM $(CFLAGS_DEP) $(INCLUDE) $< | perl -pe 's/^\w+\.o:/$(OUTDIR)\/$$&/' > $(OUTDIR)/$*.d

## Default rule
$(OUTDIR)/%.o: %.cpp
	$(CXX) -c $(CFLAGS) $(PKGFLAGS) $(INCLUDE) -o $@ $<
	$(CXX) -MM $(CFLAGS_DEP) $(PKGFLAGS) $(INCLUDE) $< | perl -pe 's/^\w+\.o:/$(OUTDIR)\/$$&/' > $(OUTDIR)/$*.d
	
## Build release using OCCT7 libs
occt7_release: OCCT=$(OCCT7)
occt7_release: OCCT_LIBDIR=$(OCCT7_LIBDIR)
occt7_release: INCLUDE=-I $(MATHSTUFF) -I $(STLIMPORT) -I $(STLUTIL) -I $(OCCT)
occt7_release: all
	
clean:
	rm -rf $(OUTDIR)/*.o $(OUTEXE) $(OUTDIR)/*.d
	
#################################
## Debug config
## I don't know why the fuck I can't
## just set OUTDIR=Debug and do $(OUTEXE)
## but I'm sick of screwing with it and
## I need to get on with my life.

debug: $(OUTEXE_DEBUG)

$(OUTEXE_DEBUG): $(OUTOBJS_DEBUG)
	$(CXX) $(OUTOBJS_DEBUG) $(PKGLIBS) $(OCCT_LIBDIR) -o $(OUTEXE_DEBUG)
	
# This generates dependency rules
-include $(OUTOBJS_DEBUG:.o=.d)
	
# Rule for STLIMPORT stuff
$(addprefix $(OUTDIR_DEBUG)/, $(STLIMPORTOBJS)): $(OUTDIR_DEBUG)/%.o : $(STLIMPORT)/%.cpp
	$(CXX) -c $(CFLAGS_DEBUG) $(CPPFLAGS_DEBUG) $(INCLUDE) -o $@ $<
	$(CXX) -MM $(CFLAGS_DEP) $(INCLUDE) $< | perl -pe 's/^\w+\.o:/$(OUTDIR_DEBUG)\/$$&/' > $(OUTDIR_DEBUG)/$*.d

## Default rule
$(OUTDIR_DEBUG)/%.o: %.cpp
	$(CXX) -c $(CFLAGS_DEBUG) $(CPPFLAGS_DEBUG) $(PKGFLAGS) $(INCLUDE) -o $@ $<
	$(CXX) -MM $(CFLAGS_DEP) $(PKGFLAGS) $(INCLUDE) $< | perl -pe 's/^\w+\.o:/$(OUTDIR_DEBUG)\/$$&/' > $(OUTDIR_DEBUG)/$*.d
	
## Build debug using OCCT7 libs
occt7_debug: OCCT=$(OCCT7)
occt7_debug: OCCT_LIBDIR=$(OCCT7_LIBDIR)
occt7_debug: INCLUDE=-I $(MATHSTUFF) -I $(STLIMPORT) -I $(STLUTIL) -I $(OCCT)
occt7_debug: debug

# This works for some reason
debug_clean: OUTDIR=Debug
debug_clean: clean
