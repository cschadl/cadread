CXX=g++
MATHSTUFF=$(HOME)/workspace/mathstuff
STLIMPORT=$(HOME)/workspace/stl-import
STLUTIL=$(HOME)/workspace/stlutil
OCCT=/usr/include/oce
#OCCT=/usr/local/include/opencascade
#OCCT_LIBDIR=-L /usr/local/lib
OCCT_LIBDIR=
INCLUDE=-I $(MATHSTUFF) -I $(STLIMPORT) -I $(STLUTIL) -I $(OCCT)
CFLAGS=-Wall -O3 -funroll-loops -std=c++11
PKGFLAGS=-pthread
PKGLIBS=-lTKernel -lTKMath -lTKG2d -lTKBRep -lTKSTEPBase -lTKSTEP -lTKIGES -lTKMesh -lTKTopAlgo -lTKXSBase -lTKG3d -lboost_program_options -lboost_system -lboost_filesystem
CFLAGS_DEP=-std=c++11
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
CFLAGS_DEBUG=-Wall -ggdb3 -std=c++11
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

# This works for some reason
debug_clean: OUTDIR=Debug
debug_clean: clean