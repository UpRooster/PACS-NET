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
* 6. Run Simulation
*/

// E.G

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/csma-module.h"

using namespace ns3;

int main (int argc, char *argv[])
{
  Time::SetResolution (Time::NS);
  
  NodeContainer NODES, ROUTER, BRANCH;
  NODES.Create(2);
  ROUTER.Create(2);
  BRANCH.Create(3);
  
  InternetStackHelper STACK;
  STACK.Install(NODES);
  STACK.Install(ROUTER);
  STACK.Install(BRANCH);
  
  PointToPointHelper P2P;

  CsmaHelper CSMA;
  
  Ipv4AddressHelper ADDRESS;
  
  NodeContainer SUBNET1;
  SUBNET1.Add(NODES.Get(0));
  SUBNET1.Add(ROUTER.Get(0));
  
  ADDRESS.SetBase("10.1.1.0","255.255.255.0");
  
  NetDeviceContainer SUBNET1DEVICES = P2P.Install(SUBNET1);
  
  Ipv4InterfaceContainer SUBNET1INTERFACES = ADDRESS.Assign(SUBNET1DEVICES);

  NodeContainer SUBNET2;
  SUBNET2.Add(ROUTER.Get(0));
  SUBNET2.Add(ROUTER.Get(1));
  
  ADDRESS.SetBase("10.1.2.0","255.255.255.0");
  
  NetDeviceContainer SUBNET2DEVICES = P2P.Install(SUBNET2);
  
  Ipv4InterfaceContainer SUBNET2INTERFACES = ADDRESS.Assign(SUBNET2DEVICES);

  NodeContainer SUBNET3;
  SUBNET3.Add(NODES.Get(1));
  SUBNET3.Add(ROUTER.Get(1));
  
  ADDRESS.SetBase("10.1.3.0","255.255.255.0");
  
  NetDeviceContainer SUBNET3DEVICES = P2P.Install(SUBNET3);
  
  Ipv4InterfaceContainer SUBNET3INTERFACES = ADDRESS.Assign(SUBNET3DEVICES);

  NodeContainer SUBNET4;
  SUBNET4.Add(ROUTER.Get(1));
  SUBNET4.Add(BRANCH);

  NetDeviceContainer SUBNET4DEVICES = CSMA.Install(SUBNET4);

  Ipv4InterfaceContainer SUBNET4INTERFACES = ADDRESS.Assign(SUBNET4DEVICES);

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
