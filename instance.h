/*
 * =====================================================================================
 *
 *       Filename:  instance.h
 *
 *    Description:  This is a header file to declare structures to define the Network topology
 *
 *        Version:  1.0
 *        Created:  Wednesday 24 August 2017 01:51:55  IST
 *       Revision:  1.0
 *       Compiler:  gcc
 *
 *         Author:  Er. Abhishek Sagar, Networking Developer (AS), sachinites@gmail.com
 *        Company:  Brocade Communications(Jul 2012- Mar 2016), Current : Juniper Networks(Apr 2017 - Present)
 *        
 *        This file is part of the SPFComputation distribution (https://github.com/sachinites).
 *        Copyright (c) 2017 Abhishek Sagar.
 *        This program is free software: you can redistribute it and/or modify
 *        it under the terms of the GNU General Public License as published by  
 *        the Free Software Foundation, version 3.
 *
 *        This program is distributed in the hope that it will be useful, but 
 *        WITHOUT ANY WARRANTY; without even the implied warranty of 
 *        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 *        General Public License for more details.
 *
 *        You should have received a copy of the GNU General Public License 
 *        along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * =====================================================================================
 */

#ifndef __INSTANCE__
#define __INSTANCE__

#include <stdlib.h>
#include <assert.h>
#include "instanceconst.h"
#include "LinkedListApi.h"
#include "spfcomputation.h"
#include "prefix.h"
#include "rlfa.h"
#include "Libtrace/libtrace.h"
#include "glthread.h"
#include "ldp.h"
#include "rsvp.h"
#include "Tree/candidate_tree.h"
#include "spring_adjsid.h"


typedef struct edge_end_ edge_end_t;

typedef struct _node_t{
    char node_name[NODE_NAME_SIZE];
    char router_id[PREFIX_LEN+1];
    AREA area;
    edge_end_t *edges[MAX_NODE_INTF_SLOTS];
    NODE_TYPE node_type[MAX_LEVEL];
    rbnode candiate_tree_node;  /*Node to be plugged into candidate tree*/ 
    unsigned int spf_metric[MAX_LEVEL];
    unsigned int lsp_metric[MAX_LEVEL];

    internal_nh_t next_hop[MAX_LEVEL][NH_MAX][MAX_NXT_HOPS];
    //internal_nh_t old_next_hop[MAX_LEVEL][NH_MAX][MAX_NXT_HOPS];
    internal_nh_t backup_next_hop[MAX_LEVEL][NH_MAX][MAX_NXT_HOPS];
    //internal_nh_t old_backup_next_hop[MAX_LEVEL][NH_MAX][MAX_NXT_HOPS];
    internal_nh_t direct_next_hop[MAX_LEVEL][NH_MAX][MAX_NXT_HOPS];

    /*Complete path spf run*/
    glthread_t pred_lst[MAX_LEVEL][NH_MAX];

    /*list of spf_path_result_t*/
    glthread_t spf_path_result[MAX_LEVEL][NH_MAX];

    /*Fields to handle pseudonode case*/
    edge_end_t *pn_intf[MAX_LEVEL];

    ll_t *local_prefix_list[MAX_LEVEL];
    ll_t *self_spf_result[MAX_LEVEL];                       /*Used for LFA and RLFA computation*/ 
    /*For SPF computation only*/ 
    ll_t *spf_run_result[MAX_LEVEL];                        /*List of nodes of instance which contain result of SPF skeleton run*/
    char attached;                                          /*Set if the router is L1L2 router. Admin responsibility to configure it as per the topology*/

    /*Every node in production has its own spf_info and 
     * instance flags*/
    spf_info_t spf_info;
    unsigned int instance_flags;                            /*Hope instance flags are not level specific, is there any ? If we come across later, we will have level specific flags*/
    boolean is_node_on_heap;

    char attributes[MAX_LEVEL];                             /*1 Bytes of router attributes*/
    char traversing_bit;                                    /*This bit is only used to traverse the instance, otherwise it is not specification requirement. 1 if the node has been visited, zero otherwise*/
    char lsp_distribution_bit;
    internal_nh_t pq_nodes[MAX_LEVEL][MAX_NXT_HOPS];
    unsigned int backup_spf_options;

    /*segment routing related members*/
    boolean spring_enabled;
    srgb_t *srgb;
    glthread_t prefix_sids_thread_lst[MAX_LEVEL];
    boolean use_spring_backups;
    
    /*Our implementation specific*/
    char flags[MAX_LEVEL];

    /*LDP related config*/
    ldp_config_t ldp_config;

    /*RSVP related config*/
    rsvp_config_t rsvp_config;
    
    unsigned int am_i_mapping_client:1,/*TRUE is node is a mapping client*/
                 am_i_mapping_server:1;

    /*Mapping Server local policy database*/
    glthread_t srms_lcl_policy_db; /*Advertised in al levels - pg349*/

    /*Mapping client prefix-sid Mapping db*/
    ll_t *active_mapping_policy[MAX_LEVEL];
    ll_t *backup_mapping_policy[MAX_LEVEL];

} node_t;


typedef enum{
    
    PTP,
    LAN,
    INTF_TYPE_MAX
} INTF_OP_MODE;

struct edge_end_{
    node_t *node;
    char intf_name[IF_NAME_SIZE];
    unsigned int ifindex;
    common_pfx_key_t if_prefix;/*ToDo : use this sata structure instead if below prefix_t */
    prefix_t * prefix[MAX_LEVEL];
    char mac_address[MAC_LEN];
    EDGE_END_DIRN dirn; /*dirn of edge is not level dependant*/
    FLAG edge_config_flags;
    /*SR support, per level Adj SIDs*/
    /*User configured Adjacency SIDs*/
    p2p_intf_adj_sid_t cfg_p2p_adj_sid_db[MAX_LEVEL][ADJ_SID_PROTECTION_MAX];
    glthread_t cfg_lan_adj_sid_db[MAX_LEVEL][ADJ_SID_PROTECTION_MAX];
    /*Dynamic Adjacency SIDs*/
    //p2p_intf_adj_sid_t dyn_p2p_adj_sid_db[MAX_LEVEL][ADJ_SID_PROTECTION_MAX];
    //glthread_t dyn_adj_sid_db[MAX_LEVEL][ADJ_SID_PROTECTION_MAX];
};

typedef enum {

    UNICAST,
    LSP
} edge_type_t;

typedef struct _edge_t{
    edge_end_t from;
    unsigned int metric[MAX_LEVEL];
    LEVEL level;
    edge_end_t to;
    struct _edge_t *inv_edge;
    edge_type_t etype;
    rsvp_tunnel_t *fa;      /*Forwarding adjacency*/
    char status;            /* 0 down, 1 up*/
    float bandwidth; /*bandwidth for WECMP in GIG*/
} edge_t;

typedef struct instance_{
    node_t *instance_root;
    ll_t *instance_node_list;
    candidate_tree_t ctree;/*Candidate tree is shared by all nodes for SPF run*/
    traceoptions *traceopts;
    /*SR mapping server. We support only one mapping
     * server per topology*/
    node_t *mapping_server;
} instance_t;

node_t *
create_new_node(instance_t *instance, char *node_name, AREA area, char *router_id);


edge_t *
create_new_edge(char *from_ifname, 
                char *to_ifname, 
                unsigned int metric, 
                prefix_t *from_prefix,
                prefix_t *to_prefix,
                LEVEL level);

void
insert_edge_between_2_nodes(edge_t *edge, 
                            node_t *from_node, 
                            node_t *to_node, DIRECTION dirn);

void
set_instance_root(instance_t *instance, node_t *root);

instance_t *
get_new_instance();

void
dump_nbrs(node_t *node, LEVEL level);

void
dump_node_info(node_t *node);

void 
dump_edge_info(edge_t *edge);

void
mark_node_pseudonode(node_t *node, LEVEL level);

boolean
is_two_way_nbrship(node_t *node, node_t *node_nbr, LEVEL level);

edge_t *
get_my_pseudonode_nbr(node_t *node, LEVEL level);

/*Fn to attach/dettach non interface specific prefix on the router*/
prefix_t *
attach_prefix_on_node(node_t *node, 
                      char *prefix, 
                      unsigned char mask,
                      LEVEL level,
                      unsigned int metric,
                      FLAG prefix_flags);

void
deattach_prefix_on_node(node_t *node, 
                      char *prefix, 
                      unsigned char mask,
                      LEVEL level);

prefix_t *
node_local_prefix_search(node_t *node, LEVEL level, 
                        char *prefix, char mask); /*key*/

/*Return 1, if node1 and node2 are present in same LAN segment in topo.
 * Means, node1 is 1 hop(PN) away from node2 and vice versa, else return 0
 * Note : We have assumed, that node can be connected to atmost one PN per level
 * at a time in this project*/

int
is_same_lan_segment_nodes(node_t *node1, node_t *node2, LEVEL level);

void
inverse_topology(instance_t *instance, LEVEL level);


/* Macros */

#define GET_NODE_PREFIX_LIST(node_ptr, level)   (node_ptr->local_prefix_list[level])

#define GET_EGDE_PTR_FROM_FROM_EDGE_END(edge_end_ptr)   \
    (edge_t *)((char *)edge_end_ptr - (unsigned int)&(((edge_t *)0)->from))

#define GET_EGDE_PTR_FROM_TO_EDGE_END(edge_end_ptr)     \
    (edge_t *)((char *)edge_end_ptr - (unsigned int)&(((edge_t *)0)->to))

#define IS_OVERLOADED(nodeptr, level)                   \
    IS_BIT_SET(nodeptr->attributes[level], OVERLOAD_BIT)

static inline edge_t *
GET_EGDE_PTR_FROM_EDGE_END(edge_end_t *edge_end){
    
    edge_t *edge = NULL;
    if(edge_end->dirn == OUTGOING)
        edge = GET_EGDE_PTR_FROM_FROM_EDGE_END(edge_end);
    else if(edge_end->dirn == INCOMING)
        edge = GET_EGDE_PTR_FROM_TO_EDGE_END(edge_end);
    else
        assert(0);

    return edge;
}

static inline LEVEL
GET_EDGE_END_LEVEL(edge_end_t *edge_end){
    
    edge_t *edge = GET_EGDE_PTR_FROM_EDGE_END(edge_end);
    return edge->level;
}

/* The function test whether the given edge
 * is outgoing edge or incoming edge wrt to a goven node*/
static inline EDGE_END_DIRN
get_edge_direction(node_t *node, edge_t *edge){

    if(edge->from.node == node && edge->from.dirn == OUTGOING)
        return OUTGOING;
    else if(edge->to.node == node && edge->to.dirn == INCOMING)
        return INCOMING;

    return EDGE_END_DIRN_UNKNOWN;
}

#if 0
edge_end_t *
get_min_oif(node_t *node, node_t *node_nbr, 
            LEVEL level, char *input_gw_ip_buffer, nh_type_t nh);
#endif

/*This Macro iterates over all logical nbrs of a node. Logical nbrs includes
 * pseudonodes as well. Nbrs of directly connected PN do not count*/
#define ITERATE_NODE_LOGICAL_NBRS_BEGIN(_node, _nbr_node, _edge, _level)  \
    _nbr_node = NULL;                                             \
    _edge = NULL;                                                 \
    do{                                                           \
        unsigned int _i = 0;                                      \
        edge_end_t *_edge_end = 0;                                \
        assert(_level != LEVEL12);                                \
        for(;_i < MAX_NODE_INTF_SLOTS; _i++){                     \
            _edge_end = _node->edges[_i];                         \
            if(!_edge_end) break;                                 \
            if(_edge_end->dirn != OUTGOING)                       \
                continue;                                         \
            _edge = GET_EGDE_PTR_FROM_FROM_EDGE_END(_edge_end);   \
            if(!_edge->status) continue;                          \
            if(!IS_LEVEL_SET(_edge->level, _level))               \
                continue;                                         \
            _nbr_node = _edge->to.node;
             
#define ITERATE_NODE_LOGICAL_NBRS_END   }}while(0)
             
/* This macro iterates over all physical nbrs of a node. Physical nbrs includes
 * P2P nbrs and nbrs of directly connected PN. PN itself do not count as a nbr
 * of a _node
 * _egde1 - edge connecting _node and its nbr which could be PN
 * _edge2 - if nbr is a PN, then this is the edge connecting PN and its nbr
 * _level - LEVEL1 Or LEVEL2 but not Both
 *
 * _nbr_node -- real physical nbr, could be direct real nbr or nbr connected through PN
 * __nbr_node -- logical nbr (always direct nbr)*/

#define ITERATE_NODE_PHYSICAL_NBRS_BEGIN(_node, _nbr_node, __nbr_node,     \
                                   _edge1, _edge2, _level)                 \
    _nbr_node = NULL;                                                      \
    _edge1 = NULL; _edge2 = NULL;                                          \
    assert(_level != LEVEL12);                                             \
    do{                                                                    \
        __label__ NONPN;                                                   \
        unsigned int _i = 0, _j =0;                                        \
        __nbr_node = 0;                                                    \
        edge_t *__edge = 0;                                                \
        edge_end_t *_edge_end = 0;                                         \
        for(_i = 0; _i < MAX_NODE_INTF_SLOTS; _i++){                       \
            _edge_end = _node->edges[_i];                                  \
            if(!_edge_end) break;                                          \
            if(_edge_end->dirn != OUTGOING)                                \
                continue;                                                  \
            __edge = GET_EGDE_PTR_FROM_FROM_EDGE_END(_edge_end);           \
            if(!__edge->status) continue;                                  \
            if(!IS_LEVEL_SET(__edge->level, _level))                       \
                continue;                                                  \
            __nbr_node = __edge->to.node;                                  \
            _nbr_node = __nbr_node;                                        \
            _edge1 = __edge;                                               \
            _edge2 = _edge1;                                               \
            if(__nbr_node->node_type[_level] != PSEUDONODE){               \
                goto NONPN;                                                \
            }                                                              \
            _j = 0;                                                        \
            for(_j = 0; _j < MAX_NODE_INTF_SLOTS; _j++){                   \
                _edge_end = __nbr_node->edges[_j];                         \
                if(!_edge_end) break;                                      \
                if(_edge_end->dirn != OUTGOING)                            \
                continue;                                                  \
                _edge2 = GET_EGDE_PTR_FROM_FROM_EDGE_END(_edge_end);       \
                if(!_edge2->status) continue;                              \
                if(!IS_LEVEL_SET(_edge2->level, _level))                   \
                continue;                                                  \
                _nbr_node = _edge2->to.node;                               \
                if(_nbr_node == _node)                                     \
                continue;                                                  \
                NONPN:

#define ITERATE_NODE_PHYSICAL_NBRS_CONTINUE(_node, _nbr_node, __nbr_node, __level)           \
             if(__nbr_node && __nbr_node->node_type[__level] != PSEUDONODE){                 \
                 break; continue;                                                            \
             }                                                                               \
             else continue               
                                                                  
#define ITERATE_NODE_PHYSICAL_NBRS_BREAK(_node, _nbr_node, __nbr_node, __level)              \
            {__nbr_node = _node;       /* Just act as a flag*/                               \
            break;}
             
#define ITERATE_NODE_PHYSICAL_NBRS_END(_node, _nbr_node, __nbr_node, __level)                \
            if(__nbr_node && __nbr_node->node_type[__level] != PSEUDONODE)                   \
                break;                                                                       \
            }                                                                                \
            if(_node == __nbr_node) break;                                                   \
            }}while(0)

void
attach_edge_end_prefix_on_node(node_t *node, edge_end_t *edge_end);

void
dettach_edge_end_prefix_on_node(node_t *node, edge_end_t *edge_end);

edge_t *
create_new_lsp_adj(char *lsp_name,
        unsigned int metric,
        LEVEL level);

edge_end_t *
get_interface_from_intf_name(node_t *node, char *intf_name);

node_t *
get_peer_node(edge_end_t *oif, LEVEL level, char *gw_ip);

#endif /* __INSTANCE__ */
