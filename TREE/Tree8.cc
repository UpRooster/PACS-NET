#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/csma-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Tree - 8 Nodes");

int main (int argc, char *argv[])
{
  // Clinic
  NS_LOG_UNCOND ("Starting..");
  Time::SetResolution(Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (1500)); // Limit of Ethernet
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue ("1000kb/s")); // 1Mb dataRate
  std::string animFile = "TREE8.xml" ;  // Name of file for animation output

  // Set Node Size
  uint32_t nNodes = 8;
  uint16_t timer = 60;

  CommandLine cmd;
  cmd.AddValue ("nNodes", "Nodes to place", nNodes); // Allow command line node setting
  cmd.AddValue ("animFile",  "Name for Animation File", animFile); // Allow command line animation file naming
  cmd.Parse (argc,argv);

  // Create a vector to contain the nodes for each level in the tree
  std::vector<NodeContainer> tLevel (nNodes/2); // ONLY WORKS WITH COMPLETE BINARY TREES I.E nNodes MUST BE A FACTOR OF 4 (4,8,16,32,64)
  // Create a uint32 var with value 1, used for tree level defining
  uint32_t levelSet = 1;

  // Create routing tree from defined end-tree leaf size
  for(i = nNodes; i > 2; i/2){
    tLevel[levelSet] = tLevel[levelSet].Create(i); // Create a new level, populate with nNodes /= 2
    levelSet + 1; // Increase the level by 1 count
  }
  // This code creates the tree upwards, using the nNodes variable as a measure of how large the tree should be
  // E.G
  // Level 3:    0      Size: 1
  //            /  \
  // Level 2:  0   0    Size: 2
  //          / \ / \
  // Level 1:0  00  0   Size: 4
  //        /\ /||\/\
  // NODES:0000 0000    Size: nNodes (8)
  // However no connections exist at this stage, just level containers + internal nodes
