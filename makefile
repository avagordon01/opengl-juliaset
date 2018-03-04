CFLAGS := -MMD -O3
LINKFLAGS := -lm -lGL -lglfw

all: bin/render

clean:
	git clean -Xfd

bin/%: src/%.c
	@mkdir -p bin
	$(CC) $< -o $@ $(CFLAGS) $(LINKFLAGS)

-include bin/*.d
