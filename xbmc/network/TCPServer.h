#pragma once
#include <vector>
#include <sys/socket.h>
#include "interfaces/IAnnouncer.h"
#include "interfaces/json-rpc/ITransportLayer.h"
#include "threads/Thread.h"
#include "threads/CriticalSection.h"
#include "interfaces/json-rpc/JSONUtils.h"

#if defined(__ANDROID__)
// android does not defined sockaddr_storage, this is the definition from RFC2553

#define _SS_MAXSIZE    128  /* Implementation specific max size */
#define _SS_ALIGNSIZE  (sizeof (int64_t))
                         /* Implementation specific desired alignment */
/*
 * Definitions used for sockaddr_storage structure paddings design.
 */
typedef __sa_family_t sa_family_t;
typedef __socklen_t socklen_t;

#define _SS_PAD1SIZE   (_SS_ALIGNSIZE - sizeof (sa_family_t))
#define _SS_PAD2SIZE   (_SS_MAXSIZE - (sizeof (sa_family_t)+_SS_PAD1SIZE + _SS_ALIGNSIZE))
struct sockaddr_storage {
    sa_family_t  __ss_family;     /* address family */
    /* Following fields are implementation specific */
    char      __ss_pad1[_SS_PAD1SIZE];
              /* 6 byte pad, this is to make implementation
              /* specific pad up to alignment field that */
              /* follows explicit in the data structure */
    int64_t   __ss_align;     /* field to force desired structure */
               /* storage alignment */
    char      __ss_pad2[_SS_PAD2SIZE];
              /* 112 byte pad to achieve desired size, */
              /* _SS_MAXSIZE value minus size of ss_family */
              /* __ss_pad1, __ss_align fields is 112 */
};

#endif

class CVariant;
namespace JSONRPC
{
  class CTCPServer : public ITransportLayer, public ANNOUNCEMENT::IAnnouncer, public CThread, protected CJSONUtils
  {
  public:
    static bool StartServer(int port, bool nonlocal);
    static void StopServer(bool bWait);

    virtual bool Download(const char *path, Json::Value *result);
    virtual int GetCapabilities();

    virtual void Announce(ANNOUNCEMENT::EAnnouncementFlag flag, const char *sender, const char *message, const CVariant &data);
  protected:
    void Process();
  private:
    CTCPServer(int port, bool nonlocal);
    bool Initialize();
    bool InitializeBlue();
    bool InitializeTCP();
    void Deinitialize();

    class CTCPClient : public IClient
    {
    public:
      CTCPClient();
      //Copying a CCriticalSection is not allowed, so copy everything but that
      //when adding a member variable, make sure to copy it in CTCPClient::Copy
      CTCPClient(const CTCPClient& client);
      CTCPClient& operator=(const CTCPClient& client);
      virtual int  GetPermissionFlags();
      virtual int  GetAnnouncementFlags();
      virtual bool SetAnnouncementFlags(int flags);
      void PushBuffer(CTCPServer *host, const char *buffer, int length);
      void Disconnect();

      SOCKET           m_socket;
      sockaddr_storage m_cliaddr;
      socklen_t        m_addrlen;
      CCriticalSection m_critSection;

    private:
      void Copy(const CTCPClient& client);
      int m_announcementflags;
      int m_beginBrackets, m_endBrackets;
      char m_beginChar, m_endChar;
      std::string m_buffer;
    };

    std::vector<CTCPClient> m_connections;
    std::vector<SOCKET> m_servers;
    int m_port;
    bool m_nonlocal;
    void* m_sdpd;

    static CTCPServer *ServerInstance;
  };
}
