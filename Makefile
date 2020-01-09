compile: export DIR ?= ./svolti/
compile: export FILE ?= file.c
compile:
	@gcc $(DIR)$(FILE) -o ./bin/${$(FILE)::-2} -pthread
.PHONY: compile

build:
	@docker image build -t ubuntu-rtes:1.0 .
.PHONY: build

run:
	@docker run --rm -it ubuntu-rtes:1.0
.PHONY: run

all:
	@$(MAKE) build
	@$(MAKE) run
.PHONY: all
