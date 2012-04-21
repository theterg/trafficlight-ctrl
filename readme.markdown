## Trafficlight-ctrl

A simple arduino application for using audio and serial to control a traffic light, or other simple 3 light fixture.  This was originally written for a party scenario - wanting to have lights that change with music with the ability to take manual control of the lights.

* Currently just an arduino application.
* Arduino samples from pin A0 at 500Hz and turns on three lights (D9-11) in proportion to the audio signal
* Lights are automatically scaled to changes in volume
* Any serial characters on the UART pause the VU meter operation and switch to serial mode.
* A simple serial protocol can be used to manually change the lights.  After a given timeout, the arduino revers to VU meter mode.

### Serial protocol

Consists of 2 character lines delimited by '\r\n' or just '\n'.  Currently not case sensitive.  The current format is <COMMAND><TARGET>\r\n, where <TARGET> is one of the three lights - 'r', 'y' or 'g'.
Possible commands:
* S(et) a light (turn it on).  Eg: 'sy' will turn on the yellow light
* C(lear) a light (turn it off).  Eg: 'CG' will turn off the green light
* Q(uit) will immediately exit serial mode and return to VU meter mode
