# UtilChest - Utilities Chest

[![Build Status](https://travis-ci.org/eltanin-os/utilchest.svg?branch=master)](https://travis-ci.org/eltanin-os/utilchest) [![Coverity Scan Build Status](https://img.shields.io/coverity/scan/13660.svg)](https://scan.coverity.com/projects/eltanin-os-utilchest)

It's a collection of Unix utilities written to be simple and small

#### Binaries
* basename
* cat
* chgrp
* chmod
* chown
* chroot
* clear
* cp
* date
* dirname
* domainname
* echo
* env
* false
* head
* hostname
* link
* ln
* ls
* mkdir
* mkfifo
* mv
* nice
* printenv
* pwd
* readlink
* rm
* rmdir
* sleep
* sync
* true
* tty
* uname
* unlink
* whoami
* yes

#### Building
```
	$ git clone https://github.com/eltanin-os/utilchest
	$ cd utilchest
	-- Multiple Binaries
	$ make
	# make install
	-- Single Binary
	$ make utilchest
	# make utilchest-install
```

#### Similar Software
* [Suckless Base](http://core.suckless.org/sbase) and [Unportable Base](http://core.suckless.org/ubase)
* [ToyBox](http://landley.net/toybox/about.html)
* [Heirloom Toolchest](http://heirloom.sourceforge.net/tools.html)
