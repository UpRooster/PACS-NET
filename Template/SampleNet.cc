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
  
  NodeContainer NODES;
  NODES.Create(2);
  
  InternetStackHelper STACK;
  STACK.Install(NODES);
  
  PointToPointHelper P2P;
  
  Ipv4AddressHelper ADDRESS;
  ADDRESS.SetBase("10.1.1.0","255.255.255.0");
  
  NodeContainer SUBNET1;
  SUBNET1.Add(NODES.Get(0));
  SUBNET1.Add(NODES.Get(1));
  
  NetDeviceContainer SUBNET1DEVICES = P2P.Install(SUBNET1);
  
  Ipv4InterfaceContainer SUBNET1INTERFACES = ADDRESS.Assign(SUBNET1DEVICES);
}
