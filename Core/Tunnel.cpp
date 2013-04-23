#include "Tunnel.h"
#include "Server.h"
#include "Enums.h"
#include "ApiLock.h"
#include <collection.h>

using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;


Linphone::Core::TunnelConfig::TunnelConfig(Platform::String^ host, int port, int udpMirrorPort, int roundTripDelay) :
	host(host),
	port(port),
	udpMirrorPort(udpMirrorPort),
	roundTripDelay(roundTripDelay)
{
}

Platform::String^ Linphone::Core::TunnelConfig::Host::get()
{
	return this->host;
}

void Linphone::Core::TunnelConfig::Host::set(Platform::String^ value)
{
	this->host = value;
}

int Linphone::Core::TunnelConfig::Port::get()
{
	return this->port;
}

void Linphone::Core::TunnelConfig::Port::set(int value)
{
	this->port = value;
}

int Linphone::Core::TunnelConfig::UdpMirrorPort::get()
{
	return this->udpMirrorPort;
}

void Linphone::Core::TunnelConfig::UdpMirrorPort::set(int value)
{
	this->udpMirrorPort = value;
}

int Linphone::Core::TunnelConfig::RoundTripDelay::get()
{
	return this->roundTripDelay;
}

void Linphone::Core::TunnelConfig::RoundTripDelay::set(int value)
{
	this->roundTripDelay = value;
}

Platform::String^ Linphone::Core::TunnelConfig::ToString()
{
	return "host[" + this->host + "] port[" + this->port + "] udpMirrorPort[" + this->udpMirrorPort + "] roundTripDelay[" + this->roundTripDelay + "]";
}



Linphone::Core::Tunnel::Tunnel(::LinphoneTunnel *tunnel) :
	lt(tunnel)
{
}

Linphone::Core::Tunnel::~Tunnel()
{
}


Platform::Boolean Linphone::Core::Tunnel::IsEnabled()
{
	gApiLock.Lock();
	Platform::Boolean enabled = linphone_tunnel_enabled(this->lt);
	gApiLock.Unlock();
	return enabled;
}

void Linphone::Core::Tunnel::Enable(Platform::Boolean enable)
{
	gApiLock.Lock();
	linphone_tunnel_enable(this->lt, enable);
	gApiLock.Unlock();
}

void Linphone::Core::Tunnel::AutoDetect()
{
	gApiLock.Lock();
	linphone_tunnel_auto_detect(this->lt);
	gApiLock.Unlock();
}

static void AddServerConfigToVector(void *vServerConfig, void *vector)
{
	::LinphoneTunnelConfig *pc = (LinphoneTunnelConfig *)vServerConfig;
	Linphone::Core::RefToPtrProxy<IVector<Object^>^> *list = reinterpret_cast< Linphone::Core::RefToPtrProxy<IVector<Object^>^> *>(vector);
	IVector<Object^>^ serverconfigs = (list) ? list->Ref() : nullptr;

	const char *chost = linphone_tunnel_config_get_host(pc);
	int port = linphone_tunnel_config_get_port(pc);
	int udpMirrorPort = linphone_tunnel_config_get_remote_udp_mirror_port(pc);
	int roundTripDelay = linphone_tunnel_config_get_delay(pc);
	Linphone::Core::TunnelConfig^ serverConfig = ref new Linphone::Core::TunnelConfig(Linphone::Core::Utils::cctops(chost), port, udpMirrorPort, roundTripDelay);
	serverconfigs->Append(serverConfig);
}

IVector<Object^>^ Linphone::Core::Tunnel::GetServers()
{
	gApiLock.Lock();
	IVector<Object^>^ serverconfigs = ref new Vector<Object^>();
	const MSList *configList = linphone_tunnel_get_servers(this->lt);
	RefToPtrProxy<IVector<Object^>^> *serverConfigPtr = new RefToPtrProxy<IVector<Object^>^>(serverconfigs);
	ms_list_for_each2(configList, AddServerConfigToVector, serverConfigPtr);
	gApiLock.Unlock();
	return serverconfigs;
}

void Linphone::Core::Tunnel::CleanServers()
{
	gApiLock.Lock();
	linphone_tunnel_clean_servers(this->lt);
	gApiLock.Unlock();
}

void Linphone::Core::Tunnel::SetHttpProxy(Platform::String^ host, int port, Platform::String^ username, Platform::String^ password)
{
	gApiLock.Lock();
	const char* h = Linphone::Core::Utils::pstoccs(host);
	const char* u = Linphone::Core::Utils::pstoccs(username);
	const char* pwd = Linphone::Core::Utils::pstoccs(password);
	linphone_tunnel_set_http_proxy(this->lt, h, port, u, pwd);
	delete(h);
	delete(u);
	delete(pwd);
	gApiLock.Unlock();
}

void Linphone::Core::Tunnel::AddServer(Platform::String^ host, int port)
{
	gApiLock.Lock();
	const char* h = Linphone::Core::Utils::pstoccs(host);
	LinphoneTunnelConfig* config = linphone_tunnel_config_new();
	linphone_tunnel_config_set_host(config, h);
	linphone_tunnel_config_set_port(config, port);
	linphone_tunnel_add_server(this->lt, config);
	delete(h);
	gApiLock.Unlock();
}

void Linphone::Core::Tunnel::AddServer(Platform::String^ host, int port, int udpMirrorPort, int roundTripDelay)
{
	gApiLock.Lock();
	const char* h = Linphone::Core::Utils::pstoccs(host);
	LinphoneTunnelConfig* config = linphone_tunnel_config_new();
	linphone_tunnel_config_set_host(config, h);
	linphone_tunnel_config_set_port(config, port);
	linphone_tunnel_config_set_delay(config, roundTripDelay);
	linphone_tunnel_config_set_remote_udp_mirror_port(config, udpMirrorPort);
	linphone_tunnel_add_server(this->lt, config);
	delete(h);
	gApiLock.Unlock();
}