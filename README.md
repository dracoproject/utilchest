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
* cksum
* clear
* cmp
* cp
* date
* dirname
* domainname
* du
* echo
* env
* false
* head
* hostname
* id
* link
* ln
* logname
* ls
* mkdir
* mkfifo
* mknod
* mv
* nice
* nohup
* pathchk
* printenv
* pwd
* readlink
* renice
* rm
* rmdir
* sleep
* sync
* tee
* time
* touch
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
	# make install-man
	-- Single Binary
	$ make utilchest
	# make utilchest-install
	# make install-man
```

#### Similar Software
* [Suckless Base](http://core.suckless.org/sbase) and [Unportable Base](http://core.suckless.org/ubase)
* [ToyBox](http://landley.net/toybox/about.html)
* [Heirloom Toolchest](http://heirloom.sourceforge.net/tools.html)
