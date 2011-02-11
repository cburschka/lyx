
all: $(if $(XVKBD_HACKED),build-xvkbd,) run-tests

.PHONY:
build-xvkbd:
	cd xvkbd && xmkmf && make

.PHONY:
run-tests:
	./run-tests.sh

clean:
	rm -rf locale out-*
