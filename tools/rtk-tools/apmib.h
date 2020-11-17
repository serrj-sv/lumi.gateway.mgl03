#ifndef __APMIB_H
#define __APMIB_H

#ifndef WIN32
#define __PACK__        __attribute__ ((packed))
#else
#define __PACK__
#endif

#define BOOT_HEADER         ((char *)"boot")

#if defined(CONFIG_RTL_8196B)
#define ROOT_HEADER         ((char *)"r6br")
#elif defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A)
#define ROOT_HEADER         ((char *)"r6cr")
#else
#define ROOT_HEADER         ((char *)"root")
#endif //#if defined(CONFIG_RTL_8196B)

#if defined(CONFIG_RTL_8196B)
#if (defined(HOME_GATEWAY) && defined(VPN_SUPPORT))
#define WEB_HEADER          ((char *)"w6bv")
#elif (defined(HOME_GATEWAY))
#define WEB_HEADER          ((char *)"w6bg")
#else
#define WEB_HEADER          ((char *)"w6ba")
#endif
#elif defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A)
#if (defined(HOME_GATEWAY) && defined(VPN_SUPPORT))
#define WEB_HEADER          ((char *)"w6cv")
#elif (defined(HOME_GATEWAY))
#define WEB_HEADER          ((char *)"w6cg")
#else
#define WEB_HEADER          ((char *)"w6ca")
#endif
#else
#if (defined(HOME_GATEWAY) && defined(VPN_SUPPORT))
#define WEB_HEADER          ((char *)"webv")
#elif (defined(HOME_GATEWAY))
#define WEB_HEADER          ((char *)"webg")
#else
#define WEB_HEADER          ((char *)"weba")
#endif
#endif //#if defined(CONFIG_RTL_8196B)

#if defined(CONFIG_RTL_8196B)
#define FW_HEADER_WITH_ROOT ((char *)"cr6b")
#define FW_HEADER           ((char *)"cs6b")
#elif defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8881A)
#define FW_HEADER_WITH_ROOT ((char *)"cr6c")
#define FW_HEADER           ((char *)"cs6c")
#else
#define FW_HEADER_WITH_ROOT ((char *)"csro")
#define FW_HEADER           ((char *)"csys")
#endif //#if defined(CONFIG_RTL_8196B)

#define ALL_HEADER          ((char *)"allp")

#define HW_SETTING_OFFSET			0x6000
#define DEFAULT_SETTING_OFFSET		0x8000
#define CURRENT_SETTING_OFFSET		0xc000

#define DWORD_SWAP(v) ( (((v&0xff)<<24)&0xff000000) | ((((v>>8)&0xff)<<16)&0xff0000) | \
				                ((((v>>16)&0xff)<<8)&0xff00) | (((v>>24)&0xff)&0xff) )

#define WORD_SWAP(v) ((unsigned short)(((v>>8)&0xff) | ((v<<8)&0xff00)))

#define SIGNATURE_LEN           4

typedef struct img_header {
    unsigned char signature[SIGNATURE_LEN];
    unsigned int startAddr;
    unsigned int burnAddr;
    unsigned int len;
}__PACK__ IMG_HEADER_T, *IMG_HEADER_Tp;

#endif
