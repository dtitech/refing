# Makefile for rEFInd

# This program is licensed under the terms of the GNU GPL, version 3,
# or (at your option) any later version.
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

include Make.common

SHELL           = /bin/bash
LOADER_DIR      = refind
FS_DIR          = filesystems
LIBEG_DIR       = libeg
MOK_DIR         = mok
GZIP_DIR        = gzip
GPTSYNC_DIR     = gptsync
EFILIB_DIR      = efilib

all:
	+make -C $(LIBEG_DIR)
	+make -C $(MOK_DIR)
	+make -C $(GZIP_DIR)
	+make -C $(EFILIB_DIR)
	+make -C $(LOADER_DIR)
	+make -C $(GPTSYNC_DIR)

gptsync:
	+make -C $(GPTSYNC_DIR)

fs:
	+make -C $(FS_DIR) all

###########################################################################
#
# Build rules that are not dependent on the toolkit....
#
###########################################################################

# NOTE: This "clean" rule cleans intermediate components
clean:
	make -C $(LIBEG_DIR) clean
	make -C $(MOK_DIR) clean
	make -C $(GZIP_DIR) clean
	make -C $(LOADER_DIR) clean
	make -C $(EFILIB_DIR) clean
	make -C $(FS_DIR) clean
	make -C $(GPTSYNC_DIR) clean
	rm -f include/*~
	rm -rf drivers_$(FILENAME_CODE)/*

# NOTE TO DISTRIBUTION MAINTAINERS:
# The "install" target installs the program directly to the ESP
# and it modifies the *CURRENT COMPUTER's* NVRAM. Thus, you should
# *NOT* use this target as part of the build process for your
# binary packages (RPMs, Debian packages, etc.). (Gentoo could
# use it in an ebuild, though....) You COULD, however, copy the
# files to a directory somewhere (/usr/share/refind or whatever)
# and then call refind-install as part of the binary package
# installation process.

install:
	./refind-install

# DO NOT DELETE
