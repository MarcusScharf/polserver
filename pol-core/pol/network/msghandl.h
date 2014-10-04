/*
History
=======
2009/08/03 MuadDib:   Renamed secondary handler class to *_V2 for naming convention

Notes
=======

*/

#ifndef MSGHANDL_H
#define MSGHANDL_H

#include "../../clib/passert.h"

namespace Pol {
  namespace Network {
	class Client;
  }
  namespace Core {

      typedef void(*PktHandlerFunc)(Network::Client *client, void *msg);

	class MessageHandler
	{
	public:
	  MessageHandler( unsigned char msgtype,
					  int msglen,
                      PktHandlerFunc func);
	};
#define MSGLEN_2BYTELEN_DATA -2

#define MESSAGE_HANDLER( type, func ) \
  MessageHandler type##_handler( type##_ID, sizeof( type ), reinterpret_cast<PktHandlerFunc>( func ) )

#define MESSAGE_HANDLER_VARLEN( type, func ) \
  MessageHandler type##_handler( type##_ID, MSGLEN_2BYTELEN_DATA, (PktHandlerFunc) func )

	class MessageHandler_V2
	{
	public:
	  MessageHandler_V2( unsigned char msgtype,
						 int msglen,
                         PktHandlerFunc func );
	};

#define MESSAGE_HANDLER_V2( type, func ) \
  MessageHandler_V2 type##_handler_v2( type##_ID, sizeof( type ), reinterpret_cast<PktHandlerFunc>( func ) )

#define MESSAGE_HANDLER_VARLEN_V2( type, func ) \
  MessageHandler_V2 type##_handler_v2( type##_ID, MSGLEN_2BYTELEN_DATA, (PktHandlerFunc) func )

    /*
    handler[] is used for storing the core MESSAGE_HANDLER
    calls.
    */
    typedef struct
    {
      int msglen; // if 0, no message handler defined.
      PktHandlerFunc func;
    } MSG_HANDLER;
    extern MSG_HANDLER handler[256];

    /*
    handler_v2[] is used for storing the core MESSAGE_HANDLER
    calls for packets that was changed in client 6.0.1.7 (or technically
    any version where a second handler is required due to changed incoming
    packet structure).
    */
    extern MSG_HANDLER handler_v2[256];

    // This class will be responsible for tracking the handlers. For now, it's only using the previously 
    // defined arrays.
    class PacketRegistry {
    public:
        MSG_HANDLER get_handler(unsigned char msgid);
        MSG_HANDLER get_handler_v2(unsigned char msgid);

        PktHandlerFunc get_func(unsigned char msgid);
        PktHandlerFunc get_func_v2(unsigned char msgid);

        int msglen(unsigned char msgid);
        int msglen_v2(unsigned char msgid);
                
        void set_handler(unsigned char msgid, int len, PktHandlerFunc func);
        void set_handler_v2(unsigned char msgid, int len, PktHandlerFunc func);
        
        bool isDefined(unsigned char msgid);

        // This finds the appropriate handler for the client
        MSG_HANDLER find_handler(unsigned char msgid, const Network::Client *client);
    };
    extern PacketRegistry pktRegistry;


    // Implementation for PacketRegistry inlined functions

    inline MSG_HANDLER PacketRegistry::get_handler(unsigned char msgid) {
        return handler[msgid];
    }

    inline MSG_HANDLER PacketRegistry::get_handler_v2(unsigned char msgid) {
        return handler_v2[msgid];
    }

    inline PktHandlerFunc PacketRegistry::get_func(unsigned char msgid) {
        return handler[msgid].func;
    }
    inline PktHandlerFunc PacketRegistry::get_func_v2(unsigned char msgid) {
        return handler_v2[msgid].func;
    }

    inline int PacketRegistry::msglen(unsigned char msgid) {
        return handler[msgid].msglen;
    }
    inline int PacketRegistry::msglen_v2(unsigned char msgid) {
        return handler_v2[msgid].msglen;
    }

    inline void PacketRegistry::set_handler(unsigned char msgid, int len, PktHandlerFunc func) {
        passert(len != 0);
        handler[msgid].func = func;
        handler[msgid].msglen = len;
    }
    inline void PacketRegistry::set_handler_v2(unsigned char msgid, int len, PktHandlerFunc func) {
        passert(len != 0);
        handler_v2[msgid].func = func;
        handler_v2[msgid].msglen = len;
    }

    inline bool PacketRegistry::isDefined(unsigned char msgid) {
        return handler[msgid].msglen || handler_v2[msgid].msglen;
    }
  }
}
#endif
