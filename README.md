# IOUlive64

**IOUlive64** is an open-source, drop-in replacement for **IOUlive**, a proprietary bridging tool originally developed by Cisco Systems, Inc.

It allows you to connect **Cisco IOU (IOS-on-UNIX)** virtual routers and switches to a **physical Ethernet network**, by bridging IOU interfaces with real interfaces on your system.

---

## About

IOUlive64 is a modern refactor of [IOUlive86](https://github.com/jlgaddis/ioulive86), originally maintained by an anonymous developer.

This version was restructured for better maintainability, clearer threading logic, and safe system integration. It supports raw Ethernet sockets via UNIX domain sockets in a non-root-friendly way.

> Maintained by: Jeremy L. Gaddis  
> Source: [github.com/jlgaddis/ioulive86](https://github.com/jlgaddis/ioulive86)

Licensed under the **GNU General Public License v3**. See the [LICENSE.txt](./LICENSE.txt) file for details.

---

## Usage

IOUlive64 bridges an IOU virtual interface with a physical Ethernet interface. This allows packets from a virtual lab to reach a real LAN.

### 1. Build the application

Make sure you have `make` and a C compiler (`gcc`):

```bash
make
```

This will produce an executable called ioulive64.

### 2. Grant raw socket permissions

To allow `ioulive64` to capture and send raw Ethernet frames **without needing root access**, you must assign it the appropriate Linux capability:

```bash
sudo setcap cap_net_raw=eip ./ioulive64
```
 This command grants cap_net_raw, allowing the program to open raw sockets while still running as a regular user.

    ⚠️ Without this step, the application will fail to open the Ethernet socket unless run as root — which is not recommended.

✅ You can check the capability with:
```
ark@iou17box:~/ioulive64$ sudo getcap ioulive64
ioulive64 cap_net_raw=eip
ark@iou17box:~/ioulive64$ 
```

### 3. Run IOUlive64

ioulive64 must be started with the syntax:
```
./ioulive64 [-v] -i <interface> [-n NETMAP] <instance-ID>
```
Arguments:
    - -i <interface>: Ethernet interface to bind to (e.g., eth0)
    - -n NETMAP: Optional path to the NETMAP file (default: NETMAP)
    - <instance-ID>: IOUlive64 instance number (e.g., 99)

The instance number will be used to bind to a UNIX socket: /tmp/netio<UID>/<instance-ID>

⚠️ Important: IOUlive64 must be run under the same Linux user as the IOU processes it communicates with.
This ensures it can access the correct UNIX domain sockets in /tmp/netio<UID>/.

## NETMAP Format

The NETMAP file defines peer mappings, one per line:

### Format:
```
<source-instance>:<slot>/<port>@host   <dest-instance>:<slot>/<port>@host
```

## Example:

NETMAP content :
```
10:0/2@localhost     98:0/0@localhost
```

Start IOUlive with: 
```
./ioulive64 -i eth0 98
```

This setup means:
    You are launching IOUlive instance 98
    It connects to the peer router 10, interface 0/2
    It bridges router 10’s interface 0/2 to the physical interface eth0

In terms of socket paths:
    IOUlive binds to: /tmp/netio$(id -u)/98
    It connects to: /tmp/netio$(id -u)/10

So when the IOU router with ID 10 sends frames on its interface 0/2, they are picked up by IOUlive64 and forwarded to eth0.

# Notes

    Works only on Linux (requires AF_PACKET)
    Designed for use with Cisco IOU virtual routers
    Raw Ethernet access required (CAP_NET_RAW)
    Multithreaded architecture

## Acknowledgements

    Original idea: Cisco Systems, Inc.
    Legacy project: ioulive86 by Jeremy Gaddis
    Inspired by the network emulator community

## License

This project is licensed under the terms of the GNU GPL v3.
