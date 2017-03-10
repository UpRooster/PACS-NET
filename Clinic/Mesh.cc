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

  NodeContainer imgScn, imgWrk, imgStr, routers;
  imgScn.Create(1);
  imgWrk.Create(1);
  imgStr.Create(1);

  InternetStackHelper Stack;
  Stack.Install(imgScn);
  Stack.Install(imgWrk);
  Stack.Install(imgStr);
  Stack.Install(routers);

  PointToPointHelper p2p;

  Ipv4AddressHelper address;

  NodeContainer subNet1;    #-----------------SUBNET1-----------------#
  subNet1.Add(imgScn.Get(0));
  subNet1.Add(imgStr.Get(0));

  NetDeviceContainer subNet1Devices = p2p.Install(subNet1);

  address.SetBase("10.1.1.0","255.255.255.0");
  Ipv4InterfaceContainer subNet1Interfaces = address.Assign(subNet1Devices);

  NodeContainer subNet2;    #-----------------SUBNET2-----------------#
  subNet2.Add(imgStr.Get(0));
  subNet2.Add(imgWrk.Get(0));

  NetDeviceContainer subNet2Devices = p2p.Install(subNet2);

  address.SetBase("10.1.2.0","255.255.255.0");
  Ipv4InterfaceContainer subNet2Interfaces = address.Assign(subNet2Devices);

  NodeContainer subNet3;    #-----------------SUBNET3-----------------#
  subNet3.Add(imgScn.Get(0));
  subNet3.Add(imgWrk.Get(0));

  NetDeviceContainer subNet3Devices = p2p.Install(subNet3);

  address.SetBase("10.1.3.0","255.255.255.0");
  Ipv4InterfaceContainer subNet3Interfaces = address.Assign(subNet3Devices);
}
