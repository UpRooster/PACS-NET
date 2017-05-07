/*
* 1. DECLARE NODES
* 2. INSTALL STACKS TO NODES
* 3. CALL P2P HELPER
* 4. CALL IPv4 HELPER
* 5. DECLARE SUBNET
* -5a. NODECONTAINER SUBNET
* -5b. ADD NODES
* -5c. CREATE DEVICE CONTAINER
* -5d. SET ADDRESS
* -5e. CREATE INTERFACE CONTAINER
* ^ REPEAT SUBNET PROCESS ^
*/

// E.G

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

int main (int argc, char *argv[])
{
  Time::SetResolution (Time::NS);

  NodeContainer NODES;  // Create a Node Container
  NODES.Create(2);      // Define the Node Count

  InternetStackHelper STACK;  // Create a stack helper object
  STACK.Install(NODES);       // Create internet stacks on each node in NODES

  PointToPointHelper P2P; // Create a point-to-point connection object

  Ipv4AddressHelper ADDRESS;  // Create an IPv4 address object
  ADDRESS.SetBase("10.1.1.0","255.255.255.0");  // Set the address base (Increments)

  NodeContainer SUBNET1;      // Create the subnet/channel object
  SUBNET1.Add(NODES.Get(0));  // Assign the first node
  SUBNET1.Add(NODES.Get(1));  // Assign the second node

  NetDeviceContainer SUBNET1DEVICES = P2P.Install(SUBNET1); // Assign a MAC to the subnet/channel

  Ipv4InterfaceContainer SUBNET1INTERFACES = ADDRESS.Assign(SUBNET1DEVICES); // Assign addresses to devices in the channel
}
