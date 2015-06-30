#EFM - A filemanager based on Elementary and EFL#

Like the header says. A filemanager, written in C using the EFL and elementary

## Building and installing ##

Right now there are just install targets for the libraries. The application itself will not be installed!!

To build do:
```
cd <here is the clonedplace> 

mkdir build 

cd build 

cmake .. 

make all 

./src/bin/efm 

->have fun 
```

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
