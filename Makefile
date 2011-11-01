

build:
	./build.pl

install:
	./build.pl
	rsync -r output/ /var/www/rephial.org/public-test/
	cp mappings /var/www/rephial.org/
	sudo /etc/init.d/lighttpd reload
