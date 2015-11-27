# pcsensor

```
sudo apt-get install libusb-dev
make
sudo cp 60-temper.rules /lib/udev/rules.d/
# reconnect the USB device after copying the udev rule
sudo ln -s `pwd`/pcsensor /usr/local/bin/pcsensor
pcsensor
```

The output looks like:

```
2015/11/27 14:47:36 Temperature 25.437500
```

# See also

- https://github.com/padelt/pcsensor-temper
- https://github.com/petervojtek/usb-thermometer
- https://github.com/stefansundin/temper-collectd

# LICENSE

	/*
	 * pcsensor.c by Philipp Adelt (c) 2012 (info@philipp.adelt.net)
	 * based on Juan Carlos Perez (c) 2011 (cray@isp-sl.com)
	 * based on Temper.c by Robert Kavaler (c) 2009 (relavak.com)
	 * All rights reserved.
	 *
	 * Temper driver for linux. This program can be compiled either as a library
	 * or as a standalone program (-DUNIT_TEST). The driver will work with some
	 * TEMPer usb devices from RDing (www.PCsensor.com).
	 *
	 * This driver works with USB devices presenting ID 0c45:7401.
	 *
	 * Redistribution and use in source and binary forms, with or without
	 * modification, are permitted provided that the following conditions are met:
	 *     * Redistributions of source code must retain the above copyright
	 *       notice, this list of conditions and the following disclaimer.
	 *
	 * THIS SOFTWARE IS PROVIDED BY Philipp Adelt (and other contributors) ''AS IS'' AND ANY
	 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	 * DISCLAIMED. IN NO EVENT SHALL Philipp Adelt (or other contributors) BE LIABLE FOR ANY
	 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
	 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
	 *
	 */
