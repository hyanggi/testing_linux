# network

The code in this directory demonstrates network communication with sockets.

Four types of network communication is included here:

  - IP transport layer protocols (layer 4) (under `ip_transport`). Supports TCP, UDP, UDPLite, SCTP, DCCP.

  - IP (Internet Protocol) (layer 3) (under `ip`). Supports both IPv4 and IPv6.

  - Ethernet (layer 2) (under `ether`).

  - 802.2 LLC (layer 2) (under `llc`). Supports LLC type 1 and 2. They are like UDP and TCP directly over Ethernet.
