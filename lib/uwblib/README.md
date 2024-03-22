# Realtime location Network Library

## Overview

This library is a realtime scalable location network library using uwb technology with up to hundred of nodes in the network.

Author: Toan Huynh

<br>

## Real-Time Location Network
Supporting TWR and location engine.
Reconfiguration process for faster local network reconfiguration, and accounting for node mobility.
Ensuring collision-free scheduling of bi-directional communication links to support TWR measurements by enlarging the contention area.
Proposing a ranging pair selection protocol to maximize the ranging rate between nodes, which supports node mobility and high-dynamic applications.

<br>

## 1. Network Topology
### 1.1  2E-ASAP
2E-ASAP is an adaptive time division multiple access (TDMA) slot assignment protocol (ATSA) extension. This section tasks about contention area, frame format, a data format that enables bi-directional communication per slot, slot management changes, and 2E-ASAP’s network reconfiguration processes.

### 1.2  Contention Area
2E-ASAP enables collision-free, bi-directional communications within each slot by enlarging the contention area to three hops. A three-hop contention area is necessary because it allows each slot’s assigned node to select any neighbor when establishing a two-way link.

### 1.3  TDMA Frame Format
The TDMA frame format has special features regarding frame length and the first slot. Uniform frame length is not required across the network and is adjusted dynamically with network configuration to minimize the number of slots per frame and increase channel utilization. 

### 1.4  Data Format
2E-ASAP stores and shares the same assignment information as E-ASAP, but 2E-ASAP includes an additional hop of nodes to support a larger contention area. Each node maintains the address, frame length, slot assignments, and number of hops for each node in its three-hop contention area. INF packets are populated with addresses, frame lengths, slot assignments, and a number of hops for the sending node and each node within two hops.

### 1.5  Slot Management
In 2E-ASAP, nodes always transmit INF packets in their assigned slots. Regular broadcasts of INF packets increase the availability of assignment information throughout the network and facilitate faster local reconfiguration. Nodes continuously receive, process, and transmit INF packets, allowing them to detect, integrate, and broadcast network changes.

### 1.6  New Node Joins
When a new node joins the network, it determines the network configuration and the reserved slot by collecting INF packets for an initial listening time. All assignment information is copied and at the end of the collection period, the joining node resolves conflicts and assigns itself a slot. The node transmits an INF packet in the reserved slot to announce itself along with the new network configuration. At this point, the new node has joined its local network and is eligible to transmit and receive payload data from neighboring nodes.

### 1.7  Node Leaves
The process for nodes leaving the network is designed to account for node mobility and the possibility that the node has remained within the contention area but is now more hops away. Existing nodes are detected via the expiration of timeout timers, which are refreshed upon direct communication and the presence of the node within received INF packets. When a timeout occurs, the detecting node does not immediately free the timed-out node’s assignments but instead assumes it moved one hop away. If timeouts continue, the process repeats until the timed-out node reaches four hops and is no longer in the contention area. When this happens, the detecting node releases all slots assigned to itself and the timed-out node and then assigns itself new slots.

### 1.8 Networks Meet
Packet collisions may occur when existing networks initially meet, but once a node from one network receives an INF from the other network, it handles
reconfiguration and propagates the changes to the rest of the combined network.

## 2. Two-Way Ranging
The ranging protocol utilizes 2E-ASAP’s bi-directional links to schedule TWR in such a way that maximizes ranging rate between each node pair while being robust to communication errors and transient network states. The ranging rate is maximized by prioritizing selection according to ranging timestamps. When a ranging measurement completes successfully, both nodes store the ranging timestamp. The older the timestamp, the higher priority a node has for selection.

In the assigned slot, the tag ranges with 4 anchors after that it sends an INF packet.

## 4 Ranging rate
To help reduce power consumption the tag can adjust the ranging rate. 
See application\application_definitions.h to know how to configure it.

### 4.1 Sleep Mode
After making TWR with anchors in the assigned slot, the tag (DW1000) will go to idle time. It has 3 options in idle time, default is deep sleep mode:
Do nothing: the tag will continue to listen to INF packets from the network. MCU will wake up to process all received packets. In this mode, power consumption is very high.
Turn off receiver: the tag turns off the receiver, UWB device clock still remains. 
Deep Sleep: the tag enters deep sleep, this mode is the lowest power state. It also schedules to wake up for the next ranging time.

See application\application_definitions.h to know how to configure sleep mode.

## Debug

Enable Terminal Debug On DW1001C

- Disable host connection: HOST_CONNECTION_ENABLE_WRITE = 0
- Enable serial printf: CONFIG_ENABLE_UART = 1

<br>

## License
This source is Toan Huynh proprietary and confidential! 
No part of this source may be disclosed in any manner to a third party without prior written consent of Toan Huynh.
