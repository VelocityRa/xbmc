/*
 *  Copyright (C) 2018-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "NetworkSwitch.h"

#include "PlatformDefs.h"
#include "Util.h"
#include "utils/log.h"
#include "utils/StringUtils.h"

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/sockio.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_arp.h>
#include <net/route.h>

#include <cstdlib>

CNetworkInterfaceSwitch::CNetworkInterfaceSwitch(CNetworkSwitch* network, std::string interfaceName, char interfaceMacAddrRaw[6]):
  m_interfaceName(interfaceName),
  m_interfaceMacAdr(StringUtils::Format("%02X:%02X:%02X:%02X:%02X:%02X",
                                        (uint8_t)interfaceMacAddrRaw[0],
                                        (uint8_t)interfaceMacAddrRaw[1],
                                        (uint8_t)interfaceMacAddrRaw[2],
                                        (uint8_t)interfaceMacAddrRaw[3],
                                        (uint8_t)interfaceMacAddrRaw[4],
                                        (uint8_t)interfaceMacAddrRaw[5]))
{
   m_network = network;
   memcpy(m_interfaceMacAddrRaw, interfaceMacAddrRaw, sizeof(m_interfaceMacAddrRaw));
}

CNetworkInterfaceSwitch::~CNetworkInterfaceSwitch(void) = default;

const std::string& CNetworkInterfaceSwitch::GetName(void) const
{
   return m_interfaceName;
}

bool CNetworkInterfaceSwitch::IsWireless() const
{
   return true;
}

bool CNetworkInterfaceSwitch::IsEnabled() const
{
   struct ifreq ifr;
   strcpy(ifr.ifr_name, m_interfaceName.c_str());
   if (ioctl(m_network->GetSocket(), SIOCGIFFLAGS, &ifr) < 0)
      return false;
   return ((ifr.ifr_flags[0] & IFF_UP) == IFF_UP);
}

bool CNetworkInterfaceSwitch::IsConnected() const
{
   struct ifreq ifr;
   int zero = 0;
   memset(&ifr,0,sizeof(struct ifreq));
   strcpy(ifr.ifr_name, m_interfaceName.c_str());
   if (ioctl(m_network->GetSocket(), SIOCGIFFLAGS, &ifr) < 0)
      return false;

   int iRunning = ( (ifr.ifr_flags[0] & IFF_RUNNING) && (!(ifr.ifr_flags[0] & IFF_LOOPBACK)));

   if (ioctl(m_network->GetSocket(), SIOCGIFADDR, &ifr) < 0)
      return false;

   // return only interfaces which has ip address
   return iRunning && (0 != memcmp(ifr.ifr_addr.sa_data+sizeof(short), &zero, sizeof(int)));
}

std::string CNetworkInterfaceSwitch::GetMacAddress() const
{
  return m_interfaceMacAdr;
}

void CNetworkInterfaceSwitch::GetMacAddressRaw(char rawMac[6]) const
{
  memcpy(rawMac, m_interfaceMacAddrRaw, 6);
}

std::string CNetworkInterfaceSwitch::GetCurrentIPAddress(void) const
{
   std::string result;

   struct ifreq ifr;
   strcpy(ifr.ifr_name, m_interfaceName.c_str());
   ifr.ifr_addr.sa_family = AF_INET;
   if (ioctl(m_network->GetSocket(), SIOCGIFADDR, &ifr) >= 0)
   {
      result = inet_ntoa((*((struct sockaddr_in *)&ifr.ifr_addr)).sin_addr);
   }

   return result;
}

std::string CNetworkInterfaceSwitch::GetCurrentNetmask(void) const
{
   std::string result;

   struct ifreq ifr;
   strcpy(ifr.ifr_name, m_interfaceName.c_str());
   ifr.ifr_addr.sa_family = AF_INET;
   if (ioctl(m_network->GetSocket(), SIOCGIFNETMASK, &ifr) >= 0)
   {
      result = inet_ntoa((*((struct sockaddr_in*)&ifr.ifr_addr)).sin_addr);
   }

   return result;
}

std::string CNetworkInterfaceSwitch::GetCurrentWirelessEssId(void) const
{
   std::string result;
   return result;
}

std::string CNetworkInterfaceSwitch::GetCurrentDefaultGateway(void) const
{
   std::string result;

   size_t needed;
   int mib[6];
   char *buf, *next, *lim;
   char line[16];
   struct rt_msghdr *rtm;
   struct sockaddr *sa;
   struct sockaddr_in *sockin;

   mib[0] = CTL_NET;
   mib[1] = PF_ROUTE;
   mib[2] = 0;
   mib[3] = 0;
   mib[4] = NET_RT_DUMP;
   mib[5] = 0;
   if (sysctl(mib, 6, NULL, &needed, NULL, 0) < 0)
      return result;

   if ((buf = (char *)malloc(needed)) == NULL)
      return result;

   if (sysctl(mib, 6, buf, &needed, NULL, 0) < 0) {
      free(buf);
      return result;
   }

   lim  = buf + needed;
   for (next = buf; next < lim; next += rtm->rtm_msglen) {
      rtm = (struct rt_msghdr *)next;
      sa = (struct sockaddr *)(rtm + 1);
      sa = (struct sockaddr *)(SA_SIZE(sa) + (char *)sa);
      sockin = (struct sockaddr_in *)sa;
      if (inet_ntop(AF_INET, &sockin->sin_addr.s_addr,
         line, sizeof(line)) == NULL) {
            free(buf);
            return result;
	  }
	  result = line;
      break;
   }
   free(buf);

   return result;
}

CNetworkSwitch::CNetworkSwitch()
 : CNetworkBase()
{
   m_sock = socket(AF_INET, SOCK_DGRAM, 0);
   queryInterfaceList();
}

CNetworkSwitch::~CNetworkSwitch(void)
{
  if (m_sock != -1)
    close(CNetworkSwitch::m_sock);

  std::vector<CNetworkInterface*>::iterator it = m_interfaces.begin();
  while(it != m_interfaces.end())
  {
    CNetworkInterface* nInt = *it;
    delete nInt;
    it = m_interfaces.erase(it);
  }
}

std::vector<CNetworkInterface*>& CNetworkSwitch::GetInterfaceList(void)
{
   return m_interfaces;
}

//! @bug
//! Overwrite the GetFirstConnectedInterface and requery
//! the interface list if no connected device is found
//! this fixes a bug when no network is available after first start of xbmc
//! and the interface comes up during runtime
CNetworkInterface* CNetworkSwitch::GetFirstConnectedInterface(void)
{
    CNetworkInterface *pNetIf=CNetworkBase::GetFirstConnectedInterface();

    // no connected Interfaces found? - requeryInterfaceList
    if (!pNetIf)
    {
        CLog::Log(LOGDEBUG,"%s no connected interface found - requery list",__FUNCTION__);
        queryInterfaceList();
        //retry finding a connected if
        pNetIf = CNetworkBase::GetFirstConnectedInterface();
    }

    return pNetIf;
}


void CNetworkSwitch::GetMacAddress(const std::string& interfaceName, char rawMac[6])
{
  memset(rawMac, 0, 6);
  // TODO(velocity): unimplemented
  CLog::Log(LOGWARNING, "[Switch] %s unimplemented'", __FUNCTION__);
  return;
}

void CNetworkSwitch::queryInterfaceList()
{
  m_interfaces.clear();
}

std::vector<std::string> CNetworkSwitch::GetNameServers(void)
{
   std::vector<std::string> result;
   return result;
}

void CNetworkSwitch::SetNameServers(const std::vector<std::string>& nameServers)
{
}

bool CNetworkSwitch::PingHost(unsigned long remote_ip, unsigned int timeout_ms)
{
  return false;
}

bool CNetworkInterfaceSwitch::GetHostMacAddress(unsigned long host_ip, std::string& mac) const
{
  return false;
}

std::vector<NetworkAccessPoint> CNetworkInterfaceSwitch::GetAccessPoints(void) const
{
   std::vector<NetworkAccessPoint> result;
   return result;
}

void CNetworkInterfaceSwitch::GetSettings(NetworkAssignment& assignment, std::string& ipAddress, std::string& networkMask, std::string& defaultGateway, std::string& essId, std::string& key, EncMode& encryptionMode)  const
{
   ipAddress = "0.0.0.0";
   networkMask = "0.0.0.0";
   defaultGateway = "0.0.0.0";
   essId = "";
   key = "";
   encryptionMode = ENC_NONE;
   assignment = NETWORK_DISABLED;
}

void CNetworkInterfaceSwitch::SetSettings(const NetworkAssignment& assignment, const std::string& ipAddress, const std::string& networkMask, const std::string& defaultGateway, const std::string& essId, const std::string& key, const EncMode& encryptionMode)
{
}

void CNetworkInterfaceSwitch::WriteSettings(FILE* fw, NetworkAssignment assignment, const std::string& ipAddress, const std::string& networkMask, const std::string& defaultGateway, const std::string& essId, const std::string& key, const EncMode& encryptionMode)
{
   if (assignment == NETWORK_DHCP)
   {
      fprintf(fw, "iface %s inet dhcp\n", GetName().c_str());
   }
   else if (assignment == NETWORK_STATIC)
   {
      fprintf(fw, "iface %s inet static\n", GetName().c_str());
      fprintf(fw, "  address %s\n", ipAddress.c_str());
      fprintf(fw, "  netmask %s\n", networkMask.c_str());
      fprintf(fw, "  gateway %s\n", defaultGateway.c_str());
   }

   if (assignment != NETWORK_DISABLED && IsWireless())
   {
      if (encryptionMode == ENC_NONE)
      {
         fprintf(fw, "  wireless-essid %s\n", essId.c_str());
      }
      else if (encryptionMode == ENC_WEP)
      {
         fprintf(fw, "  wireless-essid %s\n", essId.c_str());
         fprintf(fw, "  wireless-key s:%s\n", key.c_str());
      }
      else if (encryptionMode == ENC_WPA || encryptionMode == ENC_WPA2)
      {
         fprintf(fw, "  wpa-ssid %s\n", essId.c_str());
         fprintf(fw, "  wpa-psk %s\n", key.c_str());
         fprintf(fw, "  wpa-proto %s\n", encryptionMode == ENC_WPA ? "WPA" : "WPA2");
      }
   }

   if (assignment != NETWORK_DISABLED)
      fprintf(fw, "auto %s\n\n", GetName().c_str());
}


