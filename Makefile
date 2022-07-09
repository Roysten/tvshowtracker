# This Makefile will build the MinGW Win32 application.

# Object files to create for the executable
OBJS = obj/About_dialog.o obj/Main_window.o obj/Resource.o obj/Win_main.o \
	obj/Rest_client.o obj/Logger.o obj/Listbox.o obj/Textbox.o obj/Statusbar.o \
	obj/Groupbox.o obj/pdjson.o obj/Tv_show.o obj/Button.o obj/Requester_thread.o \
	obj/App.o obj/str_util.o obj/Listview.o

# Warnings to be raised by the C compiler
WARNS = -Wall -Wpedantic -Wno-variadic-macros -Wno-pedantic-ms-format -Wno-overlength-strings

# Names of tools to use when building
CC = x86_64-w64-mingw32-gcc
RC = x86_64-w64-mingw32-windres
EXE = Win32App.exe

# Compiler flags. Compile ANSI build only if CHARSET=ANSI.
ifeq (${CHARSET}, ANSI)
  CFLAGS =-Os -std=gnu99 -D _WIN32_IE=0x0500 -D WINVER=0x0500 ${WARNS} -D WIN32_LEAN_AND_MEAN -Iinclude
else
  CFLAGS =-Os -std=gnu99 -D UNICODE -D _UNICODE -D _WIN32_IE=0x0500 -D WINVER=0x0500 ${WARNS} -D WIN32_LEAN_AND_MEAN -Iinclude
endif

# Linker flags
# LDFLAGS =-lcomctl32 -lgdi32 -lwininet -Wl,--subsystem,windows
# Without symbols
LDFLAGS =-s -lcomctl32 -lgdi32 -lwininet -Wl,--subsystem,windows

.PHONY: all clean

# Build executable by default
all: check-env
all: bin/${EXE}

# Delete all build output
clean:
	rm bin/${EXE} || true
	rm -r obj/*

copy:
	cp bin/${EXE} /mnt/f/

run: copy
	/mnt/f/Win32App.exe

# Create build output directories if they don't exist
bin obj:
	mkdir -p "$@"

# Compile object files for executable
obj/%.o: src/%.c | obj
	${CC} -DREST_API_KEY=L${REST_API_KEY} ${CFLAGS} -c "$<" -o "$@"

obj/%.o: src/rest/%.c | obj
	${CC} ${CFLAGS} -c "$<" -o "$@"

obj/%.o: src/ui/%.c | obj
	${CC} ${CFLAGS} -c "$<" -o "$@"

obj/%.o: src/dialog/%.c | obj
	${CC} ${CFLAGS} -c "$<" -o "$@"

obj/%.o: src/json/%.c | obj
	${CC} ${CFLAGS} -c "$<" -o "$@"

obj/%.o: src/util/%.c | obj
	${CC} ${CFLAGS} -c "$<" -o "$@"

# Build the resources
obj/Resource.o: res/Resource.rc res/Application.manifest res/Application.ico | obj
	${RC} -I./include -I./res -i "$<" -o "$@"

# Build the exectuable
bin/${EXE}: ${OBJS} | bin
	${CC} -o "$@" ${OBJS} ${LDFLAGS}

check-env:
ifndef REST_API_KEY
	$(error REST_API_KEY is not set)
endif