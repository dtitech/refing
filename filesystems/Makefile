# meta-Makefile for rEFIng filesystem drivers
#
# Most of the functionality is in Make.driver; this Makefile merely
# deletes critical temporary files and calls Make.driver with the
# name of the driver to be built. This is done because of a dependency
# in the fsw_efi.c file on the filesystem type; this file must be
# recompiled for each new filesystem built.

# This program is licensed under the terms of the GNU GPL, version 3,
# or (at your option) any later version.
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

INSTALL_DIR = /boot/efi/EFI/refing/drivers

FILESYSTEMS = ext2 ext4 reiserfs iso9660 hfs btrfs
TEXTFILES = $(FILESYSTEMS:=*.txt)

# Build the drivers

all:	$(FILESYSTEMS)

ext2:
	rm -f fsw_efi.o
	+make DRIVERNAME=ext2 -f Make.driver

ext4:
	rm -f fsw_efi.o
	+make DRIVERNAME=ext4 -f Make.driver

reiserfs:
	rm -f fsw_efi.o
	+make DRIVERNAME=reiserfs -f Make.driver

iso9660:
	rm -f fsw_efi.o
	+make DRIVERNAME=iso9660 -f Make.driver

hfs:
	rm -f fsw_efi.o
	+make DRIVERNAME=hfs -f Make.driver

btrfs:
	rm -f fsw_efi.o
	+make DRIVERNAME=btrfs -f Make.driver

ntfs:
	rm -f fsw_efi.o
	+make DRIVERNAME=ntfs -f Make.driver

# Compiling filesystem drivers in parallel with GNU-EFI can cause build errors
# and frozen systems on boot with drivers that do build OK, so disallow this....
.NOTPARALLEL:

# utility rules

clean:
	rm -f *~ *.bak *.o *.obj *.so *.efi *.dll err.txt $(TEXTFILES)
	+make -C test clean


install:
	mkdir -p $(INSTALL_DIR)
	cp *.efi $(INSTALL_DIR)

# DO NOT DELETE
