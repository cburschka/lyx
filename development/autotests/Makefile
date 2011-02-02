
all: build-xvkbd run-tests

.PHONY:
build-xvkbd:
	cd xvkbd && xmkmf && make

.PHONY:
run-tests: xvkbd
	./run-tests.sh
