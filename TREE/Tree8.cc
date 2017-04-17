#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/csma-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Tree - 8 Nodes");

// Functions -------- -------- -------- --------
void startUp ()
{
  Time::SetResolution(Time::NS);  // Set the simulation time resolution

  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);  // Enable client UDP logging - Unused without OnOffApplication
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);  // Enable server UDP logging - Unused without OnOffApplication
  Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (1500));   // Packet size (Bytes) - Unused without OnOffApplication
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue ("1000kb/s")); // Data transmission rate for OnOffApplication - Unused without OnOffApplication
}

std::vector<NodeContainer> levelCreation (uint32_t nNodes, uint32_t* levelSet, uint32_t* channelCount)
{
  uint32_t i = nNodes/2; // Set variables
  std::vector<NodeContainer> tLevel (nNodes/2);                   // Create a NodeContainer array, with a container for each seperate level

  // Create nodes for tree structure, use nNodes as base of tree
  while(i >= 2){ // Divided by two so the first level is not the node level
    tLevel[*levelSet].Create(i); // Create a new level, populate with nNodes /= 2
    *channelCount = *channelCount + (i*2); // Add up nodes in the tree, multiply by 2 for the leaf count
    NS_LOG_UNCOND ("Level Size:" << i << " Level No#:" << *levelSet << " Channel Count:" << *channelCount);
    i = i/2; // Divide by two to go up the tree one level
    *levelSet = *levelSet + 1; // Increase the level by 1 count
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
  return tLevel;
}

std::vector<NodeContainer> channelCreation (uint32_t levelSet, std::vector<NodeContainer> tLevel, std::vector<NodeContainer> subnetList, NodeContainer Nodes)
{
  uint32_t z = 0, j = 0, x = 0, y = 0; // Create variables for loops
  for(j = levelSet; j > 0; j--){  // Create channels from the top of the tree to the bottom
    NS_LOG_UNCOND ("Creating Level:" << j);
    uint32_t levelB = 0;
    if(j <= 1){
      for(x = 0; x < tLevel[j].GetN(); x++){  // For each node in the current level (Penultimate level)
        for(y = 0; y < 2; y++){ // For each node in nNodes (Bottom Level of Tree)
          // NS_LOG_UNCOND ("Channel#:" << z << "  From:" << j << ":" << x << " To:" << j-1 << ":" << levelB );
          subnetList[z] = NodeContainer (tLevel[j].Get(x), Nodes.Get(levelB));
          z++;
          levelB++;
        }
      }
    }
    else{
      for(x = 0; x < tLevel[j].GetN(); x++){  // For each node in the current level
        for(y = 0; y < 2; y++){ // For each node in the lower level
          // NS_LOG_UNCOND ("Channel#:" << z << "  From:" << j << ":" << x << " To:" << j-1 << ":" << levelB );
          subnetList[z] = NodeContainer (tLevel[j].Get(x), tLevel[j-1].Get(levelB));
          z++;
          levelB++;
        }
      }
    }
  }
  return subnetList;
}

std::vector<Ipv4InterfaceContainer> subnetCreation (std::vector<NetDeviceContainer> deviceList, std::vector<NodeContainer> subnetList, std::vector<Ipv4InterfaceContainer> subNetInterfaces, PointToPointHelper p2p)
{
  // Create variables for subnet control
  uint32_t sb1=1,sb2=1;
  // CREATE IPV4 HELPER & IP STRING
  Ipv4AddressHelper address;
  std::ostringstream subnetAddr;
  for(uint32_t i=0; i<deviceList.size(); i++)
  {
    subnetAddr.str("");
    deviceList[i] = p2p.Install (subnetList[i]);
    subnetAddr << "10." << sb1 << "." << sb2 << ".0";
    //NS_LOG_UNCOND ("Creating Address " << subnetAddr.str().c_str ());
    address.SetBase(subnetAddr.str().c_str(),"255.255.255.0");
    subNetInterfaces[i] = address.Assign(deviceList[i]);
    sb2 = sb2+1;
    if(sb2 == 255){
      sb1 = sb1+1;
      sb2 = 0;
    }
  }
  return subNetInterfaces;
}

// Main -------- -------- -------- --------
int main (int argc, char *argv[])
{
  NS_LOG_UNCOND ("Starting..");   // Declare startup in console

  startUp();  // Call startup function

  // SET VARIABLES
  std::string animFile = "TREE8.xml" ;  // Name of file for animation output
  uint32_t nNodes = 8;  // Declare the number of nodes desired (Overridden by commandline args)
  uint32_t timer = 60;    // Declare the length of the simulation (Seconds)
  uint32_t x = 0, i = 0, j = 0; // Loop variable
  uint32_t levelSet = 1, channelCount = 0;  // Start the tree creation at 1, as level 0 exists already as nNodes.

  // PARSE COMMANDLINE
  CommandLine cmd;        // Create CommandLine object
  cmd.AddValue ("nNodes", "Nodes to place", nNodes);                // Add value for desired nodes from argc
  cmd.AddValue ("animFile",  "Name for Animation File", animFile);  // Add value for output file name from argv
  cmd.Parse (argc,argv);  // Parse argc, argv from commandline args (If present)

  // CREATE NODES
  NS_LOG_UNCOND ("Creating Nodes: " << nNodes); // Log: Node Creation
  NodeContainer Nodes;  // Create a node container to store a cluster of nodes in a single object
  Nodes.Create(nNodes); // Create nNodes worth of nodes in the container

  // CREATE TREE NODES
  std::vector<NodeContainer> tLevel = levelCreation(nNodes, &levelSet, &channelCount);  // Call level creation method

  // CREATE TREE ROOT
  tLevel[levelSet].Create(1); // Here we create the root of the tree with a single node
  channelCount += 2; // Add channels for root node
  uint32_t rootSize = tLevel[levelSet].GetN();
  NS_LOG_UNCOND ("ROOT Level Size:" << rootSize << " Level No#:" << levelSet << " Channel Count:" << channelCount);

  // CREATE NODE INDEX
  uint32_t nodeIndex = ns3::NodeList::GetNNodes();
  std::vector<NodeContainer> nodeList (nodeIndex);

  // INSTALL INTERNET STACKS
  InternetStackHelper Stack;
  Stack.Install(Nodes);
  for(x = levelSet; x > 0; x--){  // Install stacks on all created levels
    Stack.Install(tLevel[x]);
  }

  // CREATE CHANNEL CONTAINER
  NS_LOG_UNCOND ("Creating Subnet List:" << channelCount);
  std::vector<NodeContainer> subnetList (channelCount);

  // CREATE CHANNELS
  NS_LOG_UNCOND ("Creating Subnets, Levels:" << levelSet);
  subnetList = channelCreation(levelSet, tLevel, subnetList, Nodes);

  // CREATE P2P HELPER
  NS_LOG_UNCOND ("Creating P2P Helper");
  PointToPointHelper p2p;

  // DEBUG : PRINT SUBNET COUNT
  uint16_t NSize =  subnetList.size();
  NS_LOG_UNCOND ("SubnetListSize: "<< NSize);

  // CREATE NET DEVICES
  NS_LOG_UNCOND ("Creating Devices");
  std::vector<NetDeviceContainer> deviceList (NSize);
  std::vector<Ipv4InterfaceContainer> subNetInterfaces (NSize);

  // ASSIGN CHANNELS ADDRESSES
  NS_LOG_UNCOND ("Assigning Addresses to Channels");
  subNetInterfaces = subnetCreation(deviceList, subnetList, subNetInterfaces, p2p);
  uint16_t ISize =  NSize-1;

  // DEBUG : PRINT DEVICE COUNT
  NS_LOG_UNCOND ("DeviceListSize: "<< ISize);

  // CREATE APPLICATIONS FOR COMMUNICATION
  NS_LOG_UNCOND ("Setting Server Port");
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
