

# **Handover Performance Analysis in 5G Networks using SDN Integration**

This repository contains the code for simulating and analyzing traditional and SDN-based handover mechanisms in 5G networks. The project evaluates performance metrics such as throughput, latency, and packet loss, demonstrating the potential of SDN in optimizing handover processes.

---

## **Technologies Used**
- **Simulation Tools:** ns-3, Mininet
- **Frameworks:** Ryu SDN Controller
- **Programming Languages:** C++, Python

---

## **How to Use**

### **Traditional Handover Simulation (ns-3)**
1. Navigate to the `ns-3` directory.
2. Compile and run the simulation:
   ```bash
   ./waf --run "handover-simulation"
   ```

### **SDN-Based Handover Simulation (Mininet)**
1. Navigate to the `Mininet` directory.
2. Start the Mininet topology:
   ```bash
   sudo python3 sdn_handover.py
   ```
3. Start the Ryu SDN controller:
   ```bash
   ryu-manager ryu_controller.py
   ```

---

## **Key Features**
- Simulates and compares **traditional** and **SDN-based handover mechanisms**.
- Centralized SDN controller for dynamic, real-time decision-making.
- Performance metrics: **Throughput**, **latency**, and **packet loss**.

