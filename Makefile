MAKE = make
MAKE_ENGINE = $(MAKE) -C engine
MAKE_CLIENT = $(MAKE) -C client
MAKE_TEST = $(MAKE) -C test

all: build_engine build_client

build_engine: 
	$(MAKE_ENGINE) all

build_client: build_engine
	$(MAKE_CLIENT) all

build_test: build_engine
	$(MAKE_TEST) all

clean: clean_engine clean_client clean_test

clean_engine: 
	$(MAKE_ENGINE) clean

clean_client: 
	$(MAKE_CLIENT) clean

clean_test: 
	$(MAKE_TEST) clean

.PHONY: all build_engine build_client build_test \
		clean  clean_engine clean_client clean_test

