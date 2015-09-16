/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   nodemgr.h
 * @author Vicent Ferrer
 * @date   March, 2013
 * @brief  Node information manager
 *
 * API to store and acquire node (eNB, SGW, PGW, other MME) information (connection status, IP addr, name) and current MME configuration parameters
 *
 * Config parser library
 * Website: http://www.hyperrealm.com/libconfig/libconfig.html
 * Manual:  http://www.hyperrealm.com/libconfig/libconfig_manual.html
 */

/* TODO @Vicent: Decide an storage implementation, DB or config files
 * Currently the nodes are hardcoded for testing*/
#include "nodemgr.h"
#include "string.h"
#include "logmgr.h"

#include <libconfig.h>

#define CFGFILENAME "mme.cfg"

/*Config structure*/
config_t cfg;

int init_nodemgr(){
    config_init(&cfg);
    if(!config_read_file(&cfg, CFGFILENAME)){
      if (config_error_file(&cfg)== NULL){
          log_msg(LOG_ERR ,0, "Node Config file not found \"%s\"", CFGFILENAME);
      }
      else{
          log_msg(LOG_ERR ,0, "%s:%d - %s\n", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
          config_destroy(&cfg);
          return 0;
      }
    }
    else{
        log_msg(LOG_INFO, 0, "Node file opened: %s", CFGFILENAME);
    }
    return 1;
}

void free_nodemgr(){
    config_destroy(&cfg);
}

void getNode(struct nodeinfo_t *node, const enum nodeType type, Subscription subs){
    static uint8_t imme = 0, ieNB = 0, iSgw = 0, iPgw = 0, iCtrl = 0; /*Index variables*/
    uint8_t count;
    config_setting_t *nodes, *nodecfg;
    const char *name, *ip4, *ip6, *status;

    switch(type){
    case MME:
        node->type = invalid;
        nodes = config_lookup(&cfg, "nodes.mme");
        nodecfg = config_setting_get_elem(nodes, imme);
        count = config_setting_length(nodes);
        imme = (imme+1)%count;
        break;
    case eNB:
        node->type = invalid;
        strcpy(node->name, "Not Implemented");
        break;
    case SGW:
        node->type = SGW;
        nodes = config_lookup(&cfg, "nodes.sgw");
        nodecfg = config_setting_get_elem(nodes, iSgw);
        count = config_setting_length(nodes);
        iSgw = (iSgw+1)%count;
        break;
    case PGW:
        node->type = PGW;
        nodes = config_lookup(&cfg, "nodes.pgw");
        nodecfg = config_setting_get_elem(nodes, iPgw);
        count = config_setting_length(nodes);
        iPgw = (iPgw+1)%count;
        break;
	case CTRL:
		node->type = CTRL;
		nodes = config_lookup(&cfg, "nodes.sdn");
		nodecfg = config_setting_get_elem(nodes, iCtrl);
		count = config_setting_length(nodes);
		iCtrl = (iCtrl+1)%count;
		break;
    }
    /*Parser*/
    if(!(config_setting_lookup_string(nodecfg, "name", &name) &&
            config_setting_lookup_string(nodecfg, "ipv4", &ip4) &&
            config_setting_lookup_string(nodecfg, "ipv6", &ip6) &&
            config_setting_lookup_string(nodecfg, "status", &status) )){
        log_msg(LOG_ERR ,0, "Couldn't parse node info - %s:%d - %s\n", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
    }
    strcpy(node->name, name);

    inet_pton(AF_INET, ip4, &(node->addrv4));
    inet_pton(AF_INET6, ip6, &(node->addrv6));

    /*Conversion to status enum*/
    if(strcmp(status, "down")==0){
        node->status=down;
    }else if(strcmp(status, "up")==0){
        node->status=up;
    }else if(strcmp(status, "busy")==0){
        node->status=busy;
    }else{
        log_msg(LOG_ERR ,0, "Node status on cfg file not valid");
        node->status = invalid;
    }

}

void saveNode(const struct nodeinfo_t *node){
    /*TODO @ Vicent implement storage*/

}

void getNodeByName(const char *name, const enum nodeType type, struct nodeinfo_t *node){
    /*TODO @ Vicent implement search*/
    strcpy(node->name, "local Testing");
    node->status = up;
    inet_pton(AF_INET, "10.11.0.142", &(node->addrv4));
    inet_pton(AF_INET6,"fe80::e086:9dff:fe4c:101a", &(node->addrv6));

}

void getNodeByAddr4(const struct in_addr *addr, const enum nodeType type, struct nodeinfo_t *node){
    /*TODO @ Vicent implement search*/
    strcpy(node->name, "local Testing");
    node->status = up;
    inet_pton(AF_INET, "10.11.0.142", &(node->addrv4));
    inet_pton(AF_INET6,"fe80::e086:9dff:fe4c:101a", &(node->addrv6));

}

void getNodeByAddr6(const struct in6_addr *addr, const enum nodeType type, struct nodeinfo_t *node){
    /*TODO @ Vicent implement search*/
    strcpy(node->name, "local Testing");
    node->status = up;
    inet_pton(AF_INET, "10.11.0.142", &(node->addrv4));
    inet_pton(AF_INET6,"fe80::e086:9dff:fe4c:101a", &(node->addrv6));

}

void loadMMEinfo(struct mme_t *mme){
	config_setting_t *mmeNAMEconf, *mmeIp4, *gUMMEIsconf, *gummeiconf, *pLMNsconf, *gIDsconf, *mMECsconf, *pLMNconf, *relCapconf, *uE_DNS;
	const char *name, *mmeIpv4str, *uE_DNSstr;
    uint32_t iGUMMEI, lGUMMEI, iPLMN, lPLMN, iGID, lGID, iMMEC, lMMEC;
    int tmp;
    ServedGUMMEIsItem_t *item;
    PLMNidentity_t *pLMN;
    MME_Group_ID_t *gID;
    MME_Code_t *mmec;

    mmeNAMEconf = config_lookup(&cfg, "mme.name");
    name = config_setting_get_string(mmeNAMEconf);
    if(name != NULL && name[0]!= '\0'){
        mme->name = new_MMEname();
        strcpy(mme->name->name, name);
    }

    mmeIp4 = config_lookup(&cfg, "mme.ipv4");
    mmeIpv4str = config_setting_get_string(mmeIp4);
    if(mmeIpv4str != NULL && mmeIpv4str[0]!= '\0'){
	    memcpy(mme->ipv4, mmeIpv4str, INET_ADDRSTRLEN);
    }

    uE_DNS = config_lookup(&cfg, "mme.UE_DNS");
    if( uE_DNS != NULL) {
	        uE_DNSstr = config_setting_get_string(uE_DNS);
	        if(uE_DNSstr != NULL && uE_DNSstr[0]!= '\0'){
		        inet_pton(AF_INET, uE_DNSstr, &(mme->uE_DNS));
	        }
    }

    mme->servedGUMMEIs = new_ServedGUMMEIs();
    gUMMEIsconf = config_lookup(&cfg, "mme.servedGUMMEIs");
    lGUMMEI = config_setting_length(gUMMEIsconf);
    for (iGUMMEI=0; iGUMMEI<lGUMMEI ; iGUMMEI++){
        item = new_ServedGUMMEIsItem();
        gummeiconf = config_setting_get_elem(gUMMEIsconf, iGUMMEI);

        pLMNsconf = config_setting_get_member(gummeiconf, "Served_PLMNs");
        if(!pLMNsconf)
            log_msg(LOG_ERR ,0, "Couldn't parse Served_PLMNs key");

        gIDsconf = config_setting_get_member(gummeiconf, "Served_MME_GroupIDs");
        if(!gIDsconf)
            log_msg(LOG_ERR ,0, "Couldn't find Served_MME_GroupIDs key");

        mMECsconf = config_setting_get_member(gummeiconf, "Served_MME_Codes");
        if(!mMECsconf)
            log_msg(LOG_ERR ,0, "Couldn't find Served_MME_Codes key");

        lPLMN = config_setting_length(pLMNsconf);
        item->servedPLMNs = new_ServedPLMNs();
        for (iPLMN=0; iPLMN<lPLMN ; iPLMN++){
            pLMN = new_PLMNidentity();

            pLMNconf = config_setting_get_elem(pLMNsconf, iPLMN);
            if(!pLMNconf)
                continue;
            /*Parse MCC*/
            if( config_setting_lookup_int(pLMNconf, "MCC", &tmp) == CONFIG_FALSE){
                log_msg(LOG_ERR ,0, "Couldn't parse MCC name - %s:%d - %s\n", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
            }
            pLMN->MCC = tmp;

            /*Parse MNC*/
            if( config_setting_lookup_int(pLMNconf, "MNC", &tmp) == CONFIG_FALSE){
                log_msg(LOG_ERR ,0, "Couldn't parse MNC name - %s:%d - %s\n", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
            }
            pLMN->MNC = tmp;
            plmnId_MccMnc2tbcd(pLMN);

            item->servedPLMNs->additem(item->servedPLMNs, pLMN);
        }

        lGID = config_setting_length(gIDsconf);
        item->servedGroupIDs = new_ServedGroupIDs();
        for (iGID=0; iGID<lGID ; iGID++){
            gID = new_MME_Group_ID();
            tmp = config_setting_get_int_elem(gIDsconf, iGID);
            tmp = htons(tmp);
            memcpy(gID->s, &tmp, 2);
            item->servedGroupIDs->additem(item->servedGroupIDs, gID);
        }

        lMMEC = config_setting_length(mMECsconf);
        item->servedMMECs = new_ServedMMECs();
        for (iMMEC=0; iMMEC<lMMEC ; iMMEC++){
            mmec = new_MME_Code();
            mmec->s[0] = (uint8_t)config_setting_get_int_elem(mMECsconf, iMMEC);
            item->servedMMECs->additem(item->servedMMECs, mmec);
        }

        mme->servedGUMMEIs->additem(mme->servedGUMMEIs, item);
    }
    /*mme->servedGUMMEIs->showIE(mme->servedGUMMEIs);*/
    /*printf ("mme %#x\n", mme);*/

    /*RelativeMMECapacity*/
    mme->relativeCapacity = new_RelativeMMECapacity();
    relCapconf = config_lookup(&cfg, "mme.relative_Capacity");
    tmp = config_setting_get_int(relCapconf);
    mme->relativeCapacity->cap = tmp;

    log_msg(LOG_INFO ,0, "MME configuration loaded from file");

}

void freeMMEinfo(struct mme_t *mme){
    /* Dealocate information stored*/
    if(mme->servedGUMMEIs!=NULL)
        mme->servedGUMMEIs->freeIE(mme->servedGUMMEIs);
    if(mme->relativeCapacity!=NULL)
        mme->relativeCapacity->freeIE(mme->relativeCapacity);
    if(mme->name!=NULL)
        mme->name->freeIE(mme->name);
}
