/* || Initial Test Network || C++
 * || BER14475946	   ||
 * || Liam T. Berridge     ||
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3; 			// Declare workspace

NS_LOG_COMPONENT_DEFINE ("Init-Test"); 	// Create Log "Init-Test"
					// (Diagnostic Tool)

int main (int argc, char *argv[])	// Declare Main Method
{
	// ||||||||||||||||||||||||
	// || PREREQUISITE SETUP ||

	/* This area sets up basic debug logging and also the time resolution inside the simulation
         */

	NS_LOG_INFO("Pre-Requisite Setup");
	std::cout << "Pre-Requisite Setup.." << std::endl;
	Time::SetResolution(Time::NS);	// Set time resolution to nanoseconds

	LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
	// Enable Packet Logging -- Sending
	LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
	// Enable Packet Logging -- Recieving

	// ||||||||||||||||||||||
	// || NETWORK PRESETUP ||

	/* This area sets up node clusters as objects, as well as definining attributes
	 * for those clusters
       	 */

	NS_LOG_INFO("Network Pre-Setup");
	std::cout << "Network Pre-Setup.." << std::endl;

	NodeContainer nodes;		// Object for storing a node-set
	nodes.Create(2);		// Create two nodes in the new group

	// ||||||||||||||||||||
	// || TOPOLOGY SETUP ||

	/* This area defines the topology type to be applied to the node clusters, as well as
	 * defining attributes, such as DataRate and Delay
         */

	NS_LOG_INFO("Creating Topology");
	std::cout << "Creating Topology.." << std::endl;

	// The P-T-P Helper assists with quick
	// implementation of a topology to given nodes
	PointToPointHelper pointToPoint;

	// Set properties of connections
	// Set bandwidth
	pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
	// Set ping (Network delay) - This delay stacks between nodes created with the helper
	pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

	// |||||||||||||||||||
	// || NETWORK SETUP ||

	/* This area begins to piece the network together using the clusters and topology
	 * set up previously
	 */

	NS_LOG_INFO("Node Setup");
	std::cout << "Node Setup.." << std::endl;

	NetDeviceContainer devices;	// Object for storing simulated "Devices"
	devices = pointToPoint.Install(nodes);	// Install "nodes" using pointToPoint inside "devices"

	// ||||||||||||||||||||||
	// || SIMULATION SETUP ||

	/* This area is where we setup devices, allow assignable attributes to devices,
	 * create echo services for packets,
	 */

	NS_LOG_INFO("Stack Creation");
	std::cout << "Stack Creation.." << std::endl;

	InternetStackHelper stack;	// Instantiate the IS-Helper
	stack.Install(nodes);		// Instantiate node attributes

	NS_LOG_INFO("Address Assignment");
	std::cout << "Address Assignment.." << std::endl;

	Ipv4AddressHelper address;	// Instantiate the Ipv4-Helper
	address.SetBase("10.1.1.0","255.255.255.0");	// Declare the base IP/mask

	// Perform address assignment to all nodes in cluster "devices"
	Ipv4InterfaceContainer interfaces = address.Assign(devices);

	NS_LOG_INFO("Creating Server Echo Service");
	std::cout << "Creating Server Echo Service.." << std::endl;

	UdpEchoServerHelper echoServer(9); // Set a UDP port

	// Installs "Echo" service on node/s to relay back confirmation of packets
	ApplicationContainer serverApps = echoServer.Install(nodes.Get(1));
	serverApps.Start(Seconds(1.0)); // Called at X seconds of runtime
	serverApps.Stop(Seconds(10.0));	// Disposed at X seconds of runtime

	// Due to declaring a stop-time, the simulation will run for X seconds minimum

	NS_LOG_INFO("Creating Client Echo Service");
	std::cout << "Creating Client Echo Service" << std::endl;

	// Get the physical address for nodes on port 9
	UdpEchoClientHelper echoClient (interfaces.GetAddress(1),9);

	// Define sent packet attributes
	echoClient.SetAttribute("MaxPackets",UintegerValue(1));
	echoClient.SetAttribute("Interval",TimeValue(Seconds(1.0)));
	echoClient.SetAttribute("PacketSize",UintegerValue(1024));

	//
	ApplicationContainer clientApps = echoClient.Install(nodes.Get(0));
	clientApps.Start(Seconds(2.0));
	clientApps.Stop(Seconds(10.0));

	// |||||||||||||
	// || RUN SIM ||

	/* For simulations with recursive events, the following can be implemented
	 * Simulator::Stop(Seconds(11.0));
	 */

	NS_LOG_INFO("Running Simulation");
	std::cout << "\n-*-Running Simulation-*-\n" << std::endl;

	// Standard Simulator Function (With no recurring events)
	Simulator::Run();

	std::cout << "\n-*-Closing Simulation-*-\n" << std::endl;

	Simulator::Destroy();
	return 0;
}
