#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/csma-module.h"
#include "ns3/netanim-module.h"
#include "ns3/point-to-point-grid.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("GRID - 32 Nodes");

int main (int argc, char *argv[])
{
  // Clinic
  NS_LOG_UNCOND ("Starting..");
  Time::SetResolution(Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (1500)); // Limit of Ethernet
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue ("1000kb/s")); // 1Mb dataRate
  std::string animFile = "Grid32.xml" ;  // Name of file for animation output

  // Set Node Size
  uint32_t nNodes = 32;
  uint32_t timer = 60;

  CommandLine cmd;
  cmd.AddValue ("nNodes", "Nodes to place", nNodes); // Allow command line node setting
  cmd.AddValue ("animFile",  "Name for Animation File", animFile); // Allow command line animation file naming

  cmd.Parse (argc,argv);
  NS_LOG_UNCOND ("Creating Nodes: " << nNodes);
  // Init Nodes
  NodeContainer Nodes;
  Nodes.Create(nNodes);

  // Create grid dimensions
  uint32_t cols = (uint32_t)sqrt(nNodes);
  uint32_t rows = (uint32_t)ceil(nNodes/(double)cols);

  // Init p2p (p2p)
  PointToPointHelper p2p;

  // Create grid
  PointToPointGridHelper grid (rows,cols,p2p);

  // Init Stacks on grid nodes
  InternetStackHelper Stack;
  grid.InstallStack(Stack);

  // Init Ipv4 + Address String
  Ipv4AddressHelper address;

  NS_LOG_UNCOND ("Creating Subnet List");
  // Assign Addresses to Grid
  grid.AssignIpv4Addresses (Ipv4AddressHelper ("10.1.1.0", "255.255.255.0"),
                            Ipv4AddressHelper ("10.2.1.0", "255.255.255.0"));

  NS_LOG_UNCOND ("Setting Server Port");
  /*----------------ADDRESS/APP CREATION----------------*/
  UdpEchoServerHelper echoServer (9); // Set Server Port

  NS_LOG_UNCOND ("Creating Server");
  ApplicationContainer serverApps = echoServer.Install (grid.GetNode (rows-1,cols-1)); // Set Server Node
  serverApps.Start (Seconds (1.0)); // Set open time
  serverApps.Stop (Seconds (timer)); // Set close time

  NS_LOG_UNCOND ("Creating Client Target");
  UdpEchoClientHelper echoClient (InetSocketAddress (grid.GetIpv4Address (rows-1,cols-1), 1000)); // Set Client Target with grid.GetIpv4Address (X,Y Dimensions) + Port
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1)); // Set sending data
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  NS_LOG_UNCOND ("Creating Client");
  ApplicationContainer clientApps = echoClient.Install (grid.GetNode (0,0)); // Set Client Node
  clientApps.Start (Seconds (2.0)); // Set open time
  clientApps.Stop (Seconds (timer)); // Set close time

  NS_LOG_UNCOND ("Creating Animation");
  /*-----------------ANIMATION CREATION----------------*/
  grid.BoundingBox (1, 1, 100, 100);  // Set animtion view size
  AnimationInterface anim (animFile);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  /*----------------RUN SIMULATION----------------*/
  Simulator::Stop (Seconds (timer));
  Simulator::Run();
  std::cout << "Animation Trace file created:" << animFile.c_str ()<< std::endl;
  Simulator::Destroy();
}
