#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/csma-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Tree - 16 Nodes");

int main (int argc, char *argv[])
{
  NS_LOG_UNCOND ("Starting..");
  Time::SetResolution(Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (1500)); // Limit of Ethernet
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue ("1000kb/s")); // 1Mb dataRate
  std::string animFile = "TREE16.xml" ;  // Name of file for animation output

  // Set Node Size
  uint32_t nNodes = 16;
  uint16_t timer = 60;

  CommandLine cmd;
  cmd.AddValue ("nNodes", "Nodes to place", nNodes); // Allow command line node setting
  cmd.AddValue ("animFile",  "Name for Animation File", animFile); // Allow command line animation file naming
  cmd.Parse (argc,argv);

  // Init Nodes
  NS_LOG_UNCOND ("Creating Nodes: " << nNodes);
  NodeContainer Nodes;
  Nodes.Create(nNodes);

  // Create a vector to contain the nodes for each level in the tree
  std::vector<NodeContainer> tLevel (nNodes/2); // ONLY WORKS WITH COMPLETE BINARY TREES I.E nNodes MUST BE A FACTOR OF 4 (4,8,16,32,64)
  // Create a uint32 var with value 1, used for tree level defining
  uint32_t levelSet = 1;
  uint32_t channelCount = 0,x = 0,y = 0,i = 0,j = 0,z = 0;  // Create all loop variables & channelCount var
  i = nNodes/2;
  // Create routing tree from defined end-tree leaf size
  while(i >= 2){ // Divided by two so the first level is not the node level
    tLevel[levelSet].Create(i); // Create a new level, populate with nNodes /= 2
    channelCount = channelCount + (i*2); // Add up nodes in the tree, multiply by 2 for the leaf count
    NS_LOG_UNCOND ("Level Size:" << i << " Level No#:" << levelSet << " Channel Count:" << channelCount);
    i = i/2; // Divide by two to go up the tree one level
    levelSet = levelSet + 1; // Increase the level by 1 count
  }
  /* This code creates the tree upwards, using the nNodes variable as a measure of how large the tree should be
    E.G
    Level 3:    0      Size: 1
              /  \
    Level 2:  0   0    Size: 2
              / \ / \
     Level 1:0  00  0   Size: 4
            /\ /||\/\
     NODES:0000 0000    Size: nNodes (8)
     However no connections exist at this stage, just level containers + internal nodes
     This also misses out the root of the tree (The single node at the top), which must be created manually */
  tLevel[levelSet].Create(1); // Here we create the root of the tree with a single node
  channelCount += 2; // Add channels for root node
  uint32_t rootSize = tLevel[levelSet].GetN();
  NS_LOG_UNCOND ("ROOT Level Size:" << rootSize << " Level No#:" << levelSet << " Channel Count:" << channelCount);

  // Now that all nodes have been created, we need to index them to make access easier
  uint32_t nodeIndex = ns3::NodeList::GetNNodes();
  std::vector<NodeContainer> nodeList (nodeIndex);

  // Install stacks
  InternetStackHelper Stack;
  Stack.Install(Nodes);
  for(x = 0; x <= levelSet; x++){  // Install stacks on all created levels
    Stack.Install(tLevel[x]);
  }

  // Create a nodecontainer to hold channels
  NS_LOG_UNCOND ("Creating Subnet List");
  std::vector<NodeContainer> subnetList (channelCount);
  NS_LOG_UNCOND ("Creating Subnets");
  // Create channels from the top of the tree to the bottom
  z = 0;
  for(j = levelSet; j > 0; j--){  // For each level from the top (Root)
    NS_LOG_UNCOND ("Level:" << j);
    uint32_t levelB = 0;
    if(j <= 1){
      for(x = 0; x < tLevel[j].GetN(); x++){  // For each node in the current level (Penultimate level)
        for(y = 0; y < 2; y++){ // For each node in nNodes (Bottom Level of Tree)
          NS_LOG_UNCOND ("Channel#:" << z << "  Level#:" << j << "  from:" << x << "  to " << levelB );
          subnetList[z] = NodeContainer (tLevel[j].Get(x), Nodes.Get(levelB));
          z++;
          levelB++;
        }
      }
    }
    else{
      for(x = 0; x < tLevel[j].GetN(); x++){  // For each node in the current level
        for(y = 0; y < 2; y++){ // For each node in the lower level
          NS_LOG_UNCOND ("Channel#:" << z << "  Level#:" << j << "  from:" << x << "  to " << levelB );
          subnetList[z] = NodeContainer (tLevel[j].Get(x), tLevel[j-1].Get(levelB));
          z++;
          levelB++;
        }
      }
    }
  }

  NS_LOG_UNCOND ("Creating P2P Helper");
  // Init p2p (p2p)
  PointToPointHelper p2p;

  // Init Ipv4 + Address String
  Ipv4AddressHelper address;
  std::ostringstream subnetAddr;

  uint16_t NSize =  subnetList.size();
  NS_LOG_UNCOND ("SubnetListSize: "<< NSize);

  std::vector<NetDeviceContainer> deviceList (NSize);
  std::vector<Ipv4InterfaceContainer> subNetInterfaces (NSize);
  NS_LOG_UNCOND ("Creating Devices");
  for(uint32_t i=0; i<deviceList.size(); ++i)
  {
    subnetAddr.str("");
    deviceList[i] = p2p.Install (subnetList[i]);
    subnetAddr <<"10.1."<<i+1<<".0";
    //NS_LOG_UNCOND ("Creating Address " << subnetAddr.str().c_str ());
    address.SetBase(subnetAddr.str().c_str(),"255.255.255.0");
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
  UdpEchoClientHelper echoClient (subNetInterfaces[ISize].GetAddress (1), 9); // Set Client Target with servers subNetInterfaces[i].GetAddress & Port
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1)); // Set sending data
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  NS_LOG_UNCOND ("Creating Client");
  ApplicationContainer clientApps = echoClient.Install (Nodes.Get (0)); // Set Client Node
  clientApps.Start (Seconds (2.0)); // Set open time
  clientApps.Stop (Seconds (timer)); // Set close time

  NS_LOG_UNCOND ("Creating Animation");
  NS_LOG_UNCOND ("Levels in Tree:" << levelSet);
  /*-----------------ANIMATION CREATION----------------*/
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  AnimationInterface anim (animFile);
  for(i=0; i<Nodes.GetN(); i++){ // Set the tree base
    anim.SetConstantPosition (Nodes.Get(i), i, 0);
  }
  for(i=0; i<=levelSet; i++){ // For each level
    for(j=0;j<tLevel[i].GetN();j++){ // For the size of current level
      anim.SetConstantPosition (tLevel[i].Get(j), j, i);
    }
  }
  /*----------------RUN SIMULATION----------------*/
  Simulator::Stop (Seconds (timer));
  Simulator::Run();
  std::cout << "Animation Trace file created:" << animFile.c_str ()<< std::endl;
  Simulator::Destroy();
}
