.PHONY: all build test clean package run

all: build

build:
	./scripts/build.sh

test:
	./scripts/test.sh

clean:
	./scripts/clean.sh

package:
	./scripts/package.sh

run:
	./build-linux/graph-core
