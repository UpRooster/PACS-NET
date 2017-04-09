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
  Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (1500)); // Limit of Ethernet
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue ("1000kb/s")); // 1Mb dataRate
  std::string animFile = "P2P16.xml" ;  // Name of file for animation output

  // Set Node Size
  uint32_t nNodes = 16;

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
      NS_LOG_UNCOND ("Creating Subnet " << i);
      subnetList[i] = NodeContainer (Nodes.Get(i), Nodes.Get(i+1));
    }
  NS_LOG_UNCOND ("Creating Devices");
  /*----------------DEVICE CREATION----------------*/
  std::vector<NetDeviceContainer> deviceList (nNodes);
  std::vector<Ipv4InterfaceContainer> subNetInterfaces (nNodes);
  for(uint32_t i=0; i<deviceList.size()-1; ++i)
    {
      subnetAddr.str("");
      deviceList[i] = p2p.Install (subnetList[i]);
      subnetAddr <<"10.1."<<i+1<<".0";
      NS_LOG_UNCOND ("Creating Address " << subnetAddr.str().c_str ());
      address.SetBase(subnetAddr.str().c_str (),"255.255.255.0");
      subNetInterfaces[i] = address.Assign(deviceList[i]);
    }
  NS_LOG_UNCOND ("Creating Address/App");
  /*----------------ADDRESS/APP CREATION----------------*/
  Ipv4Address FS_Address(subNetInterfaces[1].GetAddress(1)); // Get Address of subNet Interfaces 1
  uint16_t FS_Port = 4500;

  UdpEchoClientHelper WKS1Echo(FS_Address, FS_Port);
  ApplicationContainer WKS1EchoApp = WKS1Echo.Install(subnetList[1].Get (0)); // Install App
  WKS1EchoApp.Start(Seconds(1.0));
  WKS1EchoApp.Stop(Seconds(1.0));

  UdpEchoServerHelper FS(FS_Port);
  ApplicationContainer FS_App = FS.Install(subnetList[6].Get(0));
  FS_App.Start(Seconds(1.0));
  FS_App.Stop(Seconds(10.0));
  NS_LOG_UNCOND ("Creating Animation");
  /*-----------------ANIMATION CREATION----------------*/
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  AnimationInterface anim (animFile);
  for(uint32_t i=0; i<subnetList.size(); ++i){
    anim.SetConstantPosition (Nodes.Get(i), i, 0);
  }
  /*----------------RUN SIMULATION----------------*/
  Simulator::Stop (Seconds (10.0));
  Simulator::Run();
  std::cout << "Animation Trace file created:" << animFile.c_str ()<< std::endl;
  Simulator::Destroy();
}
