# 2026-03-15  

a basic power switch to for example turn on/off a raspberry pi from esp32 gpio  

N-channel MOSFET (for example IRLZ44N) driven via NPN Transistor, so that it can switch high side (between power and load, cleanly cutting power without affecting any shared ground), without having to use a P-channel one.  

Notes: 
- R_G is optional, handles any potential noise from transistor. 10-100 ohm or so are fine.
- R_B and R_* determine how fast it switches and the idle power draw (higher resistance -> slower, lower draw). 1k/10k for very fast switching, 100k for when it doesn't really matter (like when using it as a switch). 
- R_GS ensures clean off for the MOSFET and lets any random capacitance dissipate.
IMPORTANT: R_* (+R_G) and R_GS act as a voltage divider, determining the Gate voltage! R_GS must always be considerably bigger (like 10x) than R_*. So if R_* = 100k, R_GS = 1M. 

link to falstad simulation (in case the text import doesn't work): 
https://falstad.com/circuit/circuitjs.html?ctz=CQAgjCAMB0l3BWcMBMcUHYMGZIA4UA2ATmIxAUgoqoQFMBaMMAKABdwNCQU88eALFV78qEGJTAYExQoTzYE2QhjCjoSlZkIIBvOOTBwQAEzoAzAIYBXADZsWAd3C9wYFILHuoTzyFwCfth4gawA5kEhfgJg3FSQLABKLvzMHu78ePFQIELU2RIs5v5oPPj+8mX82OkaULAoLABOFfwirf5RYgkRytVRfbmxOQnOKHmYHuPC5aN+aG0I3As+LdNVfmk5RvBz61uDW3OH3ofE6UkgWRvXUnE5edjQ2CP1CCwAzlczmcJ8r1ZbB86M1OMt-hkeP9urtfNd2pCEaCUEsNjUftsEi10WjSu0YbDnIMUOMOvjfDj2sToSwAB7gGI8QjpQiBIhIUIeRIAfQAQnSeAh0gI2hh0ggIJyQDyAFQC4IvRT8GLVQiS8DcAAyAHtLCYBShiKlziV0sQOS5pdyAOIGrhMxXlIjcKU860AZRYwU4IAAYuJwMhjExpXQPgBLD5sSwAOwAxiCgA
