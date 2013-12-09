#DIRS = $(shell ls -1 -F | grep \[\/\$\])
DIRS = cap_ada cap_mod demo

all:
	for i in $(DIRS); do make -C $$i; done

install:
	for i in $(DIRS); do cd $$i;make install;cd ..; done

clean:
	for i in $(DIRS); do cd $$i;make clean;cd ..; done

