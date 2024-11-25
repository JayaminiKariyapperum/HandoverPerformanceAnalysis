from mininet.net import Mininet 
from mininet.node import RemoteController 
from mininet.link import TCLink 
import time 
import os 
# Function to measure throughput and packet loss between two hosts 
def measure_throughput_and_loss(net, src, dst, duration=20): 
src_host = net.get(src) 
dst_host = net.get(dst) 
# Start iperf server on the destination host 
dst_host.cmd('iperf -s &') 
# Use iperf to measure the throughput and capture the packet statistics 
output = src_host.cmd(f'iperf -c {dst_host.IP()} -t {duration} -f m') 
sent_packets = 0 
received_packets = 0 
throughput = 0.0 
# Extract the throughput and packet statistics from the output 
for line in output.split('\n'): 
if "Mbits/sec" in line: 
throughput, unit = line.split()[-2:] 
if "datagrams" in line: 
35 
parts = line.split() 
sent_packets = int(parts[1]) 
received_packets = int(parts[3]) 
# Stop the iperf server 
dst_host.cmd('killall -9 iperf') 
return float(throughput), unit, sent_packets, received_packets 
# Stop the iperf server if the loop fails 
dst_host.cmd('killall -9 iperf') 
return 0, 'Mbits/sec', sent_packets, received_packets 
# Function to calculate packet loss 
def calculate_packet_loss(sent_packets, received_packets): 
if sent_packets == 0: 
return 0.0 
loss = ((sent_packets - received_packets) / sent_packets) * 100 
return loss 
# Function to measure latency between two hosts using ping 
def measure_latency(net, src, dst): 
src_host = net.get(src) 
dst_host = net.get(dst) 
# Use ping to measure the latency 
output = src_host.cmd(f'ping -c 10 {dst_host.IP()}') 
# Extract the average latency from the ping output 
for line in output.split('\n'): 
if "rtt min/avg/max" in line: 
36 
avg_latency = line.split('/')[4] 
return float(avg_latency) 
return 0.0 
# Function to log handover delay, throughput, latency, and packet loss 
def log_handover_metrics(start_time, end_time, pre_throughput, post_throughput, 
pre_latency, post_latency, pre_loss, post_loss, 
log_file="/home/jaye/handover_metrics.log"): 
delay = end_time - start_time 
with open(os.path.expanduser(log_file), "a") as f: 
f.write(f"{start_time}, {end_time}, {delay}, {pre_throughput}, {post_throughput}, 
{pre_latency}, {post_latency}, {pre_loss}, {post_loss}\n") 
print(f"Handover delay: {delay} seconds") 
print(f"Pre-handover throughput: {pre_throughput} Mbits/sec") 
print(f"Post-handover throughput: {post_throughput} Mbits/sec") 
print(f"Pre-handover latency: {pre_latency} ms") 
print(f"Post-handover latency: {post_latency} ms") 
print(f"Pre-handover packet loss: {pre_loss}%") 
print(f"Post-handover packet loss: {post_loss}%") 
# Function to trigger handover based on real-time conditions 
def check_handover_condition(pre_handover_throughput, pre_handover_latency, 
throughput_threshold=9, latency_threshold=80): 
# Check if throughput drops below the threshold or latency exceeds the threshold 
if pre_handover_throughput < throughput_threshold or pre_handover_latency > 
latency_threshold: 
print(f"Handover triggered: Throughput: {pre_handover_throughput}, Latency: 
{pre_handover_latency}") 
return True 
return False 
# Main SDN-based handover topology function 
37 
def sdn_handover_complex_topology(): 
# Create the network with a remote controller 
net = Mininet(controller=RemoteController, link=TCLink) 
# Add a remote controller 
net.addController('c0', controller=RemoteController, ip='127.0.0.1', port=6653) 
# Add switches 
s1 = net.addSwitch('s1') 
s2 = net.addSwitch('s2') 
s3 = net.addSwitch('s3') 
# Add hosts 
h1 = net.addHost('h1') 
h2 = net.addHost('h2') 
h3 = net.addHost('h3') 
# Add links between switches and hosts with bandwidth limitation and latency 
net.addLink(s1, h1, bw=5, delay='100ms')  # Simulate bad conditions on h1 
net.addLink(s2, h2, bw=10, delay='20ms') 
net.addLink(s3, h3, bw=10, delay='30ms') 
# Add links between switches 
net.addLink(s1, s2, bw=10, delay='5ms') 
net.addLink(s2, s3, bw=10, delay='10ms') 
# Start the network 
net.start() 
# Measure throughput, latency, and packet loss before handover 
38 
print("Measuring throughput, latency, and packet loss between h1 and h2 for 20 
seconds.") 
pre_handover_throughput, unit, pre_sent, pre_received = 
measure_throughput_and_loss(net, 'h1', 'h2', duration=20) 
pre_handover_latency = measure_latency(net, 'h1', 'h2') 
pre_loss = calculate_packet_loss(pre_sent, pre_received) 
print(f"Pre-handover throughput: {pre_handover_throughput} {unit}") 
print(f"Pre-handover latency: {pre_handover_latency} ms") 
print(f"Pre-handover packet loss: {pre_loss}%") 
print("Simulating handover...") 
start_time = time.time() 
# Detach h1 from s1 and attach it to s2 
print("Detaching h1 from s1 and attaching to s2...") 
s1.detach(h1.intf()) 
s2.attach(h1.intf()) 
# Wait for the network to stabilize (e.g., 5 seconds delay) 
time.sleep(5) 
end_time = time.time() 
# Measure throughput, latency, and packet loss after handover 
print("Measuring throughput, latency, and packet loss between h1 and h2 for 20 
seconds after handover.") 
post_handover_throughput, unit, post_sent, post_received = 
measure_throughput_and_loss(net, 'h1', 'h2', duration=20) 
post_handover_latency = measure_latency(net, 'h1', 'h2') 
post_loss = calculate_packet_loss(post_sent, post_received) 
print(f"Post-handover throughput: {post_handover_throughput} {unit}") 
print(f"Post-handover latency: {post_handover_latency} ms") 
39 
print(f"Post-handover packet loss: {post_loss}%") 
# Log the handover delay, throughput, latency, and packet loss 
log_handover_metrics(start_time, end_time, pre_handover_throughput, 
post_handover_throughput, pre_handover_latency, post_handover_latency, pre_loss, 
post_loss) 
else: 
print("Handover not triggered, network conditions are stable.") 
# Stop the network 
net.stop() 
if __name__ == '__main__': 
sdn_handover_complex_topology() 