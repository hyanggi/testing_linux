SUBMODULES = ip_transport ip ether llc

all: $(SUBMODULES)

.PHONY: all

# submodules

$(SUBMODULES):
	$(MAKE) -C $@

$(SUBMODULES:%=%_clean): %_clean:
	$(MAKE) -C $* clean

.PHONY: $(SUBMODULES) $(SUBMODULES:%=%_clean)

# clean

clean: $(SUBMODULES:%=%_clean)

.PHONY: clean
