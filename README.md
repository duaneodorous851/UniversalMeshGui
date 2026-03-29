# UniversalMesh GUI

**UniversalMesh GUI** is a ready-to-flash firmware collection for coordinators and sensor nodes, built on top of the [UniversalMesh](https://github.com/johestephan/UniversalMesh) mesh networking library. It provides a full-stack solution: node firmware that auto-discovers the coordinator and sends sensor data over ESP-NOW, and a coordinator firmware that bridges the mesh to your network and serves a real-time web dashboard — all without writing any code.

## Table of Contents

- [Features](#features)
- [Protocol Architecture](#protocol-architecture)
  - [Packet Structure](#packet-structure)
  - [Packet Type Reference](#packet-type-reference)
  - [AppId Reference](#appid-reference)
  - [Coordinator Discovery](#coordinator-discovery)
- [Supported Hardware](#supported-hardware)
  - [Coordinators](#coordinators)
  - [Sensor Nodes](#sensor-nodes)
- [Getting Started](#getting-started)
  - [Installation](#installation)
  - [Configuration](#configuration)
  - [Build](#build)
- [Sensor Node Behaviour](#sensor-node-behaviour)
- [Remote Commands](#remote-commands)
- [Coordinator Web Dashboard](#coordinator-web-dashboard)
  - [Panels](#panels)
  - [UI Features](#ui-features)
  - [REST API](#rest-api)
- [ETH Elite Features](#eth-elite-features)
- [RGB LED](#rgb-led)
- [messenger.py — Python Test Client](#messengerpy--python-test-client)
- [Development Stack](#development-stack)
- [License](#license)

---

## Features

### Mesh Protocol (via UniversalMesh library)
- **Auto-Relay:** Every node acts as a repeater (TTL-based) to extend range.
- **Self-Healing:** No fixed routing tables; packets find their path through broadcast/rebroadcast.
- **Transparent Discovery:** Built-in PING/PONG handling for network mapping.
- **De-duplication:** Prevents broadcast storms using unique Message IDs.

### GUI / Firmware (this project)
- **Ready-to-Flash Firmware:** Pre-built coordinator and sensor node firmware for a range of ESP32 and ESP8266 boards — no library integration needed.
- **Remote Commands:** Send `cmd:reboot`, `cmd:info`, `cmd:info:long` from the dashboard to any node; node replies with ACK and data.
- **Hybrid Coordinator:** Bridge your mesh to the internet via Wi-Fi or Ethernet.
- **Node Announce:** Sensors broadcast their name on boot and heartbeat; coordinator stores and displays it.
- **MQTT Bridge:** Coordinator automatically forwards mesh data to an MQTT broker (ETH Elite).
- **Real-Time Web Dashboard:** Live updates with a fast polling loop for packet log, node table, topology map and serial console.
- **Force-Directed Topology Map:** Visual graph of the mesh — nodes and relay paths inferred from packet headers, animated in real-time.
- **NTP Time Sync:** Coordinator syncs to NTP; packet timestamps shown as wall-clock time in the dashboard.
- **PWA Support:** Dashboard is installable on mobile as a standalone app (iOS & Android).

---

## Protocol Architecture

> Reference for the underlying [UniversalMesh](https://github.com/johestephan/UniversalMesh) library. Understanding this is useful for extending node firmware or building custom integrations.

The network consists of a **Coordinator** (the bridge) and multiple **Sensor Nodes**.

### Packet Structure

The library uses a fixed-size packed struct:

| Field | Size | Description |
| :--- | :--- | :--- |
| `type` | 1 byte | `0x12` PING, `0x13` PONG, `0x14` ACK, `0x15` DATA, `0x16` KEY_REQ, `0x17` SECURE_DATA, `0x18` PARANOID_DATA |
| `ttl` | 1 byte | Time-to-live hop limit (default 4) |
| `msgId` | 4 bytes | Unique message ID for deduplication |
| `destMac` | 6 bytes | Destination or `FF:FF:FF:FF:FF:FF` for broadcast |
| `srcMac` | 6 bytes | Original sender MAC |
| `appId` | 1 byte | Application multiplexer — see table below |
| `payloadLen` | 1 byte | Payload length (max 200) |
| `payload` | 200 bytes | Raw binary payload |

### Packet Type Reference

| Type | Name | Description |
| :--- | :--- | :--- |
| `0x00–0x11` | *(reserved)* | Reserved for future protocol extensions |
| `0x12` | PING | Broadcast by a node to discover the coordinator. Every mesh participant can reply with a PONG |
| `0x13` | PONG | Reply to PING. Coordinator marks itself with `appId=0xFF`, nodes use `appId=0x00` |
| `0x14` | ACK | Acknowledgement (reserved for future use) |
| `0x15` | DATA | Application data packet — see AppId table below |
| `0x16` | KEY_REQ | Node requests key material from coordinator |
| `0x17` | SECURE_DATA | AES-encrypted payload (library-managed on ESP32) |
| `0x18` | PARANOID_DATA | End-to-end encrypted payload pass-through |

### AppId Reference

| AppId | Name | Description |
| :--- | :--- | :--- |
| `0x00` | Protocol | Mesh control / discovery traffic |
| `0x01` | Text | Human-readable string payload |
| `0x02` | Raw Hex | Raw binary payload |
| `0x05` | Heartbeat | Periodic alive signal (single byte) |
| `0x06` | Node Announce | Node broadcasts its name; coordinator stores it in the node table |

### Coordinator Discovery

When a sensor node boots it has no knowledge of the coordinator's MAC address. Discovery works as follows:

1. The node sweeps channels and sends discovery **PING** probes.
2. Mesh participants reply with **PONG**.
3. The coordinator identifies itself using `appId=0xFF` in PONG replies.
4. The sensor node ignores non-coordinator PONGs and locks onto the first `appId=0xFF` responder.
5. If discovery fails, the node retries periodically.

This prevents a sensor node from accidentally treating a relay node or another sensor as the coordinator.

To mark a node as coordinator, pass `MESH_COORDINATOR` to `begin()`:

```cpp
mesh.begin(WIFI_CHANNEL, MESH_COORDINATOR); // coordinator — PONG replies carry appId 0xFF
mesh.begin(WIFI_CHANNEL, MESH_NODE);        // sensor / relay — PONG replies carry appId 0x00
```

---

## Supported Hardware

### Coordinators

| Environment | Board | Flash | PSRAM | Ethernet | OTA | RGB LED |
| :--- | :--- | :---: | :---: | :---: | :---: | :---: |
| `coordinator_c6` | ESP32-C6 DevKitC-1 | 4 MB | — | — | — | ✓ |
| `coordinator_s3` | ESP32-S3 DevKitC-1 | 4 MB | — | — | — | — |
| `coordinator_t8_s3` | LilyGo T8-S3 | 16 MB | ✓ | — | — | — |
| `coordinator_eth_elite` | LilyGo T-ETH Elite | 16 MB | ✓ | ✓ | ✓ | — |
| `coordinator_eth_elite_ota` | LilyGo T-ETH Elite | 16 MB | ✓ | ✓ | ✓ (IP) | — |

### Sensor Nodes

| Environment | Board | Notes |
| :--- | :--- | :--- |
| `node_sensor_esp32` | Generic ESP32 Dev Module | 4 MB flash; sends internal CPU temperature |
| `node_sensor_c6` | ESP32-C6 DevKitC-1 | USB CDC; sends internal CPU temperature |
| `node_sensor_t8_s3` | LilyGo T8-S3 | 16 MB flash, PSRAM; sends internal CPU temperature |
| `node_sensor_wemos_d1` | Wemos D1 / mini (ESP8266) | SHT30 sensor; optional OLED via `HAS_DISPLAY_SHIELD` |
| `node_sensor_esp8266` | NodeMCU v2 (ESP8266) | Generic ESP8266 node; sends simulated temperature |
| `node_sensor_esp32_pir` | Generic ESP32 Dev Module | PIR-enabled node profile |
| `node_sensor_esp8266_pir` | NodeMCU v2 (ESP8266) | PIR-enabled node profile |

---

## Getting Started

### Installation

1. Clone this repository.
2. Create `include/secrets.h` from `include/secrets.example.h` and fill in your credentials.
3. Build with one of the PlatformIO environments from `UniversalMeshGUI/coordinators.ini` or `UniversalMeshGUI/nodes.ini`.
4. ESP32 environments use the `pioarduino` ESP32 platform from `platformio.ini`; ESP8266 node environments use `espressif8266` (set per environment).

### Configuration

#### `include/secrets.h` — credentials (git-ignored)

Create this file before building. It holds secrets only:

```cpp
// WiFi (all coordinators)
#define WIFI_SSID "your-ssid"
#define WIFI_PASS "your-password"

// MQTT broker (ETH Elite only)
#define MQTT_BROKER "192.168.1.x"   // IP or hostname
#define MQTT_PORT   1883
#define MQTT_USER   ""              // leave empty if not required
#define MQTT_PASS   ""
```

#### `platformio.ini` build flags — non-secret settings

Network identity, OTA and NTP are configured as build flags per environment:

```ini
build_flags =
    -D MESH_HOSTNAME=\"universalmesh\"   ; mDNS name and MQTT path prefix
    -D MESH_NETWORK=\"mymesh\"           ; MQTT topic root
    -D OTA_PASSWORD=\"mesh1234\"         ; keep in sync with upload_flags --auth=
    -D NTP_SERVER=\"pool.ntp.org\"
    -D NTP_GMT_OFFSET_SEC=3600          ; UTC offset in seconds (e.g. 3600 = UTC+1)
```

`NODE_NAME` is set per sensor node environment:

```ini
build_flags =
    -D NODE_NAME=\"my-node\"
```

If not set, it falls back to `"sensor-node"` via an `#ifndef` guard in the source.

### Build

Flash and monitor a specific environment:

```
pio run -e coordinator_c6 -t upload -t monitor
pio run -e node_sensor_esp32 -t upload -t monitor
```

OTA upload (ETH Elite):

```
pio run -e coordinator_eth_elite_ota -t upload
```

---

## Sensor Node Behaviour

Each sensor node:

1. On boot, starts discovery to find the coordinator (see [Coordinator Discovery](#coordinator-discovery)); if no coordinator is found, discovery is retried periodically.
2. Once the coordinator is found, sends an **AppId `0x06`** Node Announce packet with its `NODE_NAME` string.
3. Every `HEARTBEAT_INTERVAL` ms (default 60 s), sends an **AppId `0x05`** heartbeat and re-sends the announce.
4. Sends sensor readings (e.g. temperature/humidity) as **AppId `0x01`** data packets.

---

## Remote Commands

Commands can be sent from the dashboard **Send Message** panel (pick from the **Cmd** dropdown or type freely) or via the `/api/tx` REST endpoint.  
Commands are plain-text `cmd:<name>` strings addressed to a specific node MAC. Nodes only execute commands whose `srcMac` matches the coordinator; unauthorized senders are ignored for execution (and logged on the node serial output).

For every recognised command the node sends an ACK first:

```
command received:<name>
```

Then sends command-specific reply packets (same `appId` as the incoming command).

### `cmd:reboot`

Reboots the node after a 100 ms flush delay. No data packet beyond the ACK.

### `cmd:info`

Single JSON reply (fits in one packet with the 200-byte payload limit):

```json
{"n":"sensor-esp32-green","mac":"AA:BB:CC:DD:EE:FF","up":142,"heap":213456,"rssi":-62,"ch":1,"chip":"ESP32","rev":1}
```

### `cmd:info:long`

In the current node firmware, `cmd:info` and `cmd:info:long` are handled the same way and return the same single JSON payload.

Replies appear in the packet log and are forwarded to MQTT on ETH Elite builds.

---

## Coordinator Web Dashboard

The coordinator serves a responsive single-page dashboard on port 80.

### Panels

| Panel | Description |
| :--- | :--- |
| **ESP** | Chip model, cores, CPU MHz, flash size, free heap, MAC, uptime, NTP time |
| **WiFi** | SSID, IP, gateway, RSSI, channel |
| **Ethernet** | Status, IP, subnet, gateway, DNS, MAC, link speed/duplex *(ETH Elite only)* |
| **Mesh Nodes** | Live list of known nodes — MAC, last-seen counter, resolved name. Coordinator always pinned at top. Click **Node** or **Last Seen** column header to sort. Green dot = seen <120 s, red = stale |
| **Mesh Channel** | Dropdown to switch ESP-NOW channel (1–13), persisted to NVS *(ETH Elite only)* |
| **Send Message** | Inject a text packet to any node or broadcast directly from the browser. Includes command presets (`cmd:info`, `cmd:info:long`, `cmd:reboot`) plus free-text mode |
| **Packet Log** | Paginated live log (200 entries with PSRAM, 10 without). Shows type, sender, app ID, payload, timestamp. Relayed packets highlighted |
| **Topology Map** | Force-directed canvas graph of all nodes and relay paths, inferred from packet headers. Click a node for details. Controls: Freeze/Resume physics, Reset layout, toggle MAC labels, toggle edge age labels, drag to pin a node, double-click to unpin, pan by dragging background, export as PNG |
| **Serial Console** | Live stream of the coordinator's internal log — like a web-based serial monitor |
| **Quick Actions (navbar)** | Toggle topology panel, toggle serial console panel, reboot coordinator, toggle dark/light theme |

### UI Features

- **Fast/slow polling model** — dashboard uses periodic fetches (`/api/nodes` + `/api/log` every 1 s, `/api/status` every 5 s, `/api/serial` every 2 s while console is open).
- **Dark / light theme** toggle (preference stored in `localStorage`)
- **Installable PWA** — add to home screen on iOS/Android for a standalone app experience
- **NTP timestamps** — packet log shows wall-clock time (e.g. `23:18:12`) instead of a relative counter when NTP is synced

### REST API

| Endpoint | Method | Description |
| :--- | :--- | :--- |
| `/api/status` | GET | Coordinator status (uptime, heap, IP, channel, NTP time, ETH info) |
| `/api/nodes` | GET | Node table (MAC, last-seen seconds, name) |
| `/api/log` | GET | Recent packet log (type, src, origSrc, appId, payload, age_s) |
| `/api/serial` | GET | Recent serial output (last 40 lines) |
| `/api/tx` | POST | Send a packet into the mesh (`dest`, `appId`, hex `payload`, `ttl`) |
| `/api/discover` | GET | Broadcast a PING to discover all mesh nodes |
| `/api/mesh/channel` | POST | Set ESP-NOW channel 1–13, optionally reboot *(ETH Elite only)* |
| `/api/reboot` | POST | Reboot the coordinator |

---

## ETH Elite Features

The `coordinator_eth_elite` / `coordinator_eth_elite_ota` builds add:

- **W5500 Ethernet** as primary network. On boot, waits up to 5 s for cable, then up to 60 s for DHCP. Falls back to WiFi if no cable is detected.
- **MQTT Bridge** — DATA packets are published to:
  ```
  {MESH_NETWORK}/{MESH_HOSTNAME}/nodes/{nodeName}/{appId}
  ```
- **Coordinator status topic** — retained runtime status is published to:
  ```
  {MESH_NETWORK}/{MESH_HOSTNAME}/coordinator/status
  ```
  Broker, port and credentials are set in `secrets.h`.
- **NTP sync** — wall-clock time displayed in dashboard and packet log.
- **OTA updates** — flash over the network:
  ```
  pio run -e coordinator_eth_elite_ota -t upload
  ```
- **mDNS** — reachable at `universalmesh.local` (hostname set via the `MESH_HOSTNAME` build flag in `platformio.ini`).
- **Mesh channel selector** — change ESP-NOW channel (1–13) via the dashboard, persisted to NVS across reboots.

---

## RGB LED

The ESP32-C6 coordinator uses pin 8 (NeoPixel) as a status indicator:

| State | Colour | Pattern |
| :--- | :--- | :--- |
| Connecting to WiFi | Green | Slow blink |
| WiFi connected | Green | Steady |
| WiFi failed | Red | Steady |
| Packet received | Yellow | 3 quick flashes |
| Packet transmitted | Blue | 3 quick flashes |

---

## `messenger.py` — Python Test Client

A CLI tool to inject packets into the mesh via the coordinator REST API.

```
python3 messenger.py                          # send one random message
python3 messenger.py -t "Hello mesh"          # send custom text
python3 messenger.py -l -i 2.0               # loop, one message every 2 s
python3 messenger.py -l -n 10 -i 1.0         # send 10 messages, 1 s apart
```

Edit `COORDINATOR_IP` at the top of the file to point to your coordinator.

---

## Development Stack

| Component | Technology | Scope |
| :--- | :--- | :--- |
| Core firmware | C++17 (Arduino / ESP-IDF) | All |
| Build system | PlatformIO | All |
| Mesh transport | ESP-NOW (ESP-IDF) | All |
| Web server | ESP Async WebServer (mathieucarbou fork) | Coordinator |
| Async TCP | AsyncTCP (mathieucarbou fork) | Coordinator (ESP32) |
| JSON | ArduinoJson 7.x | All |
| MQTT | PubSubClient 2.x | Coordinator (ETH Elite) |
| RGB LED | Adafruit NeoPixel 1.12.3 | Coordinator C6 |
| Temperature / humidity | Adafruit SHT31 Library | `node_sensor_wemos_d1` |
| OLED display | U8g2 | `node_sensor_wemos_d1` |
| Coordinator MCU | ESP32-C6, ESP32-S3, LilyGo T8-S3, LilyGo T-ETH Elite | Coordinator |
| Sensor MCU | Generic ESP32, ESP32-C6, LilyGo T8-S3 (ESP32-S3), Wemos D1 (ESP8266) | Sensor nodes |

---

## License

Licensed under the Apache License, Version 2.0.
