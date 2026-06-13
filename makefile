# ----------------------------
# Makefile Options
# ----------------------------

NAME = TIRECVRY
ICON = icon.png
DESCRIPTION = "irecovery but on a calculator."
COMPRESSED = NO
ARCHIVED = YES

CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

# ----------------------------

include $(shell cedev-config --makefile)
