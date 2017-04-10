/*
* This File Contains The Basic Classes and Methods Used
* To Quickly Create New Network Simulations
*/

#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/netanim-module.h"
#include "ns3/point-to-point-layout-module.h" // ????

// How To Set Default Packet Sizes (Random Traffic)
Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (1500)); // Limit of Ethernet
Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue ("1000kb/s")); // 1Mb dataRate

std::string animFile = "output.xml" ;  // Name of file for animation output

uint32_t N = 9 // Number of Nodes

CommandLine cmd; // Allow changing of nodes through console E.G --nNodes=25
cmd.AddValue ("nNodes", "Number of nodes to place in the star", N);
cmd.Parse (argc, argv);

NodeContainer clientNodes; // Create a CONTAINER
clientNodes.Create (N-1); // Create size based on N

NodeContainer allNodes = NodeContainer (serverNode, clientNodes); // Contain allNodes

InternetStackHelper internet;
internet.Install (allNodes); // Install internet stack on all nodes

// Create a node list
std::vector<NodeContainer> nodeAdjacencyList (N-1);
for(uint32_t i=0; i<nodeAdjacencyList.size (); ++i)
  {
    nodeAdjacencyList[i] = NodeContainer (serverNode, clientNodes.Get (i));
  }
// Collects subnets with server and clientNodes[i]

NS_LOG_INFO ("Create channels.");
PointToPointHelper p2p;
p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
std::vector<NetDeviceContainer> deviceAdjacencyList (N-1);
for(uint32_t i=0; i<deviceAdjacencyList.size (); ++i)
  {
    deviceAdjacencyList[i] = p2p.Install (nodeAdjacencyList[i]);
  } // creates channels for all subnets

  // Create the OnOff applications to send TCP to the server
OnOffHelper clientHelper ("ns3::TcpSocketFactory", Address ());
clientHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
clientHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));

// Create apps on each client, set address to server
ApplicationContainer clientApps;
for(uint32_t i=0; i<clientNodes.GetN (); ++i)
  {
    AddressValue remoteAddress
    (InetSocketAddress (interfaceAdjacencyList[i].GetAddress (0), port));
    clientHelper.SetAttribute ("Remote", remoteAddress);
    clientApps.Add (clientHelper.Install (clientNodes.Get (i)));
  }
clientApps.Start (Seconds (1.0));
clientApps.Stop (Seconds (10.0));

Ipv4GlobalRoutingHelper::PopulateRoutingTables (); // Create a routing table

  //configure tracing
AsciiTraceHelper ascii;
p2p.EnableAsciiAll (ascii.CreateFileStream ("tcp-star-server.tr")); // Traces Queues + Packets
p2p.EnablePcapAll ("tcp-star-server"); // pcap Trace Files for each "channel"

NS_LOG_INFO ("Run Simulation.");
Simulator::Run ();
Simulator::Destroy ();
NS_LOG_INFO ("Done.");
