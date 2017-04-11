#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/csma-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("P2P - 16 Nodes");

int main (int argc, char *argv[])
{
  // Clinic
  NS_LOG_UNCOND ("Starting..");
  Time::SetResolution(Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (1500)); // Limit of Ethernet
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue ("1000kb/s")); // 1Mb dataRate
  std::string animFile = "P2P16.xml" ;  // Name of file for animation output

  // Set Node Size
  uint32_t nNodes = 16;
  uint16_t timer = 60;

  CommandLine cmd;
  cmd.AddValue ("nNodes", "Nodes to place", nNodes); // Allow command line node setting
  cmd.AddValue ("animFile",  "Name for Animation File", animFile); // Allow command line animation file naming

  cmd.Parse (argc,argv);
  NS_LOG_UNCOND ("Creating Nodes: " << nNodes);
  // Init Nodes
  NodeContainer Nodes;
  Nodes.Create(nNodes);

  // Init Stacks
  InternetStackHelper Stack;
  Stack.Install(Nodes);

  // Init p2p (p2p)
  PointToPointHelper p2p;

  // Init Ipv4 + Address String
  Ipv4AddressHelper address;
  std::ostringstream subnetAddr;
  
  NS_LOG_UNCOND ("Creating Subnet List");
  /*----------------SUBNET CREATION----------------*/
  std::vector<NodeContainer> subnetList (nNodes);
  NS_LOG_UNCOND ("Creating Subnets");
  for(uint32_t i=0; i<subnetList.size()-1; ++i)
    {
      //NS_LOG_UNCOND ("Creating Subnet " << i);
      subnetList[i] = NodeContainer (Nodes.Get(i), Nodes.Get(i+1));
    }
  uint16_t NSize =  subnetList.size();

  NS_LOG_UNCOND ("Creating Devices");
  /*----------------DEVICE CREATION----------------*/
  std::vector<NetDeviceContainer> deviceList (NSize);
  std::vector<Ipv4InterfaceContainer> subNetInterfaces (NSize);
  for(uint32_t i=0; i<deviceList.size()-1; ++i)
    {
      subnetAddr.str("");
      deviceList[i] = p2p.Install (subnetList[i]);
      subnetAddr <<"10.1."<<i+1<<".0";
      //NS_LOG_UNCOND ("Creating Address " << subnetAddr.str().c_str ());
      address.SetBase(subnetAddr.str().c_str (),"255.255.255.0");
      subNetInterfaces[i] = address.Assign(deviceList[i]);
    }
  uint16_t ISize =  NSize-1;
  NS_LOG_UNCOND ("DeviceListSize: "<< ISize);

  NS_LOG_UNCOND ("Setting Server Port");
  /*----------------ADDRESS/APP CREATION----------------*/
  UdpEchoServerHelper echoServer (9); // Set Server Port
  NS_LOG_UNCOND ("Creating Server");
  ApplicationContainer serverApps = echoServer.Install (Nodes.Get (nNodes-1)); // Set Server Node
  serverApps.Start (Seconds (1.0)); // Set open time
  serverApps.Stop (Seconds (timer)); // Set close time
  NS_LOG_UNCOND ("Creating Client Target");
  UdpEchoClientHelper echoClient (subNetInterfaces[ISize-1].GetAddress (1), 9); // Set Client Target with servers subNetInterfaces[i].GetAddress & Port
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1)); // Set sending data
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));
  NS_LOG_UNCOND ("Creating Client");
  ApplicationContainer clientApps = echoClient.Install (Nodes.Get (0)); // Set Client Node
  clientApps.Start (Seconds (2.0)); // Set open time
  clientApps.Stop (Seconds (timer)); // Set close time

  NS_LOG_UNCOND ("Creating Animation");
  /*-----------------ANIMATION CREATION----------------*/
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  AnimationInterface anim (animFile);
  for(uint32_t i=0; i<subnetList.size(); ++i){
    anim.SetConstantPosition (Nodes.Get(i), i, 0);
  }

  /*----------------RUN SIMULATION----------------*/
  Simulator::Stop (Seconds (timer));
  Simulator::Run();
  std::cout << "Animation Trace file created:" << animFile.c_str ()<< std::endl;
  Simulator::Destroy();
}
