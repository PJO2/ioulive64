IOUlive64 is an open-source drop-in replacement for IOUlive, a proprietary
application written by cisco Systems, Inc.

IOUlive64 allows you to "bridge" virtual routers and switches running on
the Cisco IOS-on-UNIX (IOU) software to a physical network.

ioulive64 is a refactor from IOUlive86 which has been developed by an author 
who wishes to remain anonymous.

The source code of the original project is maintained by Jeremy L. Gaddis <jlgaddis@gnu.org>
and hosted at https://github.com/jlgaddis/ioulive86/

IOUlive64 is licensed under the terms of version 2 of the GNU General
Public License. See the enclosed LICENSE.txt file for more details.


🚀 Usage

IOUlive64 bridges a virtual IOU router interface to a physical Ethernet interface, enabling packet exchange between virtual and real networks.
🛠️ 1. Build the application

Make sure you have a C compiler (e.g., gcc) and the necessary development headers:

make

This should produce an executable named ioulive64.
🔒 2. Grant permission for raw socket access

To avoid running ioulive64 as root, give it permission to open raw sockets:

sudo setcap cap_net_raw=eip ./ioulive64

    📌 This allows ioulive64 to capture/send raw Ethernet frames without requiring root privileges.

📁 3. Create the UNIX socket directory

Make sure the required socket directory exists:

mkdir -p /tmp/netio$(id -u)

🚦 4. Run IOUlive64

./ioulive64 <ioulive-instance> <iou-instance> <ethernet-interface> <interface-id>

Arguments:

    <ioulive-instance>: Numeric ID for this IOUlive64 instance (e.g., 99)

    <iou-instance>: Numeric ID of the IOU router you’re connecting to (e.g., 98)

    <ethernet-interface>: Name of the physical interface to use (e.g., eth0)

    <interface-id>: Interface ID on the IOU router side (e.g., 0 for 0/0)

Example:

./ioulive64 99 98 eth0 0

This command:

    Connects to the router’s socket: /tmp/netio$(id -u)/98

    Binds its own socket at: /tmp/netio$(id -u)/99

    Bridges IOU interface 0/0 to the physical device eth0