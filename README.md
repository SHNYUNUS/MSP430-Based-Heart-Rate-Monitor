# MSP430-Based-Heart-Rate-Monitor
A heart rate monitor system built with MSP430 microcontroller and pulse sensor.
# MSP430 Based Heart Rate Monitor

A heart rate monitoring system built with the MSP430 microcontroller and a pulse sensor.  
Developed with **IAR Embedded Workbench IDE**, this project reads real-time pulse signals via the MSP430 ADC module and displays the heart rate on a computer screen.  
A custom PCB was designed for stable hardware integration, and MATLAB was used for real-time heart rate visualization.

---

## 📌 Features
- MSP430G2553 microcontroller firmware written in C
- Pulse sensor for BPM (beats per minute) measurement
- Real-time BPM display on a computer screen
- Custom-designed PCB for compact and reliable hardware
- MATLAB integration for live heart rate graph plotting

---

## 🛠 Hardware Used
- **MSP430G2553** Microcontroller (LaunchPad or custom PCB)
- **Pulse Sensor**
- **USB to UART** interface (for PC communication)
- **Custom PCB** (KiCad design)
- Breadboard, jumper wires, and connectors

---

## 💻 Software Used
- **IAR Embedded Workbench IDE** (for MSP430 firmware development)
- **MATLAB** (for real-time graph plotting)
- **KiCad** (for PCB design)

---

## ⚙️ How It Works
1. The pulse sensor detects heartbeat signals from the user’s finger.
2. The MSP430 samples the analog signal using its **ADC module**.
3. The firmware processes the signal and calculates the BPM.
4. BPM data is sent to the computer over UART.
5. MATLAB script reads the serial data and plots it in real-time.

---

## 📂 Project Structure





---


---

## 🖼 PCB Design
All schematic and PCB layout files are available in the **hardware/** folder.  
You can open them with **KiCad** or view the exported PDF/PNG files.

---

## 📊 MATLAB Output Example
*(Add a screenshot of your MATLAB real-time graph here)*

---

## 🚀 How to Run
### MSP430 Firmware
1. Open the `main.c` file in **IAR Embedded Workbench**.
2. Compile and upload the code to the MSP430.
3. Connect the MSP430 to your computer via USB.

### MATLAB Script
1. Open `pulse_graphc.m` in MATLAB.
2. Set the correct **serial port name** and **baud rate** in the script.
3. Run the script and observe the real-time heart rate graph.

---

## 📜 License
This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.

