# Enable MIDI

1. First install timidity (TiMidity++, a MIDI-to-sound converter), and start it:

```
timidity -iAD -Os
```

`-iA` means to launch as an ALSA sequencer client (for input). `D` means to run as a daemon.

`-Os` means to output to ALSA.

2. Then create virtual raw MIDI devices (which are mapped to ALSA sequencer clients):

```
sudo modprobe snd-virmidi
```

We'll see virtual raw MIDI devices appear under `/dev/snd/`.

3. Then on the ALSA sequencer system connect a virtual MIDI port to a timidity port:

First see the port numbers to connect:

```
aconnect --list
```

Then connect the ports, for example:

```
aconnect 20:0 128:0
```
