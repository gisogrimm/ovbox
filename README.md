# ORLANDOviols Consort box (ovbox)

The ovbox is a remote collaboration box developed by the ensemble [ORLANDOviols](http://orlandoviols.com) primarily to allow rehearsals during the lockdown due to Covid19 pandemia. This box is completely built upon open source software and open or standardized hardware.

![consortbox](doc/consortbox.jpg)

## Hardware components

Raspberry Pi 3B+

TASCAM US2x2 (any other class-compliant sound card would do as well)

basic condenser mic, headphones, cables

optionally: a small head tracking device based on MPU6050+ESP8266



## Software components

Raspbian Linux operating system
[https://www.raspberrypi.org/downloads/raspbian/](https://www.raspberrypi.org/downloads/raspbian/)

jack audio connection kit (audio server)
[https://jackaudio.org/](https://jackaudio.org/)

zita-njbridge (network audio/adaptive resampler)
[https://kokkinizita.linuxaudio.org/linuxaudio/](https://kokkinizita.linuxaudio.org/linuxaudio/)

TASCAR (virtual acoustic engine)
[http://tascar.org/](http://tascar.org/)

a self-written UDP tunnel and multiplexer (see folder udpmirror)


## Performance

We use this box since April 15th 2020, almost every day. The software is optimized continouosly. With the current settings we achieve delays between musicians ranging from 40ms (optical fiber network/DSL) and 67ms (connection via mobile network), with a tolerable amount of dropouts. This delay contains the network transmission time, the jitter buffer (currently 20ms), and the delay of the audio hardware (currently 12ms). We connect 4-5 devices.

The device sends 16bit audio at 48 kHz sampling rate. The signal is rendered to headphones using virtual 3D audio. Streaming to platforms such as youtube is possible using a session on a PC, with OBS studio and other pro-audio software.

## Architecture

On a central server (or one endpoint reachable from outside) the multiplexer/tunnel server `mplx_server` needs to be running. This listens at a single UDP port. The boxes connect to this server with the multiplexer client software `mplx_client` and  can receive UDP messages from the other participants sent their client software. UDP messages sent to the local client are transferred to the server. To minimize data manipulation, the packages are protected by a 32bit secret (but not encrypted).

At each endpoint, one intance of zita-n2j is started for each potential participant. One instance of zita-j2n is started on each box to stream the local audio to the other participants. The session management and audio mixing is performed in TASCAR.

The underlying adaptive resampling of zita-njbridge is described in [https://kokkinizita.linuxaudio.org/papers/adapt-resamp.pdf](https://kokkinizita.linuxaudio.org/papers/adapt-resamp.pdf).

A simple mixing interface is provided with `node-js` and some TASCAR extensions. This mixer interface can be opened from any html5-compatible browser in the same network as the ovbox, and is optimized for smartphone/touch use.