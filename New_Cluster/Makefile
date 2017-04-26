OBJS_DIR = .objs

EXE_CLIENT = client
EXE_SERVER = server
EXES_STUDENT = $(EXE_CLIENT) $(EXE_SERVER)

OBJS_CLIENT = $(EXE_CLIENT).o format.o common.o client.o
OBJS_SERVER = $(EXE_SERVER).o format.o common.o

CC = clang
WARNINGS = -Wall -Wextra -Werror -Wno-error=unused-parameter
INC=-I./includes/
CFLAGS_DEBUG   = -O0 $(WARNINGS) $(INC) -g -std=c99 -c -MMD -MP -D_GNU_SOURCE -DTHREAD_SAFE -pthread -DDEBUG
CFLAGS_RELEASE = -O2 $(WARNINGS) $(INC) -std=c99 -c -MMD -MP -D_GNU_SOURCE -DTHREAD_SAFE -pthread

# tsan needs some funky flags
CFLAGS_TSAN    = $(CFLAGS_DEBUG)
CFLAGS_TSAN    += -fsanitize=thread -DSANITIZE_THREADS

# set up linker
LD = clang
LDFLAGS = -pthread -Llibs/ -lprovided
LDFLAGS_TSAN = $(LDFLAGS) -ltsan

.PHONY: all
all: release

# build types
# run clean before building debug so that all of the release executables
# disappear
.PHONY: debug
.PHONY: release
.PHONY: tsan

release: $(EXES_STUDENT)
debug:   clean $(EXES_STUDENT:%=%-debug)
tsan:    clean $(EXES_STUDENT:%=%-tsan)

# include dependencies
-include $(OBJS_DIR)/*.d

$(OBJS_DIR):
	@mkdir -p $(OBJS_DIR)

# patterns to create objects
# keep the debug and release postfix for object files so that we can always
# separate them correctly
$(OBJS_DIR)/%-debug.o: %.c | $(OBJS_DIR)
	$(CC) $(CFLAGS_DEBUG) $< -o $@

$(OBJS_DIR)/%-tsan.o: %.c | $(OBJS_DIR)
	$(CC) $(CFLAGS_TSAN) $< -o $@

$(OBJS_DIR)/%-release.o: %.c | $(OBJS_DIR)
	$(CC) $(CFLAGS_RELEASE) $< -o $@

# exes
# you will need a triple of exe and exe-debug and exe-tsan for each exe (other
# than provided exes)
$(EXE_SERVER): $(OBJS_SERVER:%.o=$(OBJS_DIR)/%-release.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_SERVER)-debug: $(OBJS_SERVER:%.o=$(OBJS_DIR)/%-debug.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_SERVER)-tsan: $(OBJS_SERVER:%.o=$(OBJS_DIR)/%-tsan.o)
	$(LD) $^ $(LDFLAGS_TSAN) -o $@

$(EXE_CLIENT): $(OBJS_CLIENT:%.o=$(OBJS_DIR)/%-release.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_CLIENT)-debug: $(OBJS_CLIENT:%.o=$(OBJS_DIR)/%-debug.o)
	$(LD) $^ $(LDFLAGS) -o $@

$(EXE_CLIENT)-tsan: $(OBJS_CLIENT:%.o=$(OBJS_DIR)/%-tsan.o)
	$(LD) $^ $(LDFLAGS_TSAN) -o $@

.PHONY: clean
clean:
	-rm -rf .objs $(EXES_STUDENT) $(EXES_STUDENT:%=%-tsan) $(EXES_STUDENT:%=%-debug) $(EXES_PROVIDED) $(EXES_OPTIONAL)
