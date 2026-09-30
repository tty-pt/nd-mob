all := libnd-mob

LDLIBS-libnd-mob := -lxylem

# Sibling -I for a dev build. In CI the siblings do not exist and all headers
# come from the installed packages named in .github/workflows/ci.yml.
CFLAGS += -I$(shell cd .. && pwd)/axil-nd/include
CFLAGS += -I$(shell cd .. && pwd)/axil-nd-fight/include
CFLAGS += -I$(shell cd .. && pwd)/axil-nd-plant/include
CFLAGS += -I$(shell cd .. && pwd)/axil-nd-race/include

FOLDER := nd

-include ./../mk/include.mk
