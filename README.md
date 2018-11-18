Uclogic-tools
=============

Uclogic-tools is a collection of programs for collecting and analyzing
diagnostic information from UC-Logic graphics tablets (rebranded as Huion,
Yiynova, Ugee, Monoprice, Turcom and others).

Installation
------------

Requirements:

* libusb >= 1.0

Libusb development packages are usually named `libusb-1.0-0-dev` or
`libusbx-devel`. Debian-based systems also need the `pkg-config` package
installed.

Download one of the release packages from the [releases
page](https://github.com/DIGImend/uclogic-tools/releases).

Use your Linux distribution tools to install either .rpm or .deb packages.

Otherwise, to build and install from a source tarball the usual
`./configure && make` is sufficient.

To build from the Git tree autotools are required and `autoreconf -i -f &&
./configure && make` is sufficient.

To install uclogic-tools from source, use `make install`.

Usage
-----

Uclogic-tools contains two utilities: `uclogic-probe` and `uclogic-decode`.

`Uclogic-probe` dumps diagnostics information from UC-Logic (and rebranded)
graphics tablets and attempts to enable additional functionality.

`Uclogic-decode` attempts to extract tablet parameters from the information
dumped by `uclogic-probe`.

Note that the additional functions might be incompatible with the tablet
driver you're currently using and the tablet might stop working properly after
you execute `uclogic-probe`. To fix that simply reconnect the tablet.

`Uclogic-probe` accepts two arguments: bus number and device address. You can
find them in `lsusb` output by looking for a device with vendor ID 256c and
product ID 006e.

For example, in this `lsusb` output:

    Bus 001 Device 003: ID 256c:006e  
    Bus 001 Device 001: ID 1d6b:0001 Linux Foundation 1.1 root hub
    Bus 002 Device 001: ID 1d6b:0001 Linux Foundation 1.1 root hub
    Bus 003 Device 001: ID 1d6b:0001 Linux Foundation 1.1 root hub
    Bus 004 Device 001: ID 1d6b:0002 Linux Foundation 2.0 root hub
    Bus 005 Device 001: ID 1d6b:0002 Linux Foundation 2.0 root hub
    Bus 006 Device 001: ID 1d6b:0002 Linux Foundation 2.0 root hub

The first line corresponds to a Huion tablet, and so its bus number is 1,
device address is 3 and you probe it like this:

    sudo uclogic-probe 1 3

The output will be something like this:

    M 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    P 31 00 30 00 35 00 39 00 34 00 00 00 00 00 00 00 00 00 00 00
    S 64 0E 03 40 9C A8 61 03 00 FF 07 A0 0F 08 00
    S 65 04 03 20 A0
    S 6E 04 03 00 30
    S 79 14 03 48 00 41 00 36 00 30 00 2D 00 46 00 34 00 30 00 30 00
    S 7A 08 03 01 08 00 00 00 00
    S 7B 0C 03 48 00 4B 00 20 00 4F 00 6E 00

The above is what a driver developer would need when asking about a
`uclogic-probe` output.

`Uclogic-decode` simply expects `uclogic-probe` output on its input. E.g. if
you saved the output of `uclogic-probe` into a file named "probe.txt", then
this command would decode it:

    uclogic-decode < probe.txt

You can pipe `uclogic-probe` output directly to `uclogic-decode` too:

    sudo uclogic-probe 1 3 | uclogic-decode

For the diagnostics dump above either of these commands will produce this:

      Manufacturer: ????????
           Product: 10594?????
             Max X: 40000
             Max Y: 25000
      Max pressure: 2047
        Resolution: 4000
    Internal model: HA60-F400
    Buttons status: HK On
