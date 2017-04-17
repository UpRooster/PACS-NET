#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/csma-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("RING - 8 Nodes");

void startUp ()
{
  Time::SetResolution(Time::NS);  // Set the simulation time resolution

  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);  // Enable client UDP logging - Unused without OnOffApplication
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);  // Enable server UDP logging - Unused without OnOffApplication
  Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (1500));   // Packet size (Bytes) - Unused without OnOffApplication
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue ("1000kb/s")); // Data transmission rate for OnOffApplication - Unused without OnOffApplication
}

std::vector<NodeContainer> channelCreation (std::vector<NodeContainer> subnetList, NodeContainer Nodes)
{
  for(uint32_t i=0; i<subnetList.size(); ++i)
  {
    if (i == subnetList.size()-1)
    {
      NS_LOG_UNCOND ("Closing Ring!");
      subnetList[i] = NodeContainer (Nodes.Get(i), Nodes.Get(0));
    }
    else
    {
      NS_LOG_UNCOND ("Creating Subnet " << i);
      subnetList[i] = NodeContainer (Nodes.Get(i), Nodes.Get(i+1));
    }
  }
  return subnetList;
}

std::vector<Ipv4InterfaceContainer> subnetCreation (std::vector<NetDeviceContainer> deviceList, std::vector<NodeContainer> subnetList, std::vector<Ipv4InterfaceContainer> subNetInterfaces, PointToPointHelper p2p)
{
  // CREATE IPV4 HELPER & IP STRING
  Ipv4AddressHelper address;
  std::ostringstream subnetAddr;
  for(uint32_t i=0; i<deviceList.size(); ++i)
  {
    subnetAddr.str("");
    deviceList[i] = p2p.Install (subnetList[i]);
    subnetAddr <<"10.1."<<i+1<<".0";
    NS_LOG_UNCOND ("Creating Address " << subnetAddr.str().c_str ());
    address.SetBase(subnetAddr.str().c_str (),"255.255.255.0");
    subNetInterfaces[i] = address.Assign(deviceList[i]);
  }
  return subNetInterfaces;
}

int main (int argc, char *argv[])
{
  // Clinic
  NS_LOG_UNCOND ("Starting..");

  startUp();  // Call startup function

  // SET VARIABLES
  std::string animFile = "RING8.xml" ;  // Name of file for animation output
  uint32_t nNodes = 8;
  uint16_t timer = 60;

  // PARSE COMMANDLINE
  CommandLine cmd;        // Create CommandLine object
  cmd.AddValue ("nNodes", "Nodes to place", nNodes);                // Add value for desired nodes from argc
  cmd.AddValue ("animFile",  "Name for Animation File", animFile);  // Add value for output file name from argv
  cmd.Parse (argc,argv);  // Parse argc, argv from commandline args (If present)

  // CREATE NODES
  NS_LOG_UNCOND ("Creating Nodes: " << nNodes);
  NodeContainer Nodes;
  Nodes.Create(nNodes);

  // INSTALL INTERNET STACKS
  InternetStackHelper Stack;
  Stack.Install(Nodes);

  // CREATE P2P HELPER
  PointToPointHelper p2p;

  // CREATE CHANNEL CONTAINER
  NS_LOG_UNCOND ("Creating Subnet List");
  std::vector<NodeContainer> subnetList (nNodes);

  // CREATE CHANNELS
  NS_LOG_UNCOND ("Creating Subnets");
  subnetList = channelCreation(subnetList, Nodes);
  uint16_t NSize =  subnetList.size();

  // CREATE NET DEVICES
  NS_LOG_UNCOND ("Creating Devices");
  std::vector<NetDeviceContainer> deviceList (nNodes);
  std::vector<Ipv4InterfaceContainer> subNetInterfaces (nNodes);

  // ASSIGN CHANNELS ADDRESSES
  NS_LOG_UNCOND ("Assigning Addresses to Channels");
  subNetInterfaces = subnetCreation(deviceList, subnetList, subNetInterfaces, p2p);
  uint16_t ISize =  NSize-1;

  NS_LOG_UNCOND ("DeviceListSize: "<< ISize);
  NS_LOG_UNCOND ("Creating Address/App");
  /*----------------ADDRESS/APP CREATION----------------*/
  UdpEchoServerHelper echoServer (9); // Set Server Port
  NS_LOG_UNCOND ("Creating Server");
  ApplicationContainer serverApps = echoServer.Install (Nodes.Get (nNodes/2)); // Set Server Node
  serverApps.Start (Seconds (1.0)); // Set open time
  serverApps.Stop (Seconds (timer)); // Set close time
  NS_LOG_UNCOND ("Creating Client Target");
  UdpEchoClientHelper echoClient (subNetInterfaces[ISize/2].GetAddress (1), 9); // Set Client Target with servers subNetInterfaces[i].GetAddress & Port
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1)); // Set sending data
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));
  NS_LOG_UNCOND ("Creating Client");
  ApplicationContainer clientApps = echoClient.Install (Nodes.Get (0)); // Set Client Node
  clientApps.Start (Seconds (2.0)); // Set open time
  clientApps.Stop (Seconds (timer)); // Set close time

  NS_LOG_UNCOND ("Creating Animation");
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  AnimationInterface anim (animFile);
  for(uint32_t i=0; i<subnetList.size(); ++i)
  {
    anim.SetConstantPosition (Nodes.Get(i), i, i*i);
  }
  /*----------------RUN SIMULATION----------------*/
  Simulator::Stop (Seconds (timer));
  Simulator::Run();
  std::cout << "Animation Trace file created:" << animFile.c_str ()<< std::endl;
  Simulator::Destroy();
}
