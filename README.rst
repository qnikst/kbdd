kbdd - XKB daemon
===================

Simple daemon and library to make per window layout using XKB (X KeyBoard
Extension).

Features
--------

  * dbus interface
  * set layout group by its number
  * switch to the previous layout 
  * kbdd supports only EWMH compatible systems, if you need support for
    others, please request

Requirements
------------

In order to build kbdd you need:
 * xorg header files with xkb support
 * glib header files 
 * dbus-glib header files (optional) 

Installation
------------
Program uses autotools installation system, so installation can be done in next
steps [1]_::

    ./configure options
    make
    make install

Configuration options:
  * ``enable-debug`` - *[default: disabled]* adds additional debuging info
  * ``enable-dbus``  - *[default:  enabled]* enable dbus support

.. [1] If you use git version, you should generate installation files: you
   should use::

        aclocal ; automake --add-missing ; autoreconf

   and then proceed to ordinary installation. Of cause you should need to have
   autotools package installed

Running kbdd
------------
To run kbdd you can just run ``/usr/bin/kbdd`` to use kbdd in daemon mode, or
use ``/usr/bin/kbdd -n`` to run in verbose mode.

More info
---------
You can go to `kbdd Wiki`_ to check usecases described.

.. _kbdd Wiki: http://github.com/qnikst/kbdd/wiki/Usecases

Known bugs
----------
Sometimes meta4 key lips in awesome, this bug was never reproduced anywere but
on one host.

Troubleshooting
---------------
All bugs and issues can be posted to http://github.com/qnikst/kbdd/issues or
sent to jabber: qnikst@gentoo.ru

