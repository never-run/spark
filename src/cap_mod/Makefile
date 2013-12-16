CFLAGS = -Wall -fPIC --shared -g -O2 -Wno-strict-aliasing -lpcap
CC = gcc
INCLUDE_DIR = ../../inc
LINK = $(CC) -I$(INCLUDE_DIR) $(CFLAGS)

OBJS = spk_cap_mod_pcap.so

all: $(OBJS)

$(OBJS): %.so: %.c
	$(LINK) $< -o $@
install : 
	mkdir -p ../../mod
	cp $(OBJS) ../../mod/
clean:
	rm -f *.so;
