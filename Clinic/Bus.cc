#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

int main (int argc, char *argv[])
{
  # Clinic
  Time::SetResolution(Time::NS);

   #Init Nodes
  NodeContainer imgScn, imgWrk, imgStr, routers;
  imgScn.Create(1);
  imgWrk.Create(1);
  imgStr.Create(1);
  routers.Create(1);

  #Init Stacks
  InternetStackHelper Stack;
  Stack.Install(imgScn);
  Stack.Install(imgWrk);
  Stack.Install(imgStr);
  Stack.Install(routers);

  #Init p2p (p2p)
  PointToPointHelper p2p;
  #Init Csma (BUS)
  CsmaHelper csma;

  #Init Ipv4
  Ipv4AddressHelper address;
  
  #-----------------SUBNET1-----------------#
  NodeContainer subNet1;
  subNet1.Add(.Get(0));
  subNet1.Add(.Get(0));

  NetDeviceContainer subNet1Devices = p2p.Install(subNet1);

  address.SetBase("10.1.1.0","255.255.255.0");
  Ipv4InterfaceContainer subNet1Interfaces = address.Assign(subNet1Devices);
}
