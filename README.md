#EFM - A filemanager based on Elementary and EFL#

Like the header says. A filemanager, written in C using the EFL and elementary

## Building and installing ##

### Manual build

```
git clone <URL>

cd <here is the clonedplace>

mkdir build

cd build

cmake ..

make all

sudo make install

```
Be aware, this is installing 3 libraries!
Make sure to include the library path is in the ldconfig search path and the ldconfig cache is up to date!

### Distro packages:

* Arch Linux (AUR): https://aur.archlinux.org/packages/verne-git/


## Bugs ##

Bugs can be reported via github (marcelhollerbach) or phab.enlightenment.org(bu5hm4n).

## Structure ##

The filemanager is built out of a widget called filedisplay. It is written as an Elementary widget and integrates well with the other elm widgets. Looking into future we could use this widget in a fileselector or other places where files need to be crawled.

There are also three little libraries, elementary_ext efm_lib and emous.

### Elementary_Ext ###

Widgets which are built as separated library.

### efm_lib ###

A abstraction for filesystem monitoring and listing.

Once a file appears, big operations such as finding the mimetype and the like are done asynchronously.

### Emous ###

It should abstract the problems of listing and monitoring changes in mountpoints and with mountdevices.

In general nothing else than a simple signal system with a little module loader.

## General ##

The Filemanager itself does not have that many operations or features or helpers.
