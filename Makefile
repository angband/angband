.PHONY: build install deploy upload help

GREEN  := $(shell tput -Txterm setaf 2)
WHITE  := $(shell tput -Txterm setaf 7)
YELLOW := $(shell tput -Txterm setaf 3)
RESET  := $(shell tput -Txterm sgr0)
HELPME = \
	%help; \
	while(<>) { push @{$$help{$$2 // 'options'}}, [$$1, $$3] if /^([a-zA-Z\-]+)\s*:.*\#\#(?:@([a-zA-Z\-]+))?\s(.*)$$/ }; \
	for (sort keys %help) { \
	print "${WHITE}$$_:${RESET}\n"; \
	for (@{$$help{$$_}}) { \
	$$sep = " " x (20 - length $$_->[0]); \
	print "  ${YELLOW}$$_->[0]${RESET}$$sep${GREEN}$$_->[1]${RESET}\n"; \
	}; \
	print "\n"; }

help:
	@perl -e '$(HELPME)' Makefile

build:					## Build the site locally
	./build.pl
	cp -r static/* output/

install:				##@unused Install (superceded by ansible deployment)
	./build.pl
	cp -r static/* output/
	rsync -r output/ /var/www/rephial.org/public/
	cp mappings /var/www/rephial.org/
	sudo /etc/init.d/lighttpd reload

deploy:					## Run ansible deployment script
	ansible-playbook deploy.yml -K

upload:					## Upload contents of upload/ to remote downloads directory
	scp -r upload/* takkaria@rephial.org:/var/www/rephial.org/public/downloads/
