// Include necessary headers 
#include "ns3/antenna-module.h" 
#include "ns3/applications-module.h" 
#include "ns3/buildings-module.h" 
#include "ns3/config-store-module.h" 
#include "ns3/core-module.h" 
#include "ns3/flow-monitor-module.h" 
#include "ns3/internet-apps-module.h" 
#include "ns3/internet-module.h" 
#include "ns3/mobility-module.h" 
#include "ns3/nr-module.h" 
#include "ns3/point-to-point-module.h" 
#include "ns3/netanim-module.h"  // Include for NetAnim 
using namespace ns3; 
NS_LOG_COMPONENT_DEFINE("NrHandoverExample"); 
int main(int argc, char* argv[]) 
{ 
// Scenario parameters 
uint16_t gNbNum = 2; 
uint16_t ueNumPergNb = 1; 
24 
bool logging = false; 
bool enableHandover = true; 
// Simulation parameters 
Time simTime = Seconds(3.0); 
Time udpAppStartTime = Seconds(0.1); 
// NR parameters 
uint16_t numerology = 3; // corresponds to 120 kHz SCS 
double centralFrequency = 28e9; 
double bandwidth = 100e6; 
double totalTxPower = 43; 
// Command line arguments 
CommandLine cmd(__FILE__); 
cmd.AddValue("gNbNum", "The number of gNbs in the topology", gNbNum); 
cmd.AddValue("ueNumPergNb", "The number of UEs per gNb", ueNumPergNb); 
cmd.AddValue("logging", "Enable logging", logging); 
cmd.AddValue("enableHandover", "Enable handover", enableHandover); 
cmd.AddValue("simTime", "Simulation time", simTime); 
cmd.Parse(argc, argv); 
if (logging) 
{ 
LogComponentEnable("UdpClient", LOG_LEVEL_INFO); 
LogComponentEnable("UdpServer", LOG_LEVEL_INFO); 
LogComponentEnable("NrHelper", LOG_LEVEL_INFO); 
} 
Config::SetDefault("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue(999999999)); 
25 
// Create gNBs and UEs 
NodeContainer gNbNodes, ueNodes; 
gNbNodes.Create(gNbNum); 
ueNodes.Create(gNbNum * ueNumPergNb); 
// Configure mobility for gNBs and UEs 
MobilityHelper gNbMobility; 
gNbMobility.SetMobilityModel("ns3::ConstantPositionMobilityModel"); 
gNbMobility.Install(gNbNodes); 
MobilityHelper ueMobility; 
ueMobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel"); 
ueMobility.Install(ueNodes); 
// Position gNBs and set UE velocity 
Ptr<ListPositionAllocator> gNbPositionAlloc = CreateObject<ListPositionAllocator>(); 
gNbPositionAlloc->Add(Vector(0.0, 0.0, 10.0)); 
gNbPositionAlloc->Add(Vector(500.0, 0.0, 10.0)); // Place second gNB at 500m 
gNbMobility.SetPositionAllocator(gNbPositionAlloc); 
gNbMobility.Install(gNbNodes); 
for (uint32_t i = 0; i < ueNodes.GetN(); ++i) 
{ 
Ptr<ConstantVelocityMobilityModel> mob = ueNodes.Get(i)
>GetObject<ConstantVelocityMobilityModel>(); 
mob->SetVelocity(Vector(20, 0, 0)); // Move UEs with a speed of 20 m/s 
} 
// Install the internet stack on UEs before attaching to gNBs 
26 
InternetStackHelper internet; 
internet.Install(ueNodes); 
// Install NR protocol stack 
Ptr<NrHelper> nrHelper = CreateObject<NrHelper>(); 
Ptr<IdealBeamformingHelper> idealBeamformingHelper = 
CreateObject<IdealBeamformingHelper>(); 
Ptr<NrPointToPointEpcHelper> epcHelper = 
CreateObject<NrPointToPointEpcHelper>(); 
nrHelper->SetBeamformingHelper(idealBeamformingHelper); 
nrHelper->SetEpcHelper(epcHelper); 
BandwidthPartInfoPtrVector allBwps; 
CcBwpCreator ccBwpCreator; 
CcBwpCreator::SimpleOperationBandConf bandConf(centralFrequency, 
bandwidth, 
1, 
BandwidthPartInfo::UMi_StreetCanyon); 
OperationBandInfo band = 
ccBwpCreator.CreateOperationBandContiguousCc(bandConf); 
nrHelper->InitializeOperationBand(&band); 
allBwps = CcBwpCreator::GetAllBwps({band}); 
// Set antenna configuration 
nrHelper->SetUeAntennaAttribute("NumRows", UintegerValue(2)); 
nrHelper->SetUeAntennaAttribute("NumColumns", UintegerValue(4)); 
nrHelper->SetGnbAntennaAttribute("NumRows", UintegerValue(4)); 
nrHelper->SetGnbAntennaAttribute("NumColumns", UintegerValue(8)); 
27 
// Install NR devices 
NetDeviceContainer enbNetDev = nrHelper->InstallGnbDevice(gNbNodes, allBwps); 
NetDeviceContainer ueNetDev = nrHelper->InstallUeDevice(ueNodes, allBwps); 
nrHelper->GetGnbPhy(enbNetDev.Get(0), 0)->SetAttribute("Numerology", 
UintegerValue(numerology)); 
nrHelper->GetGnbPhy(enbNetDev.Get(0), 0)->SetAttribute("TxPower", 
DoubleValue(totalTxPower)); 
for (auto it = enbNetDev.Begin(); it != enbNetDev.End(); ++it) 
{ 
DynamicCast<NrGnbNetDevice>(*it)->UpdateConfig(); 
} 
for (auto it = ueNetDev.Begin(); it != ueNetDev.End(); ++it) 
{ 
DynamicCast<NrUeNetDevice>(*it)->UpdateConfig(); 
} 
// Install and configure the EPC stack 
Ptr<Node> pgw = epcHelper->GetPgwNode(); 
NodeContainer remoteHostContainer; 
remoteHostContainer.Create(1); 
Ptr<Node> remoteHost = remoteHostContainer.Get(0); 
internet.Install(remoteHostContainer); 
PointToPointHelper p2ph; 
p2ph.SetDeviceAttribute("DataRate", DataRateValue(DataRate("100Gb/s"))); 
p2ph.SetDeviceAttribute("Mtu", UintegerValue(2500)); 
p2ph.SetChannelAttribute("Delay", TimeValue(Seconds(0.000))); 
28 
NetDeviceContainer internetDevices = p2ph.Install(pgw, remoteHost); 
Ipv4AddressHelper ipv4h; 
ipv4h.SetBase("1.0.0.0", "255.0.0.0"); 
Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign(internetDevices); 
Ipv4StaticRoutingHelper ipv4RoutingHelper; 
Ptr<Ipv4StaticRouting> remoteHostStaticRouting = 
ipv4RoutingHelper.GetStaticRouting(remoteHost->GetObject<Ipv4>()); 
remoteHostStaticRouting->AddNetworkRouteTo(Ipv4Address("7.0.0.0"), 
Ipv4Mask("255.0.0.0"), 1); 
Ipv4InterfaceContainer ueIpIface = epcHelper
>AssignUeIpv4Address(NetDeviceContainer(ueNetDev)); 
// Set the default gateway for the UEs 
for (uint32_t j = 0; j < ueNodes.GetN(); ++j) 
{ 
Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting( 
ueNodes.Get(j)->GetObject<Ipv4>()); 
ueStaticRouting->SetDefaultRoute(epcHelper->GetUeDefaultGatewayAddress(), 1); 
} 
// Attach UEs to the closest gNB 
nrHelper->AttachToClosestEnb(ueNetDev, enbNetDev); 
if (enableHandover) 
{ 
// Optionally, add code here to configure any specific handover settings, if needed. 
} 
// Install and configure traffic applications 
29 
uint16_t dlPort = 1234; 
ApplicationContainer clientApps, serverApps; 
UdpServerHelper dlPacketSink(dlPort); 
serverApps.Add(dlPacketSink.Install(ueNodes)); 
UdpClientHelper dlClient; 
dlClient.SetAttribute("RemotePort", UintegerValue(dlPort)); 
dlClient.SetAttribute("MaxPackets", UintegerValue(0xFFFFFFFF)); 
dlClient.SetAttribute("PacketSize", UintegerValue(1024)); 
dlClient.SetAttribute("Interval", TimeValue(MilliSeconds(1))); 
for (uint32_t i = 0; i < ueNodes.GetN(); ++i) 
{ 
Address ueAddress = ueIpIface.GetAddress(i); 
dlClient.SetAttribute("RemoteAddress", AddressValue(ueAddress)); 
clientApps.Add(dlClient.Install(remoteHost)); 
} 
serverApps.Start(udpAppStartTime); 
clientApps.Start(udpAppStartTime); 
serverApps.Stop(simTime); 
clientApps.Stop(simTime); 
// Enable flow monitoring 
FlowMonitorHelper flowmonHelper; 
Ptr<FlowMonitor> monitor = flowmonHelper.InstallAll(); 
// Enable NetAnim trace 
AnimationInterface anim("cttc-nr-demo.xml"); 
30 
Simulator::Stop(simTime); 
Simulator::Run(); 
// Print per-flow statistics 
monitor->CheckForLostPackets(); 
Ptr<Ipv4FlowClassifier> classifier = 
DynamicCast<Ipv4FlowClassifier>(flowmonHelper.GetClassifier()); 
FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats(); 
double totalSentPackets = 0.0; 
double totalLostPackets = 0.0; 
double averageFlowThroughput = 0.0; 
double averageFlowDelay = 0.0; 
std::ofstream csvFile; 
csvFile.open("simulation_results.csv"); 
csvFile << "Flow ID,Throughput (Mbps),Delay (ms),Packet Loss Rate (%)\n"; 
for (auto i = stats.begin(); i != stats.end(); ++i) 
{ 
if (i->second.rxPackets > 0) 
{ 
double throughput = i->second.rxBytes * 8.0 / simTime.GetSeconds() / 1e6; 
double delay = i->second.delaySum.GetSeconds() / i->second.rxPackets * 1000; 
double packetLossRate = (i->second.lostPackets * 100.0) / (i->second.txPackets); 
averageFlowThroughput += throughput; 
averageFlowDelay += delay; 
totalSentPackets += i->second.txPackets; 
31 
totalLostPackets += i->second.lostPackets; 
csvFile << i->first << "," << throughput << "," << delay << "," << packetLossRate 
<< "\n"; 
} 
} 
averageFlowThroughput /= stats.size(); 
averageFlowDelay /= stats.size(); 
double overallPacketLossRate = (totalLostPackets / totalSentPackets) * 100.0; 
std::cout << "Average Flow Throughput: " << averageFlowThroughput << " Mbps\n"; 
std::cout << "Average Flow Delay: " << averageFlowDelay * 1000 << " ms\n"; 
std::cout << "Overall Packet Loss Rate: " << overallPacketLossRate << " %\n"; 
// Save FlowMonitor results 
monitor->SerializeToXmlFile("flowmon-results.xml", true, true); 
csvFile.close(); 
Simulator::Destroy(); 
return 0; 
} 