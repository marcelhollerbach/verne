#EFM - A filemanager based on Elementary and EFL#

Like the headline says. A filemanager, written in C using the EFL and elementary

## Building and installing ##

Right now there are just install targets for the librarys. The application itself will not be installed!!

To build do:

cd <here is the clonedplace>
mkdir build
cd build
cmake ..
make all
./src/bin/efm

->have fun

## Bugs ##

Bugs can be reported via github or phab.enlightenment.org(Nickname bu5hm4n).

## Structure ##

The filemanager is build out of a widget called filedisplay. It is written as Elementary widget and integrates well with the other elm widgets. Looking into futur we could use this widget in a fileselector or other places where file needs to be crawled.

There are also three little librarys, elementary_ext efm_lib and emous.

### Elementary_Ext ###

Widgets which are build as seperated library.

### efm_lib ###

A abstraction for filesystem monitoring and listing.

Once a file appeared all big things are done async, things like fetching stat´s, mimetypes and such things.

### Emous ###

It should abstract the problems of listing and monitoring changes in mountpoints and with mountdevices.

In general nothing else than a simple signal system with a little module loader.

## General ##

The Filemanager itsself does not have that many operations or features or helpers.
